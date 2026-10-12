#include "NPC.hpp"
#include "../utils.hpp"
#include "Player.hpp"
#include <filesystem>
#include <iostream>
#include <map>
#include <raylib.h>
#include <regex>

using namespace std;
using recursive_directory_iterator =
    std::filesystem::recursive_directory_iterator;

map<int, map<int, vector<Texture2D>>> NPC::assets;
map<int, Texture2D> NPC::avatars;

void NPC::loadAssets() {
    const regex pattern(R"(^npc_(\d+)_walk_(-?\d+)_(\d+)$)");

    map<int, map<int, map<int, filesystem::path>>> orderedPaths;

    for (const auto &dirEntry : recursive_directory_iterator("assets/npc")) {
        const string name = dirEntry.path().stem().string();
        smatch match;

        if (!regex_match(name, match, pattern))
            continue;
        const int type = stoi(match[1].str());
        const int direction = stoi(match[2].str());
        const int frameNumber = stoi(match[3].str());

        orderedPaths[type][direction][frameNumber] = dirEntry.path();
    }
    for (const auto &[type, directions] : orderedPaths) {
        for (const auto &[direction, paths] : directions) {
            auto &frames = assets[type][direction];
            frames.reserve(paths.size());
            for (const auto &[frameNumber, path] : paths) {
                frames.push_back(LoadTexture(path.string().c_str()));
            }
        }
    }
    avatars[0] = LoadTexture("assets/avatars/npc_1.png");
}

void NPC::update() {}

bool NPC::isInteraction(Player &player) {
    // interacting with npc
    float distance = calculateDistance(player.pos, pos);
    return distance < 64 && IsKeyDown(KEY_Q);
}

float NPC::getZLayer() const { return pos.y; }

Rectangle NPC::getHitBox() const { return {pos.x + 16, pos.y + 7, 32, 52}; }

void NPC::render() const {
    Texture2D texture = assets[npcType][direction][static_cast<int>(frame)];
    DrawTextureV(texture, pos, WHITE);
}
