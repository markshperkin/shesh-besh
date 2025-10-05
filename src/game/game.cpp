#include "backgammon/engine/irules_engine.hpp"
#include "backgammon/game/game.hpp"
#include "backgammon/game/iplayer.hpp"
#include "backgammon/engine/dice.hpp"
#include <vector>
#include <string>
#include <utility>


namespace bg {

    Game::Game(const IRulesEngine& rules, IPlayer& white, IPlayer& black, std::string gameId)
        : rules_(rules), white_(white), black_(black), gameId_(gameId) {}

    WinnerLoser Game::playToEnd() {
        state_ = rules_.initialState();
        winner_.reset();

        OpeningResult opening = rules_.openingRoll();
        state_.sideToMove = opening.winner;
        std::vector<int> dice = opening.startingDice;
        if (dice.size() == 2 && dice[0] == dice[1]) dice = {dice[0], dice[0], dice[0], dice[0]};

        while (true) {
            int over = rules_.isGameOver(state_);  // <- int {-1 black, 0 not game over, 1 white}
            if (over != 0) {
                const bool whiteWon = (over > 0);
                std::string winnerId = whiteWon ? white_.playerId() : black_.playerId();
                std::string loserId  = whiteWon ? black_.playerId() : white_.playerId();
                return { std::move(winnerId), std::move(loserId) };
            }

            IPlayer& player = (state_.sideToMove == Side::White) ? white_ : black_;
            
            playOutDice(player, dice, state_.sideToMove);

            if (winner_) {
                const bool whiteWon = (*winner_ == Side::White);
                std::string winnerId = whiteWon ? white_.playerId() : black_.playerId();
                std::string loserId  = whiteWon ? black_.playerId() : white_.playerId();
                return { std::move(winnerId), std::move(loserId) };
            }

            state_.sideToMove = (state_.sideToMove == Side::White) ? Side::Black : Side::White;
            dice = rules_.rollDice();
            if (dice[0] == dice[1]) dice = {dice[0], dice[0], dice[0], dice[0]};
        }
    }


    // helpers
    void Game::playOutDice(IPlayer& player, std::vector<int>& dice, Side side) {
        while (!dice.empty()) {
            if (rules_.isGameOver(state_)) {
                winner_ = side;
                return;
            }

            std::vector<Step> steps = rules_.getAllLegalMoves(state_, dice);
            if (steps.empty()) {
                break;
            }

            Step step = player.makeStep(steps);
            rules_.applyStep(state_, step);

            int used = std::get<int>(step.die);
            bg::Dice::consumeDie(dice, used);
        }
    }
}       


