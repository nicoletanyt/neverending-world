#pragma once

#include "Constants.hpp"
#include "entities/Collectable.hpp"
#include "entities/Enemy.hpp"
#include "entities/NPC.hpp"
#include <map>
#include <raylib.h>
#include <vector>
using namespace std;

class Player;
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
    Color fillColor = GRAY;
    float fillOpacity = 0.4f;
    Color borderColor = WHITE;
    float borderOpacity = 0.8f;
};

struct InventoryStyle {
    float margin = 50.0f;
    float padding = 50.0f;
    float width = WINDOW_WIDTH - margin * 2;
    float height = WINDOW_HEIGHT - margin * 2;
    float slotSize = 60.0f;
    float slotGap = 20.0f;
    float gridTopOffset = 50.0f;
    float gap = 50.0f; // column gap
    float fontSize = 20.0f;

    Color quantityColor = WHITE;
    Color selectedSlotColor = BLUE;
    SlotStyle slot{.roundness = 0.26f, .segments = 0};
};

struct HotbarStyle {
    float slotSize = 50.0f;
    float slotGap = 20.0f;
    float rightMargin = 30.0f;
    float bottomMargin = 30.0f;
    float fontSize = 20.0f;
    Color quantityColor = RED;
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
    Color fillColor = WHITE;
    float fillOpacity = 0.4f;
    Color profileFillColor = BLACK;
    float profileFillOpacity = 0.4f;
    Color iconColor = WHITE;
};

struct UIStyle {
    InventoryStyle inventory;
    HotbarStyle hotbar;
    TopbarStyle topbar;
    Vector2 quantityOffset = {8.0f, 8.0f};
    float quantitySpacing = 2.0f;
};

struct TextStyle {
    float fontSize = 20.0f;
    float spacing = 2.0f;
    Color colour = WHITE;
    Align horizontal = Align::Center;
    Align vertical = Align::Center;
};

class UI {
private:
    inline static constexpr UIStyle STYLES{};

    vector<Texture2D> hearts;
    map<CollectableType, ItemIcon> itemAssets;
    Texture2D profileAsset;

    void renderTextInBounds(const string &text, Rectangle bounds,
                            TextStyle style);

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
    void renderNPCName(const NPC &npc, bool isClose);
    void renderInventory(const vector<InventoryItem> &inventory);

    Rectangle getInventorySlotRect(int index);
};
