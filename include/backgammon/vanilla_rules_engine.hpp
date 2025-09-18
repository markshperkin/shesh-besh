#pragma once

#include "irules_engine.hpp"
#include "types.hpp"

namespace bg {

class VanillaRulesEngine final : public IRulesEngine {
public:

std::string name() const override;

  GameState initialState() const override;

  std::pair<int,int> rollDice() const override;

  OpeningResult openingRoll() const override;

  // Result<Void> canOfferDouble(const GameState& s, Side side) const override;

  // Result<GameState> applyOfferDouble(const GameState& s, Side side) const override;

  // Result<GameState> respondToDouble(const GameState& s,
  //                                   Side responder,
  //                                   DoubleDecision decision) const override;

  std::vector<Step> getAllLegalMoves(const GameState& s,
                               std::vector<int> dice) const override;

  bool isPassForced(const GameState& s,
                    const std::vector<int>& dice) const override;

  int isGameOver(const GameState& s) const override;

  void applyStep(GameState& s, const Step& step) const override;

private:

  int getEndIdx(Side side, int start, int die, Phase phase) const;

  bool canBearOff(Side side, int start, const bg::Board& b) const;

  bool isPointOpen(const bg::Board& b, int p, Side side) const;

  bool isAllInHome(const bg::Board& b, Side side) const;

  bool hasBar(const bg::Board& b, Side side) const;

  bool isValidMove(const bg::Board& b, Side side, int start, int end) const;

};

}
