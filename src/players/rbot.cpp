#include "backgammon/players/rbot.hpp"
#include <random>

namespace bg {

    Step RBot::makeStep(const std::vector<Step>& steps) {
        if (steps.empty()) throw std::logic_error("no steps");
        if (steps.size() == 1) return steps[0];
        std::uniform_int_distribution<size_t> dist(0, steps.size() - 1);
        return steps[dist(rng_)];
    }
}