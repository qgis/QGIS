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

#include "qgis_core.h"
#include "qgis_sip.h"
#include <cmath>

#include <QDateTime>
#include <QVariant>
#include <QImage>

#include "qgscolorrampshader.h"
#include "qgsdataprovider.h"
#include "qgsrasterattributetable.h"
#include "qgsfields.h"
#include "qgsrasterinterface.h"
#include "qgsrasterpyramid.h"
#include "qgsrasterrange.h"
#include "qgsrectangle.h"
#include "qgsrasteriterator.h"
#include "qgsrasterdataprovidertemporalcapabilities.h"

class QImage;
class QByteArray;

class QgsPointXY;
class QgsRasterIdentifyResult;
class QgsMapSettings;

/**
 * \brief Handles asynchronous download of images
 * \ingroup core
 * \since QGIS 2.8
 */
class CORE_EXPORT QgsImageFetcher : public QObject
{
    Q_OBJECT
  public:
    //! Constructor
    QgsImageFetcher( QObject *parent = nullptr ) : QObject( parent ) {}

    /**
     * Starts the image download
     * \note Make sure to connect to "finish" and "error" before starting
    */
    virtual void start() = 0;

  signals:

    /**
     * Emitted when the download completes
     *  \param legend The downloaded legend image
    */
    void finish( const QImage &legend );
    //! Emitted to report progress
    void progress( qint64 received, qint64 total );
    //! Emitted when an error occurs
    void error( const QString &msg );
};


/**
 * \ingroup core
 * \brief Base class for raster data providers.
 */
class CORE_EXPORT QgsRasterDataProvider : public QgsDataProvider, public QgsRasterInterface
{
    Q_OBJECT

  public:

    /**
     * Enumeration with capabilities that raster providers might implement.
     * \since QGIS 3.0
     */
    enum ProviderCapability
    {
      NoProviderCapabilities = 0,       //!< Provider has no capabilities
      ReadLayerMetadata = 1 << 1, //!< Provider can read layer metadata from data store. Since QGIS 3.0. See QgsDataProvider::layerMetadata()
      WriteLayerMetadata = 1 << 2, //!< Provider can write layer metadata to the data store. Since QGIS 3.0. See QgsDataProvider::writeLayerMetadata()
      ProviderHintBenefitsFromResampling = 1 << 3, //!< Provider benefits from resampling and should apply user default resampling settings (since QGIS 3.10)
      ProviderHintCanPerformProviderResampling = 1 << 4, //!< Provider can perform resampling (to be opposed to post rendering resampling) (since QGIS 3.16)
      ReloadData = 1 << 5, //!< Is able to force reload data / clear local caches. Since QGIS 3.18, see QgsDataProvider::reloadProviderData()
      DpiDependentData = 1 << 6, //! Provider's rendering is dependent on requested pixel size of the viewport (since QGIS 3.20)
      NativeRasterAttributeTable = 1 << 7, //!< Indicates that the provider supports native raster attribute table (since QGIS 3.30)
    };

    //! Provider capabilities
    Q_DECLARE_FLAGS( ProviderCapabilities, ProviderCapability )

    QgsRasterDataProvider();

    /**
     * Constructor for QgsRasterDataProvider.
     *
     * The \a uri argument gives a provider-specific uri indicating the underlying data
     * source and it's parameters.
     *
     * The \a options argument specifies generic provider options and since QGIS 3.16 creation flags are specified within the \a flags value.
     */
    QgsRasterDataProvider( const QString &uri,
                           const QgsDataProvider::ProviderOptions &providerOptions = QgsDataProvider::ProviderOptions(),
                           QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    QgsRasterDataProvider *clone() const override = 0;

    /**
     * Returns flags containing the supported capabilities of the data provider.
     * \since QGIS 3.0
     */
    virtual QgsRasterDataProvider::ProviderCapabilities providerCapabilities() const;

    /* It makes no sense to set input on provider */
    bool setInput( QgsRasterInterface *input ) override { Q_UNUSED( input ) return false; }

    QgsRectangle extent() const override = 0;

    //! Returns data type for the band specified by number
    Qgis::DataType dataType( int bandNo ) const override = 0;

    /**
     * Returns the fields of the raster layer for data providers that expose them,
     * the default implementation returns an empty list.
     * \since QGIS 3.14
     */
    virtual QgsFields fields() const { return QgsFields(); };

    /**
     * Returns source data type for the band specified by number,
     *  source data type may be shorter than dataType
     */
    Qgis::DataType sourceDataType( int bandNo ) const override = 0;

    //! Returns data type for the band specified by number
    virtual Qgis::RasterColorInterpretation colorInterpretation( int bandNo ) const;

    /**
     * Returns a string color name representation of a color interpretation.
     */
    QString colorName( Qgis::RasterColorInterpretation colorInterpretation ) const
    {
      // Modified copy from GDAL
      switch ( colorInterpretation )
      {
        case Qgis::RasterColorInterpretation::Undefined:
          return QStringLiteral( "Undefined" );

        case Qgis::RasterColorInterpretation::GrayIndex:
          return QStringLiteral( "Gray" );

        case Qgis::RasterColorInterpretation::PaletteIndex:
          return QStringLiteral( "Palette" );

        case Qgis::RasterColorInterpretation::RedBand:
          return QStringLiteral( "Red" );

        case Qgis::RasterColorInterpretation::GreenBand:
          return QStringLiteral( "Green" );

        case Qgis::RasterColorInterpretation::BlueBand:
          return QStringLiteral( "Blue" );

        case Qgis::RasterColorInterpretation::AlphaBand:
          return QStringLiteral( "Alpha" );

        case Qgis::RasterColorInterpretation::HueBand:
          return QStringLiteral( "Hue" );

        case Qgis::RasterColorInterpretation::SaturationBand:
          return QStringLiteral( "Saturation" );

        case Qgis::RasterColorInterpretation::LightnessBand:
          return QStringLiteral( "Lightness" );

        case Qgis::RasterColorInterpretation::CyanBand:
          return QStringLiteral( "Cyan" );

        case Qgis::RasterColorInterpretation::MagentaBand:
          return QStringLiteral( "Magenta" );

        case Qgis::RasterColorInterpretation::YellowBand:
          return QStringLiteral( "Yellow" );

        case Qgis::RasterColorInterpretation::BlackBand:
          return QStringLiteral( "Black" );

        case Qgis::RasterColorInterpretation::YCbCr_YBand:
          return QStringLiteral( "YCbCr_Y" );

        case Qgis::RasterColorInterpretation::YCbCr_CbBand:
          return QStringLiteral( "YCbCr_Cb" );

        case Qgis::RasterColorInterpretation::YCbCr_CrBand:
          return QStringLiteral( "YCbCr_Cr" );

        case Qgis::RasterColorInterpretation::ContinuousPalette:
          return QStringLiteral( "Continuous Palette" );
      }
      return QString();
    }

    //! Reload data (data could change)
    virtual bool reload() { return true; }

    QString colorInterpretationName( int bandNo ) const override;

    /**
     * Read band scale for raster value
     * \since QGIS 2.3
     */
    virtual double bandScale( int bandNo ) const { Q_UNUSED( bandNo ) return 1.0; }

    /**
     * Read band offset for raster value
     * \since QGIS 2.3
     */
    virtual double bandOffset( int bandNo ) const { Q_UNUSED( bandNo ) return 0.0; }

    // TODO: remove or make protected all readBlock working with void*

    //! Read block of data using given extent and size.
    QgsRasterBlock *block( int bandNo, const QgsRectangle &boundingBox, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    //! Returns TRUE if source band has no data value
    virtual bool sourceHasNoDataValue( int bandNo ) const { return mSrcHasNoDataValue.value( bandNo - 1 ); }

    //! Returns the source nodata value usage
    virtual bool useSourceNoDataValue( int bandNo ) const { return mUseSrcNoDataValue.value( bandNo - 1 ); }

    //! Sets the source nodata value usage
    virtual void setUseSourceNoDataValue( int bandNo, bool use );

    //! Value representing no data value.
    virtual double sourceNoDataValue( int bandNo ) const { return mSrcNoDataValue.value( bandNo - 1 ); }

    virtual void setUserNoDataValue( int bandNo, const QgsRasterRangeList &noData );

    //! Returns a list of user no data value ranges.
    virtual QgsRasterRangeList userNoDataValues( int bandNo ) const { return mUserNoDataValue.value( bandNo - 1 ); }

    virtual QList<QgsColorRampShader::ColorRampItem> colorTable( int bandNo ) const
    { Q_UNUSED( bandNo ) return QList<QgsColorRampShader::ColorRampItem>(); }

    /**
     * \brief Returns the sublayers of this layer - useful for providers that manage
     *  their own layers, such as WMS
    */
    QStringList subLayers() const override
    {
      return QStringList();
    }

    QgsRasterDataProviderTemporalCapabilities *temporalCapabilities() override;
    const QgsRasterDataProviderTemporalCapabilities *temporalCapabilities() const override SIP_SKIP;

    //! \brief Returns whether the provider supplies a legend graphic
    virtual bool supportsLegendGraphic() const { return false; }

    /**
     * Returns the legend rendered as pixmap
     *
     * This is useful for layers which need to get legend layers remotely as WMS.
     *
     * \param scale Optional parameter that is the Scale of the layer
     * \param forceRefresh Optional bool parameter to force refresh getLegendGraphic call
     * \param visibleExtent Visible extent for providers supporting contextual legends, in layer CRS
     * \note Parameter visibleExtent added in QGIS 2.8
     * \note Not available in Python bindings
     */
    virtual QImage getLegendGraphic( double scale = 0, bool forceRefresh = false, const QgsRectangle *visibleExtent = nullptr ) SIP_SKIP
    {
      Q_UNUSED( scale )
      Q_UNUSED( forceRefresh )
      Q_UNUSED( visibleExtent )
      return QImage();
    }

    /**
     * Returns a new image downloader for the raster legend.
     *
     * \param mapSettings map settings for legend providers supporting
     *                    contextual legends.
     *
     * \returns a download handler or NULLPTR if the provider does not support
     *         legend at all. Ownership of the returned object is transferred
     *         to caller.
     *
     *
     * \since QGIS 2.8
     */
    virtual QgsImageFetcher *getLegendGraphicFetcher( const QgsMapSettings *mapSettings ) SIP_FACTORY
    {
      Q_UNUSED( mapSettings )
      return nullptr;
    }

    /**
     * Creates pyramid overviews.
     *
     * \param pyramidList a list of QgsRasterPyramids to create overviews for. The QgsRasterPyramid::setBuild() flag
     * should be set to TRUE for every layer where pyramids are desired.
     * \param resamplingMethod resampling method to use when creating the pyramids. The pyramidResamplingMethods() method
     * can be used to retrieve a list of valid resampling methods available for specific raster data providers.
     * \param format raster pyramid format.
     * \param configOptions optional configuration options which are passed to the specific data provider
     * for use during pyramid creation.
     * \param feedback optional feedback argument for progress reports and cancellation support.
     *
     * \see buildPyramidList()
     * \see hasPyramids()
     * \see pyramidResamplingMethods()
     */
    virtual QString buildPyramids( const QList<QgsRasterPyramid> &pyramidList,
                                   const QString &resamplingMethod = "NEAREST",
                                   Qgis::RasterPyramidFormat format = Qgis::RasterPyramidFormat::GeoTiff,
                                   const QStringList &configOptions = QStringList(),
                                   QgsRasterBlockFeedback *feedback = nullptr )
    {
      Q_UNUSED( pyramidList )
      Q_UNUSED( resamplingMethod )
      Q_UNUSED( format )
      Q_UNUSED( configOptions )
      Q_UNUSED( feedback )
      return QStringLiteral( "FAILED_NOT_SUPPORTED" );
    }

    /**
     * Returns the raster layers pyramid list.
     *
     * This method returns a list of pyramid layers which are valid for the data provider. The returned list
     * is a complete list of all possible layers, and includes both pyramids layers which currently exist and
     * layers which have not yet been constructed. To know which of the pyramid layers
     * ACTUALLY exists you need to look at the QgsRasterPyramid::getExists() member for each value in the
     * list.
     *
     * The returned list is suitable for passing to the buildPyramids() method. First, modify the returned list
     * by calling `QgsRasterPyramid::setBuild( TRUE )` for every layer you want to create pyramids for, and then
     * pass the modified list to buildPyramids().
     *
     * \param overviewList used to construct the pyramid list (optional), when empty the list is defined by the provider.
     *
     * \see buildPyramids()
     * \see hasPyramids()
     */
    virtual QList<QgsRasterPyramid> buildPyramidList( const QList<int> &overviewList = QList<int>() )
    { Q_UNUSED( overviewList ) return QList<QgsRasterPyramid>(); }

    /**
     * Returns TRUE if raster has at least one existing pyramid.
     *
     * The buildPyramidList() method can be used to retrieve additional details about potential and existing
     * pyramid layers.
     *
     * \see buildPyramidList()
     * \see buildPyramids()
     */
    bool hasPyramids();

    /**
     * Returns metadata in a format suitable for feeding directly
     * into a subset of the GUI raster properties "Metadata" tab.
     */
    virtual QString htmlMetadata() = 0;

    /**
     * Identify raster value(s) found on the point position. The context
     * parameters extent, width and height are important to identify
     * on the same zoom level as a displayed map and to do effective
     * caching (WCS). If context params are not specified the highest
     * resolution is used. capabilities() may be used to test if format
     * is supported by provider. Values are set to 'no data' or empty string
     * if point is outside data source extent.
     *
     * \param point coordinates in data source CRS
     * \param format result format
     * \param boundingBox context bounding box
     * \param width context width
     * \param height context height
     * \param dpi context dpi
     * \return QgsRaster::IdentifyFormatValue: map of values for each band, keys are band numbers
     *         (from 1).
     *         QgsRaster::IdentifyFormatFeature: map of QgsRasterFeatureList for each sublayer
     *         QgsRaster::IdentifyFormatHtml: map of HTML strings for each sublayer (WMS).
     *         Empty if failed or there are no results.
     * \note The arbitraryness of the returned document is enforced by WMS standards
     *       up to at least v1.3.0
     * \see sample(), which is much more efficient for simple "value at point" queries.
     */
    virtual QgsRasterIdentifyResult identify( const QgsPointXY &point, Qgis::RasterIdentifyFormat format, const QgsRectangle &boundingBox = QgsRectangle(), int width = 0, int height = 0, int dpi = 96 );

    /**
     * Samples a raster value from the specified \a band found at the \a point position. The context
     * parameters \a boundingBox, \a width and \a height are important to identify
     * on the same zoom level as a displayed map and to do effective
     * caching (WCS). If context params are not specified the highest
     * resolution is used.
     *
     * If \a ok is specified and the point is outside data source extent, or an invalid
     * band number was specified, then \a ok will be set to FALSE. In this case the function will return
     * a NaN value.
     *
     * \see identify(), which is much more flexible but considerably less efficient.
     * \since QGIS 3.4
     */
    virtual double sample( const QgsPointXY &point, int band,
                           bool *ok SIP_OUT = nullptr,
                           const QgsRectangle &boundingBox = QgsRectangle(), int width = 0, int height = 0, int dpi = 96 );

    /**
     * \brief Returns the caption error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
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

    //! Returns the format of the error text for the last error in this provider
    virtual QString lastErrorFormat();

    //! Returns the dpi of the output device.
    int dpi() const { return mDpi; }

    //! Sets the output device resolution.
    void setDpi( int dpi ) { mDpi = dpi; }

    //! Time stamp of data source in the moment when data/metadata were loaded by provider
    QDateTime timestamp() const override { return mTimestamp; }

    //! Current time stamp of data source
    QDateTime dataTimestamp() const override { return QDateTime(); }

    /**
     * Checks whether the provider is in editing mode, i.e. raster write operations will be accepted.
     * By default providers are not editable. Use setEditable() method to enable/disable editing.
     * \see setEditable(), writeBlock()
     * \since QGIS 3.0
     */
    virtual bool isEditable() const { return false; }

    /**
     * Turns on/off editing mode of the provider. When in editing mode, it is possible
     * to overwrite data of the provider using writeBlock() calls.
     * \returns TRUE if the switch to/from editing mode was successful
     * \note Only some providers support editing mode and even those may fail to turn
     * the underlying data source into editing mode, so it is necessary to check the return
     * value whether the operation was successful.
     * \see isEditable(), writeBlock()
     * \since QGIS 3.0
     */
    virtual bool setEditable( bool enabled ) { Q_UNUSED( enabled ) return false; }

    // TODO: add data type (may be different from band type)

    //! Writes into the provider datasource
    virtual bool write( void *data, int band, int width, int height, int xOffset, int yOffset )
    {
      Q_UNUSED( data )
      Q_UNUSED( band )
      Q_UNUSED( width )
      Q_UNUSED( height )
      Q_UNUSED( xOffset )
      Q_UNUSED( yOffset )
      return false;
    }

    /**
     * Writes pixel data from a raster block into the provider data source.
     *
     * This will override previously stored pixel values. It is assumed that cells in the passed
     * raster block are aligned with the cells of the data source. If raster block does not cover
     * the whole area of the data source, only a subset of pixels covered by the raster block
     * will be overwritten. By default, writing of raster data starts from the first cell
     * of the raster - it is possible to set offset in pixels by specifying non-zero
     * xOffset and yOffset values.
     *
     * Writing is supported only by some data providers. Provider has to be in editing mode
     * in order to allow write operations.
     * \see isEditable(), setEditable()
     * \returns TRUE on success
     * \since QGIS 3.0
     */
    bool writeBlock( QgsRasterBlock *block, int band, int xOffset = 0, int yOffset = 0 );

    //! Creates a new dataset with mDataSourceURI
    static QgsRasterDataProvider *create( const QString &providerKey,
                                          const QString &uri,
                                          const QString &format, int nBands,
                                          Qgis::DataType type,
                                          int width, int height, double *geoTransform,
                                          const QgsCoordinateReferenceSystem &crs,
                                          const QStringList &createOptions = QStringList() );

    /**
     * Set no data value on created dataset
     *  \param bandNo band number
     *  \param noDataValue no data value
     */
    virtual bool setNoDataValue( int bandNo, double noDataValue ) { Q_UNUSED( bandNo ) Q_UNUSED( noDataValue ); return false; }

    //! Remove dataset
    virtual bool remove() { return false; }

    /**
     * Returns a list of pyramid resampling method name and label pairs
     * for given provider
     */
    static QList<QPair<QString, QString> > pyramidResamplingMethods( const QString &providerKey );

    /**
     * Struct that stores information of the raster used in QgsVirtualRasterProvider for the calculations,
     * this struct is  stored in the DecodedUriParameters
     * \note used by QgsVirtualRasterProvider only
     */
    struct VirtualRasterInputLayers
    {
      QString name;
      QString uri;
      QString provider;
    };

    /**
     * Struct that stores the information about the parameters that should be given to the
     * QgsVirtualRasterProvider through the QgsRasterDataProvider::DecodedUriParameters
     * \note used by QgsVirtualRasterProvider only
     */
    struct VirtualRasterParameters
    {
      QgsCoordinateReferenceSystem crs;
      QgsRectangle extent;
      int width;
      int height;
      QString formula;
      QList <QgsRasterDataProvider::VirtualRasterInputLayers> rInputLayers;

    };

    /**
     * Decodes the URI returning a struct with all the parameters for QgsVirtualRasterProvider class
     * \note used by Virtual Raster Provider only
     * \note since QGIS 3.22
     */
    static QgsRasterDataProvider::VirtualRasterParameters decodeVirtualRasterProviderUri( const QString &uri, bool *ok = nullptr );

    /**
     * Encodes the URI starting from the struct .
     * \note used by Virtual Raster Provider only
     * \note since QGIS 3.22
     */
    static QString encodeVirtualRasterProviderUri( const VirtualRasterParameters &parts );

    /**
     * Validates creation options for a specific dataset and destination format.
     * \note used by GDAL provider only
     * \note see also validateCreationOptionsFormat() in gdal provider for validating options based on format only
     */
    virtual QString validateCreationOptions( const QStringList &createOptions, const QString &format )
    { Q_UNUSED( createOptions ) Q_UNUSED( format ); return QString(); }

    /**
     * Validates pyramid creation options for a specific dataset and destination format
     * \note used by GDAL provider only
     */
    virtual QString validatePyramidsConfigOptions( Qgis::RasterPyramidFormat pyramidsFormat,
        const QStringList &configOptions, const QString &fileFormat )
    { Q_UNUSED( pyramidsFormat ) Q_UNUSED( configOptions ); Q_UNUSED( fileFormat ); return QString(); }

    /**
     * Converts a raster identify \a format to a string name.
     *
     * \see identifyFormatFromName()
     */
    static QString identifyFormatName( Qgis::RasterIdentifyFormat format );

    /**
     * Converts a string \a formatName to a raster identify format.
     *
     * \see identifyFormatName()
     */
    static Qgis::RasterIdentifyFormat identifyFormatFromName( const QString &formatName );

    /**
     * Converts a raster identify \a format to a translated string label.
     */
    static QString identifyFormatLabel( Qgis::RasterIdentifyFormat format );

    /**
     * Converts a raster identify \a format to a capability.
     */
    static Capability identifyFormatToCapability( Qgis::RasterIdentifyFormat format );

    /**
     * Step width for raster iterations.
     * \see stepHeight()
     * \since QGIS 3.0
     */
    virtual int stepWidth() const { return QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH; }

    /**
     * Step height for raster iterations.
     * \see stepWidth()
     * \since QGIS 3.0
     */
    virtual int stepHeight() const { return QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT; }

    /**
     * Returns a list of native resolutions if available, i.e. map units per pixel at which the raster source
     * was originally created.
     *
     * Resolutions are calculated in the provider's crs().
     *
     * \since QGIS 3.8.0
     */
    virtual QList< double > nativeResolutions() const;

    /**
     * Returns TRUE if the extents reported by the data provider are not reliable
     * and it's possible that there is renderable content outside of these extents.
     *
     * \since QGIS 3.10.0
     */
    virtual bool ignoreExtents() const;

    /**
     * Types of transformation in transformCoordinates() function.
     * \since QGIS 3.14
     */
    enum TransformType
    {
      TransformImageToLayer,  //!< Transforms image coordinates to layer (georeferenced) coordinates
      TransformLayerToImage,  //!< Transforms layer (georeferenced) coordinates to image coordinates
    };

    /**
     * Transforms coordinates between source image coordinate space [0..width]x[0..height] and
     * layer coordinate space (georeferenced coordinates). Often this transformation is a simple
     * 2D affine transformation (offset and scaling), but rasters with different georeferencing
     * methods like GCPs (ground control points) or RPCs (rational polynomial coefficients) may
     * require a more complex transform.
     *
     * If the transform fails (input coordinates are outside of the valid range or data provider
     * does not support this functionality), an empty point is returned.
     *
     * \since QGIS 3.14
     */
    virtual QgsPoint transformCoordinates( const QgsPoint &point, TransformType type );


    /**
     * Enable or disable provider-level resampling.
     *
     * \return TRUE if success
     * \since QGIS 3.16
     */
    virtual bool enableProviderResampling( bool enable ) { Q_UNUSED( enable ); return false; }

    /**
     * Returns whether provider-level resampling is enabled.
     *
     * \note Resampling is effective only if zoomedInResamplingMethod() and/or
     * zoomedOutResamplingMethod() return non-nearest resampling.
     *
     * \see zoomedInResamplingMethod()
     * \see zoomedOutResamplingMethod()
     * \see maxOversampling()
     *
     * \since QGIS 3.16
     */
    bool isProviderResamplingEnabled() const { return mProviderResamplingEnabled; }

    /**
     * Resampling method for provider-level resampling.
     * \since QGIS 3.16
     */
    enum class ResamplingMethod
    {
      Nearest, //!< Nearest-neighbour resampling
      Bilinear, //!< Bilinear (2x2 kernel) resampling
      Cubic,//!< Cubic Convolution Approximation (4x4 kernel) resampling
      CubicSpline, //!< Cubic B-Spline Approximation (4x4 kernel)
      Lanczos, //!< Lanczos windowed sinc interpolation (6x6 kernel)
      Average, //!< Average resampling
      Mode, //!< Mode (selects the value which appears most often of all the sampled points)
      Gauss //!< Gauss blurring
    };

    /**
     * Set resampling method to apply for zoomed-in operations.
     *
     * \return TRUE if success
     * \since QGIS 3.16
     */
    virtual bool setZoomedInResamplingMethod( ResamplingMethod method ) { Q_UNUSED( method ); return false; }

    /**
     * Returns resampling method for zoomed-in operations.
     * \since QGIS 3.16
     */
    ResamplingMethod zoomedInResamplingMethod() const { return mZoomedInResamplingMethod; }

    /**
     * Set resampling method to apply for zoomed-out operations.
     *
     * \return TRUE if success
     * \since QGIS 3.16
     */
    virtual bool setZoomedOutResamplingMethod( ResamplingMethod method ) { Q_UNUSED( method ); return false; }

    /**
     * Returns resampling method for zoomed-out operations.
     * \since QGIS 3.16
     */
    ResamplingMethod zoomedOutResamplingMethod() const { return mZoomedOutResamplingMethod; }

    /**
     * Sets maximum oversampling factor for zoomed-out operations.
     *
     * \return TRUE if success
     * \since QGIS 3.16
     */
    virtual bool setMaxOversampling( double factor ) { Q_UNUSED( factor ); return false; }

    /**
     * Returns maximum oversampling factor for zoomed-out operations.
     * \since QGIS 3.16
     */
    double maxOversampling() const { return mMaxOversampling; }

    void readXml( const QDomElement &filterElem ) override;

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    /**
     * Returns the (possibly NULL) attribute table for the specified \a bandNumber.
     *
     * \since QGIS 3.30
     */
    QgsRasterAttributeTable *attributeTable( int bandNumber ) const;

    /**
     * Set the attribute table to \a attributeTable for the specified \a bandNumber,
     * if the \a attributeTable is NULL any existing attribute table for the specified
     * band will be removed.
     *
     * \note Ownership of the attribute table is transferred to the provider.
     * \since QGIS 3.30
     */
    void setAttributeTable( int bandNumber, QgsRasterAttributeTable *attributeTable SIP_TRANSFER );

    /**
     * Remove the attribute table for the specified \a bandNumber.
     * If the attribute table does not exist this method does nothing.
     *
     * \since QGIS 3.30
     */
    void removeAttributeTable( int bandNumber );

    /**
     * Writes the filesystem-based attribute table for the specified \a bandNumber to \a path, optionally reporting any error in \a errorMessage, returns TRUE on success.
     *
     * \returns TRUE on success
     * \note No checks for Raster Attribute Table validity are performed when saving, it is client code responsibility to handle validation.
     * \since QGIS 3.30
     */
    bool writeFileBasedAttributeTable( int bandNumber, const QString &path, QString *errorMessage SIP_OUT = nullptr ) const;

    /**
     * Loads the filesystem-based attribute table for the specified \a bandNumber from \a path, optionally reporting any error in \a errorMessage, returns TRUE on success.
     *
     * \returns TRUE on success
     * \since QGIS 3.30
     */
    bool readFileBasedAttributeTable( int bandNumber, const QString &path, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Writes the native attribute table, optionally reporting any error in \a errorMessage, returns TRUE on success.
     * The default implementation does nothing and returns FALSE.
     * Data providers that have NativeRasterAttributeTable
     * provider capability will try to save the native attribute table.
     *
     * \returns TRUE on success
     * \note No checks for Raster Attribute Table validity are performed when saving, it is client code responsibility to handle validation.
     * \since QGIS 3.30
     */
    virtual bool writeNativeAttributeTable( QString *errorMessage SIP_OUT = nullptr );  //#spellok

    /**
     * Reads the native attribute table, optionally reporting any error in \a errorMessage, returns TRUE on success.
     * The default implementation does nothing and returns FALSE.
     * Data providers that have NativeRasterAttributeTable provider capability will try to read the native attribute table.
     *
     * \returns TRUE on success
     * \since QGIS 3.30
     */
    virtual bool readNativeAttributeTable( QString *errorMessage SIP_OUT  = nullptr );


  signals:

    /**
     * Emit a message to be displayed on status bar, usually used by network providers (WMS,WCS)
     * \since QGIS 2.14
     */
    void statusChanged( const QString & ) const;


  protected:

    /**
     * Reads a block of raster data into \a data.
     * \returns TRUE if the block was successfully read, or FALSE if an error occurred and the block could not be read.
     * \note not available in Python bindings
     */
    virtual bool readBlock( int bandNo, int xBlock, int yBlock, void *data ) SIP_SKIP
    { Q_UNUSED( bandNo ) Q_UNUSED( xBlock ); Q_UNUSED( yBlock ); Q_UNUSED( data ); return false; }

    /**
     * Reads a block of raster data into \a data, using the given extent and size.
     * \returns TRUE if the block was successfully read, or FALSE if an error occurred and the block could not be read.
     * \note not available in Python bindings
     */
    virtual bool readBlock( int bandNo, QgsRectangle  const &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) SIP_SKIP
    { Q_UNUSED( bandNo ) Q_UNUSED( viewExtent ); Q_UNUSED( width ); Q_UNUSED( height ); Q_UNUSED( data ); Q_UNUSED( feedback ); return false; }

    //! Returns TRUE if user no data contains value
    bool userNoDataValuesContains( int bandNo, double value ) const;

    //! Copy member variables from other raster data provider. Useful for implementation of clone() method in subclasses
    void copyBaseSettings( const QgsRasterDataProvider &other );

    /**
     * Dots per inch. Extended WMS (e.g. QGIS mapserver) support DPI dependent output and therefore
     * are suited for printing. A value of -1 means it has not been set
    */
    int mDpi = -1;

    /**
     * Source no data value is available and is set to be used or internal no data
     *  is available. Used internally only
    */
    //bool hasNoDataValue ( int bandNo );

    //! \brief Cell value representing original source no data. e.g. -9999, indexed from 0
    QList<double> mSrcNoDataValue;

    //! \brief Source no data value exists.
    QList<bool> mSrcHasNoDataValue;

    /**
     * \brief Use source nodata value. User can disable usage of source nodata
     *  value as nodata. It may happen that a value is wrongly given by GDAL
     *  as nodata (e.g. 0) and it has to be treated as regular value.
    */
    QList<bool> mUseSrcNoDataValue;

    /**
     * \brief List of lists of user defined additional no data values
     *  for each band, indexed from 0
    */
    QList< QgsRasterRangeList > mUserNoDataValue;

    mutable QgsRectangle mExtent;

    //! Whether provider resampling is enabled.
    bool mProviderResamplingEnabled = false;

    //! Resampling method for zoomed in pixel extraction
    ResamplingMethod mZoomedInResamplingMethod = ResamplingMethod::Nearest;

    //! Resampling method for zoomed out pixel extraction
    ResamplingMethod mZoomedOutResamplingMethod = ResamplingMethod::Nearest;

    //! Maximum boundary for oversampling (to avoid too much data traffic). Default: 2.0
    double mMaxOversampling = 2.0;

  private:

    /**
     * Data provider temporal properties
     */
    std::unique_ptr< QgsRasterDataProviderTemporalCapabilities > mTemporalCapabilities;

    std::map<int, std::unique_ptr<QgsRasterAttributeTable>> mAttributeTables;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsRasterDataProvider::ProviderCapabilities )

// clazy:excludeall=qstring-allocations

#endif
