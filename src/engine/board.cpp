#include "backgammon/engine/board.hpp"

#include <sstream>

namespace bg {

    Board::Board() : Board(24) {}

    Board::Board(int numPoints):
        points_(numPoints, 0),
        bar_white_(0), bar_black_(0),
        borne_white_(0), borne_black_(0) {}

    void Board::clear() {
        for (int &v : points_) v = 0;
        bar_white_ = bar_black_ = borne_white_ = borne_black_ = 0;
    }

    void Board::setPoint(int index, int signedCount) {
        points_[index] = signedCount;
    }

    void Board::setBar(Side s, int count) {
        if (s == Side::White) bar_white_ = count;
        else bar_black_ = count;
    }

    void Board::setBorneOff(Side s, int count) {
        if (s == Side::White) borne_white_ = count;
        else borne_black_ = count;
    }

    void Board::moveChecker(Side s, int from, int to) { 
        const int sign = (s == Side::White) ? +1 : -1;
        points_[from] -= sign;
        points_[to] += sign;
    }

    void Board::hit(Side s, int from, int to) { 
        const int sign = (s == Side::White) ? +1 : -1;
        points_[from] -= sign;
        points_[to] = sign;
        if (sign == +1) ++bar_black_;
        else ++bar_white_;
    }

    void Board::BarToHit(Side s, int to) {
        const int sign = (s == Side::White) ? +1 : -1;
        points_[to] = sign;
        if (sign == +1) {
            --bar_white_;
            ++bar_black_;
        }
        else {
            --bar_black_;
            ++bar_white_;
        }
    }

    void Board::enterFromBar(Side s, int to) { 
        if (s == Side::White) { 
            --bar_white_;
            points_[to] += 1;
        }
        else {
            --bar_black_;
            points_[to] -= 1;
        }
    }

    void Board::bearOff(Side s, int from) { 
        if (s == Side::White) { 
            points_[from] -= 1;
            ++borne_white_;
        }
        else {
            points_[from] += 1;
            ++borne_black_;
        }
    }

    std::vector<int> Board::locations(Side s) const {
        std::vector<int> res;

        for (int i = 0; i < static_cast<int>(points_.size()); ++i) {
            int p = point(i);
            if ((s == Side::White && p > 0) || s == Side::Black && p < 0) res.push_back(i);
        }

        return res;
    }


    std::string Board::toString() const {
        std::ostringstream oss;
        oss << "[";
        for (int i = 0; i < points_.size(); ++i) {
            oss << points_[i];
            if (i + 1 < points_.size()) oss << ", ";
        }
        oss << "] | barW=" << bar_white_ << " barB=" << bar_black_
            << " | borneW=" << borne_white_ << " borneB=" << borne_black_;
        return oss.str();
    }
    
    std::pair<int,int> Board::distanceToWin() const { // uses white and black direction oppoiste. NEED FIX
        int whiteDist = 0;
        int blackDist = 0;
        int n = points_.size();

        for (int i = 0; i < n; ++i) {
            int v = points_[i];
            if (v > 0) whiteDist += v * i;
            else if (v < 0) blackDist += v * (n - i);
        }
        whiteDist += bar_white_ * n;
        blackDist -= bar_black_ * n;

        return { whiteDist, blackDist };
    }
}