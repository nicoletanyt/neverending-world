#include "Game.hpp"
#include "Constants.hpp"
#include "Dialogue.hpp"
#include "Quest.hpp"
#include "UI.hpp"
#include "entities/Collectable.hpp"
#include "raylib.h"
#include "utils.hpp"
#include <algorithm>
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
            for (int i = 0; i < 15; i++) {
                Rectangle slot = ui.getInventorySlotRect(i);
                if (CheckCollisionPointRec(mouse, slot)) {
                    if (ui.selectedSquare == i)
                        ui.selectedSquare = -1;
                    else
                        ui.selectedSquare = i;
                }
            }
            if (CheckCollisionPointRec(mouse, ui.consumeBtn)) {
                bool ifHeal = player.heal(10);
                if (ifHeal)
                    removeFromInventory(inventory[ui.selectedSquare].type);
            }
        }

        break;
    }
    case GameMode::Dialog: {
        const Dialogue &currentDialogue =
            activeDialog->dialogues[activeQuest ? activeQuest->status
                                                : QuestStatus::InActive];
        const DialogueNode &node = currentDialogue[lineIndex];

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

                switch (node.choices[selectedOption].action) {
                case DialogueAction::AcceptQuest: {
                    activeQuest = &quests.at(activeDialog->questId);
                    activeQuest->status = QuestStatus::Active;
                    break;
                }
                case DialogueAction::CompleteQuest: {
                    activeQuest->onCompletion();
                    cout << "Quest Completed!\n";
                    activeQuest = nullptr;
                    break;
                }
                case DialogueAction::None:
                    break;
                }
                if (lineIndex == -1)
                    mode = GameMode::Playing;
            }
        }
    }
    }
}

void Game::changeLocation() {
    if (!visited.contains(location)) {
        visited.insert(location);
        generateTreeData();
        generateEnemyData();
    }
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
    if (activeQuest)
        ui.renderQuest(*activeQuest);

    for (const Enemy &enemy : enemies[location]) {
        if (enemy.health > 0)
            ui.renderEnemyHealth(enemy);
    }
    for (NPC &npc : npcs[location]) {
        float distance = calculateDistance(player.pos, npc.pos);
        if (distance < 64) {
            ui.renderNPCName(npc);
            if (IsKeyDown(KEY_Q))
                interactWithNPC(npc);
        }
    }

    if (mode == GameMode::Inventory)
        ui.renderInventory(inventory);

    if (mode == GameMode::Dialog) {
        Dialogue &current =
            activeDialog->dialogues[activeQuest ? activeQuest->status
                                                : QuestStatus::InActive];
        ui.renderDialogue(current[lineIndex], *activeNPC, selectedOption);
    }
}

void Game::interactWithNPC(NPC &npc) {
    mode = GameMode::Dialog;
    activeDialog = &dialogues.at(npc.type);
    activeNPC = &npc;
    lineIndex = 0;
}

void Game::initNPC() {
    const NPC sandy = {1, (Vector2){700.0f, 150.0f}, "Sandy"};
    npcs[location].push_back(sandy);

    Dialogue sandyIntro{
        dialogueNext("Oh! Hello there. I'm Sandy. I look after the "
                     "library nearby… or, well, I try to.",
                     1),
        dialogueNext("There are monsters wandering around the area, "
                     "and I can't exactly fight them myself.",
                     2),

        {"Would you mind helping me clear them out? I'd really "
         "appreciate it.",
         {
             {"Sure, I'll help!", 3},
             {"What do I need to do?", 4},
             {"No thanks.", 6},
         }},
        dialogueNext("Really? Thank you! Just take care of ten of the "
                     "monsters nearby, and I should be able to move around "
                     "more safely. Please be careful out there.",
                     5),
        dialogueNext("Just take care of ten of the monsters nearby. I "
                     "would "
                     "do it myself, but I'm afraid I'm not much of a "
                     "fighter.",
                     5),
        dialogueNext("Once you're done, come back and find me!", -1,
                     DialogueAction::AcceptQuest),
        dialogueNext("Oh… that's alright. I understand. I'll just have to "
                     "manage on my own for now.",
                     7),
        dialogueNext("Do let me know if you change your mind! And be "
                     "careful "
                     "if you decide to leave. It's rather easy to lose "
                     "your "
                     "way around here.",
                     -1),
    };

    QuestDialogue sandyClearQuest{
        .questId = QuestId::ClearMonsters,
        .dialogues =
            {{QuestStatus::InActive, sandyIntro},
             {
                 QuestStatus::Active,
                 {
                     {"Oh, you're back! How's the monster hunting going?",
                      {
                          {"Still working on it.", 1},
                          {"Remind me what I need to do?", 2},
                      }},
                     dialogueNext(
                         "No rush! Well… maybe a little. Just be careful "
                         "out there, alright?",
                         -1),
                     dialogueNext("Ten monsters, that's all! Come find me when "
                                  "you're done. I'll be right here!",
                                  -1),
                 },
             },
             {QuestStatus::ReadyToReport,
              {

                  dialogueNext("You're back! And in one piece! Does that mean "
                               "you've taken care of those monsters?",
                               1),
                  dialogueNext(
                      "All ten of them? That's incredible! I can finally "
                      "get back to looking after the library.",
                      2),
                  dialogueNext("Here, take this as a little thank-you. "
                               "You've really helped me out!",
                               -1, DialogueAction::CompleteQuest),
              }},
             {QuestStatus::Completed,
              {
                  dialogueNext("Oh, hello again! Things have been much quieter "
                               "around here thanks to you.",
                               1),
                  dialogueNext("If you ever get tired of adventuring, you're "
                               "always welcome to stop by the library!",
                               -1),

              }}},
    };
    dialogues[sandy.type] = sandyClearQuest;
}

void Game::initQuests() {
    quests[QuestId::ClearMonsters] = {
        .id = QuestId::ClearMonsters,
        .name = "Clear Surroundings",
        .status = QuestStatus::InActive,
        .objectives =
            {
                {
                    .type = ObjectiveType::KillEnemies,
                    .description = "Kill 10 Enemies",
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
}
