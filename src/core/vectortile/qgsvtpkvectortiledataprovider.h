/***************************************************************************
  qgsvtpkvectortiledataprovider.h
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

#ifndef QGSVTPKVECTORTILEDATAPROVIDER_H
#define QGSVTPKVECTORTILEDATAPROVIDER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsvectortiledataprovider.h"
#include "qgsprovidermetadata.h"
#include "qgsvectortilematrixset.h"

#include <QImage>

#define SIP_NO_FILE

///@cond PRIVATE

class QgsVtpkTiles;

class CORE_EXPORT QgsVtpkVectorTileDataProvider : public QgsVectorTileDataProvider
{
    Q_OBJECT

  public:
    QgsVtpkVectorTileDataProvider( const QString &uri,
                                   const QgsDataProvider::ProviderOptions &providerOptions,
                                   QgsDataProvider::ReadFlags flags );
    QgsVtpkVectorTileDataProvider( const QgsVtpkVectorTileDataProvider &other );

    /**
     * QgsVtpkVectorTileDataProvider cannot be assigned.
     */
    QgsVtpkVectorTileDataProvider &operator=( const QgsVtpkVectorTileDataProvider &other ) = delete;

    Qgis::DataProviderFlags flags() const override;
    Qgis::VectorTileProviderFlags providerFlags() const override;
    Qgis::VectorTileProviderCapabilities providerCapabilities() const override;
    QString name() const override;
    QString description() const override;
    QgsVectorTileDataProvider *clone() const override;
    QString sourcePath() const override;
    bool isValid() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsRectangle extent() const override;
    QgsLayerMetadata layerMetadata() const override;
    const QgsVectorTileMatrixSet &tileMatrixSet() const override;
    QVariantMap styleDefinition() const override;
    QVariantMap spriteDefinition() const override;
    QImage spriteImage() const override;
    QgsVectorTileRawData readTile( const QgsTileMatrixSet &tileMatrix, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr ) const override;
    QList<QgsVectorTileRawData> readTiles( const QgsTileMatrixSet &, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback = nullptr, Qgis::RendererUsage usage = Qgis::RendererUsage::Unknown ) const override;
    QString htmlMetadata() const override;

    static QString DATA_PROVIDER_KEY;
    static QString DATA_PROVIDER_DESCRIPTION;

  private:

    //! Returns raw tile data for a single tile loaded from VTPK file
    static QgsVectorTileRawData loadFromVtpk( QgsVtpkTiles &vtpkTileReader, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr );
    bool mIsValid = false;
    QgsCoordinateReferenceSystem mCrs;
    QgsRectangle mExtent;
    QgsVectorTileMatrixSet mMatrixSet;
    QgsLayerMetadata mLayerMetadata;
    QVariantMap mStyleDefinition;
    QVariantMap mSpriteDefinition;
    QImage mSpriteImage;

};


class QgsVtpkVectorTileDataProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsVtpkVectorTileDataProviderMetadata();
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    QgsVtpkVectorTileDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
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

#endif // QGSVTPKVECTORTILEDATAPROVIDER_H
