#include "Enemy.hpp"
#include "../utils.hpp"
#include "Player.hpp"
#include "Tree.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <map>
#include <raylib.h>
#include <raymath.h>
#include <regex>
#include <string>
#include <vector>

using namespace std;
using recursive_directory_iterator =
    std::filesystem::recursive_directory_iterator;

map<AnimationType, map<int, vector<Texture2D>>> Enemy::assets;

float Enemy::getZLayer() const { return pos.y; }

Rectangle Enemy::getHitBox() const { return {pos.x + 20, pos.y + 44, 24, 12}; }

void Enemy::loadAssets() {
    const regex pattern(
        R"(^enemy_\d+_(Dagger_dagger)_(walk|hurt|fight)_(-?\d+)_(\d+)$)");
    map<AnimationType, map<int, map<int, filesystem::path>>> orderedPaths;

    for (const auto &dirEntry : recursive_directory_iterator("assets/enemy")) {
        const string name = dirEntry.path().stem().string();
        smatch match;
        if (!regex_match(name, match, pattern))
            continue;

        auto animation = stringToAnimationType(match[2].str());
        if (!animation)
            continue;

        const int direction = stoi(match[3].str());
        const int frameNumber = stoi(match[4].str());

        orderedPaths[*animation][direction][frameNumber] = dirEntry.path();
    }

    for (const auto &[animation, directions] : orderedPaths) {
        for (const auto &[direction, paths] : directions) {
            auto &frames = assets[animation][direction];
            frames.reserve(paths.size());
            for (const auto &[frameNumber, path] : paths) {
                frames.push_back(LoadTexture(path.string().c_str()));
            }
        }
    }
}

void Enemy::update(Player &player, float deltaTime, const vector<Tree> &trees) {
    if (health > 0)
        alive(player, deltaTime, trees);
    else if (health == 0)
        dying(deltaTime);
}

void Enemy::alive(Player &player, float deltaTime, const vector<Tree> &trees) {
    float distance = calculateDistance(player.pos, pos);
    if (distance < 32) {
        action = EnemyAction::Fight;
        facePlayer(player);
        fight(player, deltaTime);
        if (getHit(player)) {
            health -= 10;
        }
    } else if (distance < 180) {
        // chase
        action = EnemyAction::Chase;
        move(deltaTime, player, trees);
    } else {
        action = EnemyAction::Walk;
        move(deltaTime, player, trees);
    }
}

void Enemy::dying(float deltaTime) {
    type = AnimationType::Hurt;
    facing = Direction::Up;
    animate(deltaTime);
    if (frame > 5) {
        health = -50.0;
        frame = 5;
    }
}

Vector2 Enemy::wander(float deltaTime) {
    directionTimer += deltaTime;

    if (directionTimer >= 3.0f) {
        int angle = static_cast<int>(facing);
        int turn = GetRandomValue(0, 4);

        if (turn == 0)
            angle -= 90;
        else if (turn == 1)
            angle += 90;

        if (angle > 180)
            angle -= 360;
        if (angle < -90)
            angle += 360;

        facing = static_cast<Direction>(angle);
        directionTimer = 0;
    }
    return ANGLE_TO_DIRECTIONS.at(facing);
}

Vector2 Enemy::facePlayer(Player &player) {
    Vector2 dir = Vector2Normalize(Vector2Subtract(player.pos, pos));
    float angle = atan2(dir.x, -dir.y) * 180.0f / PI;
    int rounded = round(angle / 90) * 90;
    if (rounded == -180)
        rounded = 180;
    if (rounded == 270)
        rounded = -90;
    facing = static_cast<Direction>(rounded);
    return dir;
}

void Enemy::handleBoundaries() {
    if (pos.x <= size.x) {
        facing = Direction::Right;
    } else if (pos.x >= WINDOW_WIDTH - size.x) {
        facing = Direction::Left;
    } else if (pos.y <= size.y) {
        facing = Direction::Down;
    } else if (pos.y >= WINDOW_HEIGHT - size.y) {
        facing = Direction::Up;
    }
    pos.x = clamp(pos.x, -size.x, WINDOW_WIDTH);
    pos.y = clamp(pos.y, -size.y, WINDOW_HEIGHT);
}

void Enemy::move(float deltaTime, Player &player, const vector<Tree> &trees) {
    type = AnimationType::Walk;
    float speed = 0.0f;
    Vector2 dir;

    if (action == EnemyAction::Walk) {
        speed = 50.0f;
        dir = wander(deltaTime);
    } else {
        speed = 100.0f;
        dir = facePlayer(player);
    }

    pos.x += dir.x * speed * deltaTime;
    pos.y += dir.y * speed * deltaTime;

    if (checkCollision(player, trees)) {
        pos.x -= dir.x * speed * deltaTime;
        pos.y -= dir.y * speed * deltaTime;
        return;
    }
    handleBoundaries();
    animate(deltaTime);
}

bool Enemy::checkCollision(Player &player, const vector<Tree> &trees) {
    if (CheckCollisionRecs(getHitBox(), player.getHitBox(true)))
        return true;

    for (const Tree &tree : trees) {
        if (CheckCollisionRecs(getHitBox(), tree.getHitBox()))
            return true;
    }
    return false;
}

void Enemy::render() const {
    Texture2D texture = assets[type][static_cast<int>(facing)][frame];
    DrawTextureV(texture, pos, WHITE);
}

void Enemy::animate(float deltaTime) {
    const auto &frames = assets[type][static_cast<int>(facing)];
    if (frames.empty())
        return;

    frame += 15.0f * deltaTime;

    float maxFrames = static_cast<float>(frames.size());

    if (frame >= maxFrames) {
        if (type == AnimationType::Fight) {
            type = AnimationType::Walk;
            frame = 0.0f;
        } else {
            frame = fmod(frame, maxFrames);
        }
    }
}

void Enemy::fight(Player &player, float deltaTime) {
    type = AnimationType::Fight;
    float previousFrame = frame;
    animate(deltaTime);
    if (previousFrame < 4 && frame >= 4)
        player.health = max(player.health - 10.0f, 0.0f);
}

bool Enemy::getHit(Player &player) {
    bool isRight = (pos.x > player.pos.x && player.direction == 90);
    bool isLeft = (pos.x < player.pos.x && player.direction == -90);
    bool isBottom = (pos.y > player.pos.y && player.direction == 180);
    bool isUp = (pos.y < player.pos.y && player.direction == 0);

    return ((isRight || isLeft || isBottom || isUp) && player.isFightFrame);
}
