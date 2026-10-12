#include "Animation.hpp"

using namespace std;

optional<AnimationType> stringToAnimationType(const string &value) {
    if (value == "walk")
        return AnimationType::Walk;
    if (value == "hurt")
        return AnimationType::Hurt;
    if (value == "fight")
        return AnimationType::Fight;
    if (value == "")
        return AnimationType::None;
    return nullopt;
}

string animationTypeToString(AnimationType type) {
    switch (type) {
    case AnimationType::Walk: {
        return "walk";
    }
    case AnimationType::Hurt: {
        return "hurt";
    }
    case AnimationType::Fight: {
        return "fight";
    }
    case AnimationType::None: {
        return "";
    }
    }
    return "";
}
