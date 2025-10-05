#pragma once

#include <optional>
#include <vector>
#include <string>

#include "backgammon/engine/irules_engine.hpp"
#include "backgammon/engine/enums.hpp"
#include "backgammon/engine/types.hpp"
#include "backgammon/game/iplayer.hpp"

namespace bg {
    class IRulesEngine;

    struct WinnerLoser { // return players ID for winner and loser
        std::string winner;
        std::string loser;
    };

    class Game{
        public:
            Game(const IRulesEngine& rules, IPlayer& white, IPlayer& black, std::string gameId);

            WinnerLoser playToEnd();

            const GameState& state() const noexcept { return state_; }
        
        private:
            const IRulesEngine& rules_;
            IPlayer& white_;
            IPlayer& black_;

            GameState state_{};
            std::optional<Side> winner_{};

            std::string gameId_;

            void playOutDice(IPlayer& player, std::vector<int>& dice, Side side);

    };
}
