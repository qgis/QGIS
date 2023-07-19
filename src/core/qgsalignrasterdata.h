/***************************************************************************
  qgsalignrasterdata.h
  --------------------------------------
  Date                 : June 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALIGNRASTERDATA_H
#define QGSALIGNRASTERDATA_H

/**
 * \ingroup core
 * \brief The QgsAlignRasterData class provides data structures and enums for align raster tool.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsAlignRasterData
{
  public:

    /**
     * Resampling algorithm to be used (equivalent to GDAL's enum GDALResampleAlg)
     * \note RA_Max, RA_Min, RA_Median, RA_Q1 and RA_Q3 are available on GDAL >= 2.0 builds only
     */
    enum GdalResampleAlg
    {
      RA_NearestNeighbour = 0, //!< Nearest neighbour (select on one input pixel)
      RA_Bilinear = 1,       //!< Bilinear (2x2 kernel)
      RA_Cubic = 2,          //!< Cubic Convolution Approximation (4x4 kernel)
      RA_CubicSpline = 3,    //!< Cubic B-Spline Approximation (4x4 kernel)
      RA_Lanczos = 4,        //!< Lanczos windowed sinc interpolation (6x6 kernel)
      RA_Average = 5,        //!< Average (computes the average of all non-NODATA contributing pixels)
      RA_Mode = 6,            //!< Mode (selects the value which appears most often of all the sampled points)
      RA_Max = 8, //!< Maximum (selects the maximum of all non-NODATA contributing pixels)
      RA_Min = 9, //!< Minimum (selects the minimum of all non-NODATA contributing pixels)
      RA_Median = 10, //!< Median (selects the median of all non-NODATA contributing pixels)
      RA_Q1 = 11, //!< First quartile (selects the first quartile of all non-NODATA contributing pixels)
      RA_Q3 = 12, //!< Third quartile (selects the third quartile of all non-NODATA contributing pixels)
    };

    //! Definition of one raster layer for alignment
    struct RasterItem
    {
      RasterItem( const QString &input, const QString &output )
        : inputFilename( input )
        , outputFilename( output )
        , resampleMethod( RA_NearestNeighbour )
        , rescaleValues( false )
        , srcCellSizeInDestCRS( 0.0 )
      {}

      virtual ~RasterItem() = default;

      //! filename of the source raster
      QString inputFilename;
      //! filename of the newly created aligned raster (will be overwritten if exists already)
      QString outputFilename;
      //! resampling method to be used
      GdalResampleAlg resampleMethod;
      //! rescaling of values according to the change of pixel size
      bool rescaleValues;

      // private part

      //! used for rescaling of values (if necessary)
      double srcCellSizeInDestCRS;
    };
    typedef QList<QgsAlignRasterData::RasterItem> RasterItemList;
};


#endif // QGSALIGNRASTERDATA_H
