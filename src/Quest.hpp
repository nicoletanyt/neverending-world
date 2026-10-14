#pragma once

#include <functional>
#include <string>

using namespace std;

// TODO: remove completed
enum class QuestStatus { InActive, Active, ReadyToReport, Completed };
enum class ObjectiveType { KillEnemies, FindNPC, Report, BuyItem };
enum class QuestId { ClearMonsters, FindMap };

struct QuestObjective {
    ObjectiveType type;
    string description;
    int target;
    int progress;
};

struct Quest {
    QuestId id;
    string name;
    QuestStatus status;

    vector<QuestObjective> objectives;
    int current = 0; // index of current objective

    function<void()> onCompletion;
    int npcId;
};
