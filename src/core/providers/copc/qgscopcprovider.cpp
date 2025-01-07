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
#include "qgscopcprovider.h"
#include "moc_qgscopcprovider.cpp"
#include "qgscopcpointcloudindex.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"
#include "qgsthreadingutils.h"

#include <QIcon>

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "copc" )
#define PROVIDER_DESCRIPTION QStringLiteral( "COPC point cloud data provider" )

QgsCopcProvider::QgsCopcProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  Qgis::DataProviderReadFlags flags )
  : QgsPointCloudDataProvider( uri, options, flags ), mIndex( new QgsCopcPointCloudIndex )
{
  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
    profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Open data source" ), QStringLiteral( "projectload" ) );

  loadIndex( );
  if ( !mIndex.isValid() )
  {
    appendError( mIndex.error() );
  }
}

QgsCopcProvider::~QgsCopcProvider() = default;

QgsCoordinateReferenceSystem QgsCopcProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex.crs();
}

Qgis::DataProviderFlags QgsCopcProvider::flags() const
{
  return Qgis::DataProviderFlag::FastExtent2D;
}

QgsRectangle QgsCopcProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex.extent();
}

QgsPointCloudAttributeCollection QgsCopcProvider::attributes() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex.attributes();
}

bool QgsCopcProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex.isValid();
}

QString QgsCopcProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QStringLiteral( "copc" );
}

QString QgsCopcProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QStringLiteral( "Point Clouds COPC" );
}

QgsPointCloudIndex QgsCopcProvider::index() const
{
  // non fatal for now -- 2d rendering of point clouds is not thread safe and calls this
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mIndex;
}

qint64 QgsCopcProvider::pointCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex.pointCount();
}

void QgsCopcProvider::loadIndex( )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // Index already loaded -> no need to load
  if ( mIndex.isValid() )
    return;
  mIndex.load( dataSourceUri() );
}

QVariantMap QgsCopcProvider::originalMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex.originalMetadata();
}

void QgsCopcProvider::generateIndex()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  //no-op, index is always generated
}

QgsPointCloudDataProvider::Capabilities QgsCopcProvider::capabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsPointCloudDataProvider::Capability::ChangeAttributeValues;
}

QgsCopcProviderMetadata::QgsCopcProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QIcon QgsCopcProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconPointCloudLayer.svg" ) );
}

QgsCopcProvider *QgsCopcProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
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
    details.setType( Qgis::LayerType::PointCloud );
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

QList<Qgis::LayerType> QgsCopcProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().endsWith( ".copc.laz", Qt::CaseSensitivity::CaseInsensitive ) )
    return QList< Qgis::LayerType>() << Qgis::LayerType::PointCloud;

  return QList< Qgis::LayerType>();
}

QVariantMap QgsCopcProviderMetadata::decodeUri( const QString &uri ) const
{
  QVariantMap uriComponents;
  QUrl url = QUrl::fromUserInput( uri );
  uriComponents.insert( QStringLiteral( "file-name" ), url.fileName() );
  uriComponents.insert( QStringLiteral( "path" ), uri );
  return uriComponents;
}

QString QgsCopcProviderMetadata::filters( Qgis::FileFilterType type )
{
  switch ( type )
  {
    case Qgis::FileFilterType::Vector:
    case Qgis::FileFilterType::Raster:
    case Qgis::FileFilterType::Mesh:
    case Qgis::FileFilterType::MeshDataset:
    case Qgis::FileFilterType::VectorTile:
    case Qgis::FileFilterType::TiledScene:
      return QString();

    case Qgis::FileFilterType::PointCloud:
      return QObject::tr( "COPC Point Clouds" ) + QStringLiteral( " (*.copc.laz *.COPC.LAZ)" );
  }
  return QString();
}

QgsProviderMetadata::ProviderCapabilities QgsCopcProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QList<Qgis::LayerType> QgsCopcProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::PointCloud };
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

#undef PROVIDER_KEY
#undef PROVIDER_DESCRIPTION

///@endcond

