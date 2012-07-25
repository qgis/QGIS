/***************************************************************************
                        qgsrasterhistogram.h
                              -------------------
 begin                : July 2012
 copyright            : (C) 2012 by Radim Blazek
 email                : radim dot blazek at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERHISTOGRAM
#define QGSRASTERHISTOGRAM

#include <QString>
#include <QVector>

#include <limits>

/** \ingroup core
 * The QgsRasterHistogram is a container for histogram of a single raster band.
 * It is used to cache computed histograms in raster providers.
 */
class CORE_EXPORT QgsRasterHistogram 
{
  public:
    typedef QVector<int> HistogramVector;

    QgsRasterHistogram()
    {
      bandNumber = 0;
      binCount = 0;
      nonNullCount = 0;
      //sampleSize = 0;
      includeOutOfRange = false;
      maximum = 0;
      minimum = 0;
      width = 0;
      height = 0;
      valid = false;
    }

    /** \brief The gdal band number (starts at 1)*/
    int bandNumber;

    /** \brief Number of bins (intervals,buckets) in histogram. */

    /** \brief The number of non NULL cells used to calculate histogram. */
    int nonNullCount;

    /** \brief Approximate number of cells used to calc histogram. Approximately 
     *  width * height. */
    //int sampleSize;

    /** \brief Whether histogram includes out of range values (in first and last bin) */
    bool includeOutOfRange;

    /** \brief Store the histogram for a given layer */
    HistogramVector histogramVector;

    /** \brief The maximum histogram value. */
    double maximum;

    /** \brief The minimum histogram value. */
    double minimum;

    /** \brief Number of columns used to calc histogram */
    int width;

    /** \brief Number of rows used to calc histogram */
    int height;

    /** \brief Extent used to calc histogram */
    QgsRectangle extent;

    /** \brief Histogram is valid */
    bool valid;
};
#endif
