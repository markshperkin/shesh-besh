#include "backgammon/players/rbot.hpp"
#include <random>

namespace bg {

    Step RBot::makeStep(const std::vector<Step>& steps) {
        if (steps.empty()) throw std::runtime_error("RBot::makeStep called with empty steps");
        std::uniform_int_distribution<int> dist(0, steps.size() - 1);
        return steps[dist(rng_)];
    }
}