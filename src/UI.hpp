#pragma once

#include "Constants.hpp"
#include "Dialogue.hpp"
#include "entities/Collectable.hpp"
#include "entities/Enemy.hpp"
#include "entities/NPC.hpp"
#include <map>
#include <raylib.h>
#include <string>
#include <vector>
using namespace std;

class Player;
struct InventoryItem;
struct Quest;

enum class Align { Center, Start, End };

namespace Palette {
constexpr Color Charcoal = {37, 43, 40, 255};     // #252B28
constexpr Color Parchment = {216, 208, 184, 255}; // #D8D0B8
constexpr Color FadedParchment = {216, 208, 184, 102};
constexpr Color MossGreen = {105, 124, 104, 255}; // #697C68
constexpr Color Brown = {156, 118, 91, 255};      // #9C765B
constexpr Color MutedRed = {166, 95, 89, 255};    // #A65F59
constexpr Color FogGrey = {146, 154, 154, 255};   // #929A9A
} // namespace Palette

struct ItemIcon {
    Texture2D texture;
    float scale;
};

struct SlotStyle {
    float roundness = 0.26f;
    int segments = 0;
    float borderWidth = 4.0f;
    float fillOpacity = 0.4f;
    float borderOpacity = 0.8f;
    float imagePadding;

    Vector2 quantityOffset;
    float quantityFontSize;
};

struct PanelStyle {
    float margin = 50.0f;
    float padding = 50.0f;
    float width = WINDOW_WIDTH - margin * 2;
    float height = WINDOW_HEIGHT - margin * 2;
    float roundness = 0.1f;
    int segments = 4;
    float opacity = 0.9f;
};

struct InventoryStyle : PanelStyle {
    // layout
    float layoutRatio = 0.6f; // .6 and .4
    float gap = 50.0f;        // column gap
    float slotSize = 100.0f;

    SlotStyle slot{.imagePadding = 15.0f,
                   .quantityOffset = {8.0f, 8.0f},
                   .quantityFontSize = 24.0f};
};

struct DialogueStyle : PanelStyle {
    float avatarScale = 7.0f;
    float columnGap = 20.0f;
    float nameGap = 20.0f;
    float nameFontSize = 64.0f;
    float textFontSize = 32.0f;
    float choiceHeight = 40.0f;
    float choiceGap = 15.0f;
    float bottomPadding = 10.0f;
};

struct ShopStyle : PanelStyle {};

struct QuestStyle {
    float marginLeft = 30.0f;
    float marginTop = 100.0f;
    float padding = 12.0f;
    float height = 75.0f;
    float gap = 15.0f;
};

struct HotbarStyle {
    float slotSize = 50.0f;
    float slotGap = 20.0f;
    float rightMargin = 30.0f;
    float bottomMargin = 30.0f;
    float fontSize = 16.0f;
    SlotStyle slot{.imagePadding = 7.0f,
                   .quantityOffset = {1.0f, 1.0f},
                   .quantityFontSize = 16.0f};
};

struct TopbarStyle {
    float margin = 30.0f;
    float width = 250.0f;
    float padding = 10.0f;
    float gap = 4.0f;
    int heartCount = 5;
    float roundness = 0.3f;
    int segments = 0;
    float fillOpacity = 0.4f;
};

struct UIStyle {
    InventoryStyle inventory;
    HotbarStyle hotbar;
    TopbarStyle topbar;
    DialogueStyle dialogue;
    PanelStyle panel;
    ShopStyle shop;
    QuestStyle quest;
};

struct TextStyle {
    float fontSize = 16.0f;
    float spacing = 3.0f;
    Color colour = WHITE;
    Align horizontal = Align::Start;
    Align vertical = Align::Start;
};

struct TextLayout {
    Vector2 pos;
    vector<string> lines;
    float height;
    TextStyle style;
};

struct ButtonStyle {
    Color background = Palette::MossGreen;
    Rectangle rect;
    TextStyle textStyle = {.fontSize = 24.0f,
                           .colour = Palette::Parchment,
                           .horizontal = Align::Center,
                           .vertical = Align::Center};
};

class UI {
  private:
    inline static constexpr UIStyle STYLES{};
    const float lineHeight = 1.5f;

    static Font fontSmall;
    static Font fontMedium;
    static Font fontLarge;

    vector<Texture2D> hearts;
    map<CollectableType, ItemIcon> itemAssets;
    Texture2D profileAsset;

    Vector2 getTextPos(const string &text, Rectangle bounds, TextStyle style);
    const Font &getFont(float fontSize);
    vector<string> wrapText(const string &text, const float endX,
                            TextStyle style, float textX);
    TextLayout getTextLayout(const string &text, const Rectangle &bounds,
                             TextStyle style);
    float getTextHeight(int lineCount, float fontSize);
    void renderText(TextLayout layout);

    void renderSlot(const Rectangle &rect, const SlotStyle &style,
                    Color fillColor);
    Rectangle getHotbarSlotRect(int index) const;
    void renderInventoryItem(const InventoryItem &item, const Rectangle &slot,
                             Color colour);
    void renderDetailItem(const InventoryItem &item,
                          const Rectangle &detailRect);
    Rectangle getContentRect(Rectangle rect, float padding);
    void calculateInventorySlots(const Rectangle &inventoryRect);

  public:
    ButtonStyle consumeButton;
    int selectedSquare = -1;
    array<Rectangle, SLOT_COUNT> slots;

    void loadAssets();
    void renderTopbar(Player &player);
    void renderHotbar(const vector<InventoryItem> &inventory);
    void renderEnemyHealth(const Enemy &enemy);
    void renderNPCName(const NPC &npc);
    void renderInventory(const vector<InventoryItem> &inventory);
    void renderDialogue(const DialogueNode &node, const NPC &npc,
                        int selectedOption);
    void renderQuest(const Quest &quest);
    void renderShop();
};
