#pragma once
#include <cstddef>
namespace bg {

    enum class Side { White, Black };
    constexpr std::size_t idx(Side s) { return s == Side::White ? 0u : 1u; } // check if needed

    enum class Phase { StartRoll, DoubleWindow, Normal, Re_Entry, Bear_Off , GameOver };
    enum class CubeOwner { Center, White, Black };  // doubling enums, needs checking
    enum class DoubleDecision { Take, Drop };
    // specific rule error codes the engine can report.
    enum class RuleViolation {
    NotYourTurn,
    WrongPhase,
    DoubleNotAllowed,
    TakeNotAllowed,
    DropNotAllowed,
    BarEntryRequired,
    PointBlocked,
    DiceMismatch,
    PartialMoveInvalid,
    MaxPipsNotUsed,
    BearingOffNotAllowed,
    StateInvariantBroken,
    Other
    };
}