/***************************************************************************
    qgsrasterdataprovider.h - DataProvider Interface for raster layers
     --------------------------------------
    Date                 : Mar 11, 2005
    Copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au

    async legend fetcher : Sandro Santilli < strk at keybit dot net >

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* Thank you to Marco Hugentobler for the original vector DataProvider */

#ifndef QGSRASTERDATAPROVIDER_H
#define QGSRASTERDATAPROVIDER_H

#include <cmath>

#include <QDateTime>
#include <QVariant>
#include <QImage>

#include "qgscolorrampshader.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdataprovider.h"
#include "qgserror.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsrasterbandstats.h"
#include "qgsraster.h"
#include "qgsrasterhistogram.h"
#include "qgsrasterinterface.h"
#include "qgsrasterpyramid.h"
#include "qgsrasterrange.h"
#include "qgsrectangle.h"

class QImage;
class QByteArray;

class QgsPoint;
class QgsRasterIdentifyResult;
class QgsMapSettings;

/**
 * \class Handles asynchronous download of images
 *
 * \note added in 2.8
 */
class CORE_EXPORT QgsImageFetcher : public QObject
{
    Q_OBJECT
  public:

    QgsImageFetcher() {};
    virtual ~QgsImageFetcher( ) {}

    // Make sure to connect to "finish" and "error" before starting
    virtual void start() = 0;

  signals:

    void finish( const QImage& legend );
    void progress( qint64 received, qint64 total );
    void error( const QString& msg );
};


/** \ingroup core
 * Base class for raster data providers.
 */
class CORE_EXPORT QgsRasterDataProvider : public QgsDataProvider, public QgsRasterInterface
{
    Q_OBJECT

  public:
    QgsRasterDataProvider();

    QgsRasterDataProvider( const QString & uri );

    virtual ~QgsRasterDataProvider() {};

    virtual QgsRasterInterface * clone() const = 0;

    /* It makes no sense to set input on provider */
    bool setInput( QgsRasterInterface* input ) { Q_UNUSED( input ); return false; }

    /** \brief   Renders the layer as an image
    \note When render caching (/qgis/enable_render_caching) is on the wms
       provider doesn't wait for the reply of the getmap request or all
       tiles replies and can return incomplete images.
       Temporarily disable render caching if you require the complete
       wms image in the first call.
      */
    virtual QImage* draw( const QgsRectangle & viewExtent, int pixelWidth, int pixelHeight ) = 0;

    /** Get the extent of the data source.
     * @return QgsRectangle containing the extent of the layer */
    virtual QgsRectangle extent() = 0;

    /** Returns data type for the band specified by number */
    virtual QGis::DataType dataType( int bandNo ) const = 0;

    /** Returns source data type for the band specified by number,
     *  source data type may be shorter than dataType */
    virtual QGis::DataType srcDataType( int bandNo ) const = 0;

    /** Returns data type for the band specified by number */
    virtual int colorInterpretation( int theBandNo ) const
    {
      Q_UNUSED( theBandNo );
      return QgsRaster::UndefinedColorInterpretation;
    }

    QString colorName( int colorInterpretation ) const
    {
      // Modified copy from GDAL
      switch ( colorInterpretation )
      {
        case QgsRaster::UndefinedColorInterpretation:
          return "Undefined";

        case QgsRaster::GrayIndex:
          return "Gray";

        case QgsRaster::PaletteIndex:
          return "Palette";

        case QgsRaster::RedBand:
          return "Red";

        case QgsRaster::GreenBand:
          return "Green";

        case QgsRaster::BlueBand:
          return "Blue";

        case QgsRaster::AlphaBand:
          return "Alpha";

        case QgsRaster::HueBand:
          return "Hue";

        case QgsRaster::SaturationBand:
          return "Saturation";

        case QgsRaster::LightnessBand:
          return "Lightness";

        case QgsRaster::CyanBand:
          return "Cyan";

        case QgsRaster::MagentaBand:
          return "Magenta";

        case QgsRaster::YellowBand:
          return "Yellow";

        case QgsRaster::BlackBand:
          return "Black";

        case QgsRaster::YCbCr_YBand:
          return "YCbCr_Y";

        case QgsRaster::YCbCr_CbBand:
          return "YCbCr_Cb";

        case QgsRaster::YCbCr_CrBand:
          return "YCbCr_Cr";

        default:
          return "Unknown";
      }
    }
    /** Reload data (data could change) */
    virtual bool reload() { return true; }

    virtual QString colorInterpretationName( int theBandNo ) const
    {
      return colorName( colorInterpretation( theBandNo ) );
    }

    /** Read band scale for raster value
     * @@note added in 2.3 */
    virtual double bandScale( int bandNo ) const { Q_UNUSED( bandNo ); return 1.0; }
    /** Read band offset for raster value
     * @@note added in 2.3 */
    virtual double bandOffset( int bandNo ) const { Q_UNUSED( bandNo ); return 0.0; }

    // TODO: remove or make protected all readBlock working with void*

    /** Read block of data using given extent and size. */
    virtual QgsRasterBlock *block( int theBandNo, const QgsRectangle &theExtent, int theWidth, int theHeight );

    /* Return true if source band has no data value */
    virtual bool srcHasNoDataValue( int bandNo ) const { return mSrcHasNoDataValue.value( bandNo -1 ); }

    /** \brief Get source nodata value usage */
    virtual bool useSrcNoDataValue( int bandNo ) const { return mUseSrcNoDataValue.value( bandNo -1 ); }

    /** \brief Set source nodata value usage */
    virtual void setUseSrcNoDataValue( int bandNo, bool use );

    /** Value representing no data value. */
    virtual double srcNoDataValue( int bandNo ) const { return mSrcNoDataValue.value( bandNo -1 ); }

    virtual void setUserNoDataValue( int bandNo, QgsRasterRangeList noData );

    /** Get list of user no data value ranges */
    virtual QgsRasterRangeList userNoDataValues( int bandNo ) const { return mUserNoDataValue.value( bandNo -1 ); }

    virtual QList<QgsColorRampShader::ColorRampItem> colorTable( int bandNo ) const
    { Q_UNUSED( bandNo ); return QList<QgsColorRampShader::ColorRampItem>(); }

    /** \brief Returns the sublayers of this layer - useful for providers that manage
     *  their own layers, such as WMS */
    virtual QStringList subLayers() const
    {
      return QStringList();
    }

    /** \brief Returns the legend rendered as pixmap
     *
     *  useful for that layer that need to get legend layer remotely as WMS
     * \param visibleExtent Visible extent for providers supporting
     *                      contextual legends, in layer CRS
     * \note visibleExtent parameter added in 2.8
     */
    virtual QImage getLegendGraphic( double scale = 0, bool forceRefresh = false, const QgsRectangle * visibleExtent = 0 )
    {
      Q_UNUSED( scale );
      Q_UNUSED( forceRefresh );
      Q_UNUSED( visibleExtent );
      return QImage();
    }

    /**
     * \class Get an image downloader for the raster legend
     *
     * \param mapSettings map settings for legend providers supporting
     *                    contextual legends.
     *
     * \return a download handler or null if the provider does not support
     *         legend at all. Ownership of the returned object is transferred
     *         to caller.
     *
     * \note added in 2.8
     *
     */
    virtual QgsImageFetcher* getLegendGraphicFetcher( const QgsMapSettings* mapSettings )
    {
      Q_UNUSED( mapSettings );
      return 0;
    }

    /** \brief Create pyramid overviews */
    virtual QString buildPyramids( const QList<QgsRasterPyramid> & thePyramidList,
                                   const QString & theResamplingMethod = "NEAREST",
                                   QgsRaster::RasterPyramidsFormat theFormat = QgsRaster::PyramidsGTiff,
                                   const QStringList & theConfigOptions = QStringList() )
    {
      Q_UNUSED( thePyramidList ); Q_UNUSED( theResamplingMethod );
      Q_UNUSED( theFormat ); Q_UNUSED( theConfigOptions );
      return "FAILED_NOT_SUPPORTED";
    };

    /** \brief Accessor for ths raster layers pyramid list.
     * @param overviewList used to construct the pyramid list (optional), when empty the list is defined by the provider.
     * A pyramid list defines the
     * POTENTIAL pyramids that can be in a raster. To know which of the pyramid layers
     * ACTUALLY exists you need to look at the existsFlag member in each struct stored in the
     * list.
     */
    virtual QList<QgsRasterPyramid> buildPyramidList( QList<int> overviewList = QList<int>() )
    { Q_UNUSED( overviewList ); return QList<QgsRasterPyramid>(); };

    /** \brief Returns true if raster has at least one populated histogram. */
    bool hasPyramids();

    /**
     * Get metadata in a format suitable for feeding directly
     * into a subset of the GUI raster properties "Metadata" tab.
     */
    virtual QString metadata() = 0;

    /** \brief Identify raster value(s) found on the point position. The context
     *         parameters theExtent, theWidth and theHeight are important to identify
     *         on the same zoom level as a displayed map and to do effective
     *         caching (WCS). If context params are not specified the highest
     *         resolution is used. capabilities() may be used to test if format
     *         is supported by provider. Values are set to 'no data' or empty string
     *         if point is outside data source extent.
     *
     * \note  The arbitraryness of the returned document is enforced by WMS standards
     *        up to at least v1.3.0
     * @param thePoint coordinates in data source CRS
     * @param theFormat result format
     * @param theExtent context extent
     * @param theWidth context width
     * @param theHeight context height
     * @return QgsRaster::IdentifyFormatValue: map of values for each band, keys are band numbers
     *         (from 1).
     *         QgsRaster::IdentifyFormatFeature: map of QgsRasterFeatureList for each sublayer
     *         (WMS) - TODO: it is not consistent with QgsRaster::IdentifyFormatValue.
     *         QgsRaster::IdentifyFormatHtml: map of HTML strings for each sublayer (WMS).
     *         Empty if failed or there are no results (TODO: better error reporting).
     */
    //virtual QMap<int, QVariant> identify( const QgsPoint & thePoint, QgsRaster::IdentifyFormat theFormat, const QgsRectangle &theExtent = QgsRectangle(), int theWidth = 0, int theHeight = 0 );
    virtual QgsRasterIdentifyResult identify( const QgsPoint & thePoint, QgsRaster::IdentifyFormat theFormat, const QgsRectangle &theExtent = QgsRectangle(), int theWidth = 0, int theHeight = 0 );

    /**
     * \brief   Returns the caption error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     *
     */
    virtual QString lastErrorTitle() = 0;

    /**
     * \brief   Returns the verbose error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     *
     */
    virtual QString lastError() = 0;

    /** Returns the format of the error text for the last error in this provider */
    virtual QString lastErrorFormat();

    /**Returns the dpi of the output device. */
    int dpi() const { return mDpi; }

    /**Sets the output device resolution. */
    void setDpi( int dpi ) { mDpi = dpi; }

    /** Time stamp of data source in the moment when data/metadata were loaded by provider */
    virtual QDateTime timestamp() const { return mTimestamp; }

    /** Current time stamp of data source */
    virtual QDateTime dataTimestamp() const { return QDateTime(); }

    /**Writes into the provider datasource*/
    // TODO: add data type (may be defferent from band type)
    virtual bool write( void* data, int band, int width, int height, int xOffset, int yOffset )
    {
      Q_UNUSED( data );
      Q_UNUSED( band );
      Q_UNUSED( width );
      Q_UNUSED( height );
      Q_UNUSED( xOffset );
      Q_UNUSED( yOffset );
      return false;
    }

    /** Creates a new dataset with mDataSourceURI */
    static QgsRasterDataProvider* create( const QString &providerKey,
                                          const QString &uri,
                                          const QString& format, int nBands,
                                          QGis::DataType type,
                                          int width, int height, double* geoTransform,
                                          const QgsCoordinateReferenceSystem& crs,
                                          QStringList createOptions = QStringList() );

    /** Set no data value on created dataset
     *  @param bandNo band number
     *  @param noDataValue no data value
     */
    virtual bool setNoDataValue( int bandNo, double noDataValue ) { Q_UNUSED( bandNo ); Q_UNUSED( noDataValue ); return false; }

    /** Remove dataset*/
    virtual bool remove() { return false; }

    /** Returns a list of pyramid resampling method name and label pairs
     * for given provider */
    static QList<QPair<QString, QString> > pyramidResamplingMethods( QString providerKey );

    /** Validates creation options for a specific dataset and destination format.
     * @note used by GDAL provider only
     * @note see also validateCreationOptionsFormat() in gdal provider for validating options based on format only */
    virtual QString validateCreationOptions( const QStringList& createOptions, QString format )
    { Q_UNUSED( createOptions ); Q_UNUSED( format ); return QString(); }

    /** Validates pyramid creation options for a specific dataset and destination format
     * @note used by GDAL provider only */
    virtual QString validatePyramidsConfigOptions( QgsRaster::RasterPyramidsFormat pyramidsFormat,
        const QStringList & theConfigOptions, const QString & fileFormat )
    { Q_UNUSED( pyramidsFormat ); Q_UNUSED( theConfigOptions ); Q_UNUSED( fileFormat ); return QString(); }

    static QString identifyFormatName( QgsRaster::IdentifyFormat format );
    static QgsRaster::IdentifyFormat identifyFormatFromName( QString formatName );
    static QString identifyFormatLabel( QgsRaster::IdentifyFormat format );
    static Capability identifyFormatToCapability( QgsRaster::IdentifyFormat format );

  signals:
    /** Emit a signal to notify of the progress event.
      * Emitted theProgress is in percents (0.0-100.0) */
    void progress( int theType, double theProgress, QString theMessage );
    void progressUpdate( int theProgress );

  protected:
    /** Read block of data
     * @note not available in python bindings */
    virtual void readBlock( int bandNo, int xBlock, int yBlock, void *data )
    { Q_UNUSED( bandNo ); Q_UNUSED( xBlock ); Q_UNUSED( yBlock ); Q_UNUSED( data ); }

    /** Read block of data using give extent and size
     *  @note not available in python bindings */
    virtual void readBlock( int bandNo, QgsRectangle  const & viewExtent, int width, int height, void *data )
    { Q_UNUSED( bandNo ); Q_UNUSED( viewExtent ); Q_UNUSED( width ); Q_UNUSED( height ); Q_UNUSED( data ); }

    /** Returns true if user no data contains value */
    bool userNoDataValuesContains( int bandNo, double value ) const;

    /** Copy member variables from other raster data provider. Useful for implementation of clone() method in subclasses */
    void copyBaseSettings( const QgsRasterDataProvider& other );

    static QStringList cStringList2Q_( char ** stringList );

    static QString makeTableCell( const QString & value );
    static QString makeTableCells( const QStringList & values );

    /** Dots per inch. Extended WMS (e.g. QGIS mapserver) support DPI dependent output and therefore
    are suited for printing. A value of -1 means it has not been set */
    int mDpi;

    /** Source no data value is available and is set to be used or internal no data
     *  is available. Used internally only  */
    //bool hasNoDataValue ( int theBandNo );

    /** \brief Cell value representing original source no data. e.g. -9999, indexed from 0  */
    QList<double> mSrcNoDataValue;

    /** \brief Source no data value exists. */
    QList<bool> mSrcHasNoDataValue;

    /** \brief Use source nodata value. User can disable usage of source nodata
     *  value as nodata. It may happen that a value is wrongly given by GDAL
     *  as nodata (e.g. 0) and it has to be treated as regular value. */
    QList<bool> mUseSrcNoDataValue;

    /** \brief List of lists of user defined additional no data values
     *  for each band, indexed from 0 */
    QList< QgsRasterRangeList > mUserNoDataValue;

    QgsRectangle mExtent;

    static void initPyramidResamplingDefs();
    static QStringList mPyramidResamplingListGdal;
    static QgsStringMap mPyramidResamplingMapGdal;

};
#endif
