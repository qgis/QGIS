/***************************************************************************
                         qgseptdataprovider.cpp
                         -----------------------
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

#include "qgis.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"
#include "qgseptprovider.h"
#include "qgseptpointcloudindex.h"
#include "qgsremoteeptpointcloudindex.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"
#include "qgsthreadingutils.h"

#include <QFileInfo>
#include <QIcon>

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "ept" )
#define PROVIDER_DESCRIPTION QStringLiteral( "EPT point cloud data provider" )

QgsEptProvider::QgsEptProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsPointCloudDataProvider( uri, options, flags )
{
  if ( uri.startsWith( QStringLiteral( "http" ), Qt::CaseSensitivity::CaseInsensitive ) )
    mIndex.reset( new QgsRemoteEptPointCloudIndex );
  else
    mIndex.reset( new QgsEptPointCloudIndex );

  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
    profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Open data source" ), QStringLiteral( "projectload" ) );

  loadIndex( );
}

QgsEptProvider::~QgsEptProvider() = default;

QgsCoordinateReferenceSystem QgsEptProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex->crs();
}

QgsRectangle QgsEptProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex->extent();
}

QgsPointCloudAttributeCollection QgsEptProvider::attributes() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex->attributes();
}

bool QgsEptProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex->isValid();
}

QString QgsEptProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QStringLiteral( "ept" );
}

QString QgsEptProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QStringLiteral( "Point Clouds EPT" );
}

QgsPointCloudIndex *QgsEptProvider::index() const
{
  // BAD! 2D rendering of point clouds is NOT thread safe
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mIndex.get();
}

qint64 QgsEptProvider::pointCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex->pointCount();
}

void QgsEptProvider::loadIndex( )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mIndex->isValid() )
    return;

  mIndex->load( dataSourceUri() );
}

QVariantMap QgsEptProvider::originalMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex->originalMetadata();
}

void QgsEptProvider::generateIndex()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  //no-op, index is always generated
}

QgsEptProviderMetadata::QgsEptProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QIcon QgsEptProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconPointCloudLayer.svg" ) );
}

QgsEptProvider *QgsEptProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsEptProvider( uri, options, flags );
}

QList<QgsProviderSublayerDetails> QgsEptProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags, QgsFeedback * ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().compare( QLatin1String( "ept.json" ), Qt::CaseInsensitive ) == 0 )
  {
    QgsProviderSublayerDetails details;
    details.setUri( uri );
    details.setProviderKey( QStringLiteral( "ept" ) );
    details.setType( QgsMapLayerType::PointCloudLayer );
    details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( uri ) );
    return {details};
  }
  else
  {
    return {};
  }
}

int QgsEptProviderMetadata::priorityForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().compare( QLatin1String( "ept.json" ), Qt::CaseInsensitive ) == 0 )
    return 100;

  return 0;
}

QList<QgsMapLayerType> QgsEptProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().compare( QLatin1String( "ept.json" ), Qt::CaseInsensitive ) == 0 )
    return QList< QgsMapLayerType>() << QgsMapLayerType::PointCloudLayer;

  return QList< QgsMapLayerType>();
}

bool QgsEptProviderMetadata::uriIsBlocklisted( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( !parts.contains( QStringLiteral( "path" ) ) )
    return false;

  const QFileInfo fi( parts.value( QStringLiteral( "path" ) ).toString() );

  // internal details only
  if ( fi.fileName().compare( QLatin1String( "ept-build.json" ), Qt::CaseInsensitive ) == 0 )
    return true;

  return false;
}

QVariantMap QgsEptProviderMetadata::decodeUri( const QString &uri ) const
{
  QVariantMap uriComponents;
  QUrl url = QUrl::fromUserInput( uri );
  uriComponents.insert( QStringLiteral( "file-name" ), url.fileName() );
  uriComponents.insert( QStringLiteral( "path" ), uri );
  return uriComponents;
}

QString QgsEptProviderMetadata::filters( QgsProviderMetadata::FilterType type )
{
  switch ( type )
  {
    case QgsProviderMetadata::FilterType::FilterVector:
    case QgsProviderMetadata::FilterType::FilterRaster:
    case QgsProviderMetadata::FilterType::FilterMesh:
    case QgsProviderMetadata::FilterType::FilterMeshDataset:
      return QString();

    case QgsProviderMetadata::FilterType::FilterPointCloud:
      return QObject::tr( "Entwine Point Clouds" ) + QStringLiteral( " (ept.json EPT.JSON)" );
  }
  return QString();
}

QgsProviderMetadata::ProviderCapabilities QgsEptProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QList<QgsMapLayerType> QgsEptProviderMetadata::supportedLayerTypes() const
{
  return { QgsMapLayerType::PointCloudLayer };
}

QString QgsEptProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  return path;
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsEptProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::LayerTypesForUri
         | ProviderMetadataCapability::PriorityForUri
         | ProviderMetadataCapability::QuerySublayers;
}
///@endcond

