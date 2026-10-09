#include "Coin.hpp"
#include <cmath>
#include <filesystem>
#include <map>
#include <raylib.h>
#include <vector>

using namespace std;
using recursive_directory_iterator =
    std::filesystem::recursive_directory_iterator;

void Coin::render() {
    auto &frames = assets[CollectableType::Coin];
    if (frames.empty())
        return;

    frame = fmod(frame + 0.1f, static_cast<float>(frames.size()));
    DrawTextureEx(frames[static_cast<int>(frame)], drawPos, 0, .5, WHITE);
}
