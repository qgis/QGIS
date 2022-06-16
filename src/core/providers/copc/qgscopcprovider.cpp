/***************************************************************************
                         qgscopcdataprovider.cpp
                         -----------------------
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

#include "qgis.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"
#include "qgscopcprovider.h"
#include "qgscopcpointcloudindex.h"
#include "qgsremotecopcpointcloudindex.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"
#include <QIcon>

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "copc" )
#define PROVIDER_DESCRIPTION QStringLiteral( "COPC point cloud data provider" )

QgsCopcProvider::QgsCopcProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsPointCloudDataProvider( uri, options, flags )
{
  bool isRemote = uri.startsWith( QStringLiteral( "http" ), Qt::CaseSensitivity::CaseInsensitive );
  if ( isRemote )
    mIndex.reset( new QgsRemoteCopcPointCloudIndex );
  else
    mIndex.reset( new QgsCopcPointCloudIndex );

  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
    profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Open data source" ), QStringLiteral( "projectload" ) );

  loadIndex( );
  if ( mIndex && !mIndex->isValid() )
  {
    appendError( mIndex->error() );
  }
}

QgsCopcProvider::~QgsCopcProvider() = default;

QgsCoordinateReferenceSystem QgsCopcProvider::crs() const
{
  return mIndex->crs();
}

QgsRectangle QgsCopcProvider::extent() const
{
  return mIndex->extent();
}

QgsPointCloudAttributeCollection QgsCopcProvider::attributes() const
{
  return mIndex->attributes();
}

bool QgsCopcProvider::isValid() const
{
  if ( !mIndex.get() )
  {
    return false;
  }
  return mIndex->isValid();
}

QString QgsCopcProvider::name() const
{
  return QStringLiteral( "copc" );
}

QString QgsCopcProvider::description() const
{
  return QStringLiteral( "Point Clouds COPC" );
}

QgsPointCloudIndex *QgsCopcProvider::index() const
{
  return mIndex.get();
}

qint64 QgsCopcProvider::pointCount() const
{
  return mIndex->pointCount();
}

void QgsCopcProvider::loadIndex( )
{
  // Index already loaded -> no need to load
  if ( mIndex->isValid() )
    return;
  mIndex->load( dataSourceUri() );
}

QVariantMap QgsCopcProvider::originalMetadata() const
{
  return mIndex->originalMetadata();
}

void QgsCopcProvider::generateIndex()
{
  //no-op, index is always generated
}

QgsCopcProviderMetadata::QgsCopcProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QIcon QgsCopcProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconPointCloudLayer.svg" ) );
}

QgsCopcProvider *QgsCopcProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsCopcProvider( uri, options, flags );
}

QList<QgsProviderSublayerDetails> QgsCopcProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags, QgsFeedback * ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().endsWith( ".copc.laz", Qt::CaseSensitivity::CaseInsensitive ) )
  {
    QgsProviderSublayerDetails details;
    details.setUri( uri );
    details.setProviderKey( QStringLiteral( "copc" ) );
    details.setType( QgsMapLayerType::PointCloudLayer );
    details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( uri ) );
    return {details};
  }
  else
  {
    return {};
  }
}

int QgsCopcProviderMetadata::priorityForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().endsWith( ".copc.laz", Qt::CaseSensitivity::CaseInsensitive ) )
    return 100;

  return 0;
}

QList<QgsMapLayerType> QgsCopcProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().endsWith( ".copc.laz", Qt::CaseSensitivity::CaseInsensitive ) )
    return QList< QgsMapLayerType>() << QgsMapLayerType::PointCloudLayer;

  return QList< QgsMapLayerType>();
}

QVariantMap QgsCopcProviderMetadata::decodeUri( const QString &uri ) const
{
  QVariantMap uriComponents;
  QUrl url = QUrl::fromUserInput( uri );
  uriComponents.insert( QStringLiteral( "file-name" ), url.fileName() );
  uriComponents.insert( QStringLiteral( "path" ), uri );
  return uriComponents;
}

QString QgsCopcProviderMetadata::filters( QgsProviderMetadata::FilterType type )
{
  switch ( type )
  {
    case QgsProviderMetadata::FilterType::FilterVector:
    case QgsProviderMetadata::FilterType::FilterRaster:
    case QgsProviderMetadata::FilterType::FilterMesh:
    case QgsProviderMetadata::FilterType::FilterMeshDataset:
      return QString();

    case QgsProviderMetadata::FilterType::FilterPointCloud:
      return QObject::tr( "COPC Point Clouds" ) + QStringLiteral( " (*.copc.laz *.COPC.LAZ)" );
  }
  return QString();
}

QgsProviderMetadata::ProviderCapabilities QgsCopcProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QList<QgsMapLayerType> QgsCopcProviderMetadata::supportedLayerTypes() const
{
  return { QgsMapLayerType::PointCloudLayer };
}

QString QgsCopcProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  return path;
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsCopcProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::LayerTypesForUri
         | ProviderMetadataCapability::PriorityForUri
         | ProviderMetadataCapability::QuerySublayers;
}
///@endcond

