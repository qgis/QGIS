/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterhistogram.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRasterHistogram
{
%Docstring
 The QgsRasterHistogram is a container for histogram of a single raster band.
 It is used to cache computed histograms in raster providers.
%End

%TypeHeaderCode
#include "qgsrasterhistogram.h"
%End
  public:
    typedef QVector<int> HistogramVector;

    QgsRasterHistogram();

    bool operator==( const QgsRasterHistogram &h ) const;

    int bandNumber;
%Docstring
 The gdal band number (starts at 1)
%End

    int binCount;
%Docstring
 Number of bins (intervals,buckets) in histogram.
%End

    int nonNullCount;
%Docstring
 The number of non NULL cells used to calculate histogram.
%End

    bool includeOutOfRange;
%Docstring
 Whether histogram includes out of range values (in first and last bin)
%End

    QgsRasterHistogram::HistogramVector histogramVector;
%Docstring
 Store the histogram for a given layer
.. note::

   not available via Python binding
%End

    double maximum;
%Docstring
 The maximum histogram value.
%End

    double minimum;
%Docstring
 The minimum histogram value.
%End

    int width;
%Docstring
 Number of columns used to calc histogram
%End

    int height;
%Docstring
 Number of rows used to calc histogram
%End

    QgsRectangle extent;
%Docstring
 Extent used to calc histogram
%End

    bool valid;
%Docstring
 Histogram is valid
%End
};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterhistogram.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
