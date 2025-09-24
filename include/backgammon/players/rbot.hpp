#pragma once
#include <random>
#include <string>
#include "backgammon/game/iplayer.hpp"

namespace bg {

    class RBot : public IPlayer {
        public:
            explicit RBot(std::string name = "Rbot", int seed = 123456)
                : name_(std::move(name)), rng_(seed) {}

            std::string name() override { return name_; }
            Step makeStep(const std::vector<Step>& steps) override;

        private:
            std::string name_;
            std::mt19937_64 rng_;
    };
}