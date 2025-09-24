#include <gtest/gtest.h>
#include "backgammon/engine/vanilla_rules_engine.hpp"
#include <set>
#include <string>


using namespace bg;

// helper: return a set of unique string keys representing the moves
static std::set<std::string> uniqueMoveKeys(const std::vector<Step>& moves) {
    auto squareKey = [](const Step::s& sq) -> std::string {
        if (Step::isPoint(sq))    return "P" + std::to_string(std::get<int>(sq));
        if (Step::isBar(sq))      return "B";
        return "O"; // BearOff
    };

    std::set<std::string> keys;
    for (const auto& st : moves) {
        keys.insert(squareKey(st.from) + "->" + squareKey(st.to));
    }
    return keys;
}

class VanillaEngineTest : public ::testing::Test {
protected:
    // runs before each TEST_F
    void SetUp() override {
        state_ = engine_.initialState();
    }
    // runs after each TEST_F
    void TearDown() override {
        // cleanup
    }

    VanillaRulesEngine engine_; // the object under test
    GameState state_;
};

// --- test cases

TEST_F(VanillaEngineTest, rollDiceTest) {
    for (int i = 0; i < 1000; ++i) {
        std::vector<int> dice = engine_.rollDice();
        EXPECT_GE(dice[0], 1);
        EXPECT_GE(dice[1], 1);
        EXPECT_LE(dice[0], 6);
        EXPECT_LE(dice[1], 6);
    }
}

// init
TEST_F(VanillaEngineTest, InitializesCorrectly) {
    EXPECT_FALSE(state_.winner.has_value());

    // bar and borne-off checkers start at 0
    EXPECT_EQ(state_.board.bar(Side::White), 0);
    EXPECT_EQ(state_.board.bar(Side::Black), 0);
    EXPECT_EQ(state_.board.borneOff(Side::White), 0);
    EXPECT_EQ(state_.board.borneOff(Side::Black), 0);

    // total checkers per side should be 15
    int whiteTotal = 0;
    int blackTotal = 0;
    for (int i = 0; i < 24; ++i) {
        int n = state_.board.point(i);
        if (n > 0) whiteTotal += n;
        if (n < 0) blackTotal += -n;
    }
    EXPECT_EQ(whiteTotal, 15);
    EXPECT_EQ(blackTotal, 15);

    EXPECT_EQ(state_.board.point(23), 2);
    EXPECT_EQ(state_.board.point(12), 5);
    EXPECT_EQ(state_.board.point(7), 3);
    EXPECT_EQ(state_.board.point(5), 5);

    EXPECT_EQ(state_.board.point(0), -2); 
    EXPECT_EQ(state_.board.point(11), -5);
    EXPECT_EQ(state_.board.point(16), -3);
    EXPECT_EQ(state_.board.point(18), -5);
}


// opening roll logic
TEST_F(VanillaEngineTest, OpeningRollChoosesSide) {
    OpeningResult r = engine_.openingRoll();

    EXPECT_NE(r.startingDice[0], r.startingDice[1]);
    if (r.startingDice[0] > r.startingDice[1]) EXPECT_EQ(r.winner, Side::White);
    else EXPECT_EQ(r.winner, Side::Black);
}

// --- tests for normal moves
TEST_F(VanillaEngineTest, GeneratesLegalMoves_normal) {
    // test for Side = White
    state_.sideToMove = Side::White;
    state_.phaseWhite = Phase::Normal;
    state_.phaseBlack = Phase::Normal;

    std::vector<int> dice = {1,1};
    std::vector<Step> moves = engine_.getAllLegalMoves(state_, dice);
    std::set<std::string> unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());
     

    dice = {1,2};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());

    state_.board.clear();
    state_.board.setPoint(6, +15);
    state_.board.setPoint(5, -5); 
    state_.board.setPoint(4, -5); 
    state_.board.setPoint(0, -5); 
    moves = engine_.getAllLegalMoves(state_, dice);
    EXPECT_TRUE(moves.empty()); 

    dice = {1,1};
    moves = engine_.getAllLegalMoves(state_, dice);
    EXPECT_TRUE(moves.empty()); 

    // test for Side = Black
    state_ = engine_.initialState();
    state_.sideToMove = Side::Black;

    dice = {1,1};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());
     

    dice = {1,2};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());

    state_.board.clear();
    state_.board.setPoint(6, -15);
    state_.board.setPoint(7, +5); 
    state_.board.setPoint(8, +5); 
    state_.board.setPoint(9, +5); 
    moves = engine_.getAllLegalMoves(state_, dice);
    EXPECT_TRUE(moves.empty()); 

    dice = {1,1};
    moves = engine_.getAllLegalMoves(state_, dice);
    EXPECT_TRUE(moves.empty()); 
}

// --- tests for re entry moves
TEST_F(VanillaEngineTest, GeneratesLegalMoves_re_entry) {
    // test for Side = White
    state_.sideToMove = Side::White;
    state_.phaseWhite = Phase::Re_Entry;
    state_.phaseBlack = Phase::Re_Entry;
    state_.board.clear();
    std::vector<int> dice = {6, 6};

    state_.board.setBar(Side::White, 14);
    state_.board.setPoint(23, 1);
    state_.board.setPoint(20, -5);
    state_.board.setPoint(19, -5);
    state_.board.setPoint(18, -5);

    std::vector<Step> moves = engine_.getAllLegalMoves(state_, dice);
    EXPECT_TRUE(moves.empty()); 

    dice = {4, 6};
    moves = engine_.getAllLegalMoves(state_, dice);
    EXPECT_TRUE(moves.empty()); 

    dice = {1, 3};
    moves = engine_.getAllLegalMoves(state_, dice);
    std::set<std::string>unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());

    dice = {1, 1};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());
     

    // test for Side = Black
    state_.sideToMove = Side::Black;
    state_.board.clear();
    dice = {6, 6};

    state_.board.setBar(Side::Black, 14);
    state_.board.setPoint(0, -1);
    state_.board.setPoint(3, 5);
    state_.board.setPoint(4, 5);
    state_.board.setPoint(5, 5);

    moves = engine_.getAllLegalMoves(state_, dice);
    EXPECT_TRUE(moves.empty()); 

    dice = {4, 6};
    moves = engine_.getAllLegalMoves(state_, dice);
    EXPECT_TRUE(moves.empty()); 

    dice = {1, 3};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());

    dice = {1, 1};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());
     
}

// --- tests for bear off moves
TEST_F(VanillaEngineTest, GeneratesLegalMoves_bear_off) {
    // test for Side = White
    state_.sideToMove = Side::White;
    state_.phaseWhite = Phase::Bear_Off;
    state_.phaseBlack = Phase::Bear_Off;
    state_.board.clear();
    std::vector<int> dice = {1, 1};

    state_.board.setPoint(23, -15);
    state_.board.setPoint(0, 2);
    state_.board.setPoint(1, 2);
    state_.board.setPoint(2, 2);
    state_.board.setPoint(3, 2);
    state_.board.setPoint(4, 2);
    state_.board.setPoint(5, 5);

    dice = {1, 1};
    std::vector<Step> moves = engine_.getAllLegalMoves(state_, dice);
    std::set<std::string> unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());
     

    dice = {1, 5};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());

    state_.board.setPoint(3, 0);
    state_.board.setBorneOff(Side::White, 2);

    dice = {4, 4};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());
     
    for (const auto& m : moves) {
        EXPECT_FALSE(std::holds_alternative<bg::Step::BearOffTag>(m.to)); // check that there is no bear off moves
    }

    state_.board.setPoint(5, 0);
    state_.board.setBorneOff(Side::White, 7);

    dice = {6, 6};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());
     
    for (const auto& m : moves) {
        EXPECT_TRUE(std::holds_alternative<bg::Step::BearOffTag>(m.to)); // check that there is bear off moves
    }

    // test for Side = Black
    state_.sideToMove = Side::Black;
    state_.board.clear();
    dice = {1, 1};

    state_.board.setPoint(0, 15);
    state_.board.setPoint(23, -2);
    state_.board.setPoint(22, -2);
    state_.board.setPoint(21, -2);
    state_.board.setPoint(20, -2);
    state_.board.setPoint(19, -2);
    state_.board.setPoint(18, -5);

    dice = {1, 1};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());
     

    dice = {1, 5};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());

    state_.board.setPoint(20, 0);
    state_.board.setBorneOff(Side::Black, 2);

    dice = {4, 4};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
     
    EXPECT_EQ(unique.size(), moves.size());
    for (const auto& m : moves) {
        EXPECT_FALSE(std::holds_alternative<bg::Step::BearOffTag>(m.to)); // check that there is no bear off moves
    }

    state_.board.setPoint(18, 0);
    state_.board.setBorneOff(Side::Black, 7);

    dice = {6, 6};
    moves = engine_.getAllLegalMoves(state_, dice);
    unique = uniqueMoveKeys(moves);
    EXPECT_EQ(unique.size(), moves.size());
     
    for (const auto& m : moves) {
        EXPECT_TRUE(std::holds_alternative<bg::Step::BearOffTag>(m.to)); // check that there is bear off moves
    }
}

// --- testing applying normal stpe
TEST_F(VanillaEngineTest, apply_step_normal) {
    // testing Side = White
    state_.sideToMove = Side::White;
    state_.phaseWhite = Phase::Normal;
    state_.phaseBlack = Phase::Normal;

    Step step{23, 22};
    engine_.applyStep(state_, step);

    EXPECT_EQ(state_.board.point(23), 1);
    EXPECT_EQ(state_.board.point(22), 1);

    state_.board.setPoint(21, -1);
    state_.board.setPoint(18, -4);

    step = Step{23, 21};
    engine_.applyStep(state_, step);

    EXPECT_EQ(state_.board.point(23), 0);
    EXPECT_EQ(state_.board.point(21), 1);
    EXPECT_EQ(state_.board.bar(Side::Black), 1);

    // testing Side = Black
    state_.sideToMove = Side::Black;
    engine_.initialState();

    step = Step{0, 1};
    engine_.applyStep(state_, step);

    EXPECT_EQ(state_.board.point(0), -1);
    EXPECT_EQ(state_.board.point(1), -1);

    state_.board.setPoint(2, 1);
    state_.board.setPoint(5, 4);

    step = Step{0, 2};
    engine_.applyStep(state_, step);

    EXPECT_EQ(state_.board.point(0), 0);
    EXPECT_EQ(state_.board.point(2), -1);
    EXPECT_EQ(state_.board.bar(Side::White), 1);
}

// --- testing applying re entry stpe
TEST_F(VanillaEngineTest, apply_step_re_entry) {
    // testing side = White
    state_.sideToMove = Side::White;
    state_.phaseWhite = Phase::Re_Entry;
    state_.phaseBlack = Phase::Re_Entry;

    state_.board.setBar(Side::White, 1);
    EXPECT_EQ(state_.board.bar(Side::White), 1);
    Step step{Step::BarTag{}, 23};
    engine_.applyStep(state_, step);

    EXPECT_EQ(state_.board.point(23), 3);
    EXPECT_EQ(state_.board.bar(Side::White), 0);

    state_.board.setBar(Side::White, 1);
    state_.board.setPoint(22, -1);
    step = Step{Step::BarTag{}, 22};
    engine_.applyStep(state_, step);

    EXPECT_EQ(state_.board.point(22), 1);
    EXPECT_EQ(state_.board.bar(Side::White), 0);
    EXPECT_EQ(state_.board.bar(Side::Black), 1);

    // testing side = Black
    state_.sideToMove = Side::Black;

    state_.board.setBar(Side::Black, 1);
    EXPECT_EQ(state_.board.bar(Side::Black), 1);
    step = Step{Step::BarTag{}, 0};
    engine_.applyStep(state_, step);

    EXPECT_EQ(state_.board.point(0), -3);
    EXPECT_EQ(state_.board.bar(Side::Black), 0);

    state_.board.setBar(Side::Black, 1);
    state_.board.setPoint(1, 1);
    step = Step{Step::BarTag{}, 1};
    engine_.applyStep(state_, step);

    EXPECT_EQ(state_.board.point(1), -1);
    EXPECT_EQ(state_.board.bar(Side::Black), 0);
    EXPECT_EQ(state_.board.bar(Side::White), 1);
}

// --- testing applying bear off stpe
TEST_F(VanillaEngineTest, apply_step_bear_off) {
    // testing side = White
    state_.sideToMove = Side::White;
    state_.phaseWhite = Phase::Bear_Off;
    state_.phaseBlack = Phase::Bear_Off;
    state_.board.clear();

    state_.board.setPoint(5, 15);
    Step step{5, Step::BearOffTag{}};
    engine_.applyStep(state_, step);

    EXPECT_EQ(state_.board.point(5), 14);
    EXPECT_EQ(state_.board.borneOff(Side::White), 1);

    engine_.applyStep(state_, step);
    EXPECT_EQ(state_.board.point(5), 13);
    EXPECT_EQ(state_.board.borneOff(Side::White), 2);

    // testing side = Black
    state_.sideToMove = Side::Black;

    state_.board.setPoint(18, -15);
    step = Step{18, Step::BearOffTag{}};
    engine_.applyStep(state_, step);

    EXPECT_EQ(state_.board.point(18), -14);
    EXPECT_EQ(state_.board.borneOff(Side::Black), 1);

    engine_.applyStep(state_, step);
    EXPECT_EQ(state_.board.point(18), -13);
    EXPECT_EQ(state_.board.borneOff(Side::Black), 2);
}

TEST_F(VanillaEngineTest, update_phases) {
    // testing normal phase
    engine_.updatePhases(state_);
    EXPECT_EQ(state_.phaseWhite, Phase::Normal);
    EXPECT_EQ(state_.phaseBlack, Phase::Normal);

    // testing re entry phase
    state_.board.setBar(Side::White, 1);
    state_.board.setBar(Side::Black, 1);
    engine_.updatePhases(state_);
    EXPECT_EQ(state_.phaseWhite, Phase::Re_Entry);
    EXPECT_EQ(state_.phaseBlack, Phase::Re_Entry);

    // testing bear off phase
    state_.board.clear();
    state_.board.setPoint(0,15);
    state_.board.setPoint(23,-15);
    engine_.updatePhases(state_);
    EXPECT_EQ(state_.phaseWhite, Phase::Bear_Off);
    EXPECT_EQ(state_.phaseBlack, Phase::Bear_Off);
}
