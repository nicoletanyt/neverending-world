#pragma once

#include "Constants.hpp"
#include "Dialogue.hpp"
#include "entities/Collectable.hpp"
#include "entities/Enemy.hpp"
#include "entities/NPC.hpp"
#include <map>
#include <raylib.h>
#include <set>
#include <string>
#include <vector>
using namespace std;

class Player;
struct InventoryItem;
struct ShopItem;
struct Quest;

enum class Align { Center, Start, End };

namespace Palette {
constexpr Color Charcoal = {37, 43, 40, 255};          // #252B28
constexpr Color Parchment = {216, 208, 184, 255};      // #D8D0B8
constexpr Color FadedParchment = {216, 208, 184, 191}; // 75% opacity
constexpr Color MossGreen = {105, 124, 104, 255};      // #697C68
constexpr Color Brown = {156, 118, 91, 255};           // #9C765B
constexpr Color MutedBrown = {150, 124, 101, 255};     // #967C65
constexpr Color MutedRed = {166, 95, 89, 255};         // #A65F59
constexpr Color FogGrey = {146, 154, 154, 255};        // #929A9A
} // namespace Palette

namespace FontSize {
constexpr float ExtraSmall = 12;
constexpr float Small = 16;
constexpr float Medium = 24;
constexpr float Large = 40;
} // namespace FontSize

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
    float size;
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
    float opacity = 0.98f;
};

struct MapStyle : PanelStyle {
    float sectionGap = 40.0f;
    float lineWidthOffset = 10.0f;
    float lineOffsetY = 10.0f;

    float cellSize = 180.0f;
    float cellGap = 2.0f;
};

struct InventoryStyle : PanelStyle {
    // layout
    float layoutRatio = 0.58f; // .6 and .4
    float topGap = 50.0f;
    float rowGap = 10.0f;
    float columnGap = 30.0f;

    SlotStyle slot{
        .borderWidth = 2.0f,
        .size = 80.0f,
        .imagePadding = 15.0f,
        .quantityOffset = {8.0f, 8.0f},
        .quantityFontSize = FontSize::Medium,
    };
};

struct DialogueStyle : PanelStyle {
    float layoutRatio = 0.4f;
    float avatarPadding = 10.0f;
    float rowGap = 100.0f;

    float columnGap = 50.0f;
    float nameGap = 50.0f;
    float choiceGap = 15.0f;
};

struct ShopStyle : PanelStyle {};

struct QuestStyle {
    float marginLeft = 30.0f;
    float marginTop = 115.0f;
    float padding = 12.0f;
    float height = 75.0f;
    float gap = 15.0f;
};

struct HotbarStyle {
    float slotGap = 20.0f;
    float rightMargin = 30.0f;
    float bottomMargin = 30.0f;
    SlotStyle slot{.size = 50.0f,
                   .imagePadding = 4,
                   .quantityOffset = {7, 4},
                   .quantityFontSize = FontSize::Medium};
};

struct TopbarStyle {
    float margin = 30.0f;
    float width = 300.0f;
    float height = 70.0f;
    float padding = 10.0f;
    float heartSize = 35.0f;
    float heartGap = 10.0f;
    int heartCount = 5;
    float roundness = 0.3f;
    int segments = 0;
    float fillOpacity = 0.6f;
};

struct UIStyle {
    InventoryStyle inventory;
    HotbarStyle hotbar;
    TopbarStyle topbar;
    DialogueStyle dialogue;
    PanelStyle panel;
    ShopStyle shop;
    QuestStyle quest;
    MapStyle map;
};

struct TextStyle {
    float fontSize = FontSize::Small;
    float spacing = 1.5f;
    Color colour = WHITE;
    Align horizontal = Align::Start;
    Align vertical = Align::Start;
};

struct TextLayout {
    Vector2 pos;
    vector<string> lines;
    Vector2 size;
    TextStyle style;
};

class UI {
  private:
    inline static constexpr UIStyle STYLES{};
    static map<float, Font> fonts;

    vector<Texture2D> hearts;
    map<CollectableType, ItemIcon> itemAssets;
    Texture2D profileAsset;

    Vector2 getStartPos(Rectangle bounds, TextStyle style, float textHeight);
    vector<string> wrapText(const string &text, const float maxWidth,
                            TextStyle style);
    TextLayout getTextLayout(const string &text, const Rectangle &bounds,
                             TextStyle style);
    float getTextHeight(int lineCount, float fontSize, float lineHeight = 1.0f);
    void renderText(const TextLayout &layout, float lineHeight = 1.0f);

    void renderSlot(const Rectangle &rect, const SlotStyle &style,
                    Color fillColor, bool selected = false);
    Rectangle getHotbarSlotRect(int index) const;
    void renderInventoryItem(const InventoryItem &item, const Rectangle &slot,
                             Color colour, const SlotStyle slotStyle);
    Rectangle getContentRect(Rectangle rect, float padding);
    void calculateItemSlots(const Rectangle &inventoryRect);
    void drawTextureInSquare(const Texture2D &texture, Vector2 center,
                             float size);

    template <typename T>
    Rectangle renderItemGrid(const vector<T> &items, const string &heading);
    Rectangle renderItemDetails(const InventoryItem &item,
                                const Rectangle &detailRect);
    void renderDetailItem(const InventoryItem &item,
                          const Rectangle &detailRect);
    void renderDetailItem(const ShopItem &shopItem,
                          const Rectangle &detailRect);

  public:
    int selectedSquare = -1;
    array<Rectangle, SLOT_COUNT> slots;

    void loadAssets();
    void renderTopbar(Player &player);
    void renderHotbar(const vector<InventoryItem> &inventory,
                      int selectedHotbar);
    void renderEnemyHealth(const Enemy &enemy);
    void renderNPCName(const NPC &npc);
    void renderInventory(const vector<InventoryItem> &inventory);
    void renderDialogue(const DialogueNode &node, const NPC &npc,
                        int selectedOption);
    void renderQuest(const Quest &quest);
    void renderShop(vector<ShopItem> shops);

    void renderMap(const map<Location, vector<Tree>> &trees,
                   const map<Location, vector<Enemy>> &enemies,
                   const map<Location, vector<NPC>> &npcs,
                   const set<Location> &visited, const Location &location,
                   Vector2 mapOffset);
};
