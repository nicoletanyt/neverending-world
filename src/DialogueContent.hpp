#pragma once
#include "Dialogue.hpp"

struct DialogueContent {
    static QuestDialogue makeClearMonsterDialogue();
    static QuestDialogue makeFindMapDialogue();
    static Dialogue makeShopDialogue();
};
