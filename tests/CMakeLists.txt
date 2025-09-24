add_executable(vanilla_engine_tests
  engine/vanilla_engine_tests.cpp
)

target_link_libraries(vanilla_engine_tests
  PRIVATE backgammon GTest::gtest_main

  )
add_test(NAME VanillaEngineTests COMMAND vanilla_engine_tests)

add_executable(vanilla_rules
  game/game_rbot_tests.cpp
)

target_link_libraries(vanilla_rules
  PRIVATE backgammon GTest::gtest_main
)

add_test(NAME vanilla_rules COMMAND vanilla_rules)


