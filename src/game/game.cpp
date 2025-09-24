#include "backgammon/engine/irules_engine.hpp"
#include "backgammon/game/game.hpp"
#include "backgammon/game/iplayer.hpp"
#include "backgammon/engine/dice.hpp"
#include <vector>
#include <iostream>

namespace bg {

    static const char* phaseName(bg::Phase p) {
    switch (p) {
        case bg::Phase::Normal: return "Normal";
        case bg::Phase::Re_Entry: return "Re_Entry";
        case bg::Phase::Bear_Off: return "Bear_Off";
        default: return "?";
    }
    }

    Game::Game(const IRulesEngine& rules, IPlayer& white, IPlayer& black)
        : rules_(rules), white_(white), black_(black) {}

    Side Game::playToEnd() {
        state_ = rules_.initialState();
        winner_.reset();

        OpeningResult opening = rules_.openingRoll();
        state_.sideToMove = opening.winner;
        std::vector<int> dice = opening.startingDice;
        if (dice.size() == 2 && dice[0] == dice[1]) dice = {dice[0], dice[0], dice[0], dice[0]};

        int turn = 0;
        while (true) {
            int over = rules_.isGameOver(state_);  // <- int {-1,0,1}
            if (over != 0) {
                winner_ = (over > 0) ? Side::White : Side::Black;
                return *winner_;
            }

            IPlayer& player = (state_.sideToMove == Side::White) ? white_ : black_;
            
            playOutDice(player, dice, state_.sideToMove);
            if (winner_) return *winner_;

            state_.sideToMove = (state_.sideToMove == Side::White) ? Side::Black : Side::White;
            dice = rules_.rollDice();
            if (dice[0] == dice[1]) dice = {dice[0], dice[0], dice[0], dice[0]};
            
            // if (++turn >= 400)
            //     throw std::runtime_error("turn cap hit without winner.");
        }
    }


    // helpers
    void Game::playOutDice(IPlayer& player, std::vector<int>& dice, Side side) {
        int count = 0;
        while (!dice.empty()) {
            // std::cout << count << ' ';
            count++;
            if (rules_.isGameOver(state_)) {
                winner_ = side;
                return;
            }

            std::vector<Step> steps = rules_.getAllLegalMoves(state_, dice);
            if (steps.empty()) {
                // std::cout
                // << "STM=" << (state_.sideToMove == bg::Side::White ? "W" : "B")
                // << " phaseW=" << phaseName(state_.phaseWhite)
                // << " phaseB=" << phaseName(state_.phaseBlack)
                // << " barW=" << state_.board.bar(bg::Side::White)
                // << " barB=" << state_.board.bar(bg::Side::Black)
                // << " borneW=" << state_.board.borneOff(bg::Side::White)
                // << " borneB=" << state_.board.borneOff(bg::Side::Black);
                // for (int d : dice) std::cout << " dice= " << d;
                // std::cout << "]\n";
                // for (int i = 0; i < 24; ++i) {
                //     std::cout << " " << i << ":" << state_.board.point(i);
                // }
                // std::cout << "]\n";
                break;
            }

            Step step = player.makeStep(steps);
            rules_.applyStep(state_, step);

            int used = std::get<int>(step.die);
            bg::Dice::consumeDie(dice, used);
        }
    }
}       


