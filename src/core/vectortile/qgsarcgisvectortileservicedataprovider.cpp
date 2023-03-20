/***************************************************************************
  qgsarcgisvectortileservicedataprovider.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsarcgisvectortileservicedataprovider.h"
#include "qgsthreadingutils.h"
#include "qgsapplication.h"
#include <QIcon>

///@cond PRIVATE

QString QgsArcGisVectorTileServiceDataProvider::DATA_PROVIDER_KEY = QStringLiteral( "arcgisvectortileservice" );
QString QgsArcGisVectorTileServiceDataProvider::DATA_PROVIDER_DESCRIPTION = QObject::tr( "ArcGIS Vector Tile Service data provider" );


QgsArcGisVectorTileServiceDataProvider::QgsArcGisVectorTileServiceDataProvider( const QString &uri, const QString &sourcePath, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsXyzVectorTileDataProvider( uri, providerOptions, flags )
  , mSourcePath( sourcePath )
{

}

QString QgsArcGisVectorTileServiceDataProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return DATA_PROVIDER_KEY;
}

QString QgsArcGisVectorTileServiceDataProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return DATA_PROVIDER_DESCRIPTION;
}

QgsVectorTileDataProvider *QgsArcGisVectorTileServiceDataProvider::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  ProviderOptions options;
  options.transformContext = transformContext();
  return new QgsArcGisVectorTileServiceDataProvider( dataSourceUri(), mSourcePath, options, mReadFlags );
}

QString QgsArcGisVectorTileServiceDataProvider::sourcePath() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSourcePath;
}

bool QgsArcGisVectorTileServiceDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return true;
}


//
// QgsArcGisVectorTileServiceDataProviderMetadata
//

QgsArcGisVectorTileServiceDataProviderMetadata::QgsArcGisVectorTileServiceDataProviderMetadata()
  : QgsProviderMetadata( QgsArcGisVectorTileServiceDataProvider::DATA_PROVIDER_KEY, QgsArcGisVectorTileServiceDataProvider::DATA_PROVIDER_DESCRIPTION )
{
}

QIcon QgsArcGisVectorTileServiceDataProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconVectorTileLayer.svg" ) );
}

QgsProviderMetadata::ProviderCapabilities QgsArcGisVectorTileServiceDataProviderMetadata::providerCapabilities() const
{
  return QgsProviderMetadata::ProviderCapabilities();
}

QVariantMap QgsArcGisVectorTileServiceDataProviderMetadata::decodeUri( const QString &uri ) const
{
  // TODO -- carefully thin out options which don't apply to arcgis vector tile services

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "type" ), dsUri.param( QStringLiteral( "type" ) ) );
  if ( dsUri.hasParam( QStringLiteral( "serviceType" ) ) )
    uriComponents.insert( QStringLiteral( "serviceType" ), dsUri.param( QStringLiteral( "serviceType" ) ) );

  if ( uriComponents[ QStringLiteral( "type" ) ] == QLatin1String( "mbtiles" ) ||
       ( uriComponents[ QStringLiteral( "type" ) ] == QLatin1String( "xyz" ) &&
         !dsUri.param( QStringLiteral( "url" ) ).startsWith( QLatin1String( "http" ) ) ) )
  {
    uriComponents.insert( QStringLiteral( "path" ), dsUri.param( QStringLiteral( "url" ) ) );
  }
  else
  {
    uriComponents.insert( QStringLiteral( "url" ), dsUri.param( QStringLiteral( "url" ) ) );
  }

  if ( dsUri.hasParam( QStringLiteral( "zmin" ) ) )
    uriComponents.insert( QStringLiteral( "zmin" ), dsUri.param( QStringLiteral( "zmin" ) ) );
  if ( dsUri.hasParam( QStringLiteral( "zmax" ) ) )
    uriComponents.insert( QStringLiteral( "zmax" ), dsUri.param( QStringLiteral( "zmax" ) ) );

  dsUri.httpHeaders().updateMap( uriComponents );

  if ( dsUri.hasParam( QStringLiteral( "styleUrl" ) ) )
    uriComponents.insert( QStringLiteral( "styleUrl" ), dsUri.param( QStringLiteral( "styleUrl" ) ) );

  const QString authcfg = dsUri.authConfigId();
  if ( !authcfg.isEmpty() )
    uriComponents.insert( QStringLiteral( "authcfg" ), authcfg );

  return uriComponents;
}

QString QgsArcGisVectorTileServiceDataProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  // TODO -- carefully thin out options which don't apply to arcgis vector tile services

  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "type" ), parts.value( QStringLiteral( "type" ) ).toString() );
  if ( parts.contains( QStringLiteral( "serviceType" ) ) )
    dsUri.setParam( QStringLiteral( "serviceType" ), parts[ QStringLiteral( "serviceType" ) ].toString() );
  dsUri.setParam( QStringLiteral( "url" ), parts.value( parts.contains( QStringLiteral( "path" ) ) ? QStringLiteral( "path" ) : QStringLiteral( "url" ) ).toString() );

  if ( parts.contains( QStringLiteral( "zmin" ) ) )
    dsUri.setParam( QStringLiteral( "zmin" ), parts[ QStringLiteral( "zmin" ) ].toString() );
  if ( parts.contains( QStringLiteral( "zmax" ) ) )
    dsUri.setParam( QStringLiteral( "zmax" ), parts[ QStringLiteral( "zmax" ) ].toString() );

  dsUri.httpHeaders().setFromMap( parts );

  if ( parts.contains( QStringLiteral( "styleUrl" ) ) )
    dsUri.setParam( QStringLiteral( "styleUrl" ), parts[ QStringLiteral( "styleUrl" ) ].toString() );

  if ( parts.contains( QStringLiteral( "authcfg" ) ) )
    dsUri.setAuthConfigId( parts[ QStringLiteral( "authcfg" ) ].toString() );

  return dsUri.encodedUri();
}

QString QgsArcGisVectorTileServiceDataProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext & ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );
  return uri;
}

QString QgsArcGisVectorTileServiceDataProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext & ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );
  return uri;
}

QList<Qgis::LayerType> QgsArcGisVectorTileServiceDataProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::VectorTile };
}

///@endcond


