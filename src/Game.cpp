#include "Game.hpp"
#include "Constants.hpp"
#include "Dialogue.hpp"
#include "DialogueContent.hpp"
#include "Quest.hpp"
#include "UI.hpp"
#include "entities/Collectable.hpp"
#include "entities/NPC.hpp"
#include "raylib.h"
#include "utils.hpp"
#include <algorithm>
#include <variant>
#include <vector>

void Game::update() {
    if (IsKeyPressed(KEY_E)) {
        if (mode == GameMode::Playing)
            mode = GameMode::Inventory;
        else
            mode = GameMode::Playing;
    }

    switch (mode) {
    case GameMode::Playing: {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) {
            player.fight();
            PlaySound(swordSound);
        }

        if (IsKeyPressed(KEY_Q)) {
            auto &currentCollectables = collectables[location];
            auto item = getCollectable();
            if (item != currentCollectables.end()) {
                addToInventory(item->type);
                PlaySound(beepSound);
                currentCollectables.erase(item);
            }
        }

        if (player.type == AnimationType::Fight)
            player.animate(dt);

        Vector2 oldPos = player.pos;
        bool changed = player.move(dt, &location);

        if (playerColliding())
            player.pos = oldPos;

        else if (changed)
            changeLocation();

        auto &currentEnemies = enemies[location];
        for (auto currEnemy = currentEnemies.begin();
             currEnemy != currentEnemies.end();) {
            currEnemy->update(player, dt, trees[location]);
            if (currEnemy->health == -50) {
                // enemy dies
                CollectableType type = GetRandomValue(1, 10) <= 3
                                           ? CollectableType::Potion
                                           : CollectableType::Coin;
                collectables[location].emplace_back(currEnemy->pos, type);
                currEnemy = currentEnemies.erase(currEnemy);
                // update quest
                if (activeQuest && activeQuest->status == QuestStatus::Active &&
                    activeQuest->id == QuestId::ClearMonsters) {
                    QuestObjective &objective =
                        activeQuest->objectives[activeQuest->current];
                    if (objective.type == ObjectiveType::KillEnemies) {
                        objective.progress++;
                    }
                    if (objective.progress == objective.target) {
                        activeQuest->current++;
                        objective =
                            activeQuest->objectives[activeQuest->current];
                        if (objective.type == ObjectiveType::Report)
                            activeQuest->status = QuestStatus::ReadyToReport;
                    }
                }
            } else {
                ++currEnemy;
            }
        }
        for (NPC &npc : npcs[location]) {
            npc.update();
        }

        break;
    }
    case GameMode::Inventory: {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse = GetMousePosition();
            for (int i = 0; i < SLOT_COUNT; i++) {
                Rectangle slot = ui.slots[i];
                if (CheckCollisionPointRec(mouse, slot)) {
                    if (ui.selectedSquare == i)
                        ui.selectedSquare = -1;
                    else
                        ui.selectedSquare = i;
                }
            }
            if (CheckCollisionPointRec(mouse, ui.consumeButton.rect)) {
                bool ifHeal = player.heal(10);
                if (ifHeal)
                    removeFromInventory(inventory[ui.selectedSquare].type);
            }
        }

        break;
    }
    case GameMode::Dialogue: {
        Dialogue *currentDialogue = getCurrentDialogue();
        DialogueNode &node = (*currentDialogue)[lineIndex];

        const int count = node.choices.size();
        if (count > 0) {
            if (IsKeyPressed(KEY_DOWN)) {
                selectedOption = (selectedOption + 1) % count;
                PlaySound(selectSound);
            }
            if (IsKeyPressed(KEY_UP)) {
                selectedOption = (selectedOption - 1 + count) % count;
                PlaySound(selectSound);
            }
            if (IsKeyPressed(KEY_ENTER)) {
                lineIndex = node.choices[selectedOption].nextIndex;
                selectedOption = 0;
                PlaySound(selectSound);

                if (lineIndex == -1)
                    mode = GameMode::Playing;
                switch (node.choices[selectedOption].action) {
                case DialogueAction::AcceptQuest: {
                    if (auto ptr = get_if<QuestDialogue *>(&activeDialog)) {
                        activeQuest = &quests.at((*ptr)->questId);
                        activeQuest->status = QuestStatus::Active;
                    }
                    break;
                }
                case DialogueAction::CompleteQuest: {
                    activeQuest->onCompletion();
                    completedQuests.push_back(activeQuest->id);
                    activeQuest = nullptr;
                    cout << "Quest Completed!\n";
                    break;
                }
                case DialogueAction::None:
                    break;
                case DialogueAction::OpenShop: {
                    mode = GameMode::Shop;
                    break;
                }
                }
            }
        }
    }
    case GameMode::Shop: {
        break;
    };
    }
}

void Game::changeLocation() {
    if (!visited.contains(location)) {
        visited.insert(location);
        generateTreeData();
        generateEnemyData();
    }
    // cout << location.x << " " << location.y << "\n";
}

void Game::generateTreeData() {
    for (int i = 0; i < 3; i++)
        trees[location].emplace_back();
}

void Game::generateEnemyData() {
    for (int i = 0; i < 3; i++)
        enemies[location].emplace_back();
}

bool Game::playerColliding() {
    Rectangle playerHitbox = player.getHitBox(true);

    for (const Tree &tree : trees[location]) {
        if (CheckCollisionRecs(playerHitbox, tree.getHitBox()))
            return true;
    }
    for (const Enemy &enemy : enemies[location]) {
        if (CheckCollisionRecs(playerHitbox, enemy.getHitBox()))
            return true;
    }
    for (const NPC &npc : npcs[location]) {
        if (CheckCollisionRecs(playerHitbox, npc.getHitBox()))
            return true;
    }

    return false;
}

vector<Collectable>::iterator Game::getCollectable() {
    auto &currentCollectables = collectables[location];
    Rectangle playerHitbox = player.getHitBox(false);

    return find_if(currentCollectables.begin(), currentCollectables.end(),
                   [&](const auto &collectable) {
                       return CheckCollisionRecs(playerHitbox,
                                                 collectable.getHitBox());
                   });
}

void Game::addToInventory(const CollectableType &type, int quantity) {
    for (InventoryItem &item : inventory) {
        if (item.type == type) {
            item.quantity += quantity;
            return;
        }
    }
    inventory.emplace_back(type, quantity);
}

void Game::removeFromInventory(const CollectableType &type) {
    for (auto it = inventory.begin(); it != inventory.end(); ++it) {
        if (it->type == type) {
            it->quantity--;
            if (it->quantity == 0)
                inventory.erase(it);
            return;
        }
    }
}

void Game::render() {
    DrawTextureEx(background, (Vector2){0, 0}, 0, 1, WHITE);

    vector<RenderItem> renderItems;
    renderItems.push_back({player.getZLayer(), [this]() { player.render(); }});
    for (const Tree &tree : trees[location]) {
        renderItems.push_back({tree.getZLayer(), [&tree]() { tree.render(); }});
    }
    for (const Enemy &enemy : enemies[location]) {
        renderItems.push_back(
            {enemy.getZLayer(), [&enemy]() { enemy.render(); }});
    }
    for (const NPC &npc : npcs[location]) {
        renderItems.push_back({npc.getZLayer(), [&npc]() { npc.render(); }});
    }
    for (Collectable &collectable : collectables[location]) {
        renderItems.push_back({collectable.getZLayer(),
                               [&collectable]() { collectable.render(); }});
    }
    sort(renderItems.begin(), renderItems.end(),
         [](const RenderItem &a, const RenderItem &b) { return a.z < b.z; });

    for (const RenderItem &item : renderItems) {
        item.render();
    }

    ui.renderTopbar(player);
    ui.renderHotbar(inventory);
    if (activeQuest && activeQuest->status != QuestStatus::Completed)
        ui.renderQuest(*activeQuest);

    for (const Enemy &enemy : enemies[location]) {
        if (enemy.health > 0)
            ui.renderEnemyHealth(enemy);
    }
    for (NPC &npc : npcs[location]) {
        float distance = calculateDistance(player.pos, npc.pos);
        if (distance < 64) {
            ui.renderNPCName(npc);
            if (IsKeyPressed(KEY_Q))
                interactWithNPC(npc);
        }
    }

    if (mode == GameMode::Inventory)
        ui.renderInventory(inventory);

    if (mode == GameMode::Dialogue) {
        Dialogue *current = getCurrentDialogue();
        if (current)
            ui.renderDialogue((*current).at(lineIndex), *activeNPC,
                              selectedOption);
    }

    if (mode == GameMode::Shop) {
        ui.renderShop();
    }
}

void Game::interactWithNPC(NPC &npc) {
    mode = GameMode::Dialogue;
    activeNPC = &npc;
    lineIndex = 0;
    // progress to next quest
    if (!activeQuest) {
        optional<QuestId> prevId = nullopt;
        if (!completedQuests.empty())
            prevId = completedQuests.back();
        optional<QuestId> nextId = npc.getNextQuest(prevId);
        if (nextId) {
            activeDialog = &questDialogues.at(*nextId);
            cout << "New Quest\n";
        } else {
            activeDialog = &npcDialogues.at(activeNPC->id);
        }
    } else {
        // FIX: double visit error: change npc.id to NPCId and check if npc and
        // quest are the same. if not -> default
        activeDialog = &npcDialogues.at(activeNPC->id);
    }
}

void Game::initNPC() {
    npcs[location].emplace_back(
        NPCId::Sandy, 1, (Vector2){700.0f, 150.0f}, "Sandy",
        (vector<QuestId>){QuestId::ClearMonsters, QuestId::FindMap});
    // {2, -2}
    npcs[{1, 0}].emplace_back(NPCId::Rowan, 2, (Vector2){500.0f, 500.0f},
                              "Rowan");
}

void Game::initQuests() {
    DialogueContent content;
    questDialogues.emplace(QuestId::ClearMonsters,
                           content.makeClearMonsterDialogue());
    questDialogues.emplace(QuestId::FindMap, content.makeFindMapDialogue());
    npcDialogues.emplace(NPCId::Rowan, content.makeShopDialogue());

    quests[QuestId::ClearMonsters] = {
        .id = QuestId::ClearMonsters,
        .name = "Clear Surroundings",
        .status = QuestStatus::InActive,
        .objectives =
            {
                {
                    .type = ObjectiveType::KillEnemies,
                    .description = "Kill 10 Enemies",
                    // TODO: change to 10 if not testing
                    .target = 1,
                    .progress = 0,
                },
                {
                    .type = ObjectiveType::Report,
                    .description = "Return back to Sandy",
                    .target = 1,
                    .progress = 0,
                },
            },
        .onCompletion = [this]() { addToInventory(CollectableType::Coin, 10); },
        .npcId = 1,
    };
    quests[QuestId::FindMap] = {
        .id = QuestId::FindMap,
        .name = "Find a Map",
        .status = QuestStatus::InActive,
        .objectives =
            {
                {
                    .type = ObjectiveType::FindNPC,
                    .description = "Find the travelling merchant "
                                   "that sells maps.",
                    .target = 1,
                    .progress = 0,
                },
                {
                    .type = ObjectiveType::BuyItem,
                    .description = "Buy the map from the merchant.",
                    .target = 1,
                    .progress = 0,
                },
                {
                    .type = ObjectiveType::Report,
                    .description = "Find Sandy and tell her about "
                                   "the good news.",
                    .target = 1,
                    .progress = 0,
                },
            },
        .onCompletion = [this]() { addToInventory(CollectableType::Coin, 10); },
        .npcId = 2,
    };
}

Dialogue *Game::getCurrentDialogue() {
    if (auto ptr = get_if<QuestDialogue *>(&activeDialog)) {
        QuestDialogue *dialogue = *ptr;
        return &dialogue->dialogues.at(activeQuest ? activeQuest->status
                                                   : QuestStatus::InActive);
    } else if (auto ptr = get_if<Dialogue *>(&activeDialog)) {
        return *ptr;
    }
    return nullptr;
}
