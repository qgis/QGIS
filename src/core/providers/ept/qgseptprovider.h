/***************************************************************************
                         qgseptdataprovider.h
                         ---------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEPTPROVIDER_H
#define QGSEPTPROVIDER_H

#include <memory>

#include "qgspointclouddataprovider.h"
#include "qgsprovidermetadata.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsEptPointCloudIndex;

class QgsEptProvider: public QgsPointCloudDataProvider
{
    Q_OBJECT
  public:
    QgsEptProvider( const QString &uri,
                    const QgsDataProvider::ProviderOptions &providerOptions,
                    Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );

    ~QgsEptProvider() override;

    Qgis::DataProviderFlags flags() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsRectangle extent() const override;
    QgsPointCloudAttributeCollection attributes() const override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;
    QgsPointCloudIndex index() const override;
    qint64 pointCount() const override;
    QVariantMap originalMetadata() const override;
    void loadIndex( ) override;
    void generateIndex( ) override;
    PointCloudIndexGenerationState indexingState( ) override { return PointCloudIndexGenerationState::Indexed; }

  private:
    QgsPointCloudIndex mIndex;
};

class QgsEptProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsEptProviderMetadata();
    [[nodiscard]] QIcon icon() const override;
    [[nodiscard]] QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    QgsEptProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() ) override;
    QList< QgsProviderSublayerDetails > querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags = Qgis::SublayerQueryFlags(), QgsFeedback *feedback = nullptr ) const override;
    [[nodiscard]] int priorityForUri( const QString &uri ) const override;
    [[nodiscard]] QList< Qgis::LayerType > validLayerTypesForUri( const QString &uri ) const override;
    [[nodiscard]] bool uriIsBlocklisted( const QString &uri ) const override;
    [[nodiscard]] QString encodeUri( const QVariantMap &parts ) const override;
    [[nodiscard]] QVariantMap decodeUri( const QString &uri ) const override;
    QString filters( Qgis::FileFilterType type ) override;
    [[nodiscard]] ProviderCapabilities providerCapabilities() const override;
    [[nodiscard]] QList< Qgis::LayerType > supportedLayerTypes() const override;
};

///@endcond
#endif // QGSEPTPROVIDER_H
