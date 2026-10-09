#include "Tree.hpp"
#include <raylib.h>
#include <string>
#include <vector>

using namespace std;

vector<Texture2D> Tree::assets;

float Tree::getZLayer() const {
    const Rectangle hitBox = getHitBox();
    // Player depth uses its sprite top, 56 pixels above its foot hitbox center.
    return hitBox.y + hitBox.height / 2.0f - 56.0f;
}

Rectangle Tree::getHitBox() const {
    Rectangle local = {};
    if (type == 1)
        local = {20, 51, 12, 18};
    if (type == 2)
        local = {18, 51, 14, 20};
    if (type == 3)
        local = {15, 45, 15, 18};

    return {pos.x + local.x * scale, pos.y + local.y * scale,
            local.width * scale, local.height * scale};
}

void Tree::loadAssets() {
    for (int type = 1; type <= 3; type++) {
        const string fileName = "assets/tree/tree" + to_string(type) + ".png";
        Texture2D texture = LoadTexture(fileName.c_str());
        assets.push_back(texture);
    }
}

void Tree::render() const {
    Texture2D texture = assets[type - 1];
    DrawTextureEx(texture, pos, 0, scale, WHITE);
}
