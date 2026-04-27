/***************************************************************************
    qgsimageserverprovider.h
     --------------------------------------------------
    Date                 : April 2026
    Copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSIMAGESERVERPROVIDER_H
#define QGSIMAGESERVERPROVIDER_H

#include <gdal.h>

#include "qgscoordinatereferencesystem.h"
#include "qgshttpheaders.h"
#include "qgsprovidermetadata.h"
#include "qgsrasterdataprovider.h"

#include <QNetworkRequest>

class QNetworkReply;

class QgsImageServerProvider : public QgsRasterDataProvider
{
    Q_OBJECT

  public:
    static const QString IMS_PROVIDER_KEY;
    static const QString IMS_PROVIDER_DESCRIPTION;

    QgsImageServerProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );

    explicit QgsImageServerProvider( const QgsImageServerProvider &other, const QgsDataProvider::ProviderOptions &providerOptions );
    Qgis::DataProviderFlags flags() const override;
    Qgis::RasterProviderCapabilities providerCapabilities() const override;
    /* Inherited from QgsDataProvider */
    bool isValid() const override { return mValid; }
    QString name() const override;
    QString description() const override;
    QgsCoordinateReferenceSystem crs() const override { return mCrs; }
    bool renderInPreview( const QgsDataProvider::PreviewContext &context ) override;
    QgsLayerMetadata layerMetadata() const override;

    static QString providerKey();

    /* Inherited from QgsRasterInterface */
    int bandCount() const override;
    QString generateBandName( int bandNumber ) const override;
    Qgis::RasterColorInterpretation colorInterpretation( int bandNo ) const override;
    Qgis::RasterInterfaceCapabilities capabilities() const override;
    int xSize() const override;
    int ySize() const override;
    bool hasStatistics( int bandNo, Qgis::RasterBandStatistics stats = Qgis::RasterBandStatistic::All, const QgsRectangle &boundingBox = QgsRectangle(), int sampleSize = 0 ) override;
    QgsRasterBandStats bandStatistics(
      int bandNo, Qgis::RasterBandStatistics stats = Qgis::RasterBandStatistic::All, const QgsRectangle &boundingBox = QgsRectangle(), int sampleSize = 0, QgsRasterBlockFeedback *feedback = nullptr
    ) override;

    /* Inherited from QgsRasterDataProvider */
    QgsRectangle extent() const override { return mExtent; }
    QString lastErrorTitle() override { return mErrorTitle; }
    QString lastError() override { return mError; }
    Qgis::DataType dataType( int bandNo ) const override;
    Qgis::DataType sourceDataType( int bandNo ) const override;
    QgsImageServerProvider *clone() const override;
    QString htmlMetadata() const override;
    QgsRasterIdentifyResult identify( const QgsPointXY &point, Qgis::RasterIdentifyFormat format, const QgsRectangle &extent = QgsRectangle(), int width = 0, int height = 0, int dpi = 96 ) override;
    QList<double> nativeResolutions() const override;

  protected:
    using QgsRasterDataProvider::readBlock;
    bool readBlock( int bandNo, const QgsRectangle &viewExtent, int width, int height, void *data, QgsRasterBlockFeedback *feedback = nullptr ) override;
    bool readNativeAttributeTable( QString *errorMessage = nullptr ) override;

  private:
    bool readTiledBlock( const QgsRectangle &viewExtent, int width, int height, void *data, GDALDataType gdalType, int elementSize, QgsRasterBlockFeedback *feedback );

    bool mValid = false;
    QVariantMap mServiceInfo;
    QVariantMap mLayerInfo;
    Qgis::ArcGisRestServiceCapabilities mCapabilities;
    Qgis::RasterInterfaceCapabilities mRasterCapabilities;
    QgsCoordinateReferenceSystem mCrs;
    QgsRectangle mExtent;
    double mPixelSizeX = 1;
    double mPixelSizeY = 1;
    QString mPixelType;
    Qgis::DataType mDataType = Qgis::DataType::UnknownDataType;
    int mBandCount = 1;
    QStringList mBandNames;
    QList< double > mMinValues;
    QList< double > mMaxValues;
    QList< double > mMeanValues;
    QList< double > mStdevValues;
    QString mErrorTitle;
    QString mError;
    QImage mCachedImage;
    QgsRectangle mCachedImageExtent;
    QgsHttpHeaders mRequestHeaders;
    bool mTiled = false;
    int mMinLOD = -1;
    int mMaxLOD = -1;
    int mMaxImageWidth = 4096;
    int mMaxImageHeight = 4096;
    QgsLayerMetadata mLayerMetadata;
    QList<double> mResolutions;
    QString mUrlPrefix;
    QString mAuthCfg;
    int mMaximumLercVersionSupported = 0;
    bool mHasRat = false;

    /**
     * Resets cached image
    */
    void reloadProviderData() override;

    struct TileRequest
    {
        TileRequest( const QUrl &u, int i, const QgsRectangle &mapExtent )
          : url( u )
          , index( i )
          , mapExtent( mapExtent )
        {}
        QUrl url;
        int index;
        QgsRectangle mapExtent;
    };
    typedef QList<TileRequest> TileRequests;
};

class QgsImageServerProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsImageServerProviderMetadata();
    QIcon icon() const override;
    QgsProviderMetadata::ProviderCapabilities providerCapabilities() const override;
    QgsImageServerProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() ) override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QList<Qgis::LayerType> supportedLayerTypes() const override;
};

#endif // QGSIMAGESERVERPROVIDER_H
