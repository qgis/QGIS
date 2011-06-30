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
/* $Id: qgsrasterlayer.h 4380 2005-12-26 23:37:50Z timlinux $ */

#ifndef QGSRASTERBANDSTATS
#define QGSRASTERBANDSTATS

#include <QString>
#include <QVector>

#include <limits>

#include "qgscolorrampshader.h"
/** \ingroup core
 * The RasterBandStats struct is a container for statistics about a single
 * raster band.
 */
class CORE_EXPORT QgsRasterBandStats
{
  public:
    typedef QVector<int> HistogramVector;

    QgsRasterBandStats()
    {
      bandName = "";
      statsGathered = false;
      minimumValue = std::numeric_limits<double>::max();
      maximumValue = std::numeric_limits<double>::min();
      range = 0.0;
      mean = 0.0;
      sumOfSquares = 0.0;
      stdDev = 0.0;
      sum = 0.0;
      elementCount = 0;
      histogramVector = new HistogramVector();
      isHistogramEstimated = false;
      isHistogramOutOfRange = false;
    }

    /** \brief The name of the band that these stats belong to. */
    QString bandName;

    /** \brief The gdal band number (starts at 1)*/
    int bandNumber;

    /** Color table */
    QList<QgsColorRampShader::ColorRampItem> colorTable;

    /** \brief The number of cells in the band. Equivalent to height x width.
     * TODO: check if NO_DATA are excluded!*/
    int elementCount;

    /** \brief whteher histogram values are estimated or completely calculated */
    bool isHistogramEstimated;

    /** whehter histogram compuation should include out of range values */
    bool isHistogramOutOfRange;

    /** \brief Store the histogram for a given layer */
    HistogramVector * histogramVector;

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

    /** \brief A flag to indicate whether this RasterBandStats struct
     * is completely populated */
    bool statsGathered;

    /** \brief The sum of all cells in the band. NO_DATA values are excluded. */
    double sum;

    /** \brief The sum of the squares. Used to calculate standard deviation. */
    double sumOfSquares;
};
#endif
