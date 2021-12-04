/***************************************************************************
    qgsamsprovider.h - ArcGIS MapServer Raster Provider
     --------------------------------------------------
    Date                 : Nov 24, 2015
    Copyright            : (C) 2015 by Sandro Mani
    email                : manisandro@gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPSERVERPROVIDER_H
#define QGSMAPSERVERPROVIDER_H

#include "qgsrasterdataprovider.h"

#include "qgshttpheaders.h"
#include <QNetworkRequest>

#include "qgscoordinatereferencesystem.h"
#include "qgsprovidermetadata.h"

class QgsArcGisAsyncQuery;
class QgsAmsProvider;
class QNetworkReply;

class QgsAmsLegendFetcher : public QgsImageFetcher
{
    Q_OBJECT
  public:
    QgsAmsLegendFetcher( QgsAmsProvider *provider, const QImage &fetchedImage );
    void start() override;
    bool haveImage() const { return mLegendImage.isNull(); }
    QImage getImage() const { return mLegendImage; }
    void setImage( const QImage &image ) { mLegendImage = image; }
    void clear() { mLegendImage = QImage(); }
    const QString &errorTitle() const { return mErrorTitle; }
    const QString &errorMessage() const { return mError; }

  signals:

    void fetchedNew( const QImage &image );

  private slots:
    void handleFinished();
    void handleError( const QString &errorTitle, const QString &errorMsg );
    void sendCachedImage();

  private:
    QgsAmsProvider *mProvider = nullptr;
    QgsArcGisAsyncQuery *mQuery = nullptr;
    QByteArray mQueryReply;
    QImage mLegendImage;
    QString mErrorTitle;
    QString mError;

};

class QgsAmsProvider : public QgsRasterDataProvider
{
    Q_OBJECT

  public:

    static const QString AMS_PROVIDER_KEY;
    static const QString AMS_PROVIDER_DESCRIPTION;

    QgsAmsProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    explicit QgsAmsProvider( const QgsAmsProvider &other, const QgsDataProvider::ProviderOptions &providerOptions );
    QgsRasterDataProvider::ProviderCapabilities providerCapabilities() const override;
    /* Inherited from QgsDataProvider */
    bool isValid() const override { return mValid; }
    QString name() const override;
    QString description() const override;
    QgsCoordinateReferenceSystem crs() const override { return mCrs; }
    uint subLayerCount() const override { return mSubLayers.size(); }
    QStringList subLayers() const override { return mSubLayers; }
    QStringList subLayerStyles() const override;
    void setLayerOrder( const QStringList &layers ) override;
    void setSubLayerVisibility( const QString &name, bool vis ) override;
    bool renderInPreview( const QgsDataProvider::PreviewContext &context ) override;
    QgsLayerMetadata layerMetadata() const override;

    static QString providerKey();

    /* Inherited from QgsRasterInterface */
    int bandCount() const override { return 1; }
    int capabilities() const override { return Identify | IdentifyText | IdentifyFeature | Prefetch; }

    /* Inherited from QgsRasterDataProvider */
    QgsRectangle extent() const override { return mExtent; }
    QString lastErrorTitle() override { return mErrorTitle; }
    QString lastError() override { return mError; }
    Qgis::DataType dataType( int /*bandNo*/ ) const override { return Qgis::DataType::ARGB32; }
    Qgis::DataType sourceDataType( int /*bandNo*/ ) const override { return Qgis::DataType::ARGB32; }
    QgsAmsProvider *clone() const override;
    QString htmlMetadata() override;
    bool supportsLegendGraphic() const override { return true; }
    QImage getLegendGraphic( double scale = 0, bool forceRefresh = false, const QgsRectangle *visibleExtent = nullptr ) override;
    QgsImageFetcher *getLegendGraphicFetcher( const QgsMapSettings *mapSettings ) override;
    QgsRasterIdentifyResult identify( const QgsPointXY &point, QgsRaster::IdentifyFormat format, const QgsRectangle &extent = QgsRectangle(), int width = 0, int height = 0, int dpi = 96 ) override;
    QList< double > nativeResolutions() const override;
    bool ignoreExtents() const override { return true; }

    //! Helper struct for tile requests
    struct TileRequest
    {
      TileRequest( const QUrl &u, const QRectF &r, int i, const QRectF &mapExtent )
        : url( u )
        , rect( r )
        , mapExtent( mapExtent )
        , index( i )
      {}
      QUrl url;
      QRectF rect;
      QRectF mapExtent;
      int index;
    };
    typedef QList<TileRequest> TileRequests;

    //! Helper structure to store a cached tile image with its rectangle
    typedef struct TileImage
    {
      TileImage( const QRectF &r, const QImage &i, bool smooth ): rect( r ), img( i ), smooth( smooth ) {}
      QRectF rect; //!< Destination rectangle for a tile (in screen coordinates)
      QImage img;  //!< Cached tile to be drawn
      bool smooth;
    } TileImage;

  protected:
    bool readBlock( int bandNo, const QgsRectangle &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override;

    QImage draw( const QgsRectangle &viewExtent, int pixelWidth, int pixelHeight, QgsRasterBlockFeedback *feedback = nullptr );

  private:
    bool mValid = false;
    QgsAmsLegendFetcher *mLegendFetcher = nullptr;
    QVariantMap mServiceInfo;
    QVariantMap mLayerInfo;
    QgsCoordinateReferenceSystem mCrs;
    QgsRectangle mExtent;
    QStringList mSubLayers;
    QList<bool> mSubLayerVisibilities;
    QString mErrorTitle;
    QString mError;
    QImage mCachedImage;
    QgsRectangle mCachedImageExtent;
    QgsHttpHeaders mRequestHeaders;
    int mTileReqNo = 0;
    bool mTiled = false;
    bool mImageServer = false;
    int mMaxImageWidth = 4096;
    int mMaxImageHeight = 4096;
    QgsLayerMetadata mLayerMetadata;
    QList< double > mResolutions;

    /**
     * Resets cached image
    */
    void reloadProviderData() override;
};

//! Handler for tiled MapServer requests, the data are written to the given image
class QgsAmsTiledImageDownloadHandler : public QObject
{
    Q_OBJECT
  public:

    QgsAmsTiledImageDownloadHandler( const QString &auth,  const QgsHttpHeaders &requestHeaders, int reqNo, const QgsAmsProvider::TileRequests &requests, QImage *image, const QgsRectangle &viewExtent, QgsRasterBlockFeedback *feedback );
    ~QgsAmsTiledImageDownloadHandler() override;

    void downloadBlocking();

  protected slots:
    void tileReplyFinished();
    void canceled();

  private:

    enum TileAttribute
    {
      TileReqNo = QNetworkRequest::User + 0,
      TileIndex = QNetworkRequest::User + 1,
      TileRect  = QNetworkRequest::User + 2,
      TileRetry = QNetworkRequest::User + 3,
    };

    /**
     * \brief Relaunch tile request cloning previous request parameters and managing max repeat
     *
     * \param oldRequest request to clone to generate new tile request
     *
     * request is not launched if max retry is reached. Message is logged.
     */
    void repeatTileRequest( QNetworkRequest const &oldRequest );

    void finish() { QMetaObject::invokeMethod( mEventLoop, "quit", Qt::QueuedConnection ); }

    QString mAuth;
    QgsHttpHeaders mRequestHeaders;

    QImage *mImage = nullptr;
    QgsRectangle mViewExtent;

    QEventLoop *mEventLoop = nullptr;

    int mTileReqNo;

    //! Running tile requests
    QList<QNetworkReply *> mReplies;

    QgsRasterBlockFeedback *mFeedback = nullptr;
};

class QgsAmsProviderMetadata: public QgsProviderMetadata
{
  public:
    QgsAmsProviderMetadata();
    QgsAmsProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
};

#endif // QGSMAPSERVERPROVIDER_H
