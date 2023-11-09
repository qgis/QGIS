/*****************************************************************************
 *   Copyright (c) 2020, Hobu, Inc. (info@hobu.co)                           *
 *                                                                           *
 *   All rights reserved.                                                    *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 ****************************************************************************/

#include <string>
#include <algorithm>

#pragma once

namespace untwine
{

// This key supports large levels, but requires a larger key.
class TileKey
{
public:
    TileKey() : m_x(0), m_y(0), m_z(0)
    {}

    TileKey(int x, int y, int z) : m_x(x), m_y(y), m_z(z)
    {}

    int x() const
        { return m_x; }
    int y() const
        { return m_y; }
    int z() const
        { return m_z; }

    std::string toString() const
        { return (std::string)(*this); }

    operator std::string() const
    {
        return std::to_string(m_x) + '-' + std::to_string(m_y);
    }

private:
    int m_x;
    int m_y;
    int m_z;  // MD: unused but kept for now
};

inline bool operator==(const TileKey& k1, const TileKey& k2)
{
    return k1.x() == k2.x() && k1.y() == k2.y() && k1.z() == k2.z();
}

inline bool operator!=(const TileKey& k1, const TileKey& k2)
{
    return k1.x() != k2.x() || k1.y() != k2.y() || k1.z() != k2.z();
}

inline std::ostream& operator<<(std::ostream& out, const TileKey& k)
{
    out << k.toString();
    return out;
}

inline bool operator<(const TileKey& k1, const TileKey& k2)
{
    if (k1.x() != k2.x())
        return k1.x() < k2.x();
    if (k1.y() != k2.y())
        return k1.y() < k2.y();
    return k1.z() < k2.z();
}

} // namespace untwine

namespace std
{
    template<> struct hash<untwine::TileKey>
    {
        size_t operator()(const untwine::TileKey& k) const noexcept
        {
            size_t t;

            static_assert(sizeof(size_t) == sizeof(uint64_t) ||
                sizeof(size_t) == sizeof(uint32_t),
                "Only systems with 32 and 64 bit size_t are currently supported.");

            // Counting on the compiler to optimize away the wrong branch.
            if (sizeof(size_t) == sizeof(uint64_t))
            {
                // For this to work well we just assume that the values are no more than 16 bits.
                t = size_t(k.x()) << 48;
                t |= size_t(k.y()) << 32;
                t |= size_t(k.z()) << 16;
            }
            else if (sizeof(size_t) == sizeof(uint32_t))
            {
                t = size_t((k.x() << 24) | 0xFF000000);
                t |= size_t((k.y() << 16) | 0xFF0000);
                t |= size_t((k.z() << 8) | 0xFF00);
            }
            return t;
        }
    };
}
