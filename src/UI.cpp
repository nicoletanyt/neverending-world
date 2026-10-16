#include "UI.hpp"
#include "Constants.hpp"
#include "Game.hpp"
#include "Quest.hpp"
#include "entities/Collectable.hpp"
#include "entities/Player.hpp"
#include <algorithm>
#include <cmath>
#include <format>
#include <raylib.h>
#include <string>

Font UI::fontSmall = {};
Font UI::fontMedium = {};
Font UI::fontLarge = {};

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

    profileAsset = LoadTexture("assets/profile.png");

    const char *fontFile = "assets/fonts/PixelifySans-SemiBold.ttf";
    fontSmall = LoadFontEx(fontFile, 16, 0, 250);
    fontMedium = LoadFontEx(fontFile, 32, 0, 250);
    fontLarge = LoadFontEx(fontFile, 64, 0, 250);
    SetTextureFilter(fontSmall.texture, TEXTURE_FILTER_POINT);
    SetTextureFilter(fontMedium.texture, TEXTURE_FILTER_POINT);
    SetTextureFilter(fontLarge.texture, TEXTURE_FILTER_POINT);
}

void UI::renderTopbar(Player &player) {
    const auto &style = STYLES.topbar;
    const float iconSize =
        (style.width - style.padding * 2 - style.gap * style.heartCount) /
        (style.heartCount + 1);
    const Rectangle topbar = {style.margin, style.margin, style.width,
                              style.padding * 2 + iconSize};
    DrawRectangleRounded(topbar, .3f, 0, Palette::FadedParchment);

    const Vector2 center = {topbar.x + style.padding + iconSize / 2.0f,
                            topbar.y + topbar.height / 2.0f};
    DrawCircleV(center, iconSize / 2.0f,
                Fade(Palette::Charcoal, style.fillOpacity));
    const float profileScale =
        iconSize / max(profileAsset.width, profileAsset.height);
    const Vector2 profilePos = {
        center.x - profileAsset.width * profileScale / 2.0f,
        center.y - profileAsset.height * profileScale / 2.0f};
    DrawTextureEx(profileAsset, profilePos, 0, profileScale, WHITE);

    int filled = player.health / 10;
    for (int i = 0; i < style.heartCount; i++) {
        int heartIndex = min(filled, 2);
        const Texture2D &heart = hearts[heartIndex];
        const float heartScale = iconSize / max(heart.width, heart.height);
        const Vector2 heartPos = {center.x + (i + 1) * (iconSize + style.gap) -
                                      heart.width * heartScale / 2.0f,
                                  center.y - heart.height * heartScale / 2.0f};
        DrawTextureEx(heart, heartPos, 0, heartScale, WHITE);
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
                                             .fontSize = 16.0f,
                                             .colour = Palette::Charcoal,
                                         });

    string progressText = "[" + to_string(objective.progress) + "/" +
                          to_string(objective.target) + "] " +
                          objective.description;

    contentRect.y += questName.height + style.gap;
    TextLayout objectiveText = getTextLayout(
        progressText,
        {contentRect.x, contentRect.y, contentRect.width, contentRect.height},
        {
            .fontSize = 16.0f,
            .colour = Palette::Brown,
        });

    container.height =
        questName.height + style.gap + objectiveText.height + style.padding * 2;

    DrawRectangleRounded(container, 0.15f, 8, Fade(Palette::Parchment, .8f));
    DrawRectangleRoundedLinesEx(container, 0.15f, 8, 3.0f, Palette::Brown);

    renderText(questName);
    renderText(objectiveText);
}
void UI::renderHotbar(const vector<InventoryItem> &inventory) {
    const auto &style = STYLES.hotbar;
    for (int i = 0; i < HOTBAR_SLOTS; ++i) {
        const Rectangle slot = getHotbarSlotRect(i);
        renderSlot(slot, style.slot, Palette::FogGrey);

        if (i < static_cast<int>(inventory.size())) {
            renderInventoryItem(inventory[i], slot, Palette::Charcoal);
        }
    }
}

void UI::renderSlot(const Rectangle &rect, const SlotStyle &style,
                    Color fillColor) {
    DrawRectangleRounded(rect, style.roundness, style.segments,
                         Fade(fillColor, style.fillOpacity));
    DrawRectangleRoundedLinesEx(rect, style.roundness, style.segments,
                                style.borderWidth,
                                Fade(Palette::Parchment, style.borderOpacity));
}

Rectangle UI::getHotbarSlotRect(int index) const {
    const auto &style = STYLES.hotbar;
    const float x =
        WINDOW_WIDTH - style.rightMargin - style.slotSize -
        (HOTBAR_SLOTS - index - 1) * (style.slotSize + style.slotGap);
    const float y = WINDOW_HEIGHT - style.bottomMargin - style.slotSize;
    return {x, y, style.slotSize, style.slotSize};
}

void UI::renderInventoryItem(const InventoryItem &item, const Rectangle &slot,
                             Color colour) {
    const auto &icon = itemAssets[item.type];
    const SlotStyle slotStyle = STYLES.inventory.slot;
    const Texture2D texture = icon.texture;
    // const float scale = icon.scale;
    const float scale =
        fmin((slot.width - slotStyle.imagePadding * 2) / texture.width,
             (slot.height - slotStyle.imagePadding * 2) / texture.height);
    const Vector2 texturePos = {
        slot.x + (slot.width - texture.width * scale) / 2,
        slot.y + (slot.height - texture.height * scale) / 2};
    const string text = to_string(item.quantity);

    DrawTextureEx(texture, texturePos, 0, scale, WHITE);
    DrawTextEx(fontMedium, text.c_str(),
               {slot.x + slotStyle.quantityOffset.x,
                slot.y + slotStyle.quantityOffset.y},
               slotStyle.quantityFontSize, 2.0f, colour);
}

void UI::renderDialogue(const DialogueNode &node, const NPC &npc,
                        int selectedOption) {
    const auto &style = STYLES.dialogue;
    DrawRectangleRounded(
        {style.margin, style.margin, style.width, style.height},
        style.roundness, style.segments,
        Fade(Palette::Charcoal, style.opacity));
    const Rectangle contentRect = {
        style.margin + style.padding, style.margin + style.padding,
        style.width - style.padding * 2, style.height - style.padding * 2};
    Texture2D asset = npc.avatars[npc.id];
    const float scale = style.avatarScale;
    const float leftColWidth = asset.width * scale;

    DrawTextureEx(asset, {contentRect.x, contentRect.y}, 0, scale, WHITE);

    renderText(getTextLayout(
        npc.name,
        {contentRect.x, contentRect.y + asset.height * scale + style.nameGap,
         leftColWidth,
         contentRect.height - contentRect.y - asset.height * scale},
        {.fontSize = style.nameFontSize,
         .colour = Palette::Parchment,
         .horizontal = Align::Center}));

    const Rectangle dialogueRect = {
        contentRect.x + leftColWidth + style.columnGap, contentRect.y,
        contentRect.width - leftColWidth, contentRect.height};

    renderText(getTextLayout(node.text, dialogueRect,
                             {
                                 .fontSize = style.textFontSize,
                                 .colour = Palette::Parchment,
                             }));

    const int choiceCount = static_cast<int>(node.choices.size());
    const float totalHeight =
        choiceCount * style.choiceHeight + (choiceCount - 1) * style.choiceGap;

    const float startY = dialogueRect.y + dialogueRect.height - totalHeight -
                         style.bottomPadding;

    for (int i = 0; i < choiceCount; ++i) {
        const string choiceText = "> " + node.choices[i].text;

        const Color colour =
            selectedOption == i ? Palette::FogGrey : Palette::Parchment;

        const Rectangle choiceRect = {
            dialogueRect.x, startY + i * (style.choiceHeight + style.choiceGap),
            dialogueRect.width, style.choiceHeight};

        renderText(getTextLayout(choiceText, choiceRect,
                                 {
                                     .fontSize = style.textFontSize,
                                     .colour = colour,
                                     .horizontal = Align::End,
                                 }));
    }
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
            .fontSize = 20.0f,
            .colour = Palette::Parchment,
            .horizontal = Align::Center,
            .vertical = Align::Center,
        });
    renderText(npcName);

    Rectangle keyRect = {npc.pos.x + 40.0f, npc.pos.y + 40.0f, 20.0f, 20.0f};
    DrawRectangleRounded(keyRect, 0.3f, 8, Fade(Palette::Charcoal, 0.4f));

    TextLayout interactKey = getTextLayout("Q", keyRect,
                                           {
                                               .fontSize = 16.0f,
                                               .colour = Palette::Parchment,
                                               .horizontal = Align::Center,
                                               .vertical = Align::Center,
                                           });
    renderText(interactKey);
}

void UI::renderInventory(const vector<InventoryItem> &inventory) {
    const auto &style = STYLES.inventory;
    const Rectangle containerRect = {style.margin, style.margin, style.width,
                                     style.height};
    Rectangle contentRect = getContentRect(containerRect, style.padding);
    TextLayout title = getTextLayout("Inventory", contentRect,
                                     {.fontSize = 64.0f,
                                      .colour = Palette::Brown,
                                      .horizontal = Align::Center,
                                      .vertical = Align::Start});
    float topOffset = title.height + style.topGap;

    Rectangle inventoryRect = {contentRect.x, contentRect.y + topOffset,
                               contentRect.width * style.layoutRatio,
                               contentRect.height - topOffset};
    calculateInventorySlots(inventoryRect);

    DrawRectangleRounded(containerRect, style.roundness, style.segments,
                         Fade(Palette::Charcoal, style.opacity));
    renderText(title);
    for (int i = 0; i < SLOT_COUNT; i++) {
        const Rectangle slot = slots[i];
        const Color fillColor =
            selectedSquare == i ? Palette::MossGreen : Palette::FogGrey;
        renderSlot(slot, style.slot, fillColor);

        if (i < static_cast<int>(inventory.size())) {
            renderInventoryItem(inventory[i], slot, Palette::Charcoal);
        }
    }

    if (selectedSquare >= 0 &&
        selectedSquare < static_cast<int>(inventory.size())) {
        const Rectangle detailRect = {
            contentRect.x + contentRect.width * 0.6f, contentRect.y + topOffset,
            contentRect.width * 0.4f, contentRect.height - topOffset};
        renderDetailItem(inventory[selectedSquare], detailRect);
    }
}

void UI::renderDetailItem(const InventoryItem &item,
                          const Rectangle &detailRect) {
    const auto &style = STYLES.inventory;
    const Texture2D texture = itemAssets[item.type].texture;
    float y = detailRect.y;

    // Item Name
    const string name = collectableTypeToString(item.type);
    const string text = format("{}: {}", name, item.quantity);
    TextLayout itemName = getTextLayout(text.c_str(), detailRect,
                                        {.fontSize = 32.0f,
                                         .colour = Palette::Parchment,
                                         .horizontal = Align::Center,
                                         .vertical = Align::Start});
    y += itemName.height + style.rowGap;
    TextLayout itemDescription =
        getTextLayout(collectableDescription(item.type).c_str(),
                      {detailRect.x, y, detailRect.width,
                       detailRect.height - itemName.height - style.rowGap},
                      {.fontSize = 24.0f,
                       .colour = Palette::Parchment,
                       .horizontal = Align::Center,
                       .vertical = Align::Start});
    renderText(itemName);
    renderText(itemDescription);
    y += itemDescription.height + 50.0f;

    // Image
    const float detailCenterX = detailRect.x + (detailRect.width) / 2;
    const float scale = fmin((detailRect.width - 100.0f) / texture.width,
                             (detailRect.height - 100.0f) / texture.height);
    const Vector2 size = {texture.width * scale, texture.height * scale};
    Vector2 imagePos = {detailCenterX - size.x / 2, y};

    DrawTextureEx(texture, imagePos, 0, scale, WHITE);
    y += size.y + 50.0f;

    if (item.type == CollectableType::Potion) {
        TextLayout buttonLabel = getTextLayout("Consume", consumeButton.rect,
                                               consumeButton.textStyle);
        Vector2 buttonSize = {detailRect.width - 70.0f,
                              buttonLabel.height + 15.0f * 2};
        consumeButton.rect = {detailCenterX - buttonSize.x / 2, y, buttonSize.x,
                              buttonSize.y};
        DrawRectangleRounded(consumeButton.rect, 0.2f, 8,
                             consumeButton.background);
        renderText(buttonLabel);
    }
}

void UI::calculateInventorySlots(const Rectangle &inventoryRect) {
    const auto &style = STYLES.inventory;
    const float slotGap = (fmin(inventoryRect.width, inventoryRect.height) -
                           (style.slotSize * INVENTORY_COLS)) /
                          (INVENTORY_COLS - 1);

    const float width = style.slotSize + slotGap;

    for (int i = 0; i < SLOT_COUNT; ++i) {
        const int column = i % INVENTORY_COLS, row = i / INVENTORY_COLS;
        slots[i] = {inventoryRect.x + column * width,
                    inventoryRect.y + row * width, style.slotSize,
                    style.slotSize};
    }
}

Vector2 UI::getStartPos(Rectangle bounds, TextStyle style) {
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
        pos.y = bounds.y + (bounds.height) / 2;
        break;
    case Align::End:
        pos.y = bounds.y + bounds.height;
        break;
    }
    return pos;
}

void UI::renderText(TextLayout layout) {
    const Font &font = getFont(layout.style.fontSize);
    for (const string &line : layout.lines) {
        float lineWidth =
            MeasureTextEx(font, line.c_str(), layout.style.fontSize,
                          layout.style.spacing)
                .x;
        Vector2 pos = layout.pos;

        switch (layout.style.horizontal) {
        case Align::Start:
            break;
        case Align::Center:
            pos.x -= lineWidth / 2;
            break;
        case Align::End:
            pos.x -= lineWidth;
            break;
        }
        DrawTextEx(font, line.c_str(), pos, layout.style.fontSize,
                   layout.style.spacing, layout.style.colour);
        layout.pos.y += font.baseSize * lineHeight;
    }
}

Rectangle UI::getContentRect(Rectangle rect, float padding) {
    return {rect.x + padding, rect.y + padding, rect.width - padding * 2,
            rect.height - padding * 2};
}

vector<string> UI::wrapText(const string &text, const float maxWidth,
                            TextStyle style) {
    const Font &font = getFont(style.fontSize);
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

float UI::getTextHeight(int lineCount, float fontSize) {
    if (lineCount == 0)
        return 0;
    return fontSize + (lineCount - 1) * fontSize * lineHeight;
}

const Font &UI::getFont(float fontSize) {
    return fontSize < 24.0f   ? fontSmall
           : fontSize < 48.0f ? fontMedium
                              : fontLarge;
}

TextLayout UI::getTextLayout(const string &text, const Rectangle &bounds,
                             TextStyle style) {
    vector<string> lines = wrapText(text, bounds.width, style);
    Vector2 pos = getStartPos(bounds, style);
    float height = getTextHeight(lines.size(), style.fontSize);
    return {pos, lines, height, style};
}

void UI::renderShop() {
    const auto &style = STYLES.shop;
    DrawRectangleRounded(
        {style.margin, style.margin, style.width, style.height},
        style.roundness, style.segments,
        Fade(Palette::Charcoal, style.opacity));

    Rectangle contentRect = {
        style.margin + style.padding, style.margin + style.padding,
        style.width - style.padding, style.height - style.padding};

    TextLayout heading = getTextLayout("Rowan's Shop", contentRect,
                                       {.fontSize = 64.0f,
                                        .colour = Palette::Brown,
                                        .horizontal = Align::Center,
                                        .vertical = Align::Start});
    renderText(heading);
}
