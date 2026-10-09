#pragma once
#include "../Constants.hpp"
#include "Animation.hpp"
#include <map>
#include <raylib.h>
#include <vector>
using namespace std;

struct InventoryItem;

enum class Weapon { Longsword_longsword, Smash_axe, Smash_pickaxe, None };

class Player {
private:
    static constexpr float speed = 200.0f; // Pixels per second.
    static constexpr float MAX_HEALTH = 100.0f;
    Weapon weapon = Weapon::Longsword_longsword;
    float frame = 0;

public:
    Vector2 pos = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
    AnimationType type = AnimationType::Walk;
    int direction = 0;
    float health = MAX_HEALTH;
    static constexpr Vector2 size = {64.0f, 64.0f};
    static map<Weapon, map<AnimationType, map<int, vector<Texture2D>>>> assets;

    float getZLayer() const;
    Rectangle getHitBox(bool atFoot) const;

    static void loadAssets();
    bool move(float deltaTime, Location *location);
    void fight();
    bool isFightFrame = false;

    bool heal(int amount);

    bool updateLocation(Location *location);
    void render();
    void animate(float deltaTime);
};
