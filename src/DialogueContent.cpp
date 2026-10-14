#include "DialogueContent.hpp"
#include "Dialogue.hpp"
#include "Quest.hpp"
#include "entities/NPC.hpp"
using namespace std;

QuestDialogue DialogueContent::makeClearMonsterDialogue() {
    Dialogue inactive{
        dialogueNext("Oh! Hello there. I'm Sandy. I look after the "
                     "library nearby… or, well, I try to.",
                     1),
        dialogueNext("There are monsters wandering around the area, "
                     "and I can't exactly fight them myself.",
                     2),

        {"Would you mind helping me clear them out? I'd really "
         "appreciate it.",
         {
             {"Sure, I'll help!", 3},
             {"What do I need to do?", 4},
             {"No thanks.", 6},
         }},
        dialogueNext("Really? Thank you! Just take care of ten of the "
                     "monsters nearby, and I should be able to move around "
                     "more safely. Please be careful out there.",
                     5),
        dialogueNext("Just take care of ten of the monsters nearby. I "
                     "would "
                     "do it myself, but I'm afraid I'm not much of a "
                     "fighter.",
                     5),
        dialogueNext("Once you're done, come back and find me!", -1,
                     DialogueAction::AcceptQuest),
        dialogueNext("Oh… that's alright. I understand. I'll just have to "
                     "manage on my own for now.",
                     7),
        dialogueNext("Do let me know if you change your mind! And be "
                     "careful "
                     "if you decide to leave. It's rather easy to lose "
                     "your "
                     "way around here.",
                     -1),
    };
    Dialogue active{
        {"Oh, you're back! How's the monster hunting going?",
         {
             {"Still working on it.", 1},
             {"Remind me what I need to do?", 2},
         }},
        dialogueNext("No rush! Well… maybe a little. Just be careful "
                     "out there, alright?",
                     -1),
        dialogueNext("Ten monsters, that's all! Come find me when "
                     "you're done. I'll be right here!",
                     -1),
    };
    Dialogue readyToReport{

        dialogueNext("You're back! And in one piece! Does that mean "
                     "you've taken care of those monsters?",
                     1),
        dialogueNext("All ten of them? That's incredible! I can finally "
                     "get back to looking after the library.",
                     2),
        dialogueNext("Here, take this as a little thank-you. "
                     "You've really helped me out!",
                     -1, DialogueAction::CompleteQuest),

    };
    Dialogue completed{
        dialogueNext("Oh, hello again! Things have been much quieter "
                     "around here thanks to you.",
                     1),
        dialogueNext("If you ever get tired of adventuring, you're "
                     "always welcome to stop by the library!",
                     -1),

    };
    return {.questId = QuestId::ClearMonsters,
            .dialogues =
                {
                    {QuestStatus::InActive, inactive},
                    {QuestStatus::Active, active},
                    {QuestStatus::ReadyToReport, readyToReport},
                    {QuestStatus::Completed, completed},
                },
            .npcId = NPCId::Sandy};
}

QuestDialogue DialogueContent::makeFindMapDialogue() {
    Dialogue inactive = {
        dialogueNext("Thank you again for helping me with "
                     "those monsters. "
                     "I can finally get back to looking after "
                     "the library.",
                     1),

        {"By the way, do you plan on exploring the forest? "
         "Or are you just passing through?",
         {
             {"I'd like to explore.", 2},
             {"Why do you ask?", 5},
             {"I'm not sure yet.", 7},
         }},
        dialogueNext("Oh! In that case, you might want to get "
                     "yourself a map. "
                     "The forest can be rather confusing, "
                     "especially when "
                     "you start venturing further out.",
                     3),
        dialogueNext(
            "I've heard of a travelling merchant named Rowan who sells them. "
            "I believe he was somewhere northeast of here, "
            "though I couldn't tell you exactly where.",
            4),
        {"Would you like to look for him?",
         {
             {"I'll go look for him.", 8},
             {"Maybe another time.", 9},
         }},
        dialogueNext("Oh! I only ask because the forest can be "
                     "rather confusing, "
                     "especially if you're planning to venture "
                     "further out.",
                     6),
        dialogueNext("I've heard of a travelling merchant "
                     "named Rowan who sells maps. "
                     "You might want to find him if you decide "
                     "to explore.",
                     4),
        dialogueNext("That's alright! There's no rush. Just "
                     "remember to be careful "
                     "out there. It's easy to lose your way "
                     "around here.",
                     -1),
        dialogueNext(
            "Great! Come back and let me know if you manage to find him. "
            "I'd love to hear what you discover!",
            -1, DialogueAction::AcceptQuest),
        dialogueNext(
            "That's alright! There's no rush. Just remember to be careful "
            "out there. It's easy to lose your way around here.",
            -1),

    };
    Dialogue active{
        // Node 0
        {"Oh, you're back! Have you managed to find Rowan yet?",
         {
             {"Not yet. I'll keep looking.", 1},
             {"Remind me what I'm looking for?", 2},
         }},

        // Node 1
        dialogueNext("No rush! Just keep an eye out for a "
                     "travelling merchant. "
                     "And do be careful out there, alright?",
                     -1),

        // Node 2
        dialogueNext("His name is Rowan. He's a travelling "
                     "merchant who sells maps. "
                     "I heard he was somewhere northeast of here, "
                     "but I'm afraid "
                     "I don't know his exact location.",
                     -1),
    };
    Dialogue readyToReport{
        // Node 0
        dialogueNext("Oh, you're back! Did you manage to find Rowan?", 1),

        // Node 1
        dialogueNext("And you got yourself a map? Wonderful! "
                     "That should make "
                     "exploring the forest a little easier.",
                     2),

        // Node 2
        {"Have you had a chance to look at it yet?",
         {
             {"Not yet. I'll take a look.", 3},
             {"Yeah, I have it with me.", 4},
         }},

        // Node 3
        dialogueNext("I hope it helps you find your way "
                     "around. Though I suppose "
                     "there's only so much a map can do for a "
                     "place like this.",
                     5),

        // Node 4
        dialogueNext("Good! I hope it helps you find your way around. "
                     "Though I suppose there's only so much a map can "
                     "do "
                     "for a place like this.",
                     5),

        // Node 5
        dialogueNext("Thank you for letting me know. Good luck with "
                     "your "
                     "explorations, and do be careful out there!",
                     -1, DialogueAction::CompleteQuest),
    };

    Dialogue completed{
        // Node 0
        {"Oh, hello again! How are your explorations going?",
         {
             {"I've been doing alright.", 1},
             {"The forest is more confusing than I expected.", 2},
         }},

        // Node 1
        dialogueNext("I'm glad to hear that! I hope the map "
                     "has been useful.",
                     3),

        // Node 2
        dialogueNext("I can imagine! Even with a map, it's easy to lose "
                     "your way around here. Just be careful, alright?",
                     3),

        // Node 3
        dialogueNext("And if you ever need a quiet place to "
                     "rest, you're "
                     "always welcome at the library.",
                     -1),

    };

    return {.questId = QuestId::FindMap,
            .dialogues =
                {
                    {QuestStatus::InActive, inactive},
                    {QuestStatus::Active, active},
                    {QuestStatus::ReadyToReport, readyToReport},
                    {QuestStatus::Completed, completed},
                },
            .npcId = NPCId::Sandy};
}
