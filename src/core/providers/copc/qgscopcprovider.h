/***************************************************************************
                         qgscopcdataprovider.h
                         ---------------------
    begin                : March 2022
    copyright            : (C) 2022 by Belgacem Nedjima
    email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOPCPROVIDER_H
#define QGSCOPCPROVIDER_H

#include "qgis_core.h"
#include "qgspointclouddataprovider.h"
#include "qgsprovidermetadata.h"

#include <memory>

#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsCopcPointCloudIndex;
class QgsRemoteCopcPointCloudIndex;

class QgsCopcProvider: public QgsPointCloudDataProvider
{
    Q_OBJECT
  public:
    QgsCopcProvider( const QString &uri,
                     const QgsDataProvider::ProviderOptions &providerOptions,
                     QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsCopcProvider();

    QgsCoordinateReferenceSystem crs() const override;

    QgsRectangle extent() const override;
    QgsPointCloudAttributeCollection attributes() const override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;
    QgsPointCloudIndex *index() const override;
    qint64 pointCount() const override;
    QVariant metadataStatistic( const QString &attribute, QgsStatisticalSummary::Statistic statistic ) const override;
    QVariantList metadataClasses( const QString &attribute ) const override;
    QVariant metadataClassStatistic( const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic ) const override;
    QVariantMap originalMetadata() const override;
    void loadIndex( ) override;
    void generateIndex( ) override;
    PointCloudIndexGenerationState indexingState( ) override { return PointCloudIndexGenerationState::Indexed; }

  private:
    std::unique_ptr<QgsPointCloudIndex> mIndex;

    QgsRectangle mExtent;
    uint64_t mPointCount;
    QgsCoordinateReferenceSystem mCrs;
};

class QgsCopcProviderMetadata : public QgsProviderMetadata
{
  public:
    QgsCopcProviderMetadata();
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    QgsCopcProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList< QgsProviderSublayerDetails > querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags = Qgis::SublayerQueryFlags(), QgsFeedback *feedback = nullptr ) const override;
    int priorityForUri( const QString &uri ) const override;
    QList< QgsMapLayerType > validLayerTypesForUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString filters( FilterType type ) override;
    ProviderCapabilities providerCapabilities() const override;
};

///@endcond
#endif // QGSCOPCPROVIDER_H
