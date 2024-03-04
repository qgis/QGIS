/***************************************************************************
                         qgsvirtualpointcloudprovider.h
                         ----------------
    begin                : March 2023
    copyright            : (C) 2023 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVIRTUALPOINTCLOUDPROVIDER_H
#define QGSVIRTUALPOINTCLOUDPROVIDER_H

#include "qgspointclouddataprovider.h"
#include "qgsprovidermetadata.h"

#include <memory>

///@cond PRIVATE
#define SIP_NO_FILE

class QgsCopcPointCloudIndex;
class QgsRemoteCopcPointCloudIndex;

class CORE_EXPORT QgsVirtualPointCloudProvider: public QgsPointCloudDataProvider
{
    Q_OBJECT
  public:
    QgsVirtualPointCloudProvider( const QString &uri,
                                  const QgsDataProvider::ProviderOptions &providerOptions,
                                  QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsVirtualPointCloudProvider();

    Qgis::DataProviderFlags flags() const override;
    QgsPointCloudDataProvider::Capabilities capabilities() const override;
    QgsCoordinateReferenceSystem crs() const override;

    QgsRectangle extent() const override;
    QgsPointCloudAttributeCollection attributes() const override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;
    QgsPointCloudIndex *index() const override;
    qint64 pointCount() const override;
    QVariantMap originalMetadata() const override;
    void loadIndex( ) override;
    void generateIndex( ) override;
    PointCloudIndexGenerationState indexingState( ) override { return PointCloudIndexGenerationState::Indexed; }
    QgsGeometry polygonBounds() const override;
    QVector<QgsPointCloudSubIndex> subIndexes() override { return mSubLayers; }
    void loadSubIndex( int i ) override;
    bool setSubsetString( const QString &subset, bool updateFeatureCount = false ) override;
    QgsPointCloudRenderer *createRenderer( const QVariantMap &configuration = QVariantMap() ) const override SIP_FACTORY;
    bool renderInPreview( const QgsDataProvider::PreviewContext & ) override { return false; }

  signals:
    void subIndexLoaded( int i );

  private:
    void parseFile();
    void populateAttributeCollection( QSet<QString> names );
    QVector<QgsPointCloudSubIndex> mSubLayers;
    std::unique_ptr<QgsGeometry> mPolygonBounds;
    std::unique_ptr<QgsPointCloudIndex> mIndex;
    QgsPointCloudAttributeCollection mAttributes;

    QStringList mUriList;
    QgsRectangle mExtent;
    qint64 mPointCount = 0;
    QgsCoordinateReferenceSystem mCrs;
};

class QgsVirtualPointCloudProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsVirtualPointCloudProviderMetadata();
    QIcon icon() const override;
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    QgsVirtualPointCloudProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList< QgsProviderSublayerDetails > querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags = Qgis::SublayerQueryFlags(), QgsFeedback *feedback = nullptr ) const override;
    int priorityForUri( const QString &uri ) const override;
    QList< Qgis::LayerType > validLayerTypesForUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString filters( Qgis::FileFilterType type ) override;
    ProviderCapabilities providerCapabilities() const override;
    QList< Qgis::LayerType > supportedLayerTypes() const override;
};

///@endcond
#endif // QGSVIRTUALPOINTCLOUDPROVIDER_H
