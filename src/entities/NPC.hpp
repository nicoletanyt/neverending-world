#pragma once
#include "../Quest.hpp"
#include "raylib.h"
#include <map>
#include <optional>
#include <string>
#include <vector>
using namespace std;

class Player;

enum class NPCId { Sandy, Rowan };

class NPC {
  private:
    float direction = 180;

  public:
    static map<int, map<int, vector<Texture2D>>> assets;
    static map<NPCId, Texture2D> avatars;
    static constexpr Vector2 size = {64.0f, 64.0f};

    const NPCId id;
    const int type;
    Vector2 pos = {0, 0};
    float frame = 0;
    string name;

    vector<QuestId> quests;

    NPC(NPCId id, int type, Vector2 pos, string name,
        vector<QuestId> quests = {})
        : id(id), type(type), pos(pos), name(std::move(name)),
          quests(std::move(quests)) {};

    float getZLayer() const;
    Rectangle getHitBox() const;

    static void loadAssets();
    void update();
    bool isInteraction(Player &player);
    void render() const;
    void animate(float deltaTime);

    optional<QuestId> getNextQuest(optional<QuestId> current);
};
