/***************************************************************************
  qgsmbtilesvectortiledataprovider.h
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

#ifndef QGSMBTILESVECTORTILEDATAPROVIDER_H
#define QGSMBTILESVECTORTILEDATAPROVIDER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsvectortiledataprovider.h"
#include "qgsprovidermetadata.h"
#include "qgsvectortilematrixset.h"

class QgsMbTiles;

#define SIP_NO_FILE

///@cond PRIVATE

class CORE_EXPORT QgsMbTilesVectorTileDataProvider : public QgsVectorTileDataProvider
{
    Q_OBJECT

  public:
    QgsMbTilesVectorTileDataProvider( const QString &uri,
                                      const QgsDataProvider::ProviderOptions &providerOptions,
                                      QgsDataProvider::ReadFlags flags );

    QgsMbTilesVectorTileDataProvider( const QgsMbTilesVectorTileDataProvider &other );

    /**
     * QgsMbTilesVectorTileDataProvider cannot be assigned.
     */
    QgsMbTilesVectorTileDataProvider &operator=( const QgsMbTilesVectorTileDataProvider &other ) = delete;

    Qgis::DataProviderFlags flags() const override;
    QString name() const override;
    QString description() const override;
    QgsVectorTileDataProvider *clone() const override;
    QString sourcePath() const override;
    bool isValid() const override;
    QgsRectangle extent() const override;
    QgsCoordinateReferenceSystem crs() const override;
    const QgsVectorTileMatrixSet &tileMatrixSet() const override;
    QgsVectorTileRawData readTile( const QgsTileMatrixSet &tileMatrixSet, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr ) const override;
    QList<QgsVectorTileRawData> readTiles( const QgsTileMatrixSet &, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback = nullptr, Qgis::RendererUsage usage = Qgis::RendererUsage::Unknown ) const override;

    static QString MB_TILES_VECTOR_TILE_DATA_PROVIDER_KEY;
    static QString MB_TILES_VECTOR_TILE_DATA_PROVIDER_DESCRIPTION;

  private:

    //! Returns raw tile data for a single tile loaded from MBTiles file
    static QByteArray loadFromMBTiles( QgsMbTiles &mbTileReader, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr );
    bool mIsValid = false;
    QgsRectangle mExtent;
    QgsVectorTileMatrixSet mMatrixSet;

};

class QgsMbTilesVectorTileDataProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsMbTilesVectorTileDataProviderMetadata();
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    QgsMbTilesVectorTileDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QIcon icon() const override;
    ProviderCapabilities providerCapabilities() const override;
    QString filters( Qgis::FileFilterType type ) override;
    QList< QgsProviderSublayerDetails > querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags = Qgis::SublayerQueryFlags(), QgsFeedback *feedback = nullptr ) const override;
    int priorityForUri( const QString &uri ) const override;
    QList< Qgis::LayerType > validLayerTypesForUri( const QString &uri ) const override;

    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QString absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const override;
    QString relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const override;
    QList< Qgis::LayerType > supportedLayerTypes() const override;
};


///@endcond

#endif // QGSMBTILESVECTORTILEDATAPROVIDER_H
