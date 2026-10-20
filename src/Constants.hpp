#pragma once

#include <functional>
#include <map>
#include <raylib.h>
#include <string>
using namespace std;

inline constexpr float WINDOW_WIDTH = 900.0f;
inline constexpr float WINDOW_HEIGHT = 700.0f;

inline constexpr float HOTBAR_SLOTS = 5;
const int INVENTORY_COLS = 4;
const int SLOT_COUNT = INVENTORY_COLS * INVENTORY_COLS;
inline constexpr float INVENTORY_FONT_SIZE = 20.0f;

enum class Direction { Up = 0, Right = 90, Left = -90, Down = 180 };

const map<Direction, Vector2> ANGLE_TO_DIRECTIONS = {
    {Direction::Up, {0, -1}},
    {Direction::Down, {0, 1}},
    {Direction::Left, {-1, 0}},
    {Direction::Right, {1, 0}},
};

struct Location {
    int x, y;
    bool operator<(const Location &other) const {
        if (x != other.x)
            return x < other.x;
        return y < other.y;
    }
    bool operator==(const Location &other) const {
        return x == other.x && y == other.y;
    }
    string toString() const {
        return "(" + to_string(x) + ", " + to_string(y) + ")";
    }
};

struct RenderItem {
    float z;
    function<void()> render;
};
