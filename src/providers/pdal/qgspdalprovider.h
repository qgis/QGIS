/***************************************************************************
                         qgspdaldataprovider.h
                         ---------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPDALPROVIDER_H
#define QGSPDALPROVIDER_H

#include "qgis_core.h"
#include "qgspointclouddataprovider.h"
#include "qgsprovidermetadata.h"
#include "qgscopcpointcloudindex.h"
#include <memory>

class QgsEptPointCloudIndex;
class QgsPdalIndexingTask;

class QgsPdalProvider: public QgsPointCloudDataProvider
{
    Q_OBJECT
  public:
    QgsPdalProvider( const QString &uri,
                     const QgsDataProvider::ProviderOptions &providerOptions,
                     QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags(), bool generateCopc = true );

    ~QgsPdalProvider();
    QgsCoordinateReferenceSystem crs() const override;
    QgsRectangle extent() const override;
    QgsPointCloudAttributeCollection attributes() const override;
    qint64 pointCount() const override;
    QVariantMap originalMetadata() const override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;
    QgsPointCloudIndex *index() const override;
    void loadIndex( ) override;
    void generateIndex( ) override;
    PointCloudIndexGenerationState indexingState( ) override;

  private slots:
    void onGenerateIndexFinished();
    void onGenerateIndexFailed();

  private:
    bool anyIndexingTaskExists();
    bool load( const QString &uri );
    QgsCoordinateReferenceSystem mCrs;
    QgsRectangle mExtent;
    bool mIsValid = false;
    qint64 mPointCount = 0;

    QVariantMap mOriginalMetadata;
    std::unique_ptr<QgsPointCloudIndex> mIndex;
    QgsPdalIndexingTask *mRunningIndexingTask = nullptr;
    bool mGenerateCopc = true;
    static QQueue<QgsPdalProvider *> sIndexingQueue;
};

class QgsPdalProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT

  public:
    QgsPdalProviderMetadata();
    QIcon icon() const override;
    QgsPdalProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    int priorityForUri( const QString &uri ) const override;
    QList< QgsMapLayerType > validLayerTypesForUri( const QString &uri ) const override;
    QList< QgsProviderSublayerDetails > querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags = Qgis::SublayerQueryFlags(), QgsFeedback *feedback = nullptr ) const override;
    QString filters( FilterType type ) override;
    ProviderCapabilities providerCapabilities() const override;
    QList< QgsMapLayerType > supportedLayerTypes() const override;

  private:
    static QString sFilterString;
    static QStringList sExtensions;
    void buildSupportedPointCloudFileFilterAndExtensions();

};

#endif // QGSPDALPROVIDER_H
