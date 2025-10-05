#include <gtest/gtest.h>
#include "backgammon/game/game.hpp"
#include "backgammon/players/rbot.hpp"
#include "backgammon/engine/vanilla_rules_engine.hpp"

using namespace bg;

// TEST(vanilla_rules, SingleGameTerminatesWithOneWinner) {
//   VanillaRulesEngine rules;
//   RBot white{"W", 123};
//   RBot black{"B", 456};
//   Game g(rules, white, black);
//   Side winner = g.playToEnd();
//   ASSERT_TRUE(winner == bg::Side::White || winner == bg::Side::Black);
// }

// TEST(vanilla_rules, multiGameSequentialProbe) {
//   for (int i = 0; i < 100; ++i) {
//     VanillaRulesEngine rules;
//     RBot w{"W_" + std::to_string(i), 100 + i};
//     RBot b{"B_" + std::to_string(i), 200 + i};
//     Game g(rules, w, b);

//     Side winner = g.playToEnd();
//     ASSERT_TRUE(winner == bg::Side::White || winner == bg::Side::Black);
//   }
// }


// TEST(vanilla_rules, multiGameTerminatesWithMultiWInner) {
//   constexpr int n = 100; // enforces that the value really is known at compile time

//   std::vector<std::optional<Side>> winners(n); // init of 10 winners of type side
//   std::vector<std::thread> threads; // init  threads
//   threads.reserve(n); // reseving 10 threads

//   for (int i = 0; i < n; ++i) {
//     threads.emplace_back([this, i , &winners](){ // &winners is to capture the outer variable names winner by reference, while winners& reference type to winners object.
//       RBot white{"W_" + std::to_string(i), 1000 + i};
//       RBot black{"B_" + std::to_string(i), 2000 + i};

//       VanillaRulesEngine local_rules;

//       Game g(local_rules, white, black); // init independent game instance

//       winners[i] = g.playToEnd(); // run to complete and store results
//     });
//   }

//   for (auto& t : threads) t.join(); // join all threads to wait for each thread to finish before letting the object go out of scope

//   int completed = 0;
//   for (int i = 0; i < n; ++i) {
//     ASSERT_TRUE(winners[i].has_value()); // validate that each game has finished
//     ASSERT_TRUE(*winners[i] == Side::White || *winners[i] == Side::Black); // make sure that winner is ether white or black
//     ++completed;
//   }
//   ASSERT_EQ(completed, n); // making sure all games are completed
// }

