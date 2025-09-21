add_executable(vanilla_engine_tests
  engine/vanilla_engine_tests.cpp
  engine/board_tests.cpp
)

target_link_libraries(vanilla_engine_tests
  PRIVATE backgammon GTest::gtest_main
)

add_test(NAME VanillaEngineTests COMMAND vanilla_engine_tests)
