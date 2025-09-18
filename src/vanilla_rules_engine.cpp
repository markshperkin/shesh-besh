#include "../include/backgammon/vanilla_rules_engine.hpp"
#include <algorithm>
#include <cassert>

using bg::Board;
using bg::Side;

namespace bg {

std::string VanillaRulesEngine::name() const {
  return "VanillaBackgammon";
}

GameState VanillaRulesEngine::initialState() const {
    GameState s;
    s.board.clear(); // board starts zeroed

    s.board.setPoint(23, +2);
    s.board.setPoint(12, +5);
    s.board.setPoint(7,  +3);
    s.board.setPoint(5,  +5);

    s.board.setPoint(0,  -2);
    s.board.setPoint(11, -5);
    s.board.setPoint(16, -3);
    s.board.setPoint(18, -5);

    s.cube = DoublingCube{};
    s.phase = Phase::StartRoll;
    s.sideToMove = std::nullopt;
    s.winner = std::nullopt;

    return s;
}

std::pair<int,int> VanillaRulesEngine::rollDice() const {
  Dice dice;
  return dice.rollDice(1, 6);
}

// decided which player goes first and first roll
OpeningResult VanillaRulesEngine::openingRoll() const {
  OpeningResult r;
  while (true) {
    auto roll = rollDice();
    int whiteDie = roll.first;
    int blackDie = roll.second;

    if (whiteDie == blackDie) continue;
    
    r.winner = (whiteDie > blackDie) ? Side::White : Side::Black;
    r.startingDice = { whiteDie, blackDie };

    return r;
  }
}

// Result<Void> VanillaRulesEngine::canOfferDouble(const GameState& s, Side side) const {

// }

// Result<GameState> VanillaRulesEngine::applyOfferDouble(const GameState& s, Side side) const {

// }

// Result<GameState> VanillaRulesEngine::respondToDouble(const GameState& s,
//                                                       Side responder,
//                                                       DoubleDecision decision) const {

// }

std::vector<Step> VanillaRulesEngine::getAllLegalMoves(const GameState& s,
                                                 std::vector<int> dice) const {
  
  if (!s.sideToMove) return {};
  
  if (dice[0] == dice[1]) dice.resize(1);

  const bg::Side side = *s.sideToMove;

  std::vector<Step> steps;
  const std::vector<int> fromPts = s.board.locations(side);

  if (s.phase == Phase::Normal) {
    for (int from : fromPts) {
      for (int die : dice) {
        int to = getEndIdx(side, from, die, Phase::Normal);
        if (to < 0 || to >= 24) continue;
        if (isPointOpen(s.board, to, side) && isValidMove(s.board, side, from, to)) steps.emplace_back(Step{ from, to });
      }
    }
  }
  
  else if (s.phase == Phase::Re_Entry) {
    for (int die : dice) {
      int to = getEndIdx(side, 0, die, Phase::Re_Entry);
      if (isPointOpen(s.board, to, side) && to >= 0 && to < 24) steps.emplace_back(Step{ Step::BarTag{}, to });
    }
  }

  else if (s.phase == Phase::Bear_Off) {
    for (int from : fromPts) {
      for (int die : dice) {
        int to = getEndIdx(side, from, die, Phase::Bear_Off);
        if (to >=0 && to <= 23) steps.emplace_back(Step{ from, to });
        else {
          if (to == -1 || to == 24) steps.emplace_back(Step{ from, Step::BearOffTag{} });
          else if (canBearOff(side, from, s.board)) steps.emplace_back(Step{ from, Step::BearOffTag{} });
        } 
      }
    }
  }                                     
  return steps;
}

bool VanillaRulesEngine::isPassForced(const GameState& s,
                                      const std::vector<int>& dice) const {
  return getAllLegalMoves(s, dice).empty();
}

int VanillaRulesEngine::isGameOver(const GameState& s) const {
  if (s.board.borneOff(Side::White) == 15) return 1;
  else if (s.board.borneOff(Side::Black) == 15) return -1;
  else return 0;
}

void VanillaRulesEngine::applyStep(GameState& s, const Step&  step) const {
  Board& b = s.board;
  Side side = *s.sideToMove;
  
  if (Step::isPoint(step.from) && Step::isPoint(step.to)) {
    int from = std::get<int>(step.from);
    int to = std::get<int>(step.to);
    const int sign = (side == Side::White) ? +1 : -1;
    if (b.point(to) == -sign) b.hit(side, from, to);
    else b.moveChecker(side, from, to);
  }

  else if (Step::isBar(step.from) && Step::isPoint(step.to)) {
    int to = std::get<int>(step.to);
    const int sign = (side == Side::White) ? +1 : -1;
    if (b.point(to) == -sign) b.BarToHit(side, to);
    else b.enterFromBar(side, to);
  }

  else if (Step::isPoint(step.from) && Step::isBearOff(step.to)) {
    int from = std::get<int>(step.from);
    b.bearOff(side, from);
  }
}

// helpers

int VanillaRulesEngine::getEndIdx(Side side, int start, int die, Phase phase) const {
  if (phase == Phase::Normal) return (side == Side::White) ? start - die : start + die;
  else if (phase == Phase::Re_Entry) return (side == Side::White) ? 24 - die : -1 + die;
  else if (phase == Phase::Bear_Off) return (side == Side::White) ? start - die : start + die;
  else return start;
}

bool VanillaRulesEngine::canBearOff(Side side, int start, const Board& b) const {
  if (side == Side::White) {
    for (int i = start+1; i <= 5; i++) {
      if (b.point(i) > 0) return false;
    }
  }
  else {
    for (int i = start-1; i >= 18; i--) {
      if (b.point(i) < 0) return false;
    }
  }
  return true;
}

bool VanillaRulesEngine::isPointOpen(const Board& b, int p, Side side) const {
  int n = b.point(p);
  if (n == 0) return true;                      
  if (n > 0 && side == Side::White) return true;
  if (n < 0 && side == Side::Black) return true;
  return std::abs(n) == 1;
}

bool VanillaRulesEngine::isAllInHome(const Board& b, Side side) const {
  if (b.bar(side) != 0) return false;
  const int total = 15;
  int inHome = b.borneOff(side);
  if (side == Side::White) {
    for (int i = 0; i <= 5; ++i) if (b.point(i) > 0) inHome += b.point(i);
  } else {
    for (int i = 18; i <= 23; ++i) if (b.point(i) < 0) inHome += -b.point(i);
  }
  return inHome == total;
}

bool VanillaRulesEngine::hasBar(const Board& b, Side side) const {
  return b.bar(side) != 0;
}

bool VanillaRulesEngine::isValidMove(const Board& b, Side side, int start, int end) const {
  if (start < 0 || start >= 24 || end < 0 || end >= 24) return false;

  if (b.point(start) == 0) return false;

  if ((side == Side::White && b.point(start) < 0) || side == Side::Black && b.point(start) > 0) return false;

  if (side == Side::White && end > start) return false;
  if (side == Side::Black && end < start) return false;

  return isPointOpen(b, end, side);
}

}

