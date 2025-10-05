#include "backgammon/game/match_session.hpp"
#include "backgammon/game/game.hpp"

namespace bg {

    MatchSession::MatchSession(const IRulesEngine& rules, IPlayer& white, IPlayer& black, std::string matchId)
    : rules_(rules), white_(white), black_(black), matchId_(matchId) {}

    GameResult MatchSession::play_once() {
        ++gameNo_;
        Game game(rules_, white_, black_, matchId_ + "-" + std::to_string(gameNo_));
        auto ids = game.playToEnd();
        if (ids.winner == white_.playerId()) ++scoreWhite_; else ++scoreBlack_;
        return { ids.winner, ids.loser, matchId_ };
    }

}