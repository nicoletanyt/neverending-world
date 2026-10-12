#pragma once

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

enum class AssetTypes { Background, Hearts };
enum class GameMode { Playing, Inventory, Dialog };

struct InventoryItem {
    CollectableType type;
    int quantity;
};

struct Dialog {
    const NPC *npc = nullptr;
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
    Dialog dialog;

    Sound swordSound = LoadSound("assets/sounds/swoosh.mp3");

    void renderBackground();

    void changeLocation();
    void generateTreeData();
    void generateEnemyData();

    bool playerColliding();
    bool enemyColliding();

    void addToInventory(const Collectable &collectable);
    void removeFromInventory(const CollectableType &type);
    vector<Collectable>::iterator getCollectable();

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
        npcs[location].emplace_back((Vector2){700.0f, 150.0f}, "Sandy");
    }

    void update();
    void render();
};
