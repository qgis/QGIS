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

class QgsArcGisAsyncQuery;
class QgsAmsProvider;

class QgsAmsLegendFetcher : public QgsImageFetcher
{
    Q_OBJECT
  public:
    QgsAmsLegendFetcher( QgsAmsProvider* provider );
    void start() override;
    bool haveImage() const { return mLegendImage.isNull(); }
    const QImage& getImage() const { return mLegendImage; }
    const QString& errorTitle() const { return mErrorTitle; }
    const QString& errorMessage() const { return mError; }

  private slots:
    void handleFinished();
    void handleError( QString errorTitle, QString errorMsg );

  private:
    QgsAmsProvider* mProvider;
    QgsArcGisAsyncQuery* mQuery;
    QByteArray mQueryReply;
    QImage mLegendImage;
    QString mErrorTitle;
    QString mError;
};

class QgsAmsProvider : public QgsRasterDataProvider
{
    Q_OBJECT

  public:
    QgsAmsProvider( const QString & uri );

    /* Inherited from QgsDataProvider */
    bool isValid() override { return mValid; }
    QString name() const override { return "mapserver"; }
    QString description() const override { return "ArcGIS MapServer data provider"; }
    QgsCoordinateReferenceSystem crs() override { return mCrs; }
    uint subLayerCount() const override { return mSubLayers.size(); }
    QStringList subLayers() const override { return mSubLayers; }
    QStringList subLayerStyles() const override;
    void setLayerOrder( const QStringList &layers ) override;
    void setSubLayerVisibility( const QString &name, bool vis ) override;
    void reloadData() override;

    /* Inherited from QgsRasterInterface */
    int bandCount() const override { return 1; }
    int capabilities() const override { return Identify | IdentifyText | IdentifyFeature; }

    /* Inherited from QgsRasterDataProvider */
    QgsRectangle extent() override { return mExtent; }
    QString lastErrorTitle() override { return mErrorTitle; }
    QString lastError() override { return mError; }
    QGis::DataType dataType( int /*bandNo*/ ) const override { return QGis::ARGB32; }
    QGis::DataType srcDataType( int /*bandNo*/ ) const override { return QGis::ARGB32; }
    QgsRasterInterface* clone() const override;
    QString metadata() override;
    QImage* draw( const QgsRectangle & viewExtent, int pixelWidth, int pixelHeight ) override;
    bool supportsLegendGraphic() const override { return true; }
    QImage getLegendGraphic( double scale = 0, bool forceRefresh = false, const QgsRectangle * visibleExtent = 0 ) override;
    QgsImageFetcher* getLegendGraphicFetcher( const QgsMapSettings* mapSettings ) override;
    QgsRasterIdentifyResult identify( const QgsPoint & thePoint, QgsRaster::IdentifyFormat theFormat, const QgsRectangle &theExtent = QgsRectangle(), int theWidth = 0, int theHeight = 0, int theDpi = 96 ) override;

  protected:
    void readBlock( int bandNo, const QgsRectangle & viewExtent, int width, int height, void *data ) override;

  private:
    bool mValid;
    QgsAmsLegendFetcher* mLegendFetcher;
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
};

#endif // QGSMAPSERVERPROVIDER_H
