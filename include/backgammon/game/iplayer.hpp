#pragma once
#include <string>
#include <vector>
#include "backgammon/engine/step.hpp"

namespace bg {
    struct IPlayer {
        virtual ~IPlayer() = default;
        virtual std::string name() = 0;
        virtual Step makeStep(const std::vector<Step>& steps) = 0;
    };

}