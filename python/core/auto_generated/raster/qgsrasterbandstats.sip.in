/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterbandstats.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsRasterBandStats
{
%Docstring
The RasterBandStats struct is a container for statistics about a single
raster band.
%End

%TypeHeaderCode
#include "qgsrasterbandstats.h"
%End
  public:
    enum Stats
    {
      None,
      Min,
      Max,
      Range,
      Sum,
      Mean,
      StdDev,
      SumOfSquares,
      All
    };

    QgsRasterBandStats();

    bool contains( const QgsRasterBandStats &s ) const;
%Docstring
Compares region, size etc. not collected statistics
%End

    int bandNumber;

    qgssize elementCount;

    double maximumValue;

    double minimumValue;

    double mean;

    double range;

    double stdDev;

    int statsGathered;

    double sum;

    double sumOfSquares;

    int width;

    int height;

    QgsRectangle extent;
};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterbandstats.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
