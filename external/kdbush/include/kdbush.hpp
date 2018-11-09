#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <tuple>
#include <vector>
#include <cassert>

namespace kdbush {

template <std::uint8_t I, typename T>
struct nth {
    inline static typename std::tuple_element<I, T>::type get(const T &t) {
        return std::get<I>(t);
    }
};

template <typename TPoint, typename TContainer, typename TIndex = std::size_t>
class KDBush {

public:
    using TNumber = decltype(nth<0, TPoint>::get(std::declval<TPoint>()));
    static_assert(
        std::is_same<TNumber, decltype(nth<1, TPoint>::get(std::declval<TPoint>()))>::value,
        "point component types must be identical");

    static const std::uint8_t defaultNodeSize = 64;

    KDBush(const std::uint8_t nodeSize_ = defaultNodeSize) : nodeSize(nodeSize_) {
    }

    KDBush(const std::vector<TPoint> &points_, const std::uint8_t nodeSize_ = defaultNodeSize)
        : KDBush(std::begin(points_), std::end(points_), nodeSize_) {
    }

    template <typename TPointIter>
    KDBush(const TPointIter &points_begin,
           const TPointIter &points_end,
           const std::uint8_t nodeSize_ = defaultNodeSize)
        : nodeSize(nodeSize_) {
        fill(points_begin, points_end);
    }

    void fill(const std::vector<TPoint> &points_) {
        fill(std::begin(points_), std::end(points_));
    }

    template <typename TPointIter>
    void fill(const TPointIter &points_begin, const TPointIter &points_end) {
        assert(points.empty());
        const TIndex size = static_cast<TIndex>(std::distance(points_begin, points_end));

        if (size == 0) return;

        points.reserve(size);
        //ids.reserve(size);

        TIndex i = 0;
        for (auto p = points_begin; p != points_end; p++) {
            points.emplace_back(nth<0, TPoint>::get(*p), nth<1, TPoint>::get(*p));
            //ids.push_back(i++);
        }

        sortKD(0, size - 1, 0);
    }

    template <typename TVisitor>
    void range(const TNumber minX,
               const TNumber minY,
               const TNumber maxX,
               const TNumber maxY,
               const TVisitor &visitor) {
        range(minX, minY, maxX, maxY, visitor, 0, static_cast<TIndex>(points.size() - 1), 0);
    }

    template <typename TVisitor>
    void within(const TNumber qx, const TNumber qy, const TNumber r, const TVisitor &visitor) {
        within(qx, qy, r, visitor, 0, static_cast<TIndex>(points.size() - 1), 0);
    }

protected:
    //std::vector<TIndex> ids;
    std::vector<TContainer> points;
    std::uint8_t nodeSize;

    template <typename TVisitor>
    void range(const TNumber minX,
               const TNumber minY,
               const TNumber maxX,
               const TNumber maxY,
               const TVisitor &visitor,
               const TIndex left,
               const TIndex right,
               const std::uint8_t axis) {
        if ( points.empty() )
            return;

        if (right - left <= nodeSize) {
            for (auto i = left; i <= right; i++) {
                const TNumber x = std::get<0>(points[i].coords);
                const TNumber y = std::get<1>(points[i].coords);
                if (x >= minX && x <= maxX && y >= minY && y <= maxY) visitor(points[i]);
            }
            return;
        }

        const TIndex m = (left + right) >> 1;
        const TNumber x = std::get<0>(points[m].coords);
        const TNumber y = std::get<1>(points[m].coords);

        if (x >= minX && x <= maxX && y >= minY && y <= maxY) visitor(points[m]);

        if (axis == 0 ? minX <= x : minY <= y)
            range(minX, minY, maxX, maxY, visitor, left, m - 1, (axis + 1) % 2);

        if (axis == 0 ? maxX >= x : maxY >= y)
            range(minX, minY, maxX, maxY, visitor, m + 1, right, (axis + 1) % 2);
    }

    template <typename TVisitor>
    void within(const TNumber qx,
                const TNumber qy,
                const TNumber r,
                const TVisitor &visitor,
                const TIndex left,
                const TIndex right,
                const std::uint8_t axis) {

        if ( points.empty() )
            return;

        const TNumber r2 = r * r;

        if (right - left <= nodeSize) {
            for (auto i = left; i <= right; i++) {
                const TNumber x = std::get<0>(points[i].coords);
                const TNumber y = std::get<1>(points[i].coords);
                if (sqDist(x, y, qx, qy) <= r2) visitor(points[i]);
            }
            return;
        }

        const TIndex m = (left + right) >> 1;
        const TNumber x = std::get<0>(points[m].coords);
        const TNumber y = std::get<1>(points[m].coords);

        if (sqDist(x, y, qx, qy) <= r2) visitor(points[m]);

        if (axis == 0 ? qx - r <= x : qy - r <= y)
            within(qx, qy, r, visitor, left, m - 1, (axis + 1) % 2);

        if (axis == 0 ? qx + r >= x : qy + r >= y)
            within(qx, qy, r, visitor, m + 1, right, (axis + 1) % 2);
    }

    void sortKD(const TIndex left, const TIndex right, const std::uint8_t axis) {
        if (right - left <= nodeSize) return;
        const TIndex m = (left + right) >> 1;
        if (axis == 0) {
            select<0>(m, left, right);
        } else {
            select<1>(m, left, right);
        }
        sortKD(left, m - 1, (axis + 1) % 2);
        sortKD(m + 1, right, (axis + 1) % 2);
    }

    template <std::uint8_t I>
    void select(const TIndex k, TIndex left, TIndex right) {

        while (right > left) {
            if (right - left > 600) {
                const double n = right - left + 1;
                const double m = k - left + 1;
                const double z = std::log(n);
                const double s = 0.5 * std::exp(2 * z / 3);
                const double r =
                    k - m * s / n + 0.5 * std::sqrt(z * s * (1 - s / n)) * (2 * m < n ? -1 : 1);
                select<I>(k, std::max(left, TIndex(r)), std::min(right, TIndex(r + s)));
            }

            const TNumber t = std::get<I>(points[k].coords);
            TIndex i = left;
            TIndex j = right;

            swapItem(left, k);
            if (std::get<I>(points[right].coords) > t) swapItem(left, right);

            while (i < j) {
                swapItem(i++, j--);
                while (std::get<I>(points[i].coords) < t) i++;
                while (std::get<I>(points[j].coords) > t) j--;
            }

            if (std::get<I>(points[left].coords) == t)
                swapItem(left, j);
            else {
                swapItem(++j, right);
            }

            if (j <= k) left = j + 1;
            if (k <= j) right = j - 1;
        }
    }

    void swapItem(const TIndex i, const TIndex j) {
        // std::iter_swap(ids.begin() + static_cast<std::int32_t>(i), ids.begin() + static_cast<std::int32_t>(j));
        std::iter_swap(points.begin() + static_cast<std::int32_t>(i), points.begin() + static_cast<std::int32_t>(j));
    }

    TNumber sqDist(const TNumber ax, const TNumber ay, const TNumber bx, const TNumber by) {
        auto dx = ax - bx;
        auto dy = ay - by;
        return dx * dx + dy * dy;
    }
};

} // namespace kdbush
