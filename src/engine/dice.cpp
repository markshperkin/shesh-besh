#include "backgammon/engine/dice.hpp"

namespace bg {

    Dice::Dice() {
        std::random_device rd;
        rng.seed(rd());
    }

    int Dice::rollDie(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }

    void Dice::consumeDie(std::vector<int>& dice, int used) {
        for (int i = 0; i < dice.size(); ++i) {
            if (dice[i] == used) {
                dice[i] = dice.back();
                dice.pop_back();
                return;
            }
        }
    }

    std::vector<int> Dice::rollDice(int min, int max) {
        return { rollDie(min, max), rollDie(min, max) };
    }
}