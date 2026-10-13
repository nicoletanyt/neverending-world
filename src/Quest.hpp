#pragma once

#include <functional>
#include <string>

using namespace std;

enum class QuestStatus { InActive, Active, ReadyToReport, Completed };
enum class QuestType { KillEnemies };
enum class QuestId { SandyClearMonsters };

struct Quest {
    QuestId id;
    QuestType type;
    string description;
    int target;
    int progress;
    QuestStatus status;
    function<void()> onCompletion;

    int npcId;
};
