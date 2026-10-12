#include "Player.hpp"
#include "../Game.hpp"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <raylib.h>
#include <regex>
#include <string>
#include <vector>

using namespace std;
using recursive_directory_iterator =
    std::filesystem::recursive_directory_iterator;

map<Weapon, map<AnimationType, map<int, vector<Texture2D>>>> Player::assets;

optional<Weapon> stringToWeapon(const string &value) {
    if (value == "Longsword_longsword")
        return Weapon::Longsword_longsword;
    if (value == "Smash_axe")
        return Weapon::Smash_axe;
    if (value == "Smash_pickaxe")
        return Weapon::Smash_pickaxe;
    return Weapon::None;
}

string weaponToString(Weapon weapon) {
    switch (weapon) {
    case Weapon::Longsword_longsword: {
        return "Longsword_longsword";
    }
    case Weapon::Smash_axe: {
        return "Smash_axe";
    }
    case Weapon::Smash_pickaxe: {
        return "Smash_pickaxe";
    }
    case Weapon::None: {
        return "";
    }
    }
    return "";
}

bool Player::move(float deltaTime, Location *location) {
    if (type == AnimationType::Fight)
        return false;

    const map<int, Vector2> directions = {
        {KEY_W, {0, -1}},
        {KEY_S, {0, 1}},
        {KEY_A, {-1, 0}},
        {KEY_D, {1, 0}},
    };

    for (const auto &[key, dir] : directions) {
        if (IsKeyDown(key)) {
            type = AnimationType::Walk;
            float angle = atan2(dir.x, -dir.y) * 180.0f / PI;
            direction = round(angle / 90.0f) * 90.0f;
            pos.x += dir.x * speed * deltaTime;
            pos.y += dir.y * speed * deltaTime;

            animate(deltaTime);
            return updateLocation(location);
        }
    }
    return false;
};

float Player::getZLayer() const { return pos.y; }

Rectangle Player::getHitBox(bool atFoot) const {
    if (atFoot)
        return {pos.x + 16, pos.y + 48, 32, 16};
    return {pos.x + 16, pos.y + 7, 32, 52};
}

void Player::fight() {
    if (type == AnimationType::Fight)
        return;
    // reset
    type = AnimationType::Fight;
    frame = 0.0f;
}

bool Player::updateLocation(Location *location) {
    if (pos.x <= -size.x) {
        pos.x = WINDOW_WIDTH;
        location->x--;
        return true;
    } else if (pos.x >= WINDOW_WIDTH) {
        pos.x = -size.x;
        location->x++;
        return true;
    } else if (pos.y <= -size.y) {
        pos.y = WINDOW_HEIGHT;
        location->y--;
        return true;
    } else if (pos.y >= WINDOW_HEIGHT) {
        pos.y = -size.y;
        location->y++;
        return true;
    }
    return false;
}

void Player::loadAssets() {
    const regex pattern(
        R"(^hero_\d+_(?:(Longsword_longsword|Smash_axe|Smash_pickaxe|none)_)?(walk|hurt|fight)_(-?\d+)_(\d+)$)");

    map<Weapon, map<AnimationType, map<int, map<int, filesystem::path>>>>
        orderedPaths;

    for (const auto &dirEntry : recursive_directory_iterator("assets/hero")) {
        const string name = dirEntry.path().stem().string();
        smatch match;

        if (!regex_match(name, match, pattern))
            continue;

        Weapon assetWeapon = Weapon::None;
        if (match[1].matched) {
            auto parsedWeapon = stringToWeapon(match[1].str());
            if (!parsedWeapon)
                continue;
            assetWeapon = *parsedWeapon;
        }
        auto animation = stringToAnimationType(match[2].str());
        if (!animation)
            continue;

        const int direction = stoi(match[3].str());
        const int frameNumber = stoi(match[4].str());

        orderedPaths[assetWeapon][*animation][direction][frameNumber] =
            dirEntry.path();
    }
    for (const auto &[assetWeapon, animations] : orderedPaths) {
        for (const auto &[animation, directions] : animations) {
            for (const auto &[direction, paths] : directions) {
                auto &frames = assets[assetWeapon][animation][direction];
                frames.reserve(paths.size());
                for (const auto &[frameNumber, path] : paths) {
                    frames.push_back(LoadTexture(path.string().c_str()));
                }
            }
        }
    }
}

void Player::animate(float deltaTime) {
    const auto &frames = assets[weapon][type][direction];
    if (frames.empty())
        return;
    float previousFrame = frame;

    frame += 15.0f * deltaTime;
    float maxFrames = static_cast<float>(frames.size());

    if (type == AnimationType::Fight) {
        if (previousFrame < 4 && frame >= 4)
            isFightFrame = true;
        else
            isFightFrame = false;
    }

    if (frame >= maxFrames) {
        if (type == AnimationType::Fight) {
            type = AnimationType::Walk;
            frame = 0.0f;
        } else {
            frame = fmod(frame, maxFrames);
        }
    }
}
void Player::render() {
    Texture2D texture =
        assets[weapon][type][direction][static_cast<size_t>(frame)];

    Vector2 drawPos;
    drawPos = pos;
    if (type == AnimationType::Fight) {
        // Align the padded fight sprite with the player's 64x64 world area.
        drawPos.x -= (texture.width - size.x) / 2.0f;
        drawPos.y -= (texture.height - size.y) / 2.0f;
    }
    DrawTextureV(texture, drawPos, WHITE);
}

bool Player::heal(int amount) {
    if (health >= MAX_HEALTH)
        return false;
    health = min(health + amount, MAX_HEALTH);
    return true;
}
