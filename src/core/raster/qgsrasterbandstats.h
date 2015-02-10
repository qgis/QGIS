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

#include <QString>
#include <QVector>

#include <limits>

#include "qgscolorrampshader.h"
#include "qgsrectangle.h"

/** \ingroup core
 * The RasterBandStats struct is a container for statistics about a single
 * raster band.
 */
class CORE_EXPORT QgsRasterBandStats
{
  public:
    enum Stats
    {
      None         = 0,
      Min          = 1,
      Max          = 1 << 1,
      Range        = 1 << 2,
      Sum          = 1 << 3,
      Mean         = 1 << 4,
      StdDev       = 1 << 5,
      SumOfSquares = 1 << 6,
      All          = Min | Max | Range | Sum | Mean | StdDev | SumOfSquares
    };

    QgsRasterBandStats()
    {
      statsGathered = None;
      minimumValue = std::numeric_limits<double>::max();
      maximumValue = -std::numeric_limits<double>::max();
      range = 0.0;
      mean = 0.0;
      sumOfSquares = 0.0;
      stdDev = 0.0;
      sum = 0.0;
      elementCount = 0;
      width = 0;
      height = 0;
      bandNumber = 1;
    }

    /*! Compares region, size etc. not collected statistics */
    bool contains( const QgsRasterBandStats &s ) const
    {
      return ( s.bandNumber == bandNumber &&
               s.extent == extent &&
               s.width == width &&
               s.height == height &&
               s.statsGathered == ( statsGathered & s.statsGathered ) );
    }

    /** \brief The gdal band number (starts at 1)*/
    int bandNumber;

    /** \brief The number of not no data cells in the band. */
    // TODO: check if no data are excluded in stats calculation
    qgssize elementCount;

    /** \brief The maximum cell value in the raster band. NO_DATA values
     * are ignored. This does not use the gdal GetMaximmum function. */
    double maximumValue;

    /** \brief The minimum cell value in the raster band. NO_DATA values
     * are ignored. This does not use the gdal GetMinimum function. */
    double minimumValue;

    /** \brief The mean cell value for the band. NO_DATA values are excluded. */
    double mean;

    /** \brief The range is the distance between min & max. */
    double range;

    /** \brief The standard deviation of the cell values. */
    double stdDev;

    /** \brief Collected statistics */
    int statsGathered;

    /** \brief The sum of all cells in the band. NO_DATA values are excluded. */
    double sum;

    /** \brief The sum of the squares. Used to calculate standard deviation. */
    double sumOfSquares;

    /** \brief Number of columns used to calc statistics */
    int width;

    /** \brief Number of rows used to calc statistics */
    int height;

    /** \brief Extent used to calc statistics */
    QgsRectangle extent;
};
#endif
