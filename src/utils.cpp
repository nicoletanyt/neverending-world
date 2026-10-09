#include "utils.hpp"
#include <cmath>

float calculateDistance(Vector2 pos1, Vector2 pos2) {
    float a = std::abs(pos1.x - pos2.x);
    float b = std::abs(pos1.y - pos2.y);
    return std::sqrt((a * a) + (b * b));
}
