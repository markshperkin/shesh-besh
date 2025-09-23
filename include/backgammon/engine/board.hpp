#pragma once
#include "step.hpp"
#include "enums.hpp"

#include <vector>
#include <string>


namespace bg {

    class Board {
        
        public:
            Board();
            explicit Board(int numPoints);
            void clear(); // resets the whole board back to empty, 0 points for everything.
            void setPoint(int index, int signedCount);
            void setBar(Side s, int count);
            void setBorneOff(Side s, int count);
            void moveChecker(Side s, int from, int to);
            void hit(Side victim, int from, int to);
            void BarToHit(Side s, int to);
            void enterFromBar(Side s, int to);
            void bearOff(Side s, int from);

            int bar(Side s) const { return (s == Side::White) ? bar_white_ : bar_black_; }
            int borneOff(Side s) const { return (s == Side::White) ? borne_white_ : borne_black_; }
            int point(int index) const { return points_[index]; }

            std::vector<int> locations(Side s) const;

            std::string toString() const;

            std::pair<int,int> distanceToWin() const;

        private:
            std::vector<int> points_;
            int bar_white_ {0};
            int bar_black_ {0};
            int borne_white_ {0};
            int borne_black_ {0};
    };
}