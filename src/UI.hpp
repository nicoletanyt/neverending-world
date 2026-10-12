#pragma once

#include "Constants.hpp"
#include "entities/Collectable.hpp"
#include "entities/Enemy.hpp"
#include "entities/NPC.hpp"
#include <map>
#include <raylib.h>
#include <string>
#include <vector>
using namespace std;

class Player;
struct Dialogue;
struct InventoryItem;

enum class Align { Center, Start, End };

struct ItemIcon {
    Texture2D texture;
    float scale;
};

struct SlotStyle {
    float roundness = 0.2f;
    int segments = 8;
    float borderWidth = 4.0f;
    float fillOpacity = 0.4f;
    float borderOpacity = 0.8f;
};

struct PanelStyle {
    float margin = 50.0f;
    float padding = 50.0f;
    float width = WINDOW_WIDTH - margin * 2;
    float height = WINDOW_HEIGHT - margin * 2;
};

struct InventoryStyle : PanelStyle {
    float slotSize = 60.0f;
    float slotGap = 20.0f;
    float gridTopOffset = 100.0f;
    float gap = 50.0f; // column gap
    float fontSize = 20.0f;

    SlotStyle slot{.roundness = 0.26f, .segments = 0};
};

struct DialogStyle : PanelStyle {};

struct HotbarStyle {
    float slotSize = 50.0f;
    float slotGap = 20.0f;
    float rightMargin = 30.0f;
    float bottomMargin = 30.0f;
    float fontSize = 20.0f;
    SlotStyle slot{.roundness = 0.26f, .segments = 0};
};

struct TopbarStyle {
    float margin = 30.0f;
    float padding = 10.0f;
    float profileRadius = 30.0f;
    float gap = 10.0f;
    float profileScale = 1.5f;
    float heartSpacing = 32.0f;
    int heartCount = 5;
    float roundness = 0.3f;
    int segments = 0;
    float fillOpacity = 0.4f;
    float profileFillOpacity = 0.4f;
    Color iconColor = WHITE;
};

struct UIStyle {
    InventoryStyle inventory;
    HotbarStyle hotbar;
    TopbarStyle topbar;
    DialogStyle dialog;
    PanelStyle panel;
    Vector2 quantityOffset = {8.0f, 8.0f};
    float quantitySpacing = 2.0f;
};

struct TextStyle {
    float fontSize = 20.0f;
    float spacing = 2.0f;
    Color colour = WHITE;
    Align horizontal = Align::Start;
    Align vertical = Align::Start;
};

namespace Palette {
constexpr Color Background = {37, 43, 40, 255};   // #252B28
constexpr Color Text = {216, 208, 184, 255};      // #D8D0B8
constexpr Color MossGreen = {105, 124, 104, 255}; // #697C68
constexpr Color Brown = {156, 118, 91, 255};      // #9C765B
constexpr Color MutedRed = {166, 95, 89, 255};    // #A65F59
constexpr Color FogGrey = {146, 154, 154, 255};   // #929A9A
} // namespace Palette

class UI {
  private:
    inline static constexpr UIStyle STYLES{};

    static Font font;

    vector<Texture2D> hearts;
    map<CollectableType, ItemIcon> itemAssets;
    Texture2D profileAsset;

    void renderTextInBounds(const string &text, Rectangle bounds,
                            TextStyle style, bool textWrap = false);

    void renderSlot(const Rectangle &rect, const SlotStyle &style,
                    Color fillColor);
    Rectangle getHotbarSlotRect(int index) const;
    void renderInventoryItem(const InventoryItem &item, const Rectangle &slot,
                             float fontSize, Color colour);
    void renderDetailItem(const InventoryItem &item,
                          const Rectangle &contentRect);

  public:
    Rectangle consumeBtn;
    int selectedSquare = -1;

    void loadAssets();
    void renderTopbar(Player &player);
    void renderHotbar(const vector<InventoryItem> &inventory);
    void renderEnemyHealth(const Enemy &enemy);
    void renderNPCName(const NPC &npc);
    void renderInventory(const vector<InventoryItem> &inventory);
    void renderDialogue(const Dialogue &dialogue, const NPC &npc, int lineIndex,
                        int selectedOption);

    Rectangle getInventorySlotRect(int index);
};
