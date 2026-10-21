#pragma once
#include "../Constants.hpp"
#include <raylib.h>
#include <vector>
using namespace std;

class Tree {
  private:
    static constexpr float scale = 2.0f;
    int type;

  public:
    Vector2 pos;
    static vector<Texture2D> assets;
    static void loadAssets();
    Tree() {
        type = GetRandomValue(1, static_cast<int>(assets.size()));
        const Texture2D &texture = assets[type - 1];
        const Vector2 size = {texture.width * scale, texture.height * scale};
        pos = {static_cast<float>(
                   GetRandomValue(0, static_cast<int>(WINDOW_WIDTH - size.x))),
               static_cast<float>(GetRandomValue(
                   0, static_cast<int>(WINDOW_HEIGHT - size.y)))};
    }
    float getZLayer() const;
    Rectangle getHitBox() const;
    void render() const;
};
