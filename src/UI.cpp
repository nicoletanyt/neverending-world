#include "UI.hpp"
#include "Constants.hpp"
#include "Game.hpp"
#include "Quest.hpp"
#include "entities/Collectable.hpp"
#include "entities/Player.hpp"
#include <algorithm>
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

    const Rectangle container = {style.marginLeft, style.marginTop,
                                 STYLES.topbar.width, style.height};

    DrawRectangleRounded(container, 0.15f, 8, Fade(Palette::Parchment, .8f));
    DrawRectangleRoundedLinesEx(container, 0.15f, 8, 3.0f, Palette::Brown);

    const Rectangle headingRect = {container.x + style.padding, container.y + 8,
                                   container.width - style.padding * 2, 25};
    renderTextInBounds(quest.name, headingRect,
                       {.fontSize = 22.0f,
                        .colour = Palette::Charcoal,
                        .horizontal = Align::Start,
                        .vertical = Align::Center},
                       true);
    const Rectangle questRect = {container.x + style.padding, container.y + 37,
                                 container.width - style.padding * 2, 25};

    string progressText = "[" + to_string(objective.progress) + "/" +
                          to_string(objective.target) + "] " +
                          objective.description;

    // TODO: create a measureTextInBounds then load height responsively
    renderTextInBounds(progressText, questRect,
                       {.fontSize = 16.0f,
                        .colour = Palette::Brown,
                        .horizontal = Align::Start,
                        .vertical = Align::Center},
                       true);
}
void UI::renderHotbar(const vector<InventoryItem> &inventory) {
    const auto &style = STYLES.hotbar;
    for (int i = 0; i < HOTBAR_SLOTS; ++i) {
        const Rectangle slot = getHotbarSlotRect(i);
        renderSlot(slot, style.slot, Palette::FogGrey);

        if (i < static_cast<int>(inventory.size())) {
            renderInventoryItem(inventory[i], slot, style.fontSize,
                                Palette::Charcoal);
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
                             float fontSize, Color colour) {
    const auto &icon = itemAssets[item.type];
    const Texture2D texture = icon.texture;
    const float scale = icon.scale;
    const Vector2 texturePos = {
        slot.x + (slot.width - texture.width * scale) / 2,
        slot.y + (slot.height - texture.height * scale) / 2};
    const string text = to_string(item.quantity);

    DrawTextureEx(texture, texturePos, 0, scale, WHITE);
    DrawTextEx(
        fontSmall, text.c_str(),
        {slot.x + STYLES.quantityOffset.x, slot.y + STYLES.quantityOffset.y},
        fontSize, STYLES.quantitySpacing, colour);
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
    Texture2D asset = npc.avatars[npc.type - 1];
    const float scale = style.avatarScale;
    const float leftColWidth = asset.width * scale;

    DrawTextureEx(asset, {contentRect.x, contentRect.y}, 0, scale, WHITE);

    renderTextInBounds(
        npc.name,
        {contentRect.x, contentRect.y + asset.height * scale + style.nameGap,
         leftColWidth,
         contentRect.height - contentRect.y - asset.height * scale},
        {.fontSize = style.nameFontSize,
         .colour = Palette::Parchment,
         .horizontal = Align::Center});

    const Rectangle dialogueRect = {
        contentRect.x + leftColWidth + style.columnGap, contentRect.y,
        contentRect.width - leftColWidth, contentRect.height};

    renderTextInBounds(node.text, dialogueRect,
                       {
                           .fontSize = style.textFontSize,
                           .colour = Palette::Parchment,
                       },
                       true);

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

        renderTextInBounds(choiceText, choiceRect,
                           {
                               .fontSize = style.textFontSize,
                               .colour = colour,
                               .horizontal = Align::End,
                           },
                           true);
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
    renderTextInBounds(
        npc.name, (Rectangle){npc.pos.x, npc.pos.y - 15.0f, npc.size.x, 20.0f},
        (TextStyle){
            .fontSize = 20.0f,
            .colour = Palette::Parchment,
            .horizontal = Align::Center,
            .vertical = Align::Center,
        });

    Rectangle keyRect = {npc.pos.x + 40.0f, npc.pos.y + 40.0f, 20.0f, 20.0f};
    DrawRectangleRounded(keyRect, 0.3f, 8, Fade(Palette::Charcoal, 0.4f));
    renderTextInBounds("Q", keyRect,
                       (TextStyle){
                           .fontSize = 16.0f,
                           .colour = Palette::Parchment,
                           .horizontal = Align::Center,
                           .vertical = Align::Center,
                       });
}

void UI::renderInventory(const vector<InventoryItem> &inventory) {
    const auto &style = STYLES.inventory;
    DrawRectangleRounded(
        {style.margin, style.margin, style.width, style.height},
        style.roundness, style.segments,
        Fade(Palette::Charcoal, style.opacity));
    TextStyle inventoryTextStyle = {.fontSize = 64.0f,
                                    .colour = Palette::Brown,
                                    .horizontal = Align::Center,
                                    .vertical = Align::Start};

    Rectangle contentRect = {
        style.margin + style.padding, style.margin + style.padding,
        style.width - style.padding, style.height - style.padding};
    renderTextInBounds("Inventory", contentRect, inventoryTextStyle);

    for (int i = 0; i < 15; i++) {
        const Rectangle slot = getInventorySlotRect(i);
        const Color fillColor =
            selectedSquare == i ? Palette::MossGreen : Palette::FogGrey;
        renderSlot(slot, style.slot, fillColor);

        if (i < static_cast<int>(inventory.size())) {
            renderInventoryItem(inventory[i], slot, style.fontSize,
                                Palette::Charcoal);
        }
    }

    if (selectedSquare >= 0 &&
        selectedSquare < static_cast<int>(inventory.size()))
        renderDetailItem(inventory[selectedSquare], contentRect);
}

void UI::renderDetailItem(const InventoryItem &item,
                          const Rectangle &contentRect) {
    const auto &style = STYLES.inventory;
    const Texture2D texture = itemAssets[item.type].texture;
    const Rectangle lastSlot = getInventorySlotRect(INVENTORY_COLS - 1);
    const float detailX = lastSlot.x + lastSlot.width + style.gap;
    const Rectangle detailRect = {
        detailX, contentRect.y + style.gridTopOffset,
        contentRect.x + contentRect.width - detailX - style.padding,
        contentRect.height - style.gridTopOffset - style.padding};

    float y = detailRect.y;

    // Item Name
    const string name = collectableTypeToString(item.type);
    const string text = format("{}: {}", name, item.quantity);
    renderTextInBounds(text.c_str(), detailRect,
                       (TextStyle){.fontSize = 32.0f,
                                   .colour = Palette::Parchment,
                                   .horizontal = Align::Center,
                                   .vertical = Align::Start});
    y += 100.0f;

    // Image
    const float detailCenterX = detailRect.x + (detailRect.width) / 2;
    const float scale = (detailRect.width - 100.0f) / texture.width;
    const Vector2 size = {texture.width * scale, texture.height * scale};
    Vector2 imagePos = {detailCenterX - size.x / 2, y};

    DrawTextureEx(texture, imagePos, 0, scale, WHITE);
    y += size.y + 60.0f;

    if (item.type == CollectableType::Potion) {
        Vector2 buttonSize = {160.0f, 50.0f};
        Vector2 buttonPos = {detailCenterX - buttonSize.x / 2, y};

        consumeBtn = {buttonPos.x, buttonPos.y, buttonSize.x, buttonSize.y};
        DrawRectangleRounded(consumeBtn, 0.2f, 8,
                             Fade(Palette::MossGreen, 0.4f));
        renderTextInBounds("Consume", consumeBtn,
                           (TextStyle){.fontSize = 16.0f,
                                       .colour = Palette::Parchment,
                                       .horizontal = Align::Center,
                                       .vertical = Align::Center});
    }
}

Rectangle UI::getInventorySlotRect(int index) {
    const auto &style = STYLES.inventory;
    const int column = index % INVENTORY_COLS;
    const int row = index / INVENTORY_COLS;
    const float stride = style.slotSize + style.slotGap;

    return {style.margin + style.padding + column * stride,
            style.margin + style.padding + style.gridTopOffset + row * stride,
            style.slotSize, style.slotSize};
}

void UI::renderTextInBounds(const string &text, Rectangle bounds,
                            TextStyle style, bool textWrap) {
    // Midpoints between the available font sizes: 16, 32, and 64.
    const Font &font = style.fontSize < 24.0f   ? fontSmall
                       : style.fontSize < 48.0f ? fontMedium
                                                : fontLarge;
    style.fontSize = static_cast<float>(font.baseSize);
    Vector2 textPos;
    const Vector2 textSize =
        MeasureTextEx(font, text.c_str(), style.fontSize, style.spacing);

    switch (style.horizontal) {
    case Align::Start:
        textPos.x = bounds.x;
        break;
    case Align::Center:
        textPos.x = bounds.x + (bounds.width - textSize.x) / 2;
        break;
    case Align::End:
        textPos.x = bounds.x + bounds.width - textSize.x;
        break;
    }

    switch (style.vertical) {
    case Align::Start:
        textPos.y = bounds.y;
        break;
    case Align::Center:
        textPos.y = bounds.y + (bounds.height - textSize.y) / 2;
        break;
    case Align::End:
        textPos.y = bounds.y + bounds.height - textSize.y;
        break;
    }

    if (!textWrap) {
        DrawTextEx(font, text.c_str(), textPos, style.fontSize, style.spacing,
                   style.colour);
        return;
    }

    const float endX = bounds.x + bounds.width;
    const float spaceWidth =
        MeasureTextEx(font, " ", style.fontSize, style.spacing).x +
        style.spacing + 3.0f;
    Vector2 currentPos = {textPos.x, textPos.y};
    string word = "";

    for (const char &c : text) {
        if (c == ' ' || c == '\n') {
            Vector2 wordSize = MeasureTextEx(font, word.c_str(), style.fontSize,
                                             style.spacing);
            if (currentPos.x + wordSize.x > endX) {
                currentPos = {textPos.x, currentPos.y + style.fontSize * 1.5f};
            }
            DrawTextEx(font, (word + " ").c_str(), currentPos, style.fontSize,
                       style.spacing, style.colour);
            currentPos.x += wordSize.x + spaceWidth;
            word = "";
        } else {
            word += c;
        }
        if (c == '\n') {
            currentPos = {textPos.x, currentPos.y + style.fontSize * 1.5f};
        }
    }
    Vector2 wordSize =
        MeasureTextEx(font, word.c_str(), style.fontSize, style.spacing);
    if (currentPos.x + wordSize.x > endX) {
        currentPos = {textPos.x, currentPos.y + style.fontSize * 1.5f};
        DrawTextEx(font, word.c_str(), currentPos, style.fontSize,
                   style.spacing, style.colour);
    }
    DrawTextEx(font, (word + " ").c_str(), currentPos, style.fontSize,
               style.spacing, style.colour);
    currentPos.x += wordSize.x + spaceWidth;
}
