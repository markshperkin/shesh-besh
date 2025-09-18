#pragma once
// pure data no I/O

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <variant>


#include "board.hpp"
#include "dice.hpp"
#include "step.hpp"
#include "enums.hpp"

namespace bg {


// --- Core value types

struct DoublingCube {
  int value{1}; // 1,2,4,8,16,32,64
  CubeOwner owner{CubeOwner::Center}; // who last took the cube
  bool offered{false}; // true when awaiting response
};


struct GameState {
  Board board;
  DoublingCube cube{};
  std::optional<Side> sideToMove;
  Phase phase{Phase::StartRoll};
  std::optional<Side> winner;
};

struct OpeningResult {
  Side winner;
  std::pair<int,int> startingDice;
};

// ---"Result" helpers 
using Void = std::monostate; // unit type (lets us write Result<Void>)

// expected-like alias for transition results.
// holds either a value (index 0) or a RuleViolation (index 1).
template <typename T>
using Result = std::variant<T, RuleViolation>;

template <typename T>
inline bool is_ok(const Result<T>& r) { return std::holds_alternative<T>(r); }

template <typename T>
inline T& get_ok(Result<T>& r) { return std::get<T>(r); }

template <typename T>
inline const T& get_ok(const Result<T>& r) { return std::get<T>(r); }

template <typename T>
inline RuleViolation get_err(const Result<T>& r) { return std::get<RuleViolation>(r); }

template <typename T>
inline Result<T> Ok(T v) { return Result<T>(std::in_place_index<0>, std::move(v)); }

template <typename T>
inline Result<T> Err(RuleViolation e) { return Result<T>(std::in_place_index<1>, e); }

}
