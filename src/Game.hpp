#pragma once

#include "Dialogue.hpp"
#include "Quest.hpp"
#include "UI.hpp"
#include "entities/Collectable.hpp"
#include "entities/Enemy.hpp"
#include "entities/NPC.hpp"
#include "entities/Player.hpp"
#include "entities/Tree.hpp"
#include <ctime>
#include <map>
#include <raylib.h>
#include <set>
#include <variant>
#include <vector>

enum class AssetTypes { Background, Hearts };
enum class GameMode { Playing, Inventory, Dialogue, Shop };

struct InventoryItem {
    CollectableType type;
    int quantity;
};

class Game {
  private:
    UI ui;
    Player player;
    map<Location, vector<Tree>> trees;
    map<Location, vector<Enemy>> enemies;
    map<Location, vector<NPC>> npcs;

    set<Location> visited;
    Location location;
    Texture2D background;

    vector<InventoryItem> inventory;
    map<Location, vector<Collectable>> collectables;

    GameMode mode = GameMode::Playing;

    map<QuestId, Quest> quests;
    map<QuestId, QuestDialogue> questDialogues;
    map<NPCId, Dialogue> npcDialogues;
    vector<QuestId> completedQuests;

    variant<monostate, QuestDialogue *, Dialogue *> activeDialog = monostate{};
    NPC *activeNPC = nullptr;
    Quest *activeQuest = nullptr;

    int selectedOption = 0; // for dialog
    int lineIndex = 0;      // for dialog

    Sound swordSound = LoadSound("assets/sounds/swoosh.mp3");
    Sound selectSound = LoadSound("assets/sounds/select.mp3");
    Sound beepSound = LoadSound("assets/sounds/beep.mp3");
    Font font = LoadFontEx("assets/fonts/JetBrainsMono-Medium.ttf", 32, 0, 250);

    void renderBackground();

    void changeLocation();
    void generateTreeData();
    void generateEnemyData();

    bool playerColliding();
    bool enemyColliding();

    void addToInventory(const CollectableType &type, int quantity = 1);
    void removeFromInventory(const CollectableType &type);
    vector<Collectable>::iterator getCollectable();

    void interactWithNPC(NPC &npc);
    Dialogue *getCurrentDialogue();

    void initNPC();
    void initQuests();

  public:
    Game() {
        SetRandomSeed(static_cast<unsigned int>(std::time(nullptr)));

        background = LoadTexture("assets/Background.png");
        ui.loadAssets();
        Tree::loadAssets();
        Player::loadAssets();
        Enemy::loadAssets();
        NPC::loadAssets();
        Collectable::loadAssets(CollectableType::Coin, "assets/coins");
        Collectable::loadAssets(CollectableType::Potion, "assets/potions");

        // Empty Starting Location
        location = {0, 0};
        visited.insert(location);
        trees[location] = {};
        initNPC();
        initQuests();
    }

    void update();
    void render();
};
