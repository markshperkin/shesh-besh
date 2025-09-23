#pragma once
#include <variant>

namespace bg {

    struct Step {
    struct BarTag { }; // from/to BAR
    struct BearOffTag { }; // to BEAR_OFF

    using s = std::variant<int, BarTag, BearOffTag>; // int is a board point

    s from{};
    s to{};
    s die{};

    static bool isPoint(const s& s)    noexcept { return std::holds_alternative<int>(s); }
    static bool isBar(const s& s)      noexcept { return std::holds_alternative<BarTag>(s); }
    static bool isBearOff(const s& s)  noexcept { return std::holds_alternative<BearOffTag>(s); }

    };
}