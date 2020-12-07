#include "include/kdbush.hpp"

#include <cassert>
#include <iostream>
#include <iterator>
#include <vector>

using TPoint = std::pair<int, int>;
using TIds = std::vector<std::size_t>;

static std::vector<TPoint> points = {
    { 54, 1 },  { 97, 21 }, { 65, 35 }, { 33, 54 }, { 95, 39 }, { 54, 3 },  { 53, 54 }, { 84, 72 },
    { 33, 34 }, { 43, 15 }, { 52, 83 }, { 81, 23 }, { 1, 61 },  { 38, 74 }, { 11, 91 }, { 24, 56 },
    { 90, 31 }, { 25, 57 }, { 46, 61 }, { 29, 69 }, { 49, 60 }, { 4, 98 },  { 71, 15 }, { 60, 25 },
    { 38, 84 }, { 52, 38 }, { 94, 51 }, { 13, 25 }, { 77, 73 }, { 88, 87 }, { 6, 27 },  { 58, 22 },
    { 53, 28 }, { 27, 91 }, { 96, 98 }, { 93, 14 }, { 22, 93 }, { 45, 94 }, { 18, 28 }, { 35, 15 },
    { 19, 81 }, { 20, 81 }, { 67, 53 }, { 43, 3 },  { 47, 66 }, { 48, 34 }, { 46, 12 }, { 32, 38 },
    { 43, 12 }, { 39, 94 }, { 88, 62 }, { 66, 14 }, { 84, 30 }, { 72, 81 }, { 41, 92 }, { 26, 4 },
    { 6, 76 },  { 47, 21 }, { 57, 70 }, { 71, 82 }, { 50, 68 }, { 96, 18 }, { 40, 31 }, { 78, 53 },
    { 71, 90 }, { 32, 14 }, { 55, 6 },  { 32, 88 }, { 62, 32 }, { 21, 67 }, { 73, 81 }, { 44, 64 },
    { 29, 50 }, { 70, 5 },  { 6, 22 },  { 68, 3 },  { 11, 23 }, { 20, 42 }, { 21, 73 }, { 63, 86 },
    { 9, 40 },  { 99, 2 },  { 99, 76 }, { 56, 77 }, { 83, 6 },  { 21, 72 }, { 78, 30 }, { 75, 53 },
    { 41, 11 }, { 95, 20 }, { 30, 38 }, { 96, 82 }, { 65, 48 }, { 33, 18 }, { 87, 28 }, { 10, 10 },
    { 40, 34 }, { 10, 20 }, { 47, 29 }, { 46, 78 }
};

static void testRange() {
    kdbush::KDBush<TPoint> index(points, 10);
    TIds expectedIds = { 3, 90, 77, 72, 62, 96, 47, 8, 17, 15, 69, 71, 44, 19, 18, 45, 60, 20 };
    TIds result;
    index.range(20, 30, 50, 70, [&result](const auto id) { result.push_back(id); });

    assert(std::equal(expectedIds.begin(), expectedIds.end(), result.begin()));
}

static void testRadius() {
    kdbush::KDBush<TPoint> index(points, 10);
    TIds expectedIds = { 3, 96, 71, 44, 18, 45, 60, 6, 25, 92, 42, 20 };
    TIds result;
    index.within(50, 50, 20, [&result](const auto id) { result.push_back(id); });

    assert(std::equal(expectedIds.begin(), expectedIds.end(), result.begin()));
}

static void testEmpty() {
    auto emptyPoints = std::vector<TPoint>{};
    kdbush::KDBush<TPoint> index(emptyPoints);
}

int main() {
    testRange();
    testRadius();
    testEmpty();
    return 0;
}
