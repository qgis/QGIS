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
 :rtype: bool
%End

    int bandNumber;
%Docstring
 The gdal band number (starts at 1)
%End

    qgssize elementCount;

    double maximumValue;
%Docstring
 The maximum cell value in the raster band. NO_DATA values
 are ignored. This does not use the gdal GetMaximmum function. *
%End

    double minimumValue;
%Docstring
 The minimum cell value in the raster band. NO_DATA values
 are ignored. This does not use the gdal GetMinimum function. *
%End

    double mean;
%Docstring
 The mean cell value for the band. NO_DATA values are excluded.
%End

    double range;
%Docstring
 The range is the distance between min & max.
%End

    double stdDev;
%Docstring
 The standard deviation of the cell values.
%End

    int statsGathered;
%Docstring
 Collected statistics
%End

    double sum;
%Docstring
 The sum of all cells in the band. NO_DATA values are excluded.
%End

    double sumOfSquares;
%Docstring
 The sum of the squares. Used to calculate standard deviation.
%End

    int width;
%Docstring
 Number of columns used to calc statistics
%End

    int height;
%Docstring
 Number of rows used to calc statistics
%End

    QgsRectangle extent;
%Docstring
 Extent used to calc statistics
%End
};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterbandstats.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
