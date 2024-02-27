/***************************************************************************
  qgsxyzvectortiledataprovider.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSXYZVECTORTILEDATAPROVIDER_H
#define QGSXYZVECTORTILEDATAPROVIDER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsvectortiledataprovider.h"
#include "qgsprovidermetadata.h"
#include "qgsvectortilematrixset.h"

#define SIP_NO_FILE

///@cond PRIVATE

class CORE_EXPORT QgsXyzVectorTileDataProviderBase : public QgsVectorTileDataProvider
{
    Q_OBJECT

  public:
    QgsXyzVectorTileDataProviderBase( const QString &uri,
                                      const QgsDataProvider::ProviderOptions &providerOptions,
                                      QgsDataProvider::ReadFlags flags );
    QgsXyzVectorTileDataProviderBase( const QgsXyzVectorTileDataProviderBase &other );

    /**
     * QgsXyzVectorTileDataProviderBase cannot be assigned.
     */
    QgsXyzVectorTileDataProviderBase &operator=( const QgsXyzVectorTileDataProviderBase &other ) = delete;

    bool supportsAsync() const override;
    QgsVectorTileRawData readTile( const QgsTileMatrixSet &tileMatrix, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr ) const override;
    QList<QgsVectorTileRawData> readTiles( const QgsTileMatrixSet &, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback = nullptr, Qgis::RendererUsage usage = Qgis::RendererUsage::Unknown ) const override;
    QNetworkRequest tileRequest( const QgsTileMatrixSet &tileMatrix, const QgsTileXYZ &id, Qgis::RendererUsage usage ) const override;

  protected:

    QString mAuthCfg;
    QgsHttpHeaders mHeaders;

  private:

    //! Returns raw tile data for a single tile, doing a HTTP request. Block the caller until tile data are downloaded.
    static QByteArray loadFromNetwork( const QgsTileXYZ &id,
                                       const QgsTileMatrix &tileMatrix,
                                       const QString &requestUrl,
                                       const QString &authid,
                                       const QgsHttpHeaders &headers,
                                       QgsFeedback *feedback = nullptr,
                                       Qgis::RendererUsage usage = Qgis::RendererUsage::Unknown );

};

class CORE_EXPORT QgsXyzVectorTileDataProvider : public QgsXyzVectorTileDataProviderBase
{
    Q_OBJECT

  public:
    QgsXyzVectorTileDataProvider( const QString &uri,
                                  const QgsDataProvider::ProviderOptions &providerOptions,
                                  QgsDataProvider::ReadFlags flags );
    QgsXyzVectorTileDataProvider( const QgsXyzVectorTileDataProvider &other );

    /**
     * QgsXyzVectorTileDataProvider cannot be assigned.
     */
    QgsXyzVectorTileDataProvider &operator=( const QgsXyzVectorTileDataProvider &other ) = delete;

    Qgis::DataProviderFlags flags() const override;
    QString name() const override;
    QString description() const override;
    QgsVectorTileDataProvider *clone() const override;
    QString sourcePath() const override;
    bool isValid() const override;
    QgsRectangle extent() const override;
    QgsCoordinateReferenceSystem crs() const override;
    const QgsVectorTileMatrixSet &tileMatrixSet() const override;

    static QString XYZ_DATA_PROVIDER_KEY;
    static QString XYZ_DATA_PROVIDER_DESCRIPTION;

  protected:

    bool mIsValid = false;
    QgsRectangle mExtent;
    QgsVectorTileMatrixSet mMatrixSet;

  private:

    //! Returns raw tile data for a single tile, doing a HTTP request. Block the caller until tile data are downloaded.
    static QByteArray loadFromNetwork( const QgsTileXYZ &id,
                                       const QgsTileMatrix &tileMatrix,
                                       const QString &requestUrl,
                                       const QString &authid,
                                       const QgsHttpHeaders &headers,
                                       QgsFeedback *feedback = nullptr );

};


class QgsXyzVectorTileDataProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsXyzVectorTileDataProviderMetadata();
    QgsXyzVectorTileDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QIcon icon() const override;
    ProviderCapabilities providerCapabilities() const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QString absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const override;
    QString relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const override;
    QList< Qgis::LayerType > supportedLayerTypes() const override;
};


///@endcond

#endif // QGSXYZVECTORTILEDATAPROVIDER_H
