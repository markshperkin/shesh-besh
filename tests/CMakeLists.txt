add_executable(vanilla_engine_tests
  engine/vanilla_engine_tests.cpp
)

target_link_libraries(vanilla_engine_tests
  PRIVATE backgammon GTest::gtest_main

  )
add_test(NAME VanillaEngineTests COMMAND vanilla_engine_tests)

add_executable(rbot_parallel_tests
  game/rbot_parallel_tests.cpp
)

target_link_libraries(rbot_parallel_tests
  PRIVATE backgammon GTest::gtest_main
)

add_test(NAME rbot_parallel_tests COMMAND rbot_parallel_tests)


