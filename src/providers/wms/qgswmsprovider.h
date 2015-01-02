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
#include "qgsnetworkreplyparser.h"
#include "qgswmscapabilities.h"

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

    QgsWmsLegendDownloadHandler( QgsNetworkAccessManager& networkAccessManager, const QgsWmsSettings& settings, const QUrl& url );
    ~QgsWmsLegendDownloadHandler( );

    // Make sure to connect to "finish" before starting
    void start();

  private:

    // Make sure to connect to "finish" before starting
    void startUrl( const QUrl& url );

    // Delete reply (later), emit error and finish with empty image
    void sendError( const QString& msg );
    // Delete reply (later), emit finish
    void sendSuccess( const QImage& img );

    QgsNetworkAccessManager& mNetworkAccessManager;
    const QgsWmsSettings& mSettings;
    QNetworkReply* mReply;
    QSet<QUrl> mVisitedUrls;
    QUrl mInitialUrl;

  private slots:

    void errored( QNetworkReply::NetworkError code );
    void finished();
    void progressed( qint64, qint64 );
};

class QgsCachedImageFetcher: public QgsImageFetcher
{
    Q_OBJECT;
  public:
    QgsCachedImageFetcher( const QImage& img );
    virtual ~QgsCachedImageFetcher();
    virtual void start();
  private:
    const QImage _img; // copy is intentional
  private slots:
    void send()
    {
      QgsDebugMsg( QString( "XXX Sending %1x%2 image" ).arg( _img.width() ).arg( _img.height() ) );
      emit finish( _img );
    }
};


/**

  \brief Data provider for OGC WMS layers.

  This provider implements the
  interface defined in the QgsDataProvider class to provide access to spatial
  data residing in a OGC Web Map Service.

*/
class QgsWmsProvider : public QgsRasterDataProvider
{
    Q_OBJECT

  public:
    /**
    * Constructor for the provider.
    *
    * \param   uri   HTTP URL of the Web Server.  If needed a proxy will be used
    *                otherwise we contact the host directly.
    * \param   capabilities   Optionally existing parsed capabilities for the given URI
    *
    */
    QgsWmsProvider( QString const & uri = 0, const QgsWmsCapabilities* capabilities = 0 );

    //! Destructor
    virtual ~QgsWmsProvider();

    QgsRasterInterface * clone() const;


    /*! Get the QgsCoordinateReferenceSystem for this layer
     * @note Must be reimplemented by each provider.
     * If the provider isn't capable of returning
     * its projection an empty srs will be return, ti will return 0
     */
    virtual QgsCoordinateReferenceSystem crs();

    /**
     * Reorder the list of WMS layer names to be rendered by this server
     * (in order from bottom to top)
     * \note   layers must have been previously added.
     */
    virtual void setLayerOrder( QStringList const & layers );

    /**
     * Set the visibility of the given sublayer name
     */
    virtual void setSubLayerVisibility( const QString &name, bool vis );

    /**
     * Set the name of the connection for use in authentication where required
     */
    void setConnectionName( QString const & connName );

    // TODO: Document this better.
    /** \brief   Renders the layer as an image
     *
     *  \return  A QImage - if the attempt to retrieve data for the draw was unsuccessful, returns 0
     *           and more information can be found in lastError() and lastErrorTitle()
     *
     *  \todo    Add pixel depth parameter (intended to match the display or printer device)
     *
     *  \note    Ownership of the returned QImage remains with this provider and its lifetime
     *           is guaranteed only until the next call to draw() or destruction of this provider.
     *
     *  \warning A pointer to an QImage is used, as a plain QImage seems to have difficulty being
     *           shared across library boundaries
     */
    QImage *draw( QgsRectangle const &  viewExtent, int pixelWidth, int pixelHeight );

    void readBlock( int bandNo, QgsRectangle  const & viewExtent, int width, int height, void *data );
    //void readBlock( int bandNo, QgsRectangle  const & viewExtent, int width, int height, QgsCoordinateReferenceSystem theSrcCRS, QgsCoordinateReferenceSystem theDestCRS, void *data );


    /** Return the extent for this data layer
    */
    virtual QgsRectangle extent();

    /**Returns true if layer is valid
     */
    bool isValid();

#if 0
    /**Returns true if layer has tile set profiles
     */
    virtual bool hasTiles() const;
#endif

    /**Returns the GetMap url */
    virtual QString getMapUrl() const;

    /**Returns the GetFeatureInfo url */
    virtual QString getFeatureInfoUrl() const;

    /**Return the GetTile url */
    virtual QString getTileUrl() const;

    /**Return the GetLegendGraphic url
     * @added in 2.1
     */
    virtual QString getLegendGraphicUrl() const;

    //! get WMS version string
    QString wmsVersion();

    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used to abstract the way the WMS server can combine
     * layers in some way at the server, before it serves them to this
     * WMS client.
     */
    QStringList subLayers() const;

    /**
     * Sub-layer styles for each sub-layer handled by this provider,
     * in order from bottom to top
     *
     * Sub-layer styles are used to abstract the way the WMS server can symbolise
     * layers in some way at the server, before it serves them to this
     * WMS client.
     */
    QStringList subLayerStyles() const;


    /**
     * \brief Get GetLegendGraphic if service is available otherwise QImage()
     * \note the first call needs to specify a scale parameter otherwise it always return QImage()
     * \todo some services don't expose GetLegendGraphic in capabilities, but add a LegendURL in
     * the layer element inside capabilities. Parsing for this is not implemented => getLegendGraphic is
     * only called if GetCapabilities expose it. Other drawback is that SLD_VERSION
     * is inside LegendURL, so at this moment it is fixed to 1.1.0 waiting a correct parsing of LegendURL
     * in getCapability
     * \param scale Optional parameter that is the Scale of the wms layer
     * \param forceRefresh Optional bool parameter to force refresh getLegendGraphic call
     * \param visibleExtent Visible extent for providers supporting contextual legends
     *
     * \note visibleExtent parameter added in 2.8
     */
    QImage getLegendGraphic( double scale = 0.0, bool forceRefresh = false, const QgsRectangle * visibleExtent = 0 );

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
    virtual QgsImageFetcher* getLegendGraphicFetcher( const QgsMapSettings* mapSettings );

    // TODO: Get the WMS connection

    // TODO: Get the table name associated with this provider instance

    /** Returns a bitmask containing the supported capabilities
        Note, some capabilities may change depending on which
        sublayers are visible on this provider, so it may
        be prudent to check this value per intended operation.
      */
    int capabilities() const;

    /** Server identify capabilities, used by source select. */
    int identifyCapabilities() const;

    QGis::DataType dataType( int bandNo ) const;
    QGis::DataType srcDataType( int bandNo ) const;
    int bandCount() const;

    /**
     * Get metadata in a format suitable for feeding directly
     * into a subset of the GUI raster properties "Metadata" tab.
     */
    QString metadata();

    QgsRasterIdentifyResult identify( const QgsPoint & thePoint, QgsRaster::IdentifyFormat theFormat, const QgsRectangle &theExtent = QgsRectangle(), int theWidth = 0, int theHeight = 0 );

    /**
     * \brief   Returns the caption error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    QString lastErrorTitle();

    /**
     * \brief   Returns the verbose error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    QString lastError();

    /**
     * \brief   Returns the format of the error message (text or html)
     */
    QString lastErrorFormat();

    /** return a provider name

    Essentially just returns the provider key.  Should be used to build file
    dialogs so that providers can be shown with their supported types. Thus
    if more than one provider supports a given format, the user is able to
    select a specific provider to open that file.

    @note

    Instead of being pure virtual, might be better to generalize this
    behavior and presume that none of the sub-classes are going to do
    anything strange with regards to their name or description?

    */
    QString name() const;


    /** return description

    Return a terse string describing what the provider is.

    @note

    Instead of being pure virtual, might be better to generalize this
    behavior and presume that none of the sub-classes are going to do
    anything strange with regards to their name or description?

    */
    QString description() const;

    /**Reloads the data from the source. Needs to be implemented by providers with data caches to
      synchronize with changes in the data source*/
    virtual void reloadData();

    static QVector<QgsWmsSupportedFormat> supportedFormats();

    static void showMessageBox( const QString& title, const QString& text );

    /**
     * \brief parse the full WMS ServiceExceptionReport XML document
     *
     * \note errorTitle and errorText are updated to suit the results of this function. Format of error is plain text.
     */
    static bool parseServiceExceptionReportDom( QByteArray const &xml, QString& errorTitle, QString& errorText );

    /**
     * \brief Prepare the URI so that we can later simply append param=value
     * \param uri uri to prepare
     * \retval prepared uri
     */
    static QString prepareUri( QString uri );

  signals:

    /** \brief emit a signal to notify of a progress event */
    void progressChanged( int theProgress, int theTotalSteps );

    /** \brief emit a signal to be caught by qgisapp and display a msg on status bar */
    void statusChanged( QString const &  theStatusQString );

    void dataChanged();

  private slots:
    void identifyReplyFinished();
    void getLegendGraphicReplyFinished( const QImage& );
    void getLegendGraphicReplyProgress( qint64, qint64 );

  private:

    /**
     * Try to get best extent for the layer in given CRS. Returns true on success, false otherwise (layer not found, invalid CRS, transform failed)
     */
    bool extentForNonTiledLayer( const QString& layerName, const QString& crs, QgsRectangle& extent );

    // case insensitive attribute value lookup
    static QString nodeAttribute( const QDomElement &e, QString name, QString defValue = QString::null );

    /**
     * Add the list of WMS layer names to be rendered by this server
     */
    void addLayers();

    /**
     * Set the image projection (in WMS CRS format) used in the transfer from the WMS server
     *
     * \note an empty crs value will result in the previous CRS being retained.
     */
    bool setImageCrs( QString const & crs );

    /**
     * \brief Retrieve and parse the (cached) Capabilities document from the server
     *
     * \param forceRefresh  if true, ignores any previous response cached in memory
     *                      and always contact the server for a new copy.
     * \retval false if the capabilities document could not be retrieved or parsed -
     *         see lastError() for more info
     *
     * When this returns, "layers" will make sense.
     *
     * TODO: Make network-timeout tolerant
     */
    bool retrieveServerCapabilities( bool forceRefresh = false );

    //! parse the WMS ServiceException XML element
    static void parseServiceException( QDomElement const &e, QString& errorTitle, QString& errorText );

    void parseOperationMetadata( QDomElement const &e );


    /**
     * \brief Calculates the combined extent of the layers selected by layersDrawn
     *
     * \retval false if the capabilities document could not be retrieved or parsed -
     *         see lastError() for more info
     */
    bool calculateExtent();

    /* \brief Bounding box in WMS format
     *
     * \note it does not perform any escape
     */
    QString toParamValue( const QgsRectangle& rect, bool changeXY = false );

    /* \brief add SRS or CRS parameter */
    void setSRSQueryItem( QUrl& url );

  private:

    /**Return the full url to request legend graphic
     * The visibleExtent isi only used if provider supports contextual
     * legends according to the QgsWmsSettings
     * @added in 2.8
     */
    QUrl getLegendGraphicFullURL( double scale, const QgsRectangle& visibleExtent );

    //QStringList identifyAs( const QgsPoint &point, QString format );

    QString layerMetadata( QgsWmsLayerProperty &layer );

    //! remove query item and replace it with a new value
    void setQueryItem( QUrl &url, QString key, QString value );

    //! add image FORMAT parameter to url
    void setFormatQueryItem( QUrl &url );

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
    QgsRectangle mLayerExtent;

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
    double mGetLegendGraphicScale;

    QgsRectangle mGetLegendGraphicExtent;

    QScopedPointer<QgsImageFetcher> mLegendGraphicFetcher;

    /**
     * Visibility status of the given active sublayer
     */
    QMap<QString, bool> mActiveSubLayerVisibility;

    /**
     * WMS CRS type of the image CRS used from the WMS server
     */
    QString mImageCrs;

    /**
     * The previously retrieved image from the WMS server.
     * This can be reused if draw() is called consecutively
     * with the same parameters.
     */
    QImage *mCachedImage;

    /**
     * The reply to the capabilities request
     */
    QNetworkReply *mIdentifyReply;

    /**
     * The result of the identify reply
     */
    //QString mIdentifyResult;
    QList< QgsNetworkReplyParser::RawHeaderMap > mIdentifyResultHeaders;
    QList<QByteArray> mIdentifyResultBodies;

    // TODO: better
    QString mIdentifyResultXsd;

    /**
     * The previous parameters to draw().
     */
    QgsRectangle mCachedViewExtent;
    int mCachedViewWidth;
    int mCachedViewHeight;

    /**
     * The error caption associated with the last WMS error.
     */
    QString mErrorCaption;

    /**
     * The error message associated with the last WMS error.
     */
    QString mError;


    /** The mime type of the message
     */
    QString mErrorFormat;

    //! A QgsCoordinateTransform is used for transformation of WMS layer extents
    QgsCoordinateTransform *mCoordinateTransform;

    //! See if calculateExtents() needs to be called before extent() returns useful data
    bool mExtentDirty;

    QString mServiceMetadataURL;

    //! tile request number, cache hits and misses
    int mTileReqNo;

    //! chosen tile layer
    QgsWmtsTileLayer        *mTileLayer;
    //! chosen matrix set
    QgsWmtsTileMatrixSet    *mTileMatrixSet;

    //! supported formats for GetFeatureInfo in order of preference
    QStringList mSupportedGetFeatureFormats;

    QgsCoordinateReferenceSystem mCrs;

    //! Parsed response of server's capabilities - initially (or on error) may be invalid
    QgsWmsCapabilities mCaps;

    //! User's settings (URI, authorization, layer, style, ...)
    QgsWmsSettings mSettings;
};


/** Handler for downloading of non-tiled WMS requests, the data are written to the given image */
class QgsWmsImageDownloadHandler : public QObject
{
    Q_OBJECT
  public:
    QgsWmsImageDownloadHandler( const QString& providerUri, const QUrl& url, const QgsWmsAuthorization& auth, QImage* image );
    ~QgsWmsImageDownloadHandler();

    void downloadBlocking();

  protected slots:
    void cacheReplyFinished();
    void cacheReplyProgress( qint64 bytesReceived, qint64 bytesTotal );

  protected:
    void finish() { QMetaObject::invokeMethod( mEventLoop, "quit", Qt::QueuedConnection ); }

    QString mProviderUri;

    QNetworkReply* mCacheReply;
    QImage* mCachedImage;

    QEventLoop* mEventLoop;
    QgsNetworkAccessManager* mNAM;
};


/** Handler for tiled WMS-C/WMTS requests, the data are written to the given image */
class QgsWmsTiledImageDownloadHandler : public QObject
{
    Q_OBJECT
  public:

    struct TileRequest
    {
      TileRequest( const QUrl& u, const QRectF& r, int i ) : url( u ), rect( r ), index( i ) {}
      QUrl url;
      QRectF rect;
      int index;
    };

    QgsWmsTiledImageDownloadHandler( const QString& providerUri, const QgsWmsAuthorization& auth, int reqNo, const QList<TileRequest>& requests, QImage* cachedImage, const QgsRectangle& cachedViewExtent, bool smoothPixmapTransform );
    ~QgsWmsTiledImageDownloadHandler();

    void downloadBlocking();

  protected slots:
    void tileReplyFinished();

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

    QgsWmsAuthorization mAuth;

    QImage* mCachedImage;
    QgsRectangle mCachedViewExtent;

    QEventLoop* mEventLoop;
    QgsNetworkAccessManager* mNAM;

    int mTileReqNo;
    bool mSmoothPixmapTransform;

    //! Running tile requests
    QList<QNetworkReply*> mReplies;
};


/** Class keeping simple statistics for WMS provider - per unique URI */
class QgsWmsStatistics
{
  public:
    struct Stat
    {
      Stat() : errors( 0 ), cacheHits( 0 ), cacheMisses( 0 ) {}
      int errors;
      int cacheHits;
      int cacheMisses;
    };

    //! get reference to layer's statistics - insert to map if does not exist yet
    static Stat& statForUri( const QString& uri ) { return sData[uri]; }

  protected:
    static QMap<QString, Stat> sData;
};


#endif

// ENDS
