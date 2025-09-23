#pragma once

#include <random>
#include <utility>
#include <vector>

namespace bg {

    class Dice {

    public:
        Dice(); // constructor
        int rollDie(int min, int max);
        std::vector<int> rollDice(int min, int max);
        static void consumeDie(std::vector<int>& dice, int used);
    private:
        std::mt19937 rng; // random number generator. better than rand()
    };
}