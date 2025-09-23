#include "backgammon/engine/irules_engine.hpp"
#include "backgammon/game/game.hpp"
#include "backgammon/game/iplayer.hpp"
#include "backgammon/engine/dice.hpp"
#include <vector>

namespace bg {

    Game::Game(const IRulesEngine& rules, IPlayer& white, IPlayer& black)
        : rules_(rules), white_(white), black_(black) {}

    Side Game::playToEnd() {

        // game setup
        state_ = rules_.initialState();

        OpeningResult openingResult = rules_.openingRoll(); // run opening result
        state_.sideToMove = openingResult.winner; // get first player to move
        std::vector<int> dice = openingResult.startingDice; // get first dice roll

        while (!rules_.isGameOver(state_)) {
            IPlayer& player = (state_.sideToMove == Side::White) ? white_ : black_;
            playOutDice(player, dice, state_.sideToMove); // play the dice
            if (winner_) return *winner_;
            state_.sideToMove = (state_.sideToMove == Side::White) ? Side::Black : Side::White;
            dice = rules_.rollDice(); // roll the dice for next ply
            if (dice[0] == dice[1]) dice = {dice[0], dice[0], dice[0], dice[0]};
        }
        return *winner_;
    }

    // helpers
    void Game::playOutDice(IPlayer& player, std::vector<int>& dice, Side side) {
        while (!dice.empty()) {
            std::vector<Step> steps = rules_.getAllLegalMoves(state_, dice);
            if (steps.empty()) break;

            Step step = player.makeStep(steps);
            rules_.applyStep(state_, step);

            if (rules_.isGameOver(state_)) {
                winner_ = side;
                return;
            }

            int used = std::get<int>(step.die);
            bg::Dice::consumeDie(dice, used);
        }
    }
        
}



