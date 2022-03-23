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
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"

#include <QFileInfo>

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "copc" )
#define PROVIDER_DESCRIPTION QStringLiteral( "COPC point cloud data provider" )

QgsCopcProvider::QgsCopcProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsPointCloudDataProvider( uri, options, flags )
{
  if ( uri.startsWith( QStringLiteral( "http" ), Qt::CaseSensitivity::CaseInsensitive ) )
    mIndex.reset( nullptr );
  else
    mIndex.reset( new QgsCopcPointCloudIndex );

  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
    profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Open data source" ), QStringLiteral( "projectload" ) );

  loadIndex( );
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

QVariantList QgsCopcProvider::metadataClasses( const QString &attribute ) const
{
  return mIndex->metadataClasses( attribute );
}

QVariant QgsCopcProvider::metadataClassStatistic( const QString &attribute, const QVariant &value, QgsStatisticalSummary::Statistic statistic ) const
{
  return mIndex->metadataClassStatistic( attribute, value, statistic );
}

void QgsCopcProvider::loadIndex( )
{
  if ( mIndex->isValid() )
    return;

  mIndex->load( dataSourceUri() );

  // TODO: Read metadata from LAZ into mOriginalMetadata
  // NOTE: This is not done in EPT provider either
}

QVariantMap QgsCopcProvider::originalMetadata() const
{
  return mOriginalMetadata;
}

void QgsCopcProvider::generateIndex()
{
  //no-op, index is always generated
}

QVariant QgsCopcProvider::metadataStatistic( const QString &attribute, QgsStatisticalSummary::Statistic statistic ) const
{
  return mIndex->metadataStatistic( attribute, statistic );
}

QgsCopcProviderMetadata::QgsCopcProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QgsCopcProvider *QgsCopcProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsCopcProvider( uri, options, flags );
}

QList<QgsProviderSublayerDetails> QgsCopcProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags, QgsFeedback * ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "isCopc" ), false ).toBool() )
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
  const QFileInfo fi( parts.value( QStringLiteral( "path" ) ).toString() );
  if ( fi.exists() && parts.value( QStringLiteral( "isCopc" ), false ).toBool() )
    return 100;

  return 0;
}

QList<QgsMapLayerType> QgsCopcProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  const QFileInfo fi( parts.value( QStringLiteral( "path" ) ).toString() );
  if ( fi.exists() && parts.value( QStringLiteral( "isCopc" ), false ).toBool() )
    return QList< QgsMapLayerType>() << QgsMapLayerType::PointCloudLayer;

  return QList< QgsMapLayerType>();
}

bool QgsCopcProviderMetadata::uriIsBlocklisted( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( !parts.contains( QStringLiteral( "path" ) ) )
    return false;

  const QFileInfo fi( parts.value( QStringLiteral( "path" ) ).toString() );

  // internal details only
  if ( fi.exists() && parts.value( QStringLiteral( "isCopc" ), false ).toBool() )
    return true;

  return false;
}

QVariantMap QgsCopcProviderMetadata::decodeUri( const QString &uri ) const
{
  const QString path = uri;
  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "path" ), path );
  uriComponents.insert( QStringLiteral( "isCopc" ), uri.endsWith( ".copc.laz" ) );
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
      return QObject::tr( "COPC Point Clouds" ) + QStringLiteral( "COPC LAZ files (*.copc.laz *.COPC.LAZ)" );
  }
  return QString();
}

QgsProviderMetadata::ProviderCapabilities QgsCopcProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
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

