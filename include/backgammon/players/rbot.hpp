#pragma once
#include <random>
#include <string>
#include "backgammon/game/iplayer.hpp"

namespace bg {

    class RBot : public IPlayer {
        public:
            explicit RBot(std::string playerId = "Rbot", std::string displayName = "Rbot", int seed = 123456)
                : playerId_(std::move(playerId)), displayName_(std::move(displayName)), rng_(seed) {}

            std::string playerId() override { return playerId_; }
            std::string displayName() override { return displayName_; }
            Step makeStep(const std::vector<Step>& steps) override;

        private:
            std::string playerId_;
            std::string displayName_;
            std::mt19937_64 rng_;
    };
}