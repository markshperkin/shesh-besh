#pragma once

#include <optional>
#include <vector>
#include "backgammon/engine/irules_engine.hpp"
#include "backgammon/engine/enums.hpp"
#include "backgammon/engine/types.hpp"
#include "iplayer.hpp"

namespace bg {
    class IRulesEngine;
    class IPlayer;

    class Game{
        public:
            Game(const IRulesEngine& rules, IPlayer& white, IPlayer& black);

            Side playToEnd();

            const GameState& state() const noexcept { return state_; }
        
        private:
            const IRulesEngine& rules_;
            IPlayer& white_;
            IPlayer& black_;

            GameState state_{};
            std::optional<Side> winner_{};

            void playOutDice(IPlayer& player, std::vector<int>& dice, Side side);

    };
}
