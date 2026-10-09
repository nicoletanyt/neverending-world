#pragma once
#include "Collectable.hpp"
#include <raylib.h>
using namespace std;

class Coin : public Collectable {
private:
    float frame = 0;

public:
    Coin(Vector2 pos) : Collectable(pos, CollectableType::Coin) {};

    void render() override;
};
