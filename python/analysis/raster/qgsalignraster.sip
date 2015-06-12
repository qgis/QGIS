
class QgsAlignRaster
{
%TypeHeaderCode
#include <qgsalignraster.h>
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
      QByteArray crs() const;
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

      //! write contents of the object to standard error stream - for debugging
      void dump() const;

      //! Get raster value at the given coordinates (from the first band)
      double identify( double mx, double my );
    };


    //! Resampling algorithm to be used (equivalent to GDAL's enum GDALResampleAlg)
    enum ResampleAlg
    {
      RA_NearestNeighbour = 0, //!< Nearest neighbour (select on one input pixel)
      RA_Bilinear = 1,       //!< Bilinear (2x2 kernel)
      RA_Cubic = 2,          //!< Cubic Convolution Approximation (4x4 kernel)
      RA_CubicSpline = 3,    //!< Cubic B-Spline Approximation (4x4 kernel)
      RA_Lanczos = 4,        //!< Lanczos windowed sinc interpolation (6x6 kernel)
      RA_Average = 5,        //!< Average (computes the average of all non-NODATA contributing pixels)
      RA_Mode = 6            //!< Mode (selects the value which appears most often of all the sampled points)
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

    //! Set list of rasters that will be aligned
    void setRasters( const List& list );
    //! Get list of rasters that will be aligned
    List rasters() const;

    void setGridOffset( const QPointF& offset );
    QPointF gridOffset() const;

    //! Set output cell size
    void setCellSize( double x, double y );
    //! Set output cell size
    void setCellSize( const QSizeF& size );
    //! Get output cell size
    QSizeF cellSize() const;

    // TODO: first need to run determineTransformAndSize() before this
    //QSize rasterSize() const { return QSize(mXSize, mYSize); }
    // TODO: add method for access to final extent

    //! Configure clipping extent (region of interest).
    //! No extra clipping is done if the rectangle is null
    void setClipExtent( double xmin, double ymin, double xmax, double ymax );
    //! Configure clipping extent (region of interest).
    //! No extra clipping is done if the rectangle is null
    void setClipExtent( const QgsRectangle& extent );
    //! Get clipping extent (region of interest).
    //! No extra clipping is done if the rectangle is null
    QgsRectangle clipExtent() const;

    //! Set destination CRS, cell size and grid offset from a raster file
    void setParametersFromRaster( const RasterInfo& rasterInfo );
    //! Set destination CRS, cell size and grid offset from a raster file
    void setParametersFromRaster( const QString& filename );

    //! Run the alignment process
    //! @return true on success
    bool run();

    //! write contents of the object to standard error stream - for debugging
    void dump() const;

};

