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

#include "qgserror.h"
#include "qgswcscapabilities.h"
#include "qgsrasterdataprovider.h"
#include "qgsgdalproviderbase.h"
#include "qgsrectangle.h"

#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QHash>
#include <QMap>
#include <QNetworkRequest>
#include <QVector>
#include <QUrl>

class QgsCoordinateTransform;
class QgsNetworkAccessManager;
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

#define CPL_SUPRESS_CPLUSPLUS
#include <gdal.h>
#include "cpl_conv.h"

// TODO: merge with QgsWmsAuthorization?
struct QgsWcsAuthorization
{
  QgsWcsAuthorization( const QString& userName = QString(), const QString& password = QString() ) : mUserName( userName ), mPassword( password ) {}

  //! set authorization header
  void setAuthorization( QNetworkRequest &request ) const
  {
    if ( !mUserName.isNull() || !mPassword.isNull() )
    {
      request.setRawHeader( "Authorization", "Basic " + QString( "%1:%2" ).arg( mUserName ).arg( mPassword ).toAscii().toBase64() );
    }
  }

  //! Username for basic http authentication
  QString mUserName;

  //! Password for basic http authentication
  QString mPassword;
};

/**

  \brief Data provider for OGC WCS layers.

  This provider implements the
  interface defined in the QgsDataProvider class to provide access to spatial
  data residing in a OGC Web Map Service.

*/
class QgsWcsProvider : public QgsRasterDataProvider, QgsGdalProviderBase
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

    QgsRasterInterface * clone() const override;

    /*! Get the QgsCoordinateReferenceSystem for this layer
     * @note Must be reimplemented by each provider.
     * If the provider isn't capable of returning
     * its projection an empty srs will be return, ti will return 0
     */
    virtual QgsCoordinateReferenceSystem crs() override;

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
    QImage *draw( QgsRectangle const &  viewExtent, int pixelWidth, int pixelHeight ) override;

    void readBlock( int bandNo, QgsRectangle  const & viewExtent, int width, int height, void *data ) override;

    void readBlock( int theBandNo, int xBlock, int yBlock, void *block ) override;

    /** Download cache */
    void getCache( int bandNo, QgsRectangle  const & viewExtent, int width, int height, QString crs = "" );

    /** Return the extent for this data layer
    */
    virtual QgsRectangle extent() override;

    /**Returns true if layer is valid
     */
    bool isValid() override;

    /**Returns the base url
     */
    virtual QString baseUrl() const;

    //! get WCS version string
    QString wcsVersion();

    // Reimplemented QgsRasterDataProvider virtual methods
    int capabilities() const override;
    QGis::DataType dataType( int bandNo ) const override;
    QGis::DataType srcDataType( int bandNo ) const override;
    int bandCount() const override;
    //double noDataValue() const;
    int xBlockSize() const override;
    int yBlockSize() const override;
    int xSize() const override;
    int ySize() const override;
    QString metadata() override;
    QgsRasterIdentifyResult identify( const QgsPoint & thePoint, QgsRaster::IdentifyFormat theFormat, const QgsRectangle &theExtent = QgsRectangle(), int theWidth = 0, int theHeight = 0 ) override;
    QString lastErrorTitle() override;
    QString lastError() override;
    QString lastErrorFormat() override;
    QString name() const override;
    QString description() const override;
    void reloadData() override;
    QList<QgsColorRampShader::ColorRampItem> colorTable( int bandNo )const override;

    static QMap<QString, QString> supportedMimes();

    /**
     * \brief parse the full WCS ServiceExceptionReport XML document
     *
     * \note errorTitle and errorText are updated to suit the results of this function.
     */
    static bool parseServiceExceptionReportDom( QByteArray const &xml, const QString& wcsVersion, QString& errorTitle, QString& errorText );


  signals:

    /** \brief emit a signal to notify of a progress event */
    void progressChanged( int theProgress, int theTotalSteps );

    /** \brief emit a signal to be caught by qgisapp and display a msg on status bar */
    void statusChanged( QString const &  theStatusQString );

    void dataChanged();

  private:
    void showMessageBox( const QString& title, const QString& text );

    // case insensitive attribute value lookup
    static QString nodeAttribute( const QDomElement &e, QString name, QString defValue = QString::null );

    //! parse the WCS ServiceException XML element
    static void parseServiceException( QDomElement const &e, const QString& wcsVersion, QString& errorTitle, QString& errorText );

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
     */

    bool parseUri( QString uri );

    /**
     * \brief Prepare the URI so that we can later simply append param=value
     * \param uri uri to prepare
     * \retval prepared uri
     */
    QString prepareUri( QString uri ) const;

    QString coverageMetadata( const QgsWcsCoverageSummary& c );

    //! remove query item and replace it with a new value
    void setQueryItem( QUrl &url, QString key, QString value );

    //! Release cache resources
    void clearCache();

    //! Create html cell (used by metadata)
    QString htmlCell( const QString &text );

    //! Create html row with 2 cells (used by metadata)
    QString htmlRow( const QString &text1, const QString &text2 );

    //! Data source URI of the WCS for this layer
    QString mHttpUri;

    //! URL part of URI (httpuri)
    QString mBaseUrl;

    //! Identifier / coverage / layer name
    QString mIdentifier;

    //! Time (temporalDomain), optional
    QString mTime;

    //! Format of coverage to be used in request
    QString mFormat;

    /**
     * Flag indicating if the layer data source is a valid WCS layer
     */
    bool mValid;

    /** Server capabilities */
    QgsWcsCapabilities mCapabilities;

    /** Coverage summary */
    QgsWcsCoverageSummary mCoverageSummary;

    /** Spatial reference id of the layer */
    QString mSrid;

    /** Rectangle that contains the extent (bounding box) of the layer */
    QgsRectangle mCoverageExtent;

    /** Coverage width, may be 0 if it could not be found in DescribeCoverage */
    int mWidth;

    /** Coverage width, may be 0 if it could not be found in DescribeCoverage */
    int mHeight;

    /** Block size */
    int mXBlockSize;
    int mYBlockSize;

    /** Flag if size was parsed successfully */
    bool mHasSize;

    /** Number of bands */
    int mBandCount;

    /** \brief Gdal data types used to represent data in in QGIS,
               may be longer than source data type to keep nulls
               indexed from 0 */
    QList<GDALDataType> mGdalDataType;

    /** GDAL source data types, indexed from 0 */
    QList<GDALDataType> mSrcGdalDataType;

    /** \brief Cell value representing no data. e.g. -9999, indexed from 0  */
    //QList<double> mNoDataValue;

    /** Color tables indexed from 0 */
    QList< QList<QgsColorRampShader::ColorRampItem> > mColorTables;

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

    /** Coverage CRS used for requests in Auth */
    // TODO: use QgsCoordinateReferenceSystem ?
    QString mCoverageCrs;

    /** Cached data */
    QByteArray mCachedData;

    /** Name of memory file for cached data */
    QString mCachedMemFilename;

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
    VSILFILE * mCachedMemFile;
#else
    FILE * mCachedMemFile;
#endif

    /** Pointer to cached GDAL dataset */
    GDALDatasetH mCachedGdalDataset;

    /** Current cache error last getCache() error. */
    QgsError mCachedError;

    /** The previous parameters to draw(). */
    QgsRectangle mCachedViewExtent;
    int mCachedViewWidth;
    int mCachedViewHeight;

    /** Maximum width and height of getmap requests */
    int mMaxWidth;
    int mMaxHeight;

    /** The error caption associated with the last WCS error. */
    QString mErrorCaption;

    /** The error message associated with the last WCS error. */
    QString mError;


    /** The mime type of the message */
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

    //! Errors counter
    int mErrors;

    //! http authorization details
    QgsWcsAuthorization mAuth;

    //! whether to use hrefs from GetCapabilities (default) or
    // the given base urls for GetMap and GetFeatureInfo
    bool mIgnoreGetCoverageUrl;
    bool mIgnoreAxisOrientation;
    bool mInvertAxisOrientation;

    QgsCoordinateReferenceSystem mCrs;

    // Fix for servers using bbox 1 px bigger
    bool mFixBox;

    // Fix for rasters rotated by GeoServer
    bool mFixRotate;

    QNetworkRequest::CacheLoadControl mCacheLoadControl;

};

/** Handler for downloading of coverage data - output is written to mCachedData */
class QgsWcsDownloadHandler : public QObject
{
    Q_OBJECT
  public:
    QgsWcsDownloadHandler( const QUrl& url, QgsWcsAuthorization& auth, QNetworkRequest::CacheLoadControl cacheLoadControl, QByteArray& cachedData, const QString& wcsVersion, QgsError& cachedError );
    ~QgsWcsDownloadHandler();

    void blockingDownload();

  protected slots:
    void cacheReplyFinished();
    void cacheReplyProgress( qint64, qint64 );

  protected:
    void finish() { QMetaObject::invokeMethod( mEventLoop, "quit", Qt::QueuedConnection ); }

    QgsNetworkAccessManager* mNAM;
    QEventLoop* mEventLoop;

    QNetworkReply* mCacheReply;

    QByteArray& mCachedData;
    QString mWcsVersion;
    QgsError& mCachedError;

    static int sErrors; // this should be ideally per-provider...?
};


#endif

// ENDS
