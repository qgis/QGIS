/***************************************************************************
  qgsarcgisvectortileservicedataprovider.h
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

#ifndef QGSARCGISVECTORTILESERVICEDATAPROVIDER_H
#define QGSARCGISVECTORTILESERVICEDATAPROVIDER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsxyzvectortiledataprovider.h"
#include "qgsprovidermetadata.h"

#define SIP_NO_FILE

///@cond PRIVATE

class CORE_EXPORT QgsArcGisVectorTileServiceDataProvider : public QgsXyzVectorTileDataProviderBase
{
    Q_OBJECT

  public:
    QgsArcGisVectorTileServiceDataProvider( const QString &uri,
                                            const QgsDataProvider::ProviderOptions &providerOptions,
                                            QgsDataProvider::ReadFlags flags );

    QgsArcGisVectorTileServiceDataProvider( const QgsArcGisVectorTileServiceDataProvider &other );

    /**
     * QgsArcGisVectorTileServiceDataProvider cannot be assigned.
     */
    QgsArcGisVectorTileServiceDataProvider &operator=( const QgsArcGisVectorTileServiceDataProvider &other ) = delete;

    Qgis::DataProviderFlags flags() const override;
    Qgis::VectorTileProviderFlags providerFlags() const override;
    Qgis::VectorTileProviderCapabilities providerCapabilities() const override;
    QString name() const override;
    QString description() const override;
    QgsVectorTileDataProvider *clone() const override;
    QString sourcePath() const override;
    bool isValid() const override;
    QgsRectangle extent() const override;
    const QgsVectorTileMatrixSet &tileMatrixSet() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsLayerMetadata layerMetadata() const override;
    QVariantMap styleDefinition() const override;
    QString styleUrl() const override;
    QString htmlMetadata() const override;

    static QString ARCGIS_VT_SERVICE_DATA_PROVIDER_KEY;
    static QString ARCGIS_VT_SERVICE_DATA_PROVIDER_DESCRIPTION;

  private:

    bool setupArcgisVectorTileServiceConnection();

    bool mIsValid = false;
    QgsRectangle mExtent;
    QgsVectorTileMatrixSet mMatrixSet;

    QString mSourcePath;

    QVariantMap mArcgisLayerConfiguration;
    QVariantMap mArcgisStyleConfiguration;

    QgsCoordinateReferenceSystem mCrs;

    QgsLayerMetadata mLayerMetadata;
    QString mTileMapUrl;
};


class QgsArcGisVectorTileServiceDataProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsArcGisVectorTileServiceDataProviderMetadata();
    QIcon icon() const override;
    ProviderCapabilities providerCapabilities() const override;
    QgsArcGisVectorTileServiceDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QString absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const override;
    QString relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const override;
    QList< Qgis::LayerType > supportedLayerTypes() const override;
};

///@endcond

#endif // QGSARCGISVECTORTILESERVICEDATAPROVIDER_H
