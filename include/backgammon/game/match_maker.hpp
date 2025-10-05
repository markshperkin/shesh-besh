#pragma once

#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "backgammon/game/match_session.hpp"

namespace bg {

    class IRulesEngine;
    class IPlayer;

    // single lobby, FIFO matchmaker
    class MatchMaker {
        public:
            explicit MatchMaker(const IRulesEngine& rules) : rules_(rules) {}

            // queue management
            std::size_t enqueue(IPlayer& player);
            bool leaveQueue(const std::string& playerId);
            std::size_t queueSize() const noexcept { return queue_.size(); }

            // pair two players and start one game
            // returns gameresult if match was created and played, nullopt otherwise
            std::optional<GameResult> tryMatchAndPlayOnce();

            // session lookup (to play more than one game in session)
            MatchSession* find(const std::string& matchId) noexcept;
            const MatchSession* find(const std::string& matchId) const noexcept;

            // rematch consent
            bool requestRematch(const std::string& matchId, const std::string& playerId);
            bool rematchPositive(const std::string& matchId) const;

        private:
            const IRulesEngine& rules_;
            
            // FIFO loby
            std::deque<IPlayer*> queue_;

            // active sessions keyed by matchId
            std::unordered_map<std::string, std::unique_ptr<MatchSession>> session_;

            // rematch votes per matchID (set of playerIds who voted yes)
            std::unordered_map<std::string, std::unordered_set<std::string>> rematchVotes_;

            // helpers
            std::string newMatchId() const;
    };
}