/***************************************************************************
      qgswmsprovider.h  -  QGIS Data provider for
                           OGC Web Map Service layers
                             -------------------
    begin                : 17 Mar, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSWMSPROVIDER_H
#define QGSWMSPROVIDER_H

#include "qgsrasterdataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsnetworkreplyparser.h"
#include "qgswmscapabilities.h"
#include "qgsprovidermetadata.h"

#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QHash>
#include <QMap>
#include <QVector>
#include <QUrl>

class QgsCoordinateTransform;
class QgsNetworkAccessManager;
class QgsWmsCapabilities;

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

/**
 * \class Handles asynchronous download of WMS legend
 *
 * \todo turn into a generic async image downloader ?
 *
 */
class QgsWmsLegendDownloadHandler : public QgsImageFetcher
{
    Q_OBJECT
  public:

    QgsWmsLegendDownloadHandler( QgsNetworkAccessManager &networkAccessManager, const QgsWmsSettings &settings, const QUrl &url );
    ~QgsWmsLegendDownloadHandler() override;

    // Make sure to connect to "finish" before starting
    void start() override;

  private:

    // Make sure to connect to "finish" before starting
    void startUrl( const QUrl &url );

    // Delete reply (later), emit error and finish with empty image
    void sendError( const QString &msg );
    // Delete reply (later), emit finish
    void sendSuccess( const QImage &img );

    QgsNetworkAccessManager &mNetworkAccessManager;
    const QgsWmsSettings &mSettings;
    QNetworkReply *mReply = nullptr;
    QSet<QUrl> mVisitedUrls;
    QUrl mInitialUrl;

  private slots:

    void errored( QNetworkReply::NetworkError code );
    void finished();
    void progressed( qint64, qint64 );
};

class QgsCachedImageFetcher: public QgsImageFetcher
{
    Q_OBJECT
  public:
    explicit QgsCachedImageFetcher( const QImage &img );

    void start() override;
  private:
    const QImage _img; // copy is intentional
  private slots:
    void send()
    {
      QgsDebugMsg( QStringLiteral( "XXX Sending %1x%2 image" ).arg( _img.width() ).arg( _img.height() ) );
      emit finish( _img );
    }
};

//! Abstract class to convert color to float value following an interpretation
class QgsWmsInterpretationConverter
{
  public:
    virtual ~QgsWmsInterpretationConverter() = default;

    //! Convert the \a color to a value pointed by float
    virtual void convert( const QRgb &color, float *converted ) const = 0;

    //! Returns the output datatype of this converter
    virtual Qgis::DataType dataType() const;

    //! Returns statistics related to converted values
    virtual QgsRasterBandStats statistics( int bandNo,
                                           int stats = QgsRasterBandStats::All,
                                           const QgsRectangle &extent = QgsRectangle(),
                                           int sampleSize = 0, QgsRasterBlockFeedback *feedback = nullptr ) const = 0;

    //! Returns the histogram related to converted values
    virtual QgsRasterHistogram histogram( int bandNo,
                                          int binCount = 0,
                                          double minimum = std::numeric_limits<double>::quiet_NaN(),
                                          double maximum = std::numeric_limits<double>::quiet_NaN(),
                                          const QgsRectangle &extent = QgsRectangle(),
                                          int sampleSize = 0,
                                          bool includeOutOfRange = false,
                                          QgsRasterBlockFeedback *feedback = nullptr ) const = 0;

    //! Creates a converter instance corresponding to the \a key
    static std::unique_ptr<QgsWmsInterpretationConverter> createConverter( const QString &key );
};


//! Class to convert color to float value following the mapTiler terrain RGB interpretation
class QgsWmsInterpretationConverterMapTilerTerrainRGB : public QgsWmsInterpretationConverter
{
  public:
    void convert( const QRgb &color, float *converted ) const override;

    QgsRasterBandStats statistics( int bandNo,
                                   int stats = QgsRasterBandStats::All,
                                   const QgsRectangle &extent = QgsRectangle(),
                                   int sampleSize = 0, QgsRasterBlockFeedback *feedback = nullptr ) const override;

    QgsRasterHistogram histogram( int bandNo,
                                  int binCount = 0,
                                  double minimum = std::numeric_limits<double>::quiet_NaN(),
                                  double maximum = std::numeric_limits<double>::quiet_NaN(),
                                  const QgsRectangle &extent = QgsRectangle(),
                                  int sampleSize = 0,
                                  bool includeOutOfRange = false,
                                  QgsRasterBlockFeedback *feedback = nullptr ) const override;

    static QString displayName() {return QObject::tr( "MapTiler Terrain RGB" );}
    static QString interpretationKey() {return QStringLiteral( "maptilerterrain" );}
};

//! Class to convert color to float value following the terrarium terrain RGB interpretation
class QgsWmsInterpretationConverterTerrariumRGB : public QgsWmsInterpretationConverter
{
  public:
    void convert( const QRgb &color, float *converted ) const override;

    QgsRasterBandStats statistics( int bandNo,
                                   int stats = QgsRasterBandStats::All,
                                   const QgsRectangle &extent = QgsRectangle(),
                                   int sampleSize = 0, QgsRasterBlockFeedback *feedback = nullptr ) const override;

    QgsRasterHistogram histogram( int bandNo,
                                  int binCount = 0,
                                  double minimum = std::numeric_limits<double>::quiet_NaN(),
                                  double maximum = std::numeric_limits<double>::quiet_NaN(),
                                  const QgsRectangle &extent = QgsRectangle(),
                                  int sampleSize = 0,
                                  bool includeOutOfRange = false,
                                  QgsRasterBlockFeedback *feedback = nullptr ) const override;

    static QString displayName() {return QObject::tr( "Terrarium Terrain RGB" );}
    static QString interpretationKey() {return QStringLiteral( "terrariumterrain" );}
};

/**
 *
 * \brief Data provider for OGC WMS layers.
 *
 * This provider implements the
 * interface defined in the QgsDataProvider class to provide access to spatial
 * data residing in a OGC Web Map Service.
 *
*/
class QgsWmsProvider final: public QgsRasterDataProvider
{
    Q_OBJECT

  public:

    static QString WMS_KEY;
    static QString WMS_DESCRIPTION;

    /**
     * Constructor for the provider.
     *
     * \param uri HTTP URL of the Web Server.  If needed a proxy will be used
     *                otherwise we contact the host directly.
     * \param options generic data provider options
     * \param capabilities Optionally existing parsed capabilities for the given URI
     *
     */
    QgsWmsProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, const QgsWmsCapabilities *capabilities = nullptr );


    ~QgsWmsProvider() override;

    QgsWmsProvider *clone() const override;

    QgsCoordinateReferenceSystem crs() const override;

    QgsRasterDataProvider::ProviderCapabilities providerCapabilities() const override;

    /**
     * Reorder the list of WMS layer names to be rendered by this server
     * (in order from bottom to top)
     * \note   layers must have been previously added.
     */
    void setLayerOrder( QStringList const &layers ) override;

    void setSubLayerVisibility( const QString &name, bool vis ) override;

    /**
     * Set the name of the connection for use in authentication where required
     */
    void setConnectionName( QString const &connName );

    Qgis::DataProviderFlags flags() const override;

    bool readBlock( int bandNo, QgsRectangle  const &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override;
    //void readBlock( int bandNo, QgsRectangle  const & viewExtent, int width, int height, QgsCoordinateReferenceSystem srcCRS, QgsCoordinateReferenceSystem destCRS, void *data );

    QgsRectangle extent() const override;

    bool isValid() const override;

#if 0

    /**
     * Returns true if layer has tile set profiles
     */
    virtual bool hasTiles() const;
#endif

    virtual QString getMapUrl() const;
    virtual QString getFeatureInfoUrl() const;
    virtual QString getTileUrl() const;
    virtual QString getLegendGraphicUrl() const;

    //! Gets WMS version string
    QString wmsVersion();

    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used to abstract the way the WMS server can combine
     * layers in some way at the server, before it serves them to this
     * WMS client.
     */
    QStringList subLayers() const override;

    /**
     * Sub-layer styles for each sub-layer handled by this provider,
     * in order from bottom to top
     *
     * Sub-layer styles are used to abstract the way the WMS server can symbolise
     * layers in some way at the server, before it serves them to this
     * WMS client.
     */
    QStringList subLayerStyles() const override;

    bool supportsLegendGraphic() const override { return true; }

    QImage getLegendGraphic( double scale = 0.0, bool forceRefresh = false, const QgsRectangle *visibleExtent = nullptr ) override;
    QgsImageFetcher *getLegendGraphicFetcher( const QgsMapSettings *mapSettings ) override;

    // TODO: Get the WMS connection

    // TODO: Get the table name associated with this provider instance

    int capabilities() const override;
    Qgis::DataType dataType( int bandNo ) const override;
    Qgis::DataType sourceDataType( int bandNo ) const override;
    int bandCount() const override;
    QString htmlMetadata() override;
    QgsRasterIdentifyResult identify( const QgsPointXY &point, QgsRaster::IdentifyFormat format, const QgsRectangle &boundingBox = QgsRectangle(), int width = 0, int height = 0, int dpi = 96 ) override;
    QString lastErrorTitle() override;
    QString lastError() override;
    QString lastErrorFormat() override;
    QString name() const override;
    static QString providerKey();
    QString description() const override;
    bool renderInPreview( const QgsDataProvider::PreviewContext &context ) override;
    QList< double > nativeResolutions() const override;
    QgsLayerMetadata layerMetadata() const override;
    bool enableProviderResampling( bool enable ) override { mProviderResamplingEnabled = enable; return true; }
    bool setZoomedInResamplingMethod( ResamplingMethod method ) override { mZoomedInResamplingMethod = method; return true; }
    bool setZoomedOutResamplingMethod( ResamplingMethod method ) override { mZoomedOutResamplingMethod = method; return true; }

    // Statistics could be available if the provider has a converter from colors to other value type, the returned statistics depend on the converter
    QgsRasterBandStats bandStatistics( int bandNo,
                                       int stats = QgsRasterBandStats::All,
                                       const QgsRectangle &extent = QgsRectangle(),
                                       int sampleSize = 0, QgsRasterBlockFeedback *feedback = nullptr ) override;

    QgsRasterHistogram histogram( int bandNo,
                                  int binCount = 0,
                                  double minimum = std::numeric_limits<double>::quiet_NaN(),
                                  double maximum = std::numeric_limits<double>::quiet_NaN(),
                                  const QgsRectangle &extent = QgsRectangle(),
                                  int sampleSize = 0,
                                  bool includeOutOfRange = false,
                                  QgsRasterBlockFeedback *feedback = nullptr ) override;

    static QVector<QgsWmsSupportedFormat> supportedFormats();

    static void showMessageBox( const QString &title, const QString &text );

    /**
     * \brief parse the full WMS ServiceExceptionReport XML document
     *
     * \note errorTitle and errorText are updated to suit the results of this function. Format of error is plain text.
     */
    static bool parseServiceExceptionReportDom( QByteArray const &xml, QString &errorTitle, QString &errorText );

    /**
     * \brief Prepare the URI so that we can later simply append param=value
     * \param uri uri to prepare
     * \returns prepared uri
     */
    static QString prepareUri( QString uri );

    int stepWidth() const override { return mSettings.mStepWidth; }
    int stepHeight() const override { return mSettings.mStepHeight; }

    //! Helper struct for tile requests
    struct TileRequest
    {
      TileRequest( const QUrl &u, const QRectF &r, int i )
        : url( u )
        , rect( r )
        , index( i )
      {}
      QUrl url;
      QRectF rect;
      int index;
    };
    typedef QList<TileRequest> TileRequests;

    //! Tile identifier within a tile source
    typedef struct TilePosition
    {
      TilePosition( int r, int c ): row( r ), col( c ) {}
      bool operator==( TilePosition other ) const { return row == other.row && col == other.col; }
      int row;
      int col;
    } TilePosition;
    typedef QList<TilePosition> TilePositions;

    static bool isUrlForWMTS( const QString &url );

  private slots:
    void identifyReplyFinished();
    void getLegendGraphicReplyFinished( const QImage & );
    void getLegendGraphicReplyErrored( const QString &message );
    void getLegendGraphicReplyProgress( qint64, qint64 );

  private:

    //! In case of XYZ tile layer, setup capabilities from its URI
    void setupXyzCapabilities( const QString &uri, const QgsRectangle &sourceExtent = QgsRectangle(), int sourceMinZoom = -1, int sourceMaxZoom = -1, double sourceTilePixelRatio = 0. );
    //! In case of MBTiles layer, setup capabilities from its metadata
    bool setupMBTilesCapabilities( const QString &uri );

    QImage *draw( const QgsRectangle &viewExtent, int pixelWidth, int pixelHeight, QgsRectangle &effectiveExtent, double &sourceResolution, QgsRasterBlockFeedback *feedback );

    /**
     * Try to get best extent for the layer in given CRS. Returns true on success, false otherwise (layer not found, invalid CRS, transform failed)
     */
    bool extentForNonTiledLayer( const QString &layerName, const QString &crs, QgsRectangle &extent ) const;

    // case insensitive attribute value lookup
    static QString nodeAttribute( const QDomElement &e, const QString &name, const QString &defValue = QString() );

    /**
     * Add the list of WMS layer names to be rendered by this server
     */
    bool addLayers();

    /**
     * Set the image projection (in WMS CRS format) used in the transfer from the WMS server
     *
     * \note an empty crs value will result in the previous CRS being retained.
     */
    bool setImageCrs( QString const &crs );

    /**
     * \brief Retrieve and parse the (cached) Capabilities document from the server
     *
     * \param forceRefresh  if true, ignores any previous response cached in memory
     *                      and always contact the server for a new copy.
     * \returns false if the capabilities document could not be retrieved or parsed -
     *         see lastError() for more info
     *
     * When this returns, "layers" will make sense.
     *
     * TODO: Make network-timeout tolerant
     */
    bool retrieveServerCapabilities( bool forceRefresh = false );

    //! parse the WMS ServiceException XML element
    static void parseServiceException( QDomElement const &e, QString &errorTitle, QString &errorText );

    void parseOperationMetadata( QDomElement const &e );

    /**
     * \brief Calculates the combined extent of the layers selected by layersDrawn
     *
     * \returns false if the capabilities document could not be retrieved or parsed -
     *         see lastError() for more info
     */
    bool calculateExtent() const;

    /* \brief Bounding box in WMS format
     *
     * \note it does not perform any escape
     */
    QString toParamValue( const QgsRectangle &rect, bool changeXY );

    /* \brief add SRS or CRS parameter */
    void setSRSQueryItem( QUrlQuery &url );

    bool ignoreExtents() const override;

  private:

    QUrl createRequestUrlWMS( const QgsRectangle &viewExtent, int pixelWidth, int pixelHeight );
    void createTileRequestsWMSC( const QgsWmtsTileMatrix *tm, const QgsWmsProvider::TilePositions &tiles, QgsWmsProvider::TileRequests &requests );
    void createTileRequestsWMTS( const QgsWmtsTileMatrix *tm, const QgsWmsProvider::TilePositions &tiles, QgsWmsProvider::TileRequests &requests );
    void createTileRequestsXYZ( const QgsWmtsTileMatrix *tm, const QgsWmsProvider::TilePositions &tiles, QgsWmsProvider::TileRequests &requests, QgsRasterBlockFeedback *feedback = nullptr );

    /**
      * Add WMS-T parameters to the \a query, if provider has temporal properties
      *
      * \since QGIS 3.14
      */
    void addWmstParameters( QUrlQuery &query );

    //! Helper structure to store a cached tile image with its rectangle
    typedef struct TileImage
    {
      TileImage( const QRectF &r, const QImage &i, bool smooth ): rect( r ), img( i ), smooth( smooth ) {}
      QRectF rect; //!< Destination rectangle for a tile (in screen coordinates)
      QImage img;  //!< Cached tile to be drawn
      bool smooth; //!< Whether to use antialiasing/smooth transforms when rendering tile
    } TileImage;
    //! Gets tiles from a different resolution to cover the missing areas
    void fetchOtherResTiles( QgsTileMode tileMode, const QgsRectangle &viewExtent, int imageWidth, QList<QRectF> &missing, double tres, int resOffset, QList<TileImage> &otherResTiles, QgsRasterBlockFeedback *feedback = nullptr );

    /**
     * Returns the full url to request legend graphic
     * The visibleExtent isi only used if provider supports contextual
     * legends according to the QgsWmsSettings
     * \since QGIS 2.8
     */
    QUrl getLegendGraphicFullURL( double scale, const QgsRectangle &visibleExtent );

    //QStringList identifyAs( const QgsPointXY &point, QString format );

    QString layerMetadata( QgsWmsLayerProperty &layer );

    //! remove query item and replace it with a new value
    void setQueryItem( QUrlQuery &url, const QString &key, const QString &value );

    //! add image FORMAT parameter to url
    void setFormatQueryItem( QUrlQuery &url );

    //! Name of the stored connection
    QString mConnectionName;

    /**
     * Flag indicating if the layer data source is a valid WMS layer
     */
    bool mValid;

    /**
     * Spatial reference id of the layer
     */
    QString mSrid;

    /**
     * Rectangle that contains the extent (bounding box) of the layer
     */
    mutable QgsRectangle mLayerExtent;

    /**
     * GetLegendGraphic of the WMS (raw)
     */
    QByteArray mHttpGetLegendGraphicResponse;

    /**
     * GetLegendGraphic WMS Pixmap result
     */
    QImage mGetLegendGraphicImage;

    /**
     * GetLegendGraphic scale for the WMS Pixmap result
     */
    double mGetLegendGraphicScale = 0.0;

    QgsRectangle mGetLegendGraphicExtent;

    std::unique_ptr<QgsImageFetcher> mLegendGraphicFetcher;

    //! TRUE if an error was encountered while fetching a legend graphic
    bool mLegendGraphicFetchErrored = false;

    /**
     * Visibility status of the given active sublayer
     */
    QMap<QString, bool> mActiveSubLayerVisibility;

    /**
     * WMS CRS type of the image CRS used from the WMS server
     */
    QString mImageCrs;

    /**
     * The reply to the capabilities request
     */
    QNetworkReply *mIdentifyReply = nullptr;

    /**
     * The result of the identify reply
     */
    //QString mIdentifyResult;
    QList< QgsNetworkReplyParser::RawHeaderMap > mIdentifyResultHeaders;
    QList<QByteArray> mIdentifyResultBodies;

    // TODO: better
    QString mIdentifyResultXsd;

    /**
     * The error caption associated with the last WMS error.
     */
    QString mErrorCaption;

    /**
     * The error message associated with the last WMS error.
     */
    QString mError;

    /**
     * The mime type of the message
     */
    QString mErrorFormat;

    //! See if calculateExtents() needs to be called before extent() returns useful data
    mutable bool mExtentDirty = true;

    QString mServiceMetadataURL;

    //! tile request number, cache hits and misses
    int mTileReqNo = 0;

    //! chosen tile layer
    QgsWmtsTileLayer        *mTileLayer = nullptr;
    //! chosen matrix set
    QgsWmtsTileMatrixSet    *mTileMatrixSet = nullptr;

    //! supported formats for GetFeatureInfo in order of preference
    QStringList mSupportedGetFeatureFormats;

    QgsCoordinateReferenceSystem mCrs;
    QgsLayerMetadata mLayerMetadata;

    //! Parsed response of server's capabilities - initially (or on error) may be invalid
    QgsWmsCapabilities mCaps;

    //! User's settings (URI, authorization, layer, style, ...)
    QgsWmsSettings mSettings;

    //! Temporal range member
    QgsDateTimeRange mRange;

    QList< double > mNativeResolutions;

    std::unique_ptr<QgsWmsInterpretationConverter> mConverter;

    friend class TestQgsWmsProvider;

};


//! Handler for downloading of non-tiled WMS requests, the data are written to the given image
class QgsWmsImageDownloadHandler : public QObject
{
    Q_OBJECT
  public:
    QgsWmsImageDownloadHandler( const QString &providerUri, const QUrl &url, const QgsWmsAuthorization &auth, QImage *image, QgsRasterBlockFeedback *feedback );
    ~QgsWmsImageDownloadHandler() override;

    void downloadBlocking();

  protected slots:
    void cacheReplyFinished();
    void cacheReplyProgress( qint64 bytesReceived, qint64 bytesTotal );
    void canceled();

  protected:
    void finish() { QMetaObject::invokeMethod( mEventLoop, "quit", Qt::QueuedConnection ); }

    QString mProviderUri;

    QNetworkReply *mCacheReply = nullptr;
    QImage *mCachedImage = nullptr;

    QEventLoop *mEventLoop = nullptr;

    QgsRasterBlockFeedback *mFeedback = nullptr;
};


//! Handler for tiled WMS-C/WMTS requests, the data are written to the given image
class QgsWmsTiledImageDownloadHandler : public QObject
{
    Q_OBJECT
  public:

    QgsWmsTiledImageDownloadHandler( const QString &providerUri,
                                     const QgsWmsAuthorization &auth,
                                     int reqNo,
                                     const QgsWmsProvider::TileRequests &requests,
                                     QImage *image,
                                     const QgsRectangle &viewExtent,
                                     double sourceResolution,
                                     bool resamplingEnabled,
                                     bool smoothPixmapTransform,
                                     QgsRasterBlockFeedback *feedback );
    ~QgsWmsTiledImageDownloadHandler() override;

    void downloadBlocking();

    QString error() const;

    QgsRectangle effectiveViewExtent() const;
    double sourceResolution() const;

  protected slots:
    void tileReplyFinished();
    void canceled();

  protected:

    /**
     * \brief Relaunch tile request cloning previous request parameters and managing max repeat
     *
     * \param oldRequest request to clone to generate new tile request
     *
     * request is not launched if max retry is reached. Message is logged.
     */
    void repeatTileRequest( QNetworkRequest const &oldRequest );

    void finish() { QMetaObject::invokeMethod( mEventLoop, "quit", Qt::QueuedConnection ); }

    QString mProviderUri;
    QString mBaseUrl;
    QgsWmsAuthorization mAuth;

    QImage *mImage = nullptr;
    QgsRectangle mViewExtent;

    QEventLoop *mEventLoop = nullptr;

    int mTileReqNo;
    bool mSmoothPixmapTransform;

    //! Running tile requests
    QList<QNetworkReply *> mReplies;

    QgsRasterBlockFeedback *mFeedback = nullptr;

    QString mError;

    QgsRectangle mEffectiveViewExtent;
    double mSourceResolution = -1;
    bool mResamplingEnabled = false;
};


//! Class keeping simple statistics for WMS provider - per unique URI
class QgsWmsStatistics
{
  public:
    struct Stat
    {
      Stat() = default;
      int errors = 0;
      int cacheHits = 0;
      int cacheMisses = 0;
    };

    //! Gets reference to layer's statistics - insert to map if does not exist yet
    static Stat &statForUri( const QString &uri ) { return sData[uri]; }

  protected:
    static QMap<QString, Stat> sData;
};

Q_DECLARE_TYPEINFO( QgsWmsProvider::TilePosition, Q_PRIMITIVE_TYPE );

class QgsWmsProviderMetadata final: public QgsProviderMetadata
{
  public:
    QgsWmsProviderMetadata();
    QgsWmsProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList<QgsDataItemProvider *> dataItemProviders() const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
};

#endif

// ENDS
