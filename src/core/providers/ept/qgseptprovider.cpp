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

#include "qgseptprovider.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgseptpointcloudindex.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"
#include "qgsruntimeprofiler.h"
#include "qgsthreadingutils.h"

#include <QFileInfo>
#include <QIcon>

#include "moc_qgseptprovider.cpp"

///@cond PRIVATE

#define PROVIDER_KEY u"ept"_s
#define PROVIDER_DESCRIPTION u"EPT point cloud data provider"_s

QgsEptProvider::QgsEptProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  Qgis::DataProviderReadFlags flags )
  : QgsPointCloudDataProvider( uri, options, flags ), mIndex( new QgsEptPointCloudIndex )
{
  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( QgsApplication::profiler()->groupIsActive( u"projectload"_s ) )
    profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Open data source" ), u"projectload"_s );

  loadIndex( );
  if ( mIndex && !mIndex.isValid() )
  {
    appendError( mIndex.error() );
  }
}

Qgis::DataProviderFlags QgsEptProvider::flags() const
{
  return Qgis::DataProviderFlag::FastExtent2D;
}

QgsEptProvider::~QgsEptProvider() = default;

QgsCoordinateReferenceSystem QgsEptProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex.crs();
}

QgsRectangle QgsEptProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex.extent();
}

QgsPointCloudAttributeCollection QgsEptProvider::attributes() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex.attributes();
}

bool QgsEptProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex.isValid();
}

QString QgsEptProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return u"ept"_s;
}

QString QgsEptProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return u"Point Clouds EPT"_s;
}

QgsPointCloudIndex QgsEptProvider::index() const
{
  // BAD! 2D rendering of point clouds is NOT thread safe
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mIndex;
}

qint64 QgsEptProvider::pointCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex.pointCount();
}

void QgsEptProvider::loadIndex( )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mIndex.isValid() )
    return;

  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( PROVIDER_KEY );
  const QVariantMap decodedUri = metadata->decodeUri( dataSourceUri() );
  const QString authcfg = decodedUri.value( u"authcfg"_s ).toString();
  const QString path = decodedUri.value( u"path"_s ).toString();
  mIndex.load( path, authcfg );
}

QVariantMap QgsEptProvider::originalMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIndex.originalMetadata();
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
  return QgsApplication::getThemeIcon( u"mIconPointCloudLayer.svg"_s );
}

QgsEptProvider *QgsEptProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  return new QgsEptProvider( uri, options, flags );
}

QList<QgsProviderSublayerDetails> QgsEptProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags, QgsFeedback * ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( u"file-name"_s ).toString().compare( "ept.json"_L1, Qt::CaseInsensitive ) == 0 )
  {
    QgsProviderSublayerDetails details;
    details.setUri( uri );
    details.setProviderKey( u"ept"_s );
    details.setType( Qgis::LayerType::PointCloud );
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
  if ( parts.value( u"file-name"_s ).toString().compare( "ept.json"_L1, Qt::CaseInsensitive ) == 0 )
    return 100;

  return 0;
}

QList<Qgis::LayerType> QgsEptProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( u"file-name"_s ).toString().compare( "ept.json"_L1, Qt::CaseInsensitive ) == 0 )
    return QList< Qgis::LayerType>() << Qgis::LayerType::PointCloud;

  return QList< Qgis::LayerType>();
}

bool QgsEptProviderMetadata::uriIsBlocklisted( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( !parts.contains( u"path"_s ) )
    return false;

  const QFileInfo fi( parts.value( u"path"_s ).toString() );

  // internal details only
  if ( fi.fileName().compare( "ept-build.json"_L1, Qt::CaseInsensitive ) == 0 )
    return true;

  return false;
}

QString QgsEptProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QString uri = parts.value( u"path"_s ).toString();

  const QString authcfg = parts.value( u"authcfg"_s ).toString();
  if ( !authcfg.isEmpty() )
    uri += u" authcfg='%1'"_s.arg( authcfg );

  return uri;
}

QVariantMap QgsEptProviderMetadata::decodeUri( const QString &uri ) const
{
  QVariantMap uriComponents;

  const thread_local QRegularExpression rx( u" authcfg='([^']*)'"_s );
  const QRegularExpressionMatch match = rx.match( uri );
  if ( match.hasMatch() )
    uriComponents.insert( u"authcfg"_s, match.captured( 1 ) );

  QString path = uri;
  path.remove( rx );
  path = path.trimmed();
  const QUrl url = QUrl::fromUserInput( path );

  uriComponents.insert( u"path"_s, path );
  uriComponents.insert( u"file-name"_s, url.fileName() );

  return uriComponents;
}

QString QgsEptProviderMetadata::filters( Qgis::FileFilterType type )
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
      return QObject::tr( "Entwine Point Clouds" ) + u" (ept.json EPT.JSON)"_s;
  }
  return QString();
}

QgsProviderMetadata::ProviderCapabilities QgsEptProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QList<Qgis::LayerType> QgsEptProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::PointCloud };
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsEptProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::LayerTypesForUri
         | ProviderMetadataCapability::PriorityForUri
         | ProviderMetadataCapability::QuerySublayers;
}

#undef PROVIDER_KEY
#undef PROVIDER_DESCRIPTION

///@endcond

