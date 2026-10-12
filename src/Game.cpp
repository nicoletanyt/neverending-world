#include "Game.hpp"
#include "Constants.hpp"
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
                addToInventory(*item);
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
                CollectableType type = GetRandomValue(1, 10) <= 3
                                           ? CollectableType::Potion
                                           : CollectableType::Coin;
                collectables[location].emplace_back(currEnemy->pos, type);
                currEnemy = currentEnemies.erase(currEnemy);
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
        // go next dialogue
        const DialogueElement &element =
            dialogues[activeNPC->type].elements[lineIndex];

        if (const auto *node = std::get_if<DialogueNode>(&element)) {
            if (IsKeyPressed(KEY_ENTER)) {
                lineIndex = node->nextIndex;
                PlaySound(selectSound);
                if (lineIndex == -1)
                    mode = GameMode::Playing;
            }
        } else if (const auto *choice = std::get_if<DialogueChoice>(&element)) {
            const int count = choice->options.size();
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
                    lineIndex = choice->options[selectedOption].nextIndex;
                    selectedOption = 0;
                    PlaySound(selectSound);
                }
            }
        }
        break;
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

void Game::addToInventory(const Collectable &collectable) {
    for (InventoryItem &item : inventory) {
        if (item.type == collectable.type) {
            item.quantity++;
            return;
        }
    }
    inventory.emplace_back(collectable.type, 1);
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

    for (const Enemy &enemy : enemies[location]) {
        if (enemy.health > 0)
            ui.renderEnemyHealth(enemy);
    }
    for (NPC &npc : npcs[location]) {
        float distance = calculateDistance(player.pos, npc.pos);
        if (distance < 64) {
            ui.renderNPCName(npc);
            if (IsKeyDown(KEY_Q)) {
                mode = GameMode::Dialog;
                activeDialog = &dialogues.at(npc.type);
                activeNPC = &npc;
                lineIndex = 0;
            }
        }
    }

    if (mode == GameMode::Inventory)
        ui.renderInventory(inventory);

    if (mode == GameMode::Dialog)
        ui.renderDialogue(*activeDialog, *activeNPC, lineIndex, selectedOption);
}

void Game::createNPCs() {
    const NPC sandy = {1, (Vector2){700.0f, 150.0f}, "Sandy"};
    npcs[location].push_back(sandy);

    dialogues.emplace(
        sandy.type,
        Dialogue{{
            // 0 - introduction
            DialogueNode{"Oh! Hello there. I'm Sandy. I look after the "
                         "library nearby… or, well, I try to.",
                         1},

            // 1 - explain the problem
            DialogueNode{"There are monsters wandering around the area, and I "
                         "can't exactly fight them myself.",
                         2},

            // 2 - ask for help
            DialogueNode{"Would you mind helping me clear them out? I'd "
                         "really appreciate it.",
                         3},

            // 3 - player choices
            DialogueChoice{{{"Sure, I'll help!", 4},
                            {"What do I need to do?", 5},
                            {"No thanks.", 7}}},

            // 4 - accept quest
            DialogueNode{"Really? Thank you! Just take care of ten of the "
                         "monsters nearby, and I should be able to move around "
                         "more safely. Please be careful out there.",
                         6},

            // 5 - explain quest objective
            DialogueNode{
                "Just take care of ten of the monsters nearby. I would "
                "do it myself, but I'm afraid I'm not much of a fighter.",
                6},

            // 6 - follow-up after explaining
            DialogueNode{"Once you're done, come back and find me!", -1},

            // 7 - decline quest
            DialogueNode{"Oh… that's alright. I understand. I'll just have to "
                         "manage on my own for now.",
                         8},

            // 8 - farewell after declining
            DialogueNode{
                "Do let me know if you change your mind! And be careful "
                "if you decide to leave. It's rather easy to lose your "
                "way around here.",
                -1},
        }});
}
