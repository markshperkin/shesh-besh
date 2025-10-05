#include <gtest/gtest.h>
#include "backgammon/game/game.hpp"
#include "backgammon/players/rbot.hpp"
#include "backgammon/engine/vanilla_rules_engine.hpp"

using namespace bg;

TEST(gameTests, SingleGameTerminatesWithOneWinner) {
  VanillaRulesEngine rules;
  RBot white{"Wbot", "wbot", 123};
  RBot black{"Bbot", "bbot", 456};
  Game g(rules, white, black, "game id");
  WinnerLoser res = g.playToEnd();
  std::string winner = res.winner;
  ASSERT_TRUE(winner == "Wbot" || winner == "Bbot");
  std::string loser = res.loser;
  ASSERT_TRUE(loser == "Wbot" || loser == "Bbot");
}

TEST(gameTests, multiGameSequential) {
  constexpr int n = 100; // enforces that the value really is known at compile time
  std::vector<std::optional<WinnerLoser>> winners(n);
  for (int i = 0; i < n; ++i) {
    VanillaRulesEngine rules;
    RBot w{"W_" + std::to_string(i), "W_" + std::to_string(i), 100 + i};
    RBot b{"B_" + std::to_string(i), "B_" + std::to_string(i), 300 + i};
    Game g(rules, w, b, "game_" + std::to_string(i));

    winners[i] = g.playToEnd(); // run to complete and store results
  }
  int completed = 0;
  for (int i = 0; i < n; ++i) {
    ASSERT_TRUE(winners[i].has_value()); // validate that each game has finished
    ++completed;
  }
  ASSERT_EQ(completed, n); // making sure all games are completed
}


TEST(gameTests, multiGameConcurent) {
  constexpr int n = 100; // enforces that the value really is known at compile time

  std::vector<std::optional<WinnerLoser>> winners(n);
  std::vector<std::thread> threads; // init  threads
  threads.reserve(n); // reseving 10 threads

  for (int i = 0; i < n; ++i) {
    threads.emplace_back([this, i , &winners](){ // &winners is to capture the outer variable names winner by reference, while winners& reference type to winners object.
      RBot white{"W_" + std::to_string(i), "W_" + std::to_string(i), 1000 + i};
      RBot black{"B_" + std::to_string(i), "B_" + std::to_string(i), 2000 + i};

      VanillaRulesEngine local_rules;

      Game g(local_rules, white, black, "game_" + std::to_string(i)); // init independent game instance

      winners[i] = g.playToEnd(); // run to complete and store results
    });
  }

  for (auto& t : threads) t.join(); // join all threads to wait for each thread to finish before letting the object go out of scope

  int completed = 0;
  for (int i = 0; i < n; ++i) {
    ASSERT_TRUE(winners[i].has_value()); // validate that each game has finished
    ++completed;
  }
  ASSERT_EQ(completed, n); // making sure all games are completed
}

