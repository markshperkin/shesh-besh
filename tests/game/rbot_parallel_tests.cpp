#include <gtest/gtest.h>
#include "backgammon/game/game.hpp"
#include "backgammon/players/rbot.hpp"
#include "backgammon/engine/vanilla_rules_engine.hpp"

using namespace bg;

class GameSmoke : public ::testing::Test {
protected:
  bg::VanillaRulesEngine rules;
  bg::RBot white{"W", 123};
  bg::RBot black{"B", 456};
};

TEST_F(GameSmoke, SingleGameTerminatesWithOneWinner) {
  bg::Game g(rules, white, black);
  bg::Side winner = g.playToEnd();
  ASSERT_TRUE(winner == bg::Side::White || winner == bg::Side::Black);
}

