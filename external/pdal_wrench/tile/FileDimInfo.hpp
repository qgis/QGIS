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

#include <pdal/Dimension.hpp>

namespace untwine
{

struct FileDimInfo
{
    FileDimInfo()
    {}

    FileDimInfo(const std::string& name) : name(name), extraDim(false)
    {}

    std::string name;
    pdal::Dimension::Type type;
    int offset;
    pdal::Dimension::Id dim;
    bool extraDim;
};

using DimInfoList = std::vector<FileDimInfo>;

inline std::ostream& operator<<(std::ostream& out, const FileDimInfo& fdi)
{
    out << fdi.name << " " << (int)fdi.type << " " << fdi.offset;
    return out;
}

inline std::istream& operator>>(std::istream& in, FileDimInfo& fdi)
{
    int type;
    in >> fdi.name >> type >> fdi.offset;
    fdi.type = (pdal::Dimension::Type)type;
    return in;
}

} // namespace untwine
