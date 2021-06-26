/***************************************************************************
                         qgspdaldataprovider.h
                         ---------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPDALPROVIDER_H
#define QGSPDALPROVIDER_H

#include "qgis_core.h"
#include "qgspointclouddataprovider.h"
#include "qgsprovidermetadata.h"

#include <memory>

class QgsPdalProvider: public QgsPointCloudDataProvider
{
    Q_OBJECT
  public:
    QgsPdalProvider( const QString &uri,
                     const QgsDataProvider::ProviderOptions &providerOptions,
                     QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsPdalProvider();
    QgsCoordinateReferenceSystem crs() const override;

    QgsRectangle extent() const override;
    QgsPointCloudAttributeCollection attributes() const override;
    int pointCount() const override;
    QVariantMap originalMetadata() const override;

    bool isValid() const override;

    QString name() const override;

    QString description() const override;

    QgsPointCloudIndex *index() const override;

  private:
    bool load( const QString &uri );
    QgsCoordinateReferenceSystem mCrs;
    QgsRectangle mExtent;
    bool mIsValid = false;
    int mPointCount = 0;
    QVariantMap mOriginalMetadata;
};

class QgsPdalProviderMetadata : public QgsProviderMetadata
{
  public:
    QgsPdalProviderMetadata();
    QgsPdalProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    QList< QgsDataItemProvider * > dataItemProviders() const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    int priorityForUri( const QString &uri ) const override;
    QList< QgsMapLayerType > validLayerTypesForUri( const QString &uri ) const override;
    QString filters( FilterType type ) override;
};

#endif // QGSPDALPROVIDER_H
