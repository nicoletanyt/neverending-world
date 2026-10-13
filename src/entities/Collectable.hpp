#pragma once
#include <iostream>
#include <map>
#include <raylib.h>
#include <string>
#include <vector>
using namespace std;

enum class CollectableType { Coin, Potion };

string collectableTypeToString(CollectableType type);

class Collectable {
  private:
    float scale = 1;
    Vector2 size;

  protected:
    Vector2 drawPos;
    static map<CollectableType, vector<Texture2D>> assets;

  public:
    CollectableType type;

    Collectable(Vector2 pos, CollectableType type) : type(type) {
        const auto &texture = assets.at(type).front();
        size = {static_cast<float>(texture.width),
                static_cast<float>(texture.height)};

        switch (type) {
        case CollectableType::Coin: {
            scale = 0.5;
            break;
        }
        case CollectableType::Potion: {
            scale = 1.7;
            break;
        }
        }

        drawPos.x = pos.x + 32.0f - (size.x * scale) / 2;
        drawPos.y = pos.y + 32.0f - (size.y * scale) / 2;
    };
    virtual ~Collectable() = default;

    static void loadAssets(CollectableType type, string path);
    virtual void render();

    float getZLayer() const;
    Rectangle getHitBox() const;
};
