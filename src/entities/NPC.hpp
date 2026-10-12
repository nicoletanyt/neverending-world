#pragma once
#include "raylib.h"
#include <map>
#include <string>
#include <vector>
using namespace std;

class Player;

class NPC {
  private:
    float direction = 180;

  public:
    static map<int, map<int, vector<Texture2D>>> assets;
    static map<int, Texture2D> avatars;
    static constexpr Vector2 size = {64.0f, 64.0f};

    const int npcType = 1;
    Vector2 pos = {0, 0};
    float frame = 0;
    string name;

    NPC(Vector2 pos, string name) : pos(pos), name(name) {};

    float getZLayer() const;
    Rectangle getHitBox() const;

    static void loadAssets();
    void update();
    bool isInteraction(Player &player);
    void render() const;
    void animate(float deltaTime);
};
