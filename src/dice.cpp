#include "../include/backgammon/dice.hpp"

namespace bg {

    Dice::Dice() {
        std::random_device rd;
        rng.seed(rd());
    }

    int Dice::rollDie(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
    }

    std::pair<int,int> Dice::rollDice(int min, int max) {
        return { rollDie(min, max), rollDie(min, max) };
    }
}