#include "backgammon/game/match_maker.hpp"
#include "backgammon/game/match_session.hpp"
#include "backgammon/game/iplayer.hpp"

#include <algorithm>
#include <atomic>
#include <memory>
#include <string>

namespace bg {

    static std::atomic<unsigned long long> g_match_counter{0}; // global, thread safe 64 bit counter

    std::string MatchMaker::newMatchId() const {
        const auto n = ++g_match_counter;
        return "m_" + std::to_string(n);
    }

    // add player to queue. if player already in queue, return current player position
    std::size_t MatchMaker::enqueue(IPlayer& player) {
        const std::string pid = player.playerId();
        for (std::size_t i = 0; i < queue_.size();++i ) {
            if (queue_[i] -> playerId() == pid) return i + 1;
        }
        queue_.push_back(&player);
        return queue_.size();
    }

    // dequeue player, return true if succeful, false otherwise
    bool MatchMaker::leaveQueue(const std::string& playerId) {
        for (auto it = queue_.begin(); it != queue_.end(); ++it) {
            if ((*it) -> playerId() == playerId) {
                queue_.erase(it);
                return true;
            }
        }
        return false;
    }

    // create a match session and run it
    std::optional<GameResult> MatchMaker::tryMatchAndPlayOnce() {
        if (queue_.size() < 2) return std::nullopt; // not enough players in queue

        IPlayer* white = queue_.front(); queue_.pop_front();
        IPlayer* black = queue_.front(); queue_.pop_front();

        const std::string matchId  = newMatchId();

        auto session = std::make_unique<MatchSession>(rules_, *white, *black, matchId);
        MatchSession* sessionPTr = session.get();
        session_[matchId] = std::move(session);

        rematchVotes_.erase(matchId); // reset eny prior votes just in case

        GameResult res = sessionPTr -> play_once();
    }

    const MatchSession* MatchMaker::find(const std::string& matchid) const noexcept {
        auto it = session_.find(matchid);
        return (it == session_.end()) ? nullptr : it -> second.get();
    }

    bool MatchMaker::requestRematch(const std::string& matchId, const std::string& playerId) {
        auto& voters = rematchVotes_[matchId];
        return voters.insert(playerId).second; // true if newly inserted
    }

    bool MatchMaker::rematchPositive(const std::string& matchId) const {
        auto it = rematchVotes_.find(matchId);
        if (it == rematchVotes_.end()) return false;
        return it -> second.size() >=2;
    }
}
