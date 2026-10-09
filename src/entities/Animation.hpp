#pragma once

#include <optional>
#include <string>

enum class AnimationType { Walk, Hurt, Fight, None };

std::optional<AnimationType> stringToAnimationType(const std::string &value);
std::string animationTypeToString(AnimationType type);
