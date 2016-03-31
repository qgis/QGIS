
class QgsAlignRaster
{
%TypeHeaderCode
#include <qgsalignraster.h>
#include <gdal_version.h>
%End

  public:
    QgsAlignRaster();

    //! Utility class for gathering information about rasters
    struct RasterInfo
    {
    public:
      //! Construct raster info with a path to a raster file
      RasterInfo( const QString& layerpath );
      ~RasterInfo();

      //! Check whether the given path is a valid raster
      bool isValid() const;

      //! Return CRS in WKT format
      QString crs() const;
      //! Return size of the raster grid in pixels
      QSize rasterSize() const;
      //! Return number of raster bands in the file
      int bandCount() const;
      //! Return cell size in map units
      QSizeF cellSize() const;
      //! Return grid offset
      QPointF gridOffset() const;
      //! Return extent of the raster
      QgsRectangle extent() const;
      //! Return origin of the raster
      QPointF origin() const;

      //! write contents of the object to standard error stream - for debugging
      void dump() const;

      //! Get raster value at the given coordinates (from the first band)
      double identify( double mx, double my );

      private:
        RasterInfo( const QgsAlignRaster::RasterInfo& rh );

    };


    //! Resampling algorithm to be used (equivalent to GDAL's enum GDALResampleAlg)
    //! @note RA_Max, RA_Min, RA_Median, RA_Q1 and RA_Q3 are available on GDAL >= 2.0 builds only
    enum ResampleAlg
    {
      RA_NearestNeighbour, //!< Nearest neighbour (select on one input pixel)
      RA_Bilinear,       //!< Bilinear (2x2 kernel)
      RA_Cubic,          //!< Cubic Convolution Approximation (4x4 kernel)
      RA_CubicSpline,    //!< Cubic B-Spline Approximation (4x4 kernel)
      RA_Lanczos,        //!< Lanczos windowed sinc interpolation (6x6 kernel)
      RA_Average,        //!< Average (computes the average of all non-NODATA contributing pixels)
      RA_Mode,            //!< Mode (selects the value which appears most often of all the sampled points)
      RA_Max, //!< Maximum (selects the maximum of all non-NODATA contributing pixels)
      RA_Min, //!< Minimum (selects the minimum of all non-NODATA contributing pixels)
      RA_Median, //!< Median (selects the median of all non-NODATA contributing pixels)
      RA_Q1, //!< First quartile (selects the first quartile of all non-NODATA contributing pixels)
      RA_Q3, //!< Third quartile (selects the third quartile of all non-NODATA contributing pixels)
    };

    //! Definition of one raster layer for alignment
    struct Item
    {
      Item( const QString& input, const QString& output );

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
      //! Method to be overridden for progress reporting.
      //! @param complete Overall progress of the alignment operation
      //! @return false if the execution should be cancelled, true otherwise
      virtual bool progress( double complete ) = 0;

      virtual ~ProgressHandler();
    };

    //! Assign a progress handler instance. Does not take ownership. NULL can be passed.
    void setProgressHandler( ProgressHandler* progressHandler );
    //! Get associated progress handler. May be NULL (default)
    ProgressHandler* progressHandler() const;

    //! Set list of rasters that will be aligned
    void setRasters( const List& list );
    //! Get list of rasters that will be aligned
    List rasters() const;

    void setGridOffset( QPointF offset );
    QPointF gridOffset() const;

    //! Set output cell size
    void setCellSize( double x, double y );
    //! Set output cell size
    void setCellSize( QSizeF size );
    //! Get output cell size
    QSizeF cellSize() const;

    //! Set the output CRS in WKT format
    void setDestinationCRS( const QString& crsWkt );
    //! Get the output CRS in WKT format
    QString destinationCRS() const;

    //! Configure clipping extent (region of interest).
    //! No extra clipping is done if the rectangle is null
    void setClipExtent( double xmin, double ymin, double xmax, double ymax );
    //! Configure clipping extent (region of interest).
    //! No extra clipping is done if the rectangle is null
    void setClipExtent( const QgsRectangle& extent );
    //! Get clipping extent (region of interest).
    //! No extra clipping is done if the rectangle is null
    QgsRectangle clipExtent() const;

    //! Set destination CRS, cell size and grid offset from a raster file.
    //! The user may provide custom values for some of the parameters - in such case
    //! only the remaining parameters are calculated.
    //!
    //! If default CRS is used, the parameters are set according to the raster file's geo-transform.
    //! If a custom CRS is provided, suggested reprojection is calculated first (using GDAL) in order
    //! to determine suitable defaults for cell size and grid offset.
    //!
    //! @return true on success (may fail if it is not possible to reproject raster to given CRS)
    bool setParametersFromRaster( const QgsAlignRaster::RasterInfo& rasterInfo, const QString& customCRSWkt = QString(), QSizeF customCellSize = QSizeF(), QPointF customGridOffset = QPointF( -1, -1 ) );
    //! Overridden variant for convenience, taking filename instead RasterInfo object.
    //! See the other variant for details.
    bool setParametersFromRaster( const QString& filename, const QString& customCRSWkt = QString(), QSizeF customCellSize = QSizeF(), QPointF customGridOffset = QPointF( -1, -1 ) );

    //! Determine destination extent from the input rasters and calculate derived values
    //! @return true on success, sets error on error (see errorMessage())
    bool checkInputParameters();

    //! Return expected size of the resulting aligned raster
    //! @note first need to run checkInputParameters() which returns with success
    QSize alignedRasterSize() const;
    //! Return expected extent of the resulting aligned raster
    //! @note first need to run checkInputParameters() which returns with success
    QgsRectangle alignedRasterExtent() const;

    //! Run the alignment process
    //! @return true on success, sets error on error (see errorMessage())
    bool run();

    //! Return error from a previous run() call.
    //! Error message is empty if run() succeeded (returned true)
    QString errorMessage() const;

    //! write contents of the object to standard error stream - for debugging
    void dump() const;

    //! Return index of the layer which has smallest cell size (returns -1 on error)
    int suggestedReferenceLayer() const;

  protected:

    //! Internal function for processing of one raster (1. create output, 2. do the alignment)
    bool createAndWarp( const Item& raster );

    //! Determine suggested output of raster warp to a different CRS. Returns true on success
    static bool suggestedWarpOutput( const RasterInfo& info, const QString& destWkt, QSizeF* cellSize = 0, QPointF* gridOffset = 0, QgsRectangle* rect = 0 );

};

