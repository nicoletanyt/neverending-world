#include "UI.hpp"
#include "Constants.hpp"
#include "Game.hpp"
#include "Quest.hpp"
#include "entities/Collectable.hpp"
#include "entities/Player.hpp"
#include <algorithm>
#include <cmath>
#include <format>
#include <iostream>
#include <raylib.h>
#include <string>
#include <type_traits>

map<float, Font> UI::fonts;

void UI::loadAssets() {
    const vector<string> heartSrc = {
        "assets/hearts/0.png",
        "assets/hearts/1.png",
        "assets/hearts/2.png",
    };
    for (const string &src : heartSrc) {
        hearts.emplace_back(LoadTexture(src.c_str()));
    }

    itemAssets[CollectableType::Coin] = {
        LoadTexture("assets/coins/coin_01.png"), 0.7f};
    itemAssets[CollectableType::Potion] = {
        LoadTexture("assets/potions/healing_01.png"), 2.5f};
    itemAssets[CollectableType::Map] = {LoadTexture("assets/map.png"), 0.2f};

    profileAsset = LoadTexture("assets/profile.png");

    const char *fontFile = "assets/fonts/PixelifySans-SemiBold.ttf";
    for (float size : {FontSize::ExtraSmall, FontSize::Small, FontSize::Medium,
                       FontSize::Large}) {
        fonts[size] =
            LoadFontEx(fontFile, static_cast<int>(size), nullptr, 250);
        SetTextureFilter(fonts[size].texture, TEXTURE_FILTER_POINT);
    }

    mapLegend[LegendType::Player] = LoadTexture("assets/profile.png");
    mapLegend[LegendType::Tree] = LoadTexture("assets/tree/tree2.png");
    mapLegend[LegendType::Enemy] = LoadTexture("assets/enemy-legend.png");
    mapLegend[LegendType::NPC1] = LoadTexture("assets/avatars/npc_1.png");
    mapLegend[LegendType::NPC2] = LoadTexture("assets/avatars/npc_2.png");
}

void UI::renderTopbar(Player &player) {
    const auto &style = STYLES.topbar;
    const Rectangle topbar = {style.margin, style.margin, style.width,
                              style.height};
    const float iconSize = style.height - style.padding * 2;
    const float centerY = topbar.y + topbar.height / 2.0f;
    const Vector2 avatarCenter = {topbar.x + style.padding + iconSize / 2.0f,
                                  centerY};

    DrawRectangleRounded(topbar, style.roundness, style.segments,
                         Fade(Palette::FogGrey, style.fillOpacity));
    DrawCircleV(avatarCenter, iconSize / 2.0f,
                Fade(Palette::Charcoal, style.fillOpacity));
    drawTextureInSquare(profileAsset, avatarCenter, iconSize);

    const float heartsRight = topbar.x + topbar.width - style.padding;
    const float availableHeartWidth =
        heartsRight - (avatarCenter.x + iconSize / 2.0f);
    const float heartRowWidth = style.heartCount * style.heartSize +
                                (style.heartCount - 1) * style.heartGap;
    const float avatarToHeartGap = availableHeartWidth - heartRowWidth;
    const float heartsStartX =
        avatarCenter.x + iconSize / 2.0f + avatarToHeartGap;

    int filled = player.health / 10;

    for (int i = 0; i < style.heartCount; i++) {
        int heartIndex = min(filled, 2);
        const Vector2 heartCenter = {
            heartsStartX + i * (style.heartSize + style.heartGap) +
                style.heartSize / 2.0f,
            centerY};
        drawTextureInSquare(hearts[heartIndex], heartCenter, style.heartSize);
        filled = max(0, filled - 2);
    }
}

void UI::renderQuest(const Quest &quest) {
    const auto &style = STYLES.quest;
    const QuestObjective &objective = quest.objectives[quest.current];
    Rectangle container = {style.marginLeft, style.marginTop,
                           STYLES.topbar.width, style.height};
    Rectangle contentRect = getContentRect(container, style.padding);
    TextLayout questName = getTextLayout(quest.name, contentRect,
                                         {
                                             .fontSize = FontSize::Small,
                                             .colour = Palette::Charcoal,
                                         });

    string progressText = "[" + to_string(objective.progress) + "/" +
                          to_string(objective.target) + "] " +
                          objective.description;

    contentRect.y += questName.size.y + style.gap;
    TextLayout objectiveText = getTextLayout(
        progressText,
        {contentRect.x, contentRect.y, contentRect.width, contentRect.height},
        {
            .fontSize = FontSize::Small,
            .colour = Palette::Brown,
        });

    container.height =
        questName.size.y + style.gap + objectiveText.size.y + style.padding * 2;

    DrawRectangleRounded(container, 0.15f, 8, Fade(Palette::Parchment, .8f));
    DrawRectangleRoundedLinesEx(container, 0.15f, 8, 3.0f, Palette::Brown);

    renderText(questName);
    renderText(objectiveText);
}
void UI::renderHotbar(const vector<InventoryItem> &inventory,
                      int selectedHotbar) {
    const auto &style = STYLES.hotbar;
    for (int i = 0; i < HOTBAR_SLOTS; ++i) {
        const Rectangle slot = getHotbarSlotRect(i);
        renderSlot(slot, style.slot, Palette::FogGrey, selectedHotbar == i);

        if (i < static_cast<int>(inventory.size())) {
            renderInventoryItem(inventory[i], slot, Palette::Charcoal,
                                style.slot);
        }
    }
}

void UI::renderSlot(const Rectangle &rect, const SlotStyle &style,
                    Color fillColor, bool selected) {

    if (selected)
        DrawRectangleRounded(rect, style.roundness, style.segments,
                             Fade(Palette::MutedRed, style.fillOpacity));
    else
        DrawRectangleRounded(rect, style.roundness, style.segments,
                             Fade(fillColor, style.fillOpacity));

    DrawRectangleRoundedLinesEx(rect, style.roundness, style.segments,
                                style.borderWidth,
                                Fade(Palette::Parchment, style.borderOpacity));
}

Rectangle UI::getHotbarSlotRect(int index) const {
    const auto &style = STYLES.hotbar;
    const float x =
        WINDOW_WIDTH - style.rightMargin - style.slot.size -
        (HOTBAR_SLOTS - index - 1) * (style.slot.size + style.slotGap);
    const float y = WINDOW_HEIGHT - style.bottomMargin - style.slot.size;
    return {x, y, style.slot.size, style.slot.size};
}

void UI::renderInventoryItem(const InventoryItem &item, const Rectangle &slot,
                             Color colour, const SlotStyle slotStyle) {
    const Texture2D texture = itemAssets[item.type].texture;
    const float availableWidth = slot.width - slotStyle.imagePadding * 2;
    const float availableHeight = slot.height - slotStyle.imagePadding * 2;

    const float scale =
        min(availableWidth / texture.width, availableHeight / texture.height);

    const Vector2 texturePos = {
        slot.x + (slot.width - texture.width * scale) / 2,
        slot.y + (slot.height - texture.height * scale) / 2};
    const string text = to_string(item.quantity);

    DrawTextureEx(texture, texturePos, 0, scale, WHITE);
    DrawTextEx(fonts.at(FontSize::Medium), text.c_str(),
               {slot.x + slotStyle.quantityOffset.x,
                slot.y + slotStyle.quantityOffset.y},
               slotStyle.quantityFontSize, 2.0f, colour);
}

void UI::renderDialogue(const DialogueNode &node, const NPC &npc,
                        int selectedOption) {
    const auto &style = STYLES.dialogue;
    const Rectangle containerRect = {style.margin, style.margin, style.width,
                                     style.height};
    const Rectangle contentRect = getContentRect(containerRect, style.padding);
    const Rectangle leftRect = {contentRect.x, contentRect.y,
                                contentRect.width * style.layoutRatio,
                                contentRect.height};
    const Rectangle rightRect = {
        contentRect.x + leftRect.width + style.columnGap, contentRect.y,
        contentRect.width - leftRect.width - style.columnGap,
        contentRect.height};

    Texture2D asset = npc.avatars[npc.id];
    const float scale =
        (leftRect.width - style.avatarPadding * 2) / asset.width;
    const Vector2 avatarPos = {
        (leftRect.x + (leftRect.width - asset.width * scale) / 2), leftRect.y};

    DrawRectangleRounded(containerRect, style.roundness, style.segments,
                         Fade(Palette::Charcoal, style.opacity));
    DrawTextureEx(asset, avatarPos, 0, scale, WHITE);

    renderText(getTextLayout(
        npc.name,
        {contentRect.x, contentRect.y + asset.height * scale + style.nameGap,
         leftRect.width,
         contentRect.height - asset.height * scale - style.nameGap},
        {.fontSize = FontSize::Large,
         .colour = Palette::Parchment,
         .horizontal = Align::Center}));

    TextLayout dialogueText = getTextLayout(node.text, rightRect,
                                            {
                                                .fontSize = FontSize::Medium,
                                                .colour = Palette::Parchment,
                                            });
    renderText(dialogueText, 1.5f);

    const int choiceCount = static_cast<int>(node.choices.size());
    vector<TextLayout> choiceLayout;

    Rectangle remainingRect = {
        rightRect.x, rightRect.y + dialogueText.size.y + style.rowGap,
        rightRect.width, rightRect.height - dialogueText.size.y - style.rowGap};

    for (int i = 0; i < choiceCount; ++i) {
        const string choiceText =
            (selectedOption == i ? "> (Enter) " : "  ") + node.choices[i].text;
        const Color colour =
            selectedOption == i ? Palette::MossGreen : Palette::Parchment;
        const TextLayout layout =
            getTextLayout(choiceText, remainingRect,
                          {
                              .fontSize = FontSize::Medium,
                              .colour = colour,
                          });
        choiceLayout.push_back(layout);
        remainingRect.y += layout.size.y + style.choiceGap;
    }

    for (int i = 0; i < choiceCount; ++i)
        renderText(choiceLayout[i]);
}

void UI::renderEnemyHealth(const Enemy &enemy) {
    float padding = 1.0f;
    Vector2 size = {50.0f, 6.0f};
    const Vector2 offset = {7.0f, 2.5f};

    Rectangle outer = {enemy.pos.x + offset.x, enemy.pos.y - offset.y, size.x,
                       size.y};
    DrawRectangleRounded(outer, 0.2f, 4, Fade(Palette::Parchment, 0.8f));

    size.x = (enemy.health / enemy.maxHealth) * 50.0f;
    Rectangle inner = {enemy.pos.x + offset.x + padding,
                       enemy.pos.y - offset.y + padding, size.x - padding * 2,
                       size.y - padding * 2};
    DrawRectangleRounded(inner, 0.2f, 4, Palette::MutedRed);
}

void UI::renderNPCName(const NPC &npc) {

    TextLayout npcName = getTextLayout(
        npc.name, {npc.pos.x, npc.pos.y - 15.0f, npc.size.x, 20.0f},
        {
            .fontSize = FontSize::Medium,
            .colour = Palette::Parchment,
            .horizontal = Align::Center,
            .vertical = Align::Center,
        });
    renderText(npcName);

    Rectangle keyRect = {npc.pos.x + 40.0f, npc.pos.y + 40.0f, 28.0f, 28.0f};
    DrawRectangleRounded(keyRect, 0.3f, 8, Fade(Palette::Charcoal, 0.4f));

    TextLayout interactKey = getTextLayout("Q", keyRect,
                                           {
                                               .fontSize = FontSize::Small,
                                               .spacing = 0.0f,
                                               .colour = Palette::Parchment,
                                               .horizontal = Align::Center,
                                               .vertical = Align::Center,
                                           });
    renderText(interactKey);
}

void UI::renderInventory(const vector<InventoryItem> &inventory) {
    const Rectangle detailRect = renderItemGrid(inventory, "Inventory");

    if (selectedSquare >= 0 &&
        selectedSquare < static_cast<int>(inventory.size())) {
        renderDetailItem(inventory[selectedSquare], detailRect);
    }
}

Rectangle UI::renderItemDetails(const InventoryItem &item,
                                const Rectangle &detailRect) {
    const auto &style = STYLES.inventory;

    const Texture2D texture = itemAssets[item.type].texture;
    Rectangle remainingRect = detailRect;

    // Item Name
    const string name = collectableTypeToString(item.type);
    TextLayout itemName = getTextLayout(name.c_str(), remainingRect,
                                        {
                                            .fontSize = FontSize::Medium,
                                            .colour = Palette::Parchment,
                                        });
    remainingRect.y += itemName.size.y + style.rowGap;
    remainingRect.height -= itemName.size.y + style.rowGap;

    const float thickness = 2.0f;
    DrawLineEx({remainingRect.x, remainingRect.y},
               {remainingRect.x + remainingRect.width, remainingRect.y},
               thickness, Palette::Parchment);
    remainingRect.y += thickness + style.rowGap;
    remainingRect.height -= thickness + style.rowGap;

    TextLayout itemDescription =
        getTextLayout(collectableDescription(item.type).c_str(), remainingRect,
                      {
                          .fontSize = FontSize::Medium,
                          .colour = Palette::FadedParchment,
                      });
    renderText(itemName);
    renderText(itemDescription);
    remainingRect.y += itemDescription.size.y + 50.0f;

    // Image
    const float detailCenterX = remainingRect.x + (remainingRect.width) / 2;
    const float scale = fmin((remainingRect.width - 100.0f) / texture.width,
                             (remainingRect.height - 100.0f) / texture.height);
    const Vector2 size = {texture.width * scale, texture.height * scale};
    Vector2 imagePos = {detailCenterX - size.x / 2, remainingRect.y};

    DrawTextureEx(texture, imagePos, 0, scale, WHITE);
    remainingRect.y += size.y + 50.0f;

    return remainingRect;
}

void UI::renderDetailItem(const InventoryItem &item,
                          const Rectangle &detailRect) {
    Rectangle remainingRect = renderItemDetails(item, detailRect);

    if (item.type == CollectableType::Potion) {
        TextLayout consumeText = getTextLayout("[Q] Use Potion", remainingRect,
                                               {
                                                   .fontSize = FontSize::Medium,
                                                   .colour = Palette::Parchment,
                                               });

        renderText(consumeText);
    }
}

void UI::renderDetailItem(const ShopItem &shopItem,
                          const Rectangle &detailRect) {
    Rectangle remainingRect = renderItemDetails(shopItem.item, detailRect);
    TextLayout priceText = getTextLayout(
        format("Price: {} Coins", shopItem.price).c_str(), remainingRect,
        {
            .fontSize = FontSize::Medium,
            .colour = Palette::Parchment,
        });
    renderText(priceText);
    remainingRect.y += priceText.size.y + 30.0f;
    const string buyText =
        shopItem.stockRemaining == 0 ? "No more stock!" : "[Q] Buy";
    renderText(getTextLayout(buyText.c_str(), remainingRect,
                             {
                                 .fontSize = FontSize::Medium,
                                 .colour = shopItem.stockRemaining == 0
                                               ? Palette::MutedRed
                                               : Palette::Parchment,
                             }));
}

void UI::calculateItemSlots(const Rectangle &inventoryRect) {
    const auto &style = STYLES.inventory;
    const float slotGap = (fmin(inventoryRect.width, inventoryRect.height) -
                           (style.slot.size * INVENTORY_COLS)) /
                          (INVENTORY_COLS - 1);

    const float width = style.slot.size + slotGap;

    for (int i = 0; i < SLOT_COUNT; ++i) {
        const int column = i % INVENTORY_COLS, row = i / INVENTORY_COLS;
        slots[i] = {inventoryRect.x + column * width,
                    inventoryRect.y + row * width, style.slot.size,
                    style.slot.size};
    }
}

Vector2 UI::getStartPos(Rectangle bounds, TextStyle style, float textHeight) {
    Vector2 pos;

    switch (style.horizontal) {
    case Align::Start:
        pos.x = bounds.x;
        break;
    case Align::Center:
        pos.x = bounds.x + bounds.width / 2;
        break;
    case Align::End:
        pos.x = bounds.x + bounds.width;
        break;
    }

    switch (style.vertical) {
    case Align::Start:
        pos.y = bounds.y;
        break;
    case Align::Center:
        pos.y = bounds.y + (bounds.height - textHeight) / 2;
        break;
    case Align::End:
        pos.y = bounds.y + bounds.height - textHeight;
        break;
    }
    return pos;
}

void UI::renderText(const TextLayout &layout, float lineHeight) {
    const Font &font = fonts.at(layout.style.fontSize);
    Vector2 pos = layout.pos;
    for (const string &line : layout.lines) {
        float lineWidth =
            MeasureTextEx(font, line.c_str(), layout.style.fontSize,
                          layout.style.spacing)
                .x;
        float x = pos.x;

        switch (layout.style.horizontal) {
        case Align::Start:
            break;
        case Align::Center:
            x -= lineWidth / 2;
            break;
        case Align::End:
            x -= lineWidth;
            break;
        }
        DrawTextEx(font, line.c_str(), {x, pos.y}, layout.style.fontSize,
                   layout.style.spacing, layout.style.colour);
        pos.y += font.baseSize * lineHeight;
    }
}

Rectangle UI::getContentRect(Rectangle rect, float padding) {
    return {rect.x + padding, rect.y + padding, rect.width - padding * 2,
            rect.height - padding * 2};
}

vector<string> UI::wrapText(const string &text, const float maxWidth,
                            TextStyle style) {
    const Font &font = fonts.at(style.fontSize);
    const float spaceWidth =
        MeasureTextEx(font, " ", style.fontSize, style.spacing).x +
        style.spacing + 3.0f;
    float currentX = 0;
    string word = "";
    vector<string> lines = {""};

    for (size_t i = 0; i <= text.size(); ++i) {
        if (i == text.size() || text[i] == ' ') {
            if (word.empty())
                continue;
            float wordWidth =
                MeasureTextEx(font, word.c_str(), style.fontSize, style.spacing)
                    .x;
            float requiredWidth =
                wordWidth + (lines.back().empty() ? 0.0f : spaceWidth);

            if (!lines.back().empty() && currentX + requiredWidth > maxWidth) {
                // new line
                currentX = 0;
                lines.push_back("");
            }
            if (!lines.back().empty()) {
                currentX += spaceWidth;
                lines.back() += " ";
            }
            currentX += wordWidth;
            lines.back() += word;
            word = "";
        } else {
            word += text[i];
        }
    }
    return lines;
}

float UI::getTextHeight(int lineCount, float fontSize, float lineHeight) {
    if (lineCount == 0)
        return 0;
    return fontSize + (lineCount - 1) * fontSize * lineHeight;
}

TextLayout UI::getTextLayout(const string &text, const Rectangle &bounds,
                             TextStyle style) {
    const Font &font = fonts.at(style.fontSize);
    vector<string> lines = wrapText(text, bounds.width, style);
    float height = getTextHeight(lines.size(), style.fontSize);
    float width = lines.size() == 1
                      ? MeasureTextEx(font, lines[0].c_str(), style.fontSize,
                                      style.spacing)
                            .x
                      : bounds.width;
    Vector2 pos = getStartPos(bounds, style, height);
    return {pos, lines, {width, height}, style};
}

void UI::drawTextureInSquare(const Texture2D &texture, Vector2 center,
                             float size) {
    const float scale = size / max(texture.width, texture.height);

    const Vector2 pos = {center.x - texture.width * scale / 2.0f,
                         center.y - texture.height * scale / 2.0f};

    DrawTextureEx(texture, pos, 0, scale, WHITE);
}

void UI::renderShop(vector<ShopItem> shop) {
    const Rectangle detailRect = renderItemGrid(shop, "Rowan's Shop");

    if (selectedSquare >= 0 && selectedSquare < static_cast<int>(shop.size())) {
        renderDetailItem(shop[selectedSquare], detailRect);
    }
}

template <typename T>
Rectangle UI::renderItemGrid(const vector<T> &items, const string &heading) {
    const auto &style = STYLES.inventory;
    const Rectangle containerRect = {style.margin, style.margin, style.width,
                                     style.height};
    const Rectangle contentRect = getContentRect(containerRect, style.padding);

    const Rectangle leftRect = {contentRect.x, contentRect.y,
                                contentRect.width * style.layoutRatio,
                                contentRect.height};
    const Rectangle rightRect = {
        contentRect.x + leftRect.width + style.columnGap, contentRect.y,
        contentRect.width * (1 - style.layoutRatio) - style.columnGap,
        contentRect.height};
    TextLayout title = getTextLayout(heading, leftRect,
                                     {
                                         .fontSize = FontSize::Large,
                                         .spacing = 1.5f,
                                         .colour = Palette::Brown,
                                     });
    float topOffset = title.size.y + style.topGap;

    Rectangle inventoryRect = {leftRect.x, leftRect.y + topOffset,
                               leftRect.width, leftRect.height - topOffset};
    calculateItemSlots(inventoryRect);

    DrawRectangleRounded(containerRect, style.roundness, style.segments,
                         Fade(Palette::Charcoal, style.opacity));
    renderText(title);
    for (int i = 0; i < SLOT_COUNT; i++) {
        const Rectangle slot = slots[i];
        const Color fillColor =
            selectedSquare == i ? Palette::MossGreen : Palette::FogGrey;
        renderSlot(slot, style.slot, fillColor);

        if (i < static_cast<int>(items.size())) {
            if constexpr (is_same_v<T, ShopItem>) {
                renderInventoryItem(items[i].item, slot, Palette::Charcoal,
                                    style.slot);
            } else {
                renderInventoryItem(items[i], slot, Palette::Charcoal,
                                    style.slot);
            }
        }
    }
    return rightRect;
}

template Rectangle
UI::renderItemGrid<InventoryItem>(const vector<InventoryItem> &,
                                  const string &heading);
template Rectangle UI::renderItemGrid<ShopItem>(const vector<ShopItem> &,
                                                const string &heading);

void UI::drawMarker(LegendType legendType, Vector2 pos, Rectangle innerRect) {
    DrawTextureEx(mapLegend[legendType],
                  {innerRect.x + pos.x / WINDOW_WIDTH * innerRect.width,
                   innerRect.y + pos.y / WINDOW_HEIGHT * innerRect.height},
                  0, .4f, WHITE);
}

void UI::renderMap(const map<Location, vector<Tree>> &trees,
                   const map<Location, vector<Enemy>> &enemies,
                   const map<Location, vector<NPC>> &npcs,
                   const set<Location> &visited, const Location &location,
                   Vector2 mapOffset, Vector2 playerPos) {
    const auto &style = STYLES.map;
    const Rectangle containerRect = {style.margin, style.margin, style.width,
                                     style.height};
    const Rectangle contentRect = getContentRect(containerRect, style.padding);

    DrawRectangleRounded(containerRect, style.roundness, style.segments,
                         Fade(Palette::Charcoal, style.opacity));

    TextLayout title = getTextLayout("World Map", contentRect,
                                     {
                                         .fontSize = FontSize::Large,
                                         .spacing = 1.5f,
                                         .colour = Palette::Brown,
                                     });
    renderText(title);

    TextLayout locationLabel = getTextLayout(
        "Press ESC to close the map, or R to recenter on your location.",
        contentRect,
        {.fontSize = FontSize::Medium,
         .spacing = 1.5f,
         .colour = Palette::Brown,
         .vertical = Align::End});
    renderText(locationLabel);

    const Rectangle map = {contentRect.x,
                           contentRect.y + title.size.y + style.sectionGap,
                           contentRect.width,
                           contentRect.height - locationLabel.size.y -
                               title.size.y - style.sectionGap * 2

    };

    DrawRectangleRec(map, Palette::FadedParchment);
    BeginScissorMode(map.x, map.y, map.width, map.height);
    Vector2 rectCenter = {
        location.x * style.cellSize + style.cellSize / 2,
        location.y * style.cellSize + style.cellSize / 2,
    };
    Vector2 centerOffset = {
        map.x + map.width / 2 - rectCenter.x,
        map.y + map.height / 2 - rectCenter.y,
    };
    centerOffset.x += mapOffset.x;
    centerOffset.y += mapOffset.y;

    for (const Location &currentLoc : visited) {
        Rectangle rect = {currentLoc.x * style.cellSize + centerOffset.x,
                          currentLoc.y * style.cellSize + centerOffset.y,
                          style.cellSize, style.cellSize};
        Rectangle innerRect = {
            rect.x + style.cellGap / 2, rect.y + style.cellGap / 2,
            rect.width - style.cellGap, rect.height - style.cellGap};

        DrawRectangleRec(innerRect, Palette::Parchment);
        if (location == currentLoc) {
            DrawRectangleLinesEx(rect, style.cellGap, Palette::Brown);
            drawMarker(LegendType::Player, playerPos, innerRect);
        }
        for (const auto &enemy : enemies.at(currentLoc)) {
            drawMarker(LegendType::Enemy, enemy.pos, innerRect);
        }
        for (const auto &tree : trees.at(currentLoc)) {
            drawMarker(LegendType::Tree, tree.pos, innerRect);
        }
        for (const auto &npc : npcs.at(currentLoc)) {
            drawMarker(npc.type == 1 ? LegendType::NPC1 : LegendType::NPC2,
                       npc.pos, innerRect);
        }

        const Rectangle rectContent = getContentRect(rect, rect.width / 12.0f);
        renderText(getTextLayout(currentLoc.toString(), rectContent,
                                 {.colour = Fade(Palette::MutedBrown, 0.85f),
                                  .horizontal = Align::Start,
                                  .vertical = Align::Start}));
    }
    EndScissorMode();
}
