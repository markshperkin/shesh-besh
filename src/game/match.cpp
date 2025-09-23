#include "backgammon/game/match.hpp"
#include "backgammon/game/game.hpp"

namespace bg {

    Match::Match(const IRulesEngine& rules, IPlayer& white, IPlayer& black)
    : rules_(rules), white_(white), black_(black) {}

    GameResult Match::play_once() {
        Game game(rules_, white_, black_);
        Side winner = game.playToEnd();
        Side loser = (winner == Side::White) ? Side::Black : Side::White;
        return {winner, loser};
    }
}