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
#include "qgsauthmanager.h"
#include "qgsrasterdataprovider.h"
#include "qgsgdalproviderbase.h"
#include "qgsrectangle.h"
#include "qgscoordinatetransform.h"
#include "qgsogrutils.h"
#include "qgsapplication.h"

#include "qgsprovidermetadata.h"

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

#define CPL_SUPRESS_CPLUSPLUS  //#spellok
#include <gdal.h>
#include "cpl_conv.h"

// TODO: merge with QgsWmsAuthorization?
struct QgsWcsAuthorization
{
  QgsWcsAuthorization( const QString &userName = QString(), const QString &password = QString(), const QString &authcfg = QString() )
    : mUserName( userName )
    , mPassword( password )
    , mAuthCfg( authcfg )
  {}

  //! Sets authorization header
  bool setAuthorization( QNetworkRequest &request ) const
  {
    if ( !mAuthCfg.isEmpty() )
    {
      return QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg );
    }
    else if ( !mUserName.isNull() || !mPassword.isNull() )
    {
      request.setRawHeader( "Authorization", "Basic " + QStringLiteral( "%1:%2" ).arg( mUserName, mPassword ).toLatin1().toBase64() );
    }
    return true;
  }

  //! Sets authorization reply
  bool setAuthorizationReply( QNetworkReply *reply ) const
  {
    if ( !mAuthCfg.isEmpty() )
    {
      return QgsApplication::authManager()->updateNetworkReply( reply, mAuthCfg );
    }
    return true;
  }

  //! Username for basic http authentication
  QString mUserName;

  //! Password for basic http authentication
  QString mPassword;

  //! Authentication configuration ID
  QString mAuthCfg;
};

/**
 *
 * \brief Data provider for OGC WCS layers.
 *
 * This provider implements the
 * interface defined in the QgsDataProvider class to provide access to spatial
 * data residing in a OGC Web Map Service.
 *
*/
class QgsWcsProvider final: public QgsRasterDataProvider, QgsGdalProviderBase
{
    Q_OBJECT

  public:

    static QString WCS_KEY;
    static QString WCS_DESCRIPTION;

    /**
     * Constructor for the provider.
     *
     * \param uri HTTP URL of the Web Server.  If needed a proxy will be used
     *                otherwise we contact the host directly.
     * \param options generic data provider options
     */
    explicit QgsWcsProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags );

    //! copy constructor
    explicit QgsWcsProvider( const QgsWcsProvider &other, const QgsDataProvider::ProviderOptions &providerOptions );

    ~QgsWcsProvider() override;

    QgsWcsProvider *clone() const override;

    QgsCoordinateReferenceSystem crs() const override;

    /**
     * Gets the coverage format used in the transfer from the WCS server
     */
    QString format() const;

    /**
     * Set the coverage format used in the transfer from the WCS server
     */
    void setFormat( QString const &format );

    /**
     * Set the image projection (in WCS CRS format) used in the transfer from the WCS server
     *
     * \note an empty crs value will result in the previous CRS being retained.
     */
    void setCoverageCrs( QString const &crs );

    // TODO: Document this better.

    bool readBlock( int bandNo, QgsRectangle  const &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override;

    bool readBlock( int bandNo, int xBlock, int yBlock, void *block ) override;

    //! Download cache
    void getCache( int bandNo, QgsRectangle  const &viewExtent, int width, int height, QString crs = QString(), QgsRasterBlockFeedback *feedback = nullptr ) const;

    QgsRectangle extent() const override;

    bool isValid() const override;

    /**
     * Returns the base url
     */
    virtual QString baseUrl() const;

    //! Gets WCS version string
    QString wcsVersion();

    // Reimplemented QgsRasterDataProvider virtual methods
    int capabilities() const override;
    Qgis::DataType dataType( int bandNo ) const override;
    Qgis::DataType sourceDataType( int bandNo ) const override;
    int bandCount() const override;
    //double noDataValue() const;
    int xBlockSize() const override;
    int yBlockSize() const override;
    int xSize() const override;
    int ySize() const override;
    QString htmlMetadata() override;
    QgsRasterIdentifyResult identify( const QgsPointXY &point, QgsRaster::IdentifyFormat format, const QgsRectangle &boundingBox = QgsRectangle(), int width = 0, int height = 0, int dpi = 96 ) override;
    QString lastErrorTitle() override;
    QString lastError() override;
    QString lastErrorFormat() override;
    QString name() const override;
    QString description() const override;
    QgsRasterDataProvider::ProviderCapabilities providerCapabilities() const override;
    QList<QgsColorRampShader::ColorRampItem> colorTable( int bandNo )const override;

    static QString providerKey();

    int colorInterpretation( int bandNo ) const override;

    static QMap<QString, QString> supportedMimes();

    /**
     * \brief parse the full WCS ServiceExceptionReport XML document
     *
     * \note errorTitle and errorText are updated to suit the results of this function.
     */
    static bool parseServiceExceptionReportDom( QByteArray const &xml, const QString &wcsVersion, QString &errorTitle, QString &errorText );

  private:
    // case insensitive attribute value lookup
    static QString nodeAttribute( const QDomElement &e, const QString &name, const QString &defValue = QString() );

    //! parse the WCS ServiceException XML element
    static void parseServiceException( QDomElement const &e, const QString &wcsVersion, QString &errorTitle, QString &errorText );

    /**
     * \brief Calculates the combined extent of the layers selected by layersDrawn
     *
     * \returns false if the capabilities document could not be retrieved or parsed -
     *         see lastError() for more info
     */
    bool calculateExtent() const;

    /**
     * \brief Check for parameters in the uri,
     * stripping and saving them if present.
     *
     * \param uri uri to check
     *
     */

    bool parseUri( const QString &uri );

    /**
     * \brief Prepare the URI so that we can later simply append param=value
     * \param uri uri to prepare
     * \returns prepared uri
     */
    QString prepareUri( QString uri ) const;

    QString coverageMetadata( const QgsWcsCoverageSummary &c );

    //! remove query item and replace it with a new value
    void setQueryItem( QUrl &url, const QString &key, const QString &value ) const;

    //! Release cache resources
    void clearCache() const;

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

    //! Specified bounding box
    QString mBBOX;

    //! Format of coverage to be used in request
    QString mFormat;

    /**
     * Flag indicating if the layer data source is a valid WCS layer
     */
    bool mValid;

    //! Server capabilities
    QgsWcsCapabilities mCapabilities;

    //! Coverage summary
    QgsWcsCoverageSummary mCoverageSummary;

    //! Spatial reference id of the layer
    QString mSrid;

    //! Rectangle that contains the extent (bounding box) of the layer
    mutable QgsRectangle mCoverageExtent;

    //! Coverage width, may be 0 if it could not be found in DescribeCoverage
    int mWidth = 0;

    //! Coverage width, may be 0 if it could not be found in DescribeCoverage
    int mHeight = 0;

    //! Block size
    int mXBlockSize = 0;
    int mYBlockSize = 0;

    //! Flag if size was parsed successfully
    bool mHasSize = false;

    //! Number of bands
    int mBandCount = 0;

    /**
     * \brief Gdal data types used to represent data in in QGIS,
     *          may be longer than source data type to keep nulls
     *          indexed from 0.
     */
    QList<GDALDataType> mGdalDataType;

    //! GDAL source data types, indexed from 0
    QList<GDALDataType> mSrcGdalDataType;

    //! \brief Cell value representing no data. e.g. -9999, indexed from 0
    //QList<double> mNoDataValue;

    //! Color tables indexed from 0
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

    //! Coverage CRS used for requests in Auth
    // TODO: use QgsCoordinateReferenceSystem ?
    QString mCoverageCrs;

    //! Cached data
    mutable QByteArray mCachedData;

    //! Name of memory file for cached data
    QString mCachedMemFilename;

    mutable VSILFILE *mCachedMemFile = nullptr;

    //! Pointer to cached GDAL dataset
    mutable gdal::dataset_unique_ptr mCachedGdalDataset;

    //! Current cache error last getCache() error.
    mutable QgsError mCachedError;

    //! The previous parameters to draw().
    mutable QgsRectangle mCachedViewExtent;
    mutable int mCachedViewWidth = 0;
    mutable int mCachedViewHeight = 0;

    //! Maximum width and height of getmap requests
    int mMaxWidth;
    int mMaxHeight;

    //! The error caption associated with the last WCS error.
    QString mErrorCaption;

    //! The error message associated with the last WCS error.
    QString mError;


    //! The mime type of the message
    QString mErrorFormat;

    //! A QgsCoordinateTransform is used for transformation of WCS layer extents
    mutable QgsCoordinateTransform mCoordinateTransform;

    //! See if calculateExtents() needs to be called before extent() returns useful data
    mutable bool mExtentDirty = true;

    //! Base URL for WCS GetFeatureInfo requests
    QString mGetFeatureInfoUrlBase;
    QString mServiceMetadataURL;

    //! number of layers and parents
    //int mLayerCount;
    //QMap<int, int> mLayerParents;
    //QMap<int, QStringList> mLayerParentNames;

    //! Errors counter
    int mErrors = 0;

    //! http authorization details
    mutable QgsWcsAuthorization mAuth;

    //! whether to use hrefs from GetCapabilities (default) or
    // the given base urls for GetMap and GetFeatureInfo
    bool mIgnoreGetCoverageUrl;
    bool mIgnoreAxisOrientation;
    bool mInvertAxisOrientation;

    QgsCoordinateReferenceSystem mCrs;

    // Fix for servers using bbox 1 px bigger
    bool mFixBox = false;

    // Fix for rasters rotated by GeoServer
    bool mFixRotate = false;

    QNetworkRequest::CacheLoadControl mCacheLoadControl = QNetworkRequest::PreferNetwork;

    /**
     * Clears cache
    */
    void reloadProviderData() override;

};

//! Handler for downloading of coverage data - output is written to mCachedData
class QgsWcsDownloadHandler : public QObject
{
    Q_OBJECT
  public:
    QgsWcsDownloadHandler( const QUrl &url, QgsWcsAuthorization &auth, QNetworkRequest::CacheLoadControl cacheLoadControl, QByteArray &cachedData, const QString &wcsVersion, QgsError &cachedError, QgsRasterBlockFeedback *feedback );
    ~QgsWcsDownloadHandler() override;

    void blockingDownload();

  protected slots:
    void cacheReplyFinished();
    void cacheReplyProgress( qint64, qint64 );
    void canceled();

  protected:
    void finish() { QMetaObject::invokeMethod( mEventLoop, "quit", Qt::QueuedConnection ); }

    QgsWcsAuthorization &mAuth;
    QEventLoop *mEventLoop = nullptr;

    QNetworkReply *mCacheReply = nullptr;

    QByteArray &mCachedData;
    QString mWcsVersion;
    QgsError &mCachedError;

    QgsRasterBlockFeedback *mFeedback = nullptr;

    static int sErrors; // this should be ideally per-provider...?
};

class QgsWcsProviderMetadata final: public QgsProviderMetadata
{
  public:
    QgsWcsProviderMetadata();
    QgsWcsProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList<QgsDataItemProvider *> dataItemProviders() const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
};

#endif

// ENDS
