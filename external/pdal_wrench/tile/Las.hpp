/*****************************************************************************
 *   Copyright (c) 2021, Hobu, Inc. (info@hobu.co)                           *
 *                                                                           *
 *   All rights reserved.                                                    *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 ****************************************************************************/

#include <pdal/Dimension.hpp>

namespace untwine
{

const pdal::Dimension::IdList& pdrfDims(int pdrf);
const pdal::Dimension::IdList& extentDims(int pdrf);

} // namespace untwine
