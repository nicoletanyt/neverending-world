#include "Collectable.hpp"
#include <filesystem>
#include <map>
#include <regex>

using namespace std;
using recursive_directory_iterator =
    std::filesystem::recursive_directory_iterator;

map<CollectableType, vector<Texture2D>> Collectable::assets;

string collectableTypeToString(CollectableType type) {
    switch (type) {
    case CollectableType::Coin:
        return "Coin";
    case CollectableType::Potion:
        return "Potion";
    case CollectableType::Map:
        return "Map";
    }

    return "Unknown";
}

string collectableDescription(CollectableType type) {
    switch (type) {
    case CollectableType::Coin:
        return "Useful for trading with merchants.";
    case CollectableType::Potion:
        return "Restores 10 health (½ heart).";
    case CollectableType::Map:
        return "A worn map of the surrounding lands. Who knows what secrets "
               "lie beyond its edges?";
    }

    return "";
}

void Collectable::loadAssets(CollectableType type, string path) {
    const regex pattern(R"(^.+_(\d+)\.png$)");
    map<int, filesystem::path> orderedPaths;

    for (const auto &dirEntry : recursive_directory_iterator(path)) {
        const string name = dirEntry.path().string();
        smatch match;
        if (!regex_match(name, match, pattern))
            continue;

        const int frameNumber = stoi(match[1].str());

        orderedPaths[frameNumber] = dirEntry.path();
    }

    for (const auto &[frameNumber, path] : orderedPaths) {
        assets[type].push_back(LoadTexture(path.string().c_str()));
    }
}

float Collectable::getZLayer() const { return drawPos.y; }

Rectangle Collectable::getHitBox() const {
    return {drawPos.x, drawPos.y, size.x * scale, size.y * scale};
}

void Collectable::render() {
    // by default, it only has 1 frame
    auto &frames = assets.at(type);
    if (frames.empty())
        return;

    DrawTextureEx(frames[0], drawPos, 0, scale, WHITE);
}
