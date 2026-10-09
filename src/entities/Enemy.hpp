#pragma once
#include "../Constants.hpp"
#include "Animation.hpp"
#include <map>
#include <raylib.h>
#include <vector>
using namespace std;

class Player;
class Tree;

enum EnemyAction { Walk, Chase, Fight };

class Enemy {
private:
    static constexpr Vector2 size = {64.0f, 64.0f};
    AnimationType type = AnimationType::Walk;
    EnemyAction action = EnemyAction::Walk;
    float frame = 0;

    Direction facing = Direction::Down;
    float directionTimer = 0;

    void move(float deltaTime, Player &player, const vector<Tree> &trees);
    Vector2 wander(float deltaTime);
    Vector2 facePlayer(Player &player);
    void handleBoundaries();
    void animate(float deltaTime);
    void fight(Player &player, float deltaTime);
    bool getHit(Player &player);
    bool checkCollision(Player &player, const vector<Tree> &trees);

    void alive(Player &player, float deltaTime, const vector<Tree> &trees);
    void dying(float deltaTime);
    void dead();

public:
    Vector2 pos;
    static constexpr float maxHealth = 50.0f;
    float health = maxHealth;
    static map<AnimationType, map<int, vector<Texture2D>>> assets;
    static void loadAssets();
    Enemy() {
        pos = {
            static_cast<float>(GetRandomValue(size.x, WINDOW_WIDTH - size.x)),
            static_cast<float>(GetRandomValue(size.y, WINDOW_HEIGHT - size.y))};
    }
    void update(Player &player, float deltaTime, const vector<Tree> &trees);
    float getZLayer() const;
    Rectangle getHitBox() const;

    void render() const;
};
