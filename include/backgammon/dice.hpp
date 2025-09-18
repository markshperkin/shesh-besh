#pragma once

#include <random>
#include <utility>

namespace bg {

    class Dice {

    public:
        Dice(); // constructor
        int rollDie(int min, int max);
        std::pair<int, int> rollDice(int min, int max);
    
    private:
        std::mt19937 rng; // random number generator. better than rand()
    };
}