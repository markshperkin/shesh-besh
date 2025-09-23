#pragma once
// contract for all rule engines
// pure interface: no state, no randomness, no I/O.

#include "types.hpp"
#include <string>
#include <vector>

namespace bg {

struct IRulesEngine {
  virtual ~IRulesEngine() = default;

  virtual std::string name() const = 0;

  virtual GameState initialState() const = 0;

  virtual std::vector<int> rollDice() const = 0;

  virtual OpeningResult openingRoll() const = 0;

  // --- doubling
  // virtual Result<Void> canOfferDouble(const GameState& s, Side side) const = 0;

  // virtual Result<GameState> applyOfferDouble(const GameState& s, Side side) const = 0;

  // virtual Result<GameState> respondToDouble(const GameState& s,
  //                                           Side responder,
  //                                           DoubleDecision decision) const = 0;


  virtual std::vector<Step> getAllLegalMoves(const GameState& s,
                                       const std::vector<int>& dice) const = 0;

  virtual bool isPassForced(const GameState& s,
                            const std::vector<int>& dice) const = 0;

  virtual int isGameOver(const GameState& s) const = 0;

  virtual void applyStep(GameState& s, const Step& step) const = 0;

  virtual void updatePhases(GameState& s) const = 0;

};

}