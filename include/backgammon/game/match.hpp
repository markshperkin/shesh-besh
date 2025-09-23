#pragma once
#include "backgammon/engine/enums.hpp"

namespace bg {

    class IRulesEngine;
    class IPlayer;
    class Game;

    struct GameResult {
        Side winner;
        Side loser;
    };

    class Match {
        public:
            Match(const IRulesEngine& rules, IPlayer& white, IPlayer& black);
            GameResult play_once();
        private:
            const IRulesEngine& rules_;
            IPlayer& white_;
            IPlayer& black_;
    };
}