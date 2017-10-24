/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsalignraster.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsAlignRaster
{
%Docstring
 QgsAlignRaster takes one or more raster layers and warps (resamples) them
 so they have the same:
 - coordinate reference system
 - cell size and raster size
 - offset of the raster grid

.. versionadded:: 2.12
%End

%TypeHeaderCode
#include "qgsalignraster.h"
#include <gdal_version.h>
%End
  public:
    QgsAlignRaster();

    struct RasterInfo
    {
      public:
        RasterInfo( const QString &layerpath );
%Docstring
Construct raster info with a path to a raster file
%End


        bool isValid() const;
%Docstring
Check whether the given path is a valid raster
 :rtype: bool
%End

        QString crs() const;
%Docstring
Return CRS in WKT format
 :rtype: str
%End
        QSize rasterSize() const;
%Docstring
Return size of the raster grid in pixels
 :rtype: QSize
%End
        int bandCount() const;
%Docstring
Return number of raster bands in the file
 :rtype: int
%End
        QSizeF cellSize() const;
%Docstring
Return cell size in map units
 :rtype: QSizeF
%End
        QPointF gridOffset() const;
%Docstring
Return grid offset
 :rtype: QPointF
%End
        QgsRectangle extent() const;
%Docstring
Return extent of the raster
 :rtype: QgsRectangle
%End
        QPointF origin() const;
%Docstring
Return origin of the raster
 :rtype: QPointF
%End

        void dump() const;
%Docstring
write contents of the object to standard error stream - for debugging
%End

        double identify( double mx, double my );
%Docstring
Get raster value at the given coordinates (from the first band)
 :rtype: float
%End

      protected:

      private:
        RasterInfo( const QgsAlignRaster::RasterInfo &rh );
    };


    enum ResampleAlg
    {
      RA_NearestNeighbour,
      RA_Bilinear,
      RA_Cubic,
      RA_CubicSpline,
      RA_Lanczos,
      RA_Average,
      RA_Mode,
      RA_Max,
      RA_Min,
      RA_Median,
      RA_Q1,
      RA_Q3,
    };

    struct Item
    {
      Item( const QString &input, const QString &output );

      QString inputFilename;
%Docstring
filename of the source raster
%End
      QString outputFilename;
%Docstring
filename of the newly created aligned raster (will be overwritten if exists already)
%End
      QgsAlignRaster::ResampleAlg resampleMethod;
%Docstring
resampling method to be used
%End
      bool rescaleValues;
%Docstring
rescaling of values according to the change of pixel size
%End


      double srcCellSizeInDestCRS;
%Docstring
used for rescaling of values (if necessary)
%End
    };
    typedef QList<QgsAlignRaster::Item> List;

    struct ProgressHandler
    {

      virtual bool progress( double complete ) = 0;
%Docstring
 Method to be overridden for progress reporting.
 \param complete Overall progress of the alignment operation
 :return: false if the execution should be canceled, true otherwise
 :rtype: bool
%End

      virtual ~ProgressHandler();
    };

    void setProgressHandler( ProgressHandler *progressHandler );
%Docstring
Assign a progress handler instance. Does not take ownership. None can be passed.
%End
    ProgressHandler *progressHandler() const;
%Docstring
Get associated progress handler. May be None (default)
 :rtype: ProgressHandler
%End

    void setRasters( const List &list );
%Docstring
Set list of rasters that will be aligned
%End
    List rasters() const;
%Docstring
Get list of rasters that will be aligned
 :rtype: List
%End

    void setGridOffset( QPointF offset );
    QPointF gridOffset() const;
%Docstring
 :rtype: QPointF
%End

    void setCellSize( double x, double y );
%Docstring
Set output cell size
%End
    void setCellSize( QSizeF size );
%Docstring
Set output cell size
%End
    QSizeF cellSize() const;
%Docstring
Get output cell size
 :rtype: QSizeF
%End

    void setDestinationCrs( const QString &crsWkt );
%Docstring
Set the output CRS in WKT format
%End
    QString destinationCrs() const;
%Docstring
Get the output CRS in WKT format
 :rtype: str
%End

    void setClipExtent( double xmin, double ymin, double xmax, double ymax );
%Docstring
 Configure clipping extent (region of interest).
 No extra clipping is done if the rectangle is null
%End

    void setClipExtent( const QgsRectangle &extent );
%Docstring
 Configure clipping extent (region of interest).
 No extra clipping is done if the rectangle is null
%End

    QgsRectangle clipExtent() const;
%Docstring
 Get clipping extent (region of interest).
 No extra clipping is done if the rectangle is null
 :rtype: QgsRectangle
%End

    bool setParametersFromRaster( const RasterInfo &rasterInfo, const QString &customCRSWkt = QString(), QSizeF customCellSize = QSizeF(), QPointF customGridOffset = QPointF( -1, -1 ) );
%Docstring
 Set destination CRS, cell size and grid offset from a raster file.
 The user may provide custom values for some of the parameters - in such case
 only the remaining parameters are calculated.

 If default CRS is used, the parameters are set according to the raster file's geo-transform.
 If a custom CRS is provided, suggested reprojection is calculated first (using GDAL) in order
 to determine suitable defaults for cell size and grid offset.

 :return: true on success (may fail if it is not possible to reproject raster to given CRS)
 :rtype: bool
%End

    bool setParametersFromRaster( const QString &filename, const QString &customCRSWkt = QString(), QSizeF customCellSize = QSizeF(), QPointF customGridOffset = QPointF( -1, -1 ) );
%Docstring
 Overridden variant for convenience, taking filename instead RasterInfo object.
 See the other variant for details.
 :rtype: bool
%End

    bool checkInputParameters();
%Docstring
 Determine destination extent from the input rasters and calculate derived values
 :return: true on success, sets error on error (see errorMessage())
 :rtype: bool
%End

    QSize alignedRasterSize() const;
%Docstring
 Return expected size of the resulting aligned raster
.. note::

   first need to run checkInputParameters() which returns with success
 :rtype: QSize
%End

    QgsRectangle alignedRasterExtent() const;
%Docstring
 Return expected extent of the resulting aligned raster
.. note::

   first need to run checkInputParameters() which returns with success
 :rtype: QgsRectangle
%End

    bool run();
%Docstring
 Run the alignment process
 :return: true on success, sets error on error (see errorMessage())
 :rtype: bool
%End

    QString errorMessage() const;
%Docstring
 Return error from a previous run() call.
 Error message is empty if run() succeeded (returned true)
 :rtype: str
%End

    void dump() const;
%Docstring
write contents of the object to standard error stream - for debugging
%End

    int suggestedReferenceLayer() const;
%Docstring
Return index of the layer which has smallest cell size (returns -1 on error)
 :rtype: int
%End

  protected:

    bool createAndWarp( const Item &raster );
%Docstring
Internal function for processing of one raster (1. create output, 2. do the alignment)
 :rtype: bool
%End

    static bool suggestedWarpOutput( const RasterInfo &info, const QString &destWkt, QSizeF *cellSize = 0, QPointF *gridOffset = 0, QgsRectangle *rect = 0 );
%Docstring
Determine suggested output of raster warp to a different CRS. Returns true on success
 :rtype: bool
%End

  protected:










};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsalignraster.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
