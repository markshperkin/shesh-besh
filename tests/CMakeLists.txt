add_executable(vanilla_engine_tests
  engine/vanilla_engine_tests.cpp
)
target_link_libraries(vanilla_engine_tests
  PRIVATE backgammon gtest_main
)
add_test(NAME VanillaEngineTests COMMAND vanilla_engine_tests)
