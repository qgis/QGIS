/***************************************************************************
                        qgsrasterbandstats.h  -  description
                              -------------------
 begin                : Fri Jun 28 2002
 copyright            : (C) 2005 by T.Sutton
 email                : tim@linfiniti.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERBANDSTATS
#define QGSRASTERBANDSTATS

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include <QString>
#include <QVector>

#include <limits>

#include "qgsrectangle.h"

/**
 * \ingroup core
 * \brief The RasterBandStats struct is a container for statistics about a single
 * raster band.
 */
class CORE_EXPORT QgsRasterBandStats
{
  public:

    QgsRasterBandStats() = default;

    //! Compares region, size etc. not collected statistics
    bool contains( const QgsRasterBandStats &s ) const
    {
      return ( s.bandNumber == bandNumber &&
               s.extent == extent &&
               s.width == width &&
               s.height == height &&
               s.statsGathered == ( statsGathered & s.statsGathered ) );
    }

    //! \brief The gdal band number (starts at 1)
    int bandNumber = 1;

    // TODO: check if no data are excluded in stats calculation

    //! \brief The number of not no data cells in the band.
    qgssize elementCount = 0;

    /**
     * \brief The maximum cell value in the raster band. NO_DATA values
     * are ignored. This does not use the gdal GetMaximmum function.
    */
    double maximumValue = -std::numeric_limits<double>::max();

    /**
     * \brief The minimum cell value in the raster band. NO_DATA values
     * are ignored. This does not use the gdal GetMinimum function.
    */
    double minimumValue = std::numeric_limits<double>::max();

    //! \brief The mean cell value for the band. NO_DATA values are excluded.
    double mean = 0;

    //! \brief The range is the distance between min & max.
    double range = 0;

    //! \brief The standard deviation of the cell values.
    double stdDev = 0;

    //! \brief Collected statistics
    Qgis::RasterBandStatistics statsGathered;

    //! \brief The sum of all cells in the band. NO_DATA values are excluded.
    double sum = 0;

    //! \brief The sum of the squares. Used to calculate standard deviation.
    double sumOfSquares = 0;

    //! \brief Number of columns used to calc statistics
    int width = 0;

    //! \brief Number of rows used to calc statistics
    int height = 0;

    //! \brief Extent used to calc statistics
    QgsRectangle extent;
};

#endif
