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

#include "Las.hpp"

namespace untwine
{

const pdal::Dimension::IdList& pdrfDims(int pdrf)
{
    using namespace pdal;

    if (pdrf < 0 || pdrf > 10)
        pdrf = 10;

    using D = Dimension::Id;
    static const Dimension::IdList dims[11]
    {
        // 0
        { D::X, D::Y, D::Z, D::Intensity, D::ReturnNumber, D::NumberOfReturns, D::ScanDirectionFlag,
          D::EdgeOfFlightLine, D::Classification, D::ScanAngleRank, D::UserData, D::PointSourceId },
        // 1
        { D::X, D::Y, D::Z, D::Intensity, D::ReturnNumber, D::NumberOfReturns, D::ScanDirectionFlag,
          D::EdgeOfFlightLine, D::Classification, D::ScanAngleRank, D::UserData, D::PointSourceId,
          D::GpsTime },
        // 2
        { D::X, D::Y, D::Z, D::Intensity, D::ReturnNumber, D::NumberOfReturns, D::ScanDirectionFlag,
          D::EdgeOfFlightLine, D::Classification, D::ScanAngleRank, D::UserData, D::PointSourceId,
          D::Red, D::Green, D::Blue },
        // 3
        { D::X, D::Y, D::Z, D::Intensity, D::ReturnNumber, D::NumberOfReturns, D::ScanDirectionFlag,
          D::EdgeOfFlightLine, D::Classification, D::ScanAngleRank, D::UserData, D::PointSourceId,
          D::GpsTime, D::Red, D::Green, D::Blue },
        {},
        {},
        // 6
        { D::X, D::Y, D::Z, D::Intensity, D::ReturnNumber, D::NumberOfReturns, D::ScanDirectionFlag,
          D::EdgeOfFlightLine, D::Classification, D::ScanAngleRank, D::UserData, D::PointSourceId,
          D::GpsTime, D::ScanChannel, D::ClassFlags },
        // 7
        { D::X, D::Y, D::Z, D::Intensity, D::ReturnNumber, D::NumberOfReturns, D::ScanDirectionFlag,
          D::EdgeOfFlightLine, D::Classification, D::ScanAngleRank, D::UserData, D::PointSourceId,
          D::GpsTime, D::ScanChannel, D::ClassFlags, D::Red, D::Green, D::Blue },
        // 8
        { D::X, D::Y, D::Z, D::Intensity, D::ReturnNumber, D::NumberOfReturns, D::ScanDirectionFlag,
          D::EdgeOfFlightLine, D::Classification, D::ScanAngleRank, D::UserData, D::PointSourceId,
          D::GpsTime, D::ScanChannel, D::ClassFlags, D::Red, D::Green, D::Blue, D::Infrared },
        {},
        {}
    };
    return dims[pdrf];
}

const pdal::Dimension::IdList& extentDims(int pdrf)
{
    using namespace pdal;

    if (pdrf < 0 || pdrf > 10)
        pdrf = 10;

    using D = Dimension::Id;
    static const Dimension::IdList dims[11]
    {
        {}, // 0
        {}, // 1
        {}, // 2
        {}, // 3
        {}, // 4
        {}, // 5
        // 6
        { D::X, D::Y, D::Z, D::Intensity, D::ReturnNumber, D::NumberOfReturns, D::ScanChannel,
          D::ScanDirectionFlag, D::EdgeOfFlightLine, D::Classification, D::UserData,
          D::ScanAngleRank, D::PointSourceId, D::GpsTime },
        // 7
        { D::X, D::Y, D::Z, D::Intensity, D::ReturnNumber, D::NumberOfReturns, D::ScanChannel,
          D::ScanDirectionFlag, D::EdgeOfFlightLine, D::Classification, D::UserData,
          D::ScanAngleRank, D::PointSourceId, D::GpsTime, D::Red, D::Green, D::Blue },
        // 8
        { D::X, D::Y, D::Z, D::Intensity, D::ReturnNumber, D::NumberOfReturns, D::ScanChannel,
          D::ScanDirectionFlag, D::EdgeOfFlightLine, D::Classification, D::UserData,
          D::ScanAngleRank, D::PointSourceId, D::GpsTime, D::Red, D::Green, D::Blue, D::Infrared },
        {},
        {}
    };
    return dims[pdrf];
}

} // namespace untwine
