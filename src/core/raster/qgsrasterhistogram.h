/***************************************************************************
                        qgsrasterhistogram.h
                              -------------------
 begin                : July 2012
 copyright            : (C) 2012 by Radim Blazek
 email                : radim dot blazek at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERHISTOGRAM
#define QGSRASTERHISTOGRAM

#include "qgis_core.h"
#include "qgsrectangle.h"
#include <QString>
#include <QVector>

#include <limits>

/**
 * \ingroup core
 * The QgsRasterHistogram is a container for histogram of a single raster band.
 * It is used to cache computed histograms in raster providers.
 */
class CORE_EXPORT QgsRasterHistogram
{
  public:
    typedef QVector<int> HistogramVector;

    /**
     * Constructor for an invalid QgsRasterHistogram.
     */
    QgsRasterHistogram() = default;

    //! Compares region, size etc. not histogram itself
    bool operator==( const QgsRasterHistogram &h ) const
    {
      return ( h.bandNumber == bandNumber &&
               h.binCount == binCount &&
               h.includeOutOfRange == includeOutOfRange &&
               qgsDoubleNear( h.maximum, maximum ) &&
               qgsDoubleNear( h.minimum, minimum ) &&
               h.extent == extent &&
               h.width == width &&
               h.height == height );
    }

    //! \brief The gdal band number (starts at 1)
    int bandNumber = 0;

    //! \brief Number of bins (intervals,buckets) in histogram.
    int binCount = 0;

    //! \brief The number of non NULL cells used to calculate histogram.
    int nonNullCount = 0;

    //! \brief Whether histogram includes out of range values (in first and last bin)
    bool includeOutOfRange = false;

    /**
     * Stores the histogram for a given layer
     */
    QgsRasterHistogram::HistogramVector histogramVector;

    //! \brief The maximum histogram value.
    double maximum = 0;

    //! \brief The minimum histogram value.
    double minimum = 0;

    //! \brief Number of columns used to calc histogram
    int width = 0;

    //! \brief Number of rows used to calc histogram
    int height = 0;

    //! \brief Extent used to calc histogram
    QgsRectangle extent;

    //! \brief Histogram is valid
    bool valid = false;
};
#endif
