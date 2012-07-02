/***************************************************************************
      qgswcsprovider.h  -  QGIS Data provider for
                           OGC Web Coverage Service layers
                             -------------------
    begin                : 2 July, 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail.com

    Based on qgswmsprovider.h written by Brendan Morley.

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWCSPROVIDER_H
#define QGSWCSPROVIDER_H

#include "qgswcscapabilities.h"
#include "qgsrasterdataprovider.h"
#include "qgsrectangle.h"

#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QHash>
#include <QMap>
#include <QVector>
#include <QUrl>

class QgsCoordinateTransform;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

/**

  \brief Data provider for OGC WCS layers.

  This provider implements the
  interface defined in the QgsDataProvider class to provide access to spatial
  data residing in a OGC Web Map Service.

*/
class QgsWcsProvider : public QgsRasterDataProvider
{
    Q_OBJECT

  public:
    /**
    * Constructor for the provider.
    *
    * \param   uri   HTTP URL of the Web Server.  If needed a proxy will be used
    *                otherwise we contact the host directly.
    *
    */
    QgsWcsProvider( QString const & uri = 0 );

    //! Destructor
    virtual ~QgsWcsProvider();

    /*! Get the QgsCoordinateReferenceSystem for this layer
     * @note Must be reimplemented by each provider.
     * If the provider isn't capable of returning
     * its projection an empty srs will be return, ti will return 0
     */
    virtual QgsCoordinateReferenceSystem crs();

    /**
     * Get the coverage format used in the transfer from the WCS server
     */
    QString format() const;

    /**
     * Set the coverage format used in the transfer from the WCS server
     */
    void setFormat( QString const & format );

    /**
     * Set the image projection (in WCS CRS format) used in the transfer from the WCS server
     *
     * \note an empty crs value will result in the previous CRS being retained.
     */
    void setCoverageCrs( QString const & crs );

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

    /** Return the extent for this data layer
    */
    virtual QgsRectangle extent();

    /**Returns true if layer is valid
     */
    bool isValid();

    /**Returns the base url
     */
    virtual QString baseUrl() const;

    //! get WCS version string
    QString wcsVersion();

    // Reimplemented QgsRasterDataProvider virtual methods
    int capabilities() const;
    int dataType( int bandNo ) const;
    int srcDataType( int bandNo ) const;
    int bandCount() const;
    QString metadata();
    QString identifyAsHtml( const QgsPoint& point );
    QString identifyAsText( const QgsPoint& point );
    QString lastErrorTitle();
    QString lastError();
    QString lastErrorFormat();
    QString name() const;
    QString description() const;
    void reloadData();

    // WMS specific, maybe to be removed from QgsRasterDataProvider
    void addLayers( QStringList const &layers, QStringList const &styles = QStringList() ) { Q_UNUSED( layers ); Q_UNUSED( styles ); }
    QStringList supportedImageEncodings() { return QStringList(); }
    QString imageEncoding() const { return QString(); }
    void setImageEncoding( QString const &mimeType ) { Q_UNUSED( mimeType ); }
    void setImageCrs( QString const &crs ) { Q_UNUSED( crs ); }

    static QMap<QString, QString> supportedMimes();

  signals:

    /** \brief emit a signal to notify of a progress event */
    void progressChanged( int theProgress, int theTotalSteps );

    /** \brief emit a signal to be caught by qgisapp and display a msg on status bar */
    void statusChanged( QString const &  theStatusQString );

    void dataChanged();

  private slots:
    void cacheReplyFinished();
    void cacheReplyProgress( qint64, qint64 );

  private:
    void showMessageBox( const QString& title, const QString& text );

    // case insensitive attribute value lookup
    static QString nodeAttribute( const QDomElement &e, QString name, QString defValue = QString::null );

    /**
     * \brief parse the full WCS ServiceExceptionReport XML document
     *
     * \note mErrorCaption and mError are updated to suit the results of this function.
     */
    bool parseServiceExceptionReportDom( QByteArray const &xml );

    //! parse the WCS ServiceException XML element
    void parseServiceException( QDomElement const &e );

    /**
     * \brief Calculates the combined extent of the layers selected by layersDrawn
     *
     * \retval false if the capabilities document could not be retrieved or parsed -
     *         see lastError() for more info
     */
    bool calculateExtent();

    /**
     * \brief Check for parameters in the uri,
     * stripping and saving them if present.
     *
     * \param uri uri to check
     *
     * \note added in 1.1
     */

    void parseUri( QString uri );

    /**
     * \brief Prepare the URI so that we can later simply append param=value
     * \param uri uri to prepare
     * \retval prepared uri
     */
    QString prepareUri( QString uri ) const;

    //QString layerMetadata( QgsWmsLayerProperty &layer );
    QString layerMetadata( );

    //! remove query item and replace it with a new value
    void setQueryItem( QUrl &url, QString key, QString value );

    //! set authorization header
    void setAuthorization( QNetworkRequest &request ) const;

    //! Data source URI of the WCS for this layer
    QString mHttpUri;

    //! URL part of URI (httpuri)
    QString mBaseUrl;

    //! Identifier / coverage / layer name
    QString mIdentifier;

    //! Format of coverage to be used in request
    QString mFormat;

    /**
     * Flag indicating if the layer data source is a valid WCS layer
     */
    bool mValid;

    /** Server capabilities */
    QgsWcsCapabilities mCapabilities;

    /** Coverage summary */
    QgsWcsCoverageSummary * mCoverageSummary;

    /**
     * Spatial reference id of the layer
     */
    QString mSrid;

    /**
     * Rectangle that contains the extent (bounding box) of the layer
     */
    QgsRectangle mCoverageExtent;

    /**
     * Last Service Exception Report from the WCS
     */
    QDomDocument mServiceExceptionReportDom;

    /**
     * extents per layer (in WCS CRS:84 datum)
     */
    QMap<QString, QgsRectangle> mExtentForLayer;

    /**
     * available CRSs per layer
     */
    QMap<QString, QStringList > mCrsForLayer;

    /**
     * WCS "queryable" per layer
     * Used in determining if the Identify map tool can be useful on the rendered WCS map layer.
     */
    QMap<QString, bool> mQueryableForLayer;

    /**
     * WCS CRS type of the coverage CRS requested from the WCS server
     */
    QString mCoverageCrs;

    /**
     * The previously retrieved image from the WCS server.
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
     * The reply to the capabilities request
     */
    QNetworkReply *mCapabilitiesReply;

    /**
     * The reply to the capabilities request
     */
    QNetworkReply *mIdentifyReply;

    /**
     * The result of the identify reply
     */
    QString mIdentifyResult;

    /**
     * The previous parameters to draw().
     */
    QgsRectangle mCachedViewExtent;
    int mCachedViewWidth;
    int mCachedViewHeight;

    /**
     * Maximum width and height of getmap requests
     */
    int mMaxWidth;
    int mMaxHeight;

    /**
     * The error caption associated with the last WCS error.
     */
    QString mErrorCaption;

    /**
     * The error message associated with the last WCS error.
     */
    QString mError;


    /** The mime type of the message
     */
    QString mErrorFormat;

    //! A QgsCoordinateTransform is used for transformation of WCS layer extents
    QgsCoordinateTransform *mCoordinateTransform;

    //! See if calculateExtents() needs to be called before extent() returns useful data
    bool mExtentDirty;

    //! Base URL for WCS GetFeatureInfo requests
    QString mGetFeatureInfoUrlBase;
    QString mServiceMetadataURL;

    //! number of layers and parents
    //int mLayerCount;
    //QMap<int, int> mLayerParents;
    //QMap<int, QStringList> mLayerParentNames;

    //! flag set while provider is fetching tiles synchronously
    bool mWaiting;

    //! Errors counter
    int mErrors;

    //! Username for basic http authentication
    QString mUserName;

    //! Password for basic http authentication
    QString mPassword;

    //! whether to use hrefs from GetCapabilities (default) or
    // the given base urls for GetMap and GetFeatureInfo
    bool mIgnoreGetMapUrl;
    bool mIgnoreGetFeatureInfoUrl;
    bool mIgnoreAxisOrientation;
    bool mInvertAxisOrientation;

    QgsCoordinateReferenceSystem mCrs;
};


#endif

// ENDS
