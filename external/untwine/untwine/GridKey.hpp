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

#pragma once

#include <cassert>
#include <climits>
#include <iostream>

namespace untwine
{

class GridKey
{
public:
    GridKey(int i, int j, int k)
    {
        assert(i < std::numeric_limits<uint8_t>::max());
        assert(j < std::numeric_limits<uint8_t>::max());
        assert(k < std::numeric_limits<uint8_t>::max());
        m_key = (i << (2 * CHAR_BIT)) | (j << CHAR_BIT) | k;
    }

    int i() const
        { return (m_key >> (2 * CHAR_BIT)); }

    int j() const
        { return ((m_key >> (CHAR_BIT)) & 0xFF); }

    int k() const
        { return (m_key & 0xFF); }

    int key() const
        { return m_key; }

private:
    int m_key;
};

inline bool operator==(const GridKey& k1, const GridKey& k2)
{
    return k1.key() == k2.key();
}

inline std::ostream& operator<<(std::ostream& out, const GridKey& k)
{
    out << k.i() << "/" << k.j() << "/" << k.k();
    return out;
}

} // namespace untwine

namespace std
{
    template<> struct hash<untwine::GridKey>
    {
        std::size_t operator()(const untwine::GridKey& k) const noexcept
        {
            return std::hash<int>()(k.key());
        }
    };
}
