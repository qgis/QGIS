/***************************************************************************
  qgsalignraster.h
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

#ifndef QGSALIGNRASTER_H
#define QGSALIGNRASTER_H

#include <QList>
#include <QPointF>
#include <QSizeF>
#include <QString>
#include <gdal_version.h>
#include "qgis_analysis.h"
#include "qgis.h"
#include "qgsogrutils.h"

class QgsRectangle;

typedef void *GDALDatasetH SIP_SKIP;


/**
 * \ingroup analysis
 * \brief QgsAlignRaster takes one or more raster layers and warps (resamples) them
 * so they have the same:
 * - coordinate reference system
 * - cell size and raster size
 * - offset of the raster grid
 *
 * \since QGIS 2.12
 */
class ANALYSIS_EXPORT QgsAlignRaster
{
#ifdef SIP_RUN
#include <gdal_version.h>
#endif

  public:
    QgsAlignRaster();

    //! Utility class for gathering information about rasters
    struct ANALYSIS_EXPORT RasterInfo
    {
      public:
        //! Construct raster info with a path to a raster file
        RasterInfo( const QString &layerpath );

        RasterInfo( const RasterInfo &rh ) = delete;
        RasterInfo &operator=( const RasterInfo &rh ) = delete;

        //! Check whether the given path is a valid raster
        bool isValid() const { return nullptr != mDataset; }

        //! Returns the CRS in WKT format
        QString crs() const { return mCrsWkt; }
        //! Returns the size of the raster grid in pixels
        QSize rasterSize() const { return QSize( mXSize, mYSize ); }
        //! Returns the number of raster bands in the file
        int bandCount() const { return mBandCnt; }
        //! Returns the cell size in map units
        QSizeF cellSize() const;
        //! Returns the grid offset
        QPointF gridOffset() const;
        //! Returns the extent of the raster
        QgsRectangle extent() const;
        //! Returns the origin of the raster
        QPointF origin() const;

        //! Write contents of the object to standard error stream - for debugging
        void dump() const;

        //! Gets raster value at the given coordinates (from the first band)
        double identify( double mx, double my );

      protected:
        //! handle to open GDAL dataset
        gdal::dataset_unique_ptr mDataset;
        //! CRS stored in WKT format
        QString mCrsWkt;
        //! geotransform coefficients
        double mGeoTransform[6];
        //! raster grid size X
        int mXSize = 0;
        //! raster grid size Y
        int mYSize = 0;
        //! number of raster's bands
        int mBandCnt = 0;

      private:
#ifdef SIP_RUN
        RasterInfo( const QgsAlignRaster::RasterInfo &rh );
#endif

        friend class QgsAlignRaster;
    };


    /**
     * Resampling algorithm to be used (equivalent to GDAL's enum GDALResampleAlg)
     * \note RA_Max, RA_Min, RA_Median, RA_Q1 and RA_Q3 are available on GDAL >= 2.0 builds only
     */
    enum ResampleAlg
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
    struct Item
    {
      Item( const QString &input, const QString &output )
        : inputFilename( input )
        , outputFilename( output )
        , resampleMethod( RA_NearestNeighbour )
        , rescaleValues( false )
        , srcCellSizeInDestCRS( 0.0 )
      {}

      //! filename of the source raster
      QString inputFilename;
      //! filename of the newly created aligned raster (will be overwritten if exists already)
      QString outputFilename;
      //! resampling method to be used
      QgsAlignRaster::ResampleAlg resampleMethod;
      //! rescaling of values according to the change of pixel size
      bool rescaleValues;

      // private part

      //! used for rescaling of values (if necessary)
      double srcCellSizeInDestCRS;
    };
    typedef QList<QgsAlignRaster::Item> List;

    //! Helper struct to be sub-classed for progress reporting
    struct ProgressHandler
    {

      /**
       * Method to be overridden for progress reporting.
       * \param complete Overall progress of the alignment operation
       * \returns false if the execution should be canceled, true otherwise
       */
      virtual bool progress( double complete ) = 0;

      virtual ~ProgressHandler() = default;
    };

    //! Assign a progress handler instance. Does not take ownership. nullptr can be passed.
    void setProgressHandler( ProgressHandler *progressHandler ) { mProgressHandler = progressHandler; }
    //! Gets associated progress handler. May be nullptr (default)
    ProgressHandler *progressHandler() const { return mProgressHandler; }

    //! Sets list of rasters that will be aligned
    void setRasters( const List &list ) { mRasters = list; }
    //! Gets list of rasters that will be aligned
    List rasters() const { return mRasters; }

    void setGridOffset( QPointF offset ) { mGridOffsetX = offset.x(); mGridOffsetY = offset.y(); }
    QPointF gridOffset() const { return QPointF( mGridOffsetX, mGridOffsetY ); }

    //! Sets output cell size
    void setCellSize( double x, double y ) { setCellSize( QSizeF( x, y ) ); }
    //! Sets output cell size
    void setCellSize( QSizeF size ) { mCellSizeX = size.width(); mCellSizeY = size.height(); }
    //! Gets output cell size
    QSizeF cellSize() const { return QSizeF( mCellSizeX, mCellSizeY ); }

    //! Sets the output CRS in WKT format
    void setDestinationCrs( const QString &crsWkt ) { mCrsWkt = crsWkt; }
    //! Gets the output CRS in WKT format
    QString destinationCrs() const { return mCrsWkt; }

    /**
     * Configure clipping extent (region of interest).
     * No extra clipping is done if the rectangle is null
     */
    void setClipExtent( double xmin, double ymin, double xmax, double ymax );

    /**
     * Configure clipping extent (region of interest).
     * No extra clipping is done if the rectangle is null
     */
    void setClipExtent( const QgsRectangle &extent );

    /**
     * Gets clipping extent (region of interest).
     * No extra clipping is done if the rectangle is null
     */
    QgsRectangle clipExtent() const;

    /**
     * Set destination CRS, cell size and grid offset from a raster file.
     * The user may provide custom values for some of the parameters - in such case
     * only the remaining parameters are calculated.
     *
     * If default CRS is used, the parameters are set according to the raster file's geo-transform.
     * If a custom CRS is provided, suggested reprojection is calculated first (using GDAL) in order
     * to determine suitable defaults for cell size and grid offset.
     *
     * \returns true on success (may fail if it is not possible to reproject raster to given CRS)
     */
    bool setParametersFromRaster( const RasterInfo &rasterInfo, const QString &customCRSWkt = QString(), QSizeF customCellSize = QSizeF(), QPointF customGridOffset = QPointF( -1, -1 ) );

    /**
     * Overridden variant for convenience, taking filename instead RasterInfo object.
     * See the other variant for details.
     */
    bool setParametersFromRaster( const QString &filename, const QString &customCRSWkt = QString(), QSizeF customCellSize = QSizeF(), QPointF customGridOffset = QPointF( -1, -1 ) );

    /**
     * Determine destination extent from the input rasters and calculate derived values
     * \returns true on success, sets error on error (see errorMessage())
     */
    bool checkInputParameters();

    /**
     * Returns the expected size of the resulting aligned raster
     * \note first need to run checkInputParameters() which returns with success
     */
    QSize alignedRasterSize() const;

    /**
     * Returns the expected extent of the resulting aligned raster
     * \note first need to run checkInputParameters() which returns with success
     */
    QgsRectangle alignedRasterExtent() const;

    /**
     * Run the alignment process
     * \returns true on success, sets error on error (see errorMessage())
     */
    bool run();

    /**
     * Returns the error from a previous run() call.
     * Error message is empty if run() succeeded (returned true)
     */
    QString errorMessage() const { return mErrorMessage; }

    //! write contents of the object to standard error stream - for debugging
    void dump() const;

    //! Returns the index of the layer which has smallest cell size (returns -1 on error)
    int suggestedReferenceLayer() const;

  protected:

    //! Internal function for processing of one raster (1. create output, 2. do the alignment)
    bool createAndWarp( const Item &raster );

    //! Determine suggested output of raster warp to a different CRS. Returns true on success
    static bool suggestedWarpOutput( const RasterInfo &info, const QString &destWkt, QSizeF *cellSize = nullptr, QPointF *gridOffset = nullptr, QgsRectangle *rect = nullptr );

  protected:

    // set by the client

    //! Object that facilitates reporting of progress / cancelation
    ProgressHandler *mProgressHandler = nullptr;

    //! Last error message from run()
    QString mErrorMessage;

    //! List of rasters to be aligned (with their output files and other options)
    List mRasters;

    //! Destination CRS - stored in well-known text (WKT) format
    QString mCrsWkt;
    //! Destination cell size
    double mCellSizeX, mCellSizeY;
    //! Destination grid offset - expected to be in interval <0,cellsize)
    double mGridOffsetX, mGridOffsetY;

    /**
     * Optional clip extent: sets "requested area" which be extended to fit the raster grid.
     * Clipping not done if all coords are zeroes.
     */
    double mClipExtent[4];

    // derived data from other members

    //! Computed geo-transform
    double mGeoTransform[6];
    //! Computed raster grid width
    int mXSize;

    //! Computed raster grid height
    int mYSize;

};


#endif // QGSALIGNRASTER_H
