#pragma once

#include "Quest.hpp"
#include <map>
#include <string>
#include <vector>

using namespace std;

enum class DialogueAction { None, AcceptQuest, CompleteQuest };

struct DialogueChoice;

struct DialogueNode {
    string text;
    vector<DialogueChoice> choices;
};

struct DialogueChoice {
    string text;
    int nextIndex = -1;
    DialogueAction action = DialogueAction::None;
};

using Dialogue = vector<DialogueNode>;
struct QuestDialogue {
    QuestId questId;
    map<QuestStatus, Dialogue> dialogues;
};

inline DialogueNode dialogueNext(string text, int index,
                                 DialogueAction action = DialogueAction::None) {
    return {std::move(text), {{"Next", index, action}}};
}
