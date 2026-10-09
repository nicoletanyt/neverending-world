#include "UI.hpp"
#include "Constants.hpp"
#include "Game.hpp"
#include "entities/Collectable.hpp"
#include "entities/Player.hpp"
#include <algorithm>
#include <format>
#include <string>

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
}

void UI::renderTopbar(Player &player) {
    const auto &style = STYLES.topbar;
    const float heartWidth =
        (style.heartCount - 1) * style.heartSpacing + hearts[0].width;
    const Rectangle topbar = {style.margin, style.margin,
                              style.padding * 2 + style.profileRadius * 2 +
                                  style.gap + heartWidth,
                              style.padding * 2 + style.profileRadius * 2};
    DrawRectangleRounded(topbar, style.roundness, style.segments,
                         Fade(style.fillColor, style.fillOpacity));

    const Vector2 center = {topbar.x + style.padding + style.profileRadius,
                            topbar.y + topbar.height / 2.0f};
    DrawCircleV(center, style.profileRadius,
                Fade(style.profileFillColor, style.profileFillOpacity));
    const Vector2 profilePos = {
        center.x - profileAsset.width * style.profileScale / 2.0f,
        center.y - profileAsset.height * style.profileScale / 2.0f};
    DrawTextureEx(profileAsset, profilePos, 0, style.profileScale,
                  style.iconColor);

    int filled = player.health / 10;
    for (int i = 0; i < style.heartCount; i++) {
        int heartIndex = min(filled, 2);
        const Vector2 heartPos = {center.x + style.profileRadius + style.gap +
                                      i * style.heartSpacing,
                                  center.y - hearts[heartIndex].height / 2.0f};
        DrawTextureV(hearts[heartIndex], heartPos, style.iconColor);
        filled = max(0, filled - 2);
    }
}

void UI::renderHotbar(const vector<InventoryItem> &inventory) {
    const auto &style = STYLES.hotbar;
    for (int i = 0; i < HOTBAR_SLOTS; ++i) {
        const Rectangle slot = getHotbarSlotRect(i);
        renderSlot(slot, style.slot, style.slot.fillColor);

        if (i < static_cast<int>(inventory.size())) {
            renderInventoryItem(inventory[i], slot, style.fontSize,
                                style.quantityColor);
        }
    }
}

void UI::renderSlot(const Rectangle &rect, const SlotStyle &style,
                    Color fillColor) {
    DrawRectangleRounded(rect, style.roundness, style.segments,
                         Fade(fillColor, style.fillOpacity));
    DrawRectangleRoundedLinesEx(rect, style.roundness, style.segments,
                                style.borderWidth,
                                Fade(style.borderColor, style.borderOpacity));
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
        GetFontDefault(), text.c_str(),
        {slot.x + STYLES.quantityOffset.x, slot.y + STYLES.quantityOffset.y},
        fontSize, STYLES.quantitySpacing, colour);
}

void UI::renderEnemyHealth(const Enemy &enemy) {
    float padding = 1.0f;
    Vector2 size = {50.0f, 6.0f};
    const Vector2 offset = {7.0f, 2.5f};

    Rectangle outer = {enemy.pos.x + offset.x, enemy.pos.y - offset.y, size.x,
                       size.y};
    DrawRectangleRounded(outer, 0.2f, 4, Fade(WHITE, 0.8f));

    size.x = (enemy.health / enemy.maxHealth) * 50.0f;
    Rectangle inner = {enemy.pos.x + offset.x + padding,
                       enemy.pos.y - offset.y + padding, size.x - padding * 2,
                       size.y - padding * 2};
    DrawRectangleRounded(inner, 0.2f, 4, RED);
}

void UI::renderNPCName(const NPC &npc, bool isClose) {
    renderTextInBounds(
        npc.name, (Rectangle){npc.pos.x, npc.pos.y - 10.0f, npc.size.x, 20.0f},
        (TextStyle){
            .fontSize = 15.0f,
            .colour = WHITE,
            .horizontal = Align::Center,
            .vertical = Align::Center,
        });

    if (isClose) {
        Rectangle keyRect = {npc.pos.x + 30.0f, npc.pos.y + 40.0f, 20.0f,
                             20.0f};
        DrawRectangleRounded(keyRect, 0.3f, 8, Fade(BLACK, 0.4f));
        renderTextInBounds("Q", keyRect,
                           (TextStyle){
                               .fontSize = 10.0f,
                               .colour = WHITE,
                               .horizontal = Align::Center,
                               .vertical = Align::Center,
                           });
    }
}

void UI::renderInventory(const vector<InventoryItem> &inventory) {
    const auto &style = STYLES.inventory;
    DrawRectangleRounded(
        {style.margin, style.margin, style.width, style.height}, 0.1f, 4,
        Fade(BLACK, 0.8f));
    TextStyle inventoryTextStyle = {.fontSize = 30.0f,
                                    .colour = WHITE,
                                    .horizontal = Align::Center,
                                    .vertical = Align::Start};

    Rectangle contentRect = {
        style.margin + style.padding, style.margin + style.padding,
        style.width - style.padding, style.height - style.padding};
    renderTextInBounds("Inventory", contentRect, inventoryTextStyle);

    for (int i = 0; i < 15; i++) {
        const Rectangle slot = getInventorySlotRect(i);
        const Color fillColor = selectedSquare == i ? style.selectedSlotColor
                                                    : style.slot.fillColor;
        renderSlot(slot, style.slot, fillColor);

        if (i < static_cast<int>(inventory.size())) {
            renderInventoryItem(inventory[i], slot, style.fontSize,
                                style.quantityColor);
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
                       (TextStyle){.fontSize = 20.0f,
                                   .colour = WHITE,
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
        DrawRectangleRounded(consumeBtn, 0.2f, 8, Fade(GRAY, 0.4f));
        renderTextInBounds("Consume", consumeBtn,
                           (TextStyle){.fontSize = 20.0f,
                                       .colour = WHITE,
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
                            TextStyle style) {
    const Vector2 textSize = MeasureTextEx(GetFontDefault(), text.c_str(),
                                           style.fontSize, style.spacing);
    Vector2 textPos;

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

    DrawTextEx(GetFontDefault(), text.c_str(), textPos, style.fontSize,
               style.spacing, style.colour);
}
