#pragma once
#include "backgammon/engine/enums.hpp"
#include <string>

namespace bg {

    class IRulesEngine;
    class IPlayer;
    class Game;

    struct GameResult {
        std::string winnerId;
        std::string loserId;
        std::string matchId;
    };

    struct Scores {
        int white;
        int black;
    };

    class MatchSession {
        public:
            MatchSession(const IRulesEngine& rules, IPlayer& white, IPlayer& black, std::string matchId);
            GameResult play_once();
            Scores scores() const noexcept { return {scoreWhite_, scoreBlack_}; }
        private:
            const IRulesEngine& rules_;
            IPlayer& white_;
            IPlayer& black_;
            std::string matchId_;

            int gameNo_ = 0;
            int scoreWhite_ = 0;
            int scoreBlack_ = 0;
    };
}