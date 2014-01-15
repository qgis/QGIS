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
     * \note added in 1.1
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
     * @added in 1.5
     */
    virtual bool hasTiles() const;
#endif

    /**Returns the GetMap url
     * @added in 1.5
     */
    virtual QString getMapUrl() const;

    /**Returns the GetFeatureInfo url
     * @added in 1.5
     */
    virtual QString getFeatureInfoUrl() const;

    /**Return the GetTile url
     * @added in 1.9
     */
    virtual QString getTileUrl() const;

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
     * \brief Get GetLegendGraphic if service available otherwise QImage()
     * BEWARE call it the first time specifying scale parameter otherwise it always return QImage()
     * \todo some services doesn't expose getLegendGraphic in capabilities but adds LegendURL in
     * the layer tags inside capabilities, but LegendURL parsing is still not developed => getLegendGraphic is
     * always called assuming that error means service is not available. Other drawback is that SLD_VERSION
     * is inside LegendURL, so at this moment it is fixed to 1.1.0 waiting a correct parsing of LegendURL
     * in getCapability
     * \param scale Optional parameter that is the Scale of the wms layer
     * \param forceRefresh Optional bool parameter to force refresh getLegendGraphic call
     */
    QImage getLegendGraphic( double scale = 0, bool forceRefresh = false );

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

  signals:

    /** \brief emit a signal to notify of a progress event */
    void progressChanged( int theProgress, int theTotalSteps );

    /** \brief emit a signal to be caught by qgisapp and display a msg on status bar */
    void statusChanged( QString const &  theStatusQString );

    void dataChanged();

  private slots:
    void cacheReplyFinished();
    void cacheReplyProgress( qint64, qint64 );
    void identifyReplyFinished();
    void tileReplyFinished();
    void getLegendGraphicReplyFinished();
    void getLegendGraphicReplyProgress( qint64, qint64 );

  private:
    void showMessageBox( const QString& title, const QString& text );

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
     * \brief Relaunch tile request cloning previous request parameters and managing max repeat
     *
     * \note Development funded by Regione Toscana - SITA
     *
     * \param oldRequest request to clone to generate new tile request
     *
     * request is not launched if max retry is reached. Message is logged.
     */
    void repeatTileRequest( QNetworkRequest const &oldRequest );

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

    /**
     * \brief parse the full WMS ServiceExceptionReport XML document
     *
     * \note mErrorCaption and mError are updated to suit the results of this function.
     */
    bool parseServiceExceptionReportDom( QByteArray const &xml );

    //! parse the WMS ServiceException XML element
    void parseServiceException( QDomElement const &e );

    void parseOperationMetadata( QDomElement const &e );


    /**
     * \brief Calculates the combined extent of the layers selected by layersDrawn
     *
     * \retval false if the capabilities document could not be retrieved or parsed -
     *         see lastError() for more info
     */
    bool calculateExtent();

public:
    /**
     * \brief Prepare the URI so that we can later simply append param=value
     * \param uri uri to prepare
     * \retval prepared uri
     */
    static QString prepareUri( QString uri );

private:

    //QStringList identifyAs( const QgsPoint &point, QString format );

    QString layerMetadata( QgsWmsLayerProperty &layer );

    //! remove query item and replace it with a new value
    void setQueryItem( QUrl &url, QString key, QString value );

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

    /**
     * Last Service Exception Report from the WMS
     */
    QDomDocument mServiceExceptionReportDom;

    /**
     * Visibility status of the given active sublayer
     */
    QMap<QString, bool> mActiveSubLayerVisibility;

    /**
     * MIME type of the image encoding used from the WMS server
     */
    QString mImageMimeType;

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
     * The reply to the on going request to fill the cache
     */
    QNetworkReply *mCacheReply;

    /**
     * Running tile requests
     */
    QList<QNetworkReply*> mTileReplies;

    /**
     * The reply to the GetLegendGraphic request
     */
    QNetworkReply *mGetLegendGraphicReply;

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

    //! Base URL for WMS GetFeatureInfo requests
    QString mGetFeatureInfoUrlBase;
    QString mServiceMetadataURL;

    //! tile request number, cache hits and misses
    int mTileReqNo;
    int mCacheHits;
    int mCacheMisses;
    int mErrors;

    //! chosen tile layer
    QgsWmtsTileLayer        *mTileLayer;
    //! chosen matrix set
    QgsWmtsTileMatrixSet    *mTileMatrixSet;

    //! supported formats for GetFeatureInfo in order of preference
    QStringList mSupportedGetFeatureFormats;

    QgsCoordinateReferenceSystem mCrs;

    QgsNetworkAccessManager* mNAM;

    QgsNetworkAccessManager* nam();

    //! Parsed response of server's capabilities - initially (or on error) may be invalid
    QgsWmsCapabilities mCaps;

    //! User's settings (URI, authorization, layer, style, ...)
    QgsWmsSettings mSettings;
};


#endif

// ENDS
