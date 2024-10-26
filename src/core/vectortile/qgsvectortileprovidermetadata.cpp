/***************************************************************************
  qgsvectortileprovidermetadata.cpp
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

#include "qgsvectortileprovidermetadata.h"
#include "moc_qgsvectortileprovidermetadata.cpp"

#include "qgsvectortileconnection.h"
#include "qgsvectortiledataitems.h"
#include "qgsapplication.h"
#include "qgsreadwritecontext.h"

#include <QUrl>

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "vectortile" )
#define PROVIDER_DESCRIPTION QStringLiteral( "Vector tile provider" )

QgsVectorTileProviderMetadata::QgsVectorTileProviderMetadata()
  : QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QIcon QgsVectorTileProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconVectorTileLayer.svg" ) );
}

QList<QgsDataItemProvider *> QgsVectorTileProviderMetadata::dataItemProviders() const
{
  QList< QgsDataItemProvider * > providers;
  providers << new QgsVectorTileDataItemProvider;
  return providers;
}

QMap<QString, QgsAbstractProviderConnection *> QgsVectorTileProviderMetadata::connections( bool cached )
{
  return connectionsProtected<QgsVectorTileProviderConnection, QgsVectorTileProviderConnection>( cached );
}

QgsAbstractProviderConnection *QgsVectorTileProviderMetadata::createConnection( const QString &name )
{
  return new QgsVectorTileProviderConnection( name );
}

void QgsVectorTileProviderMetadata::deleteConnection( const QString &name )
{
  deleteConnectionProtected<QgsVectorTileProviderConnection>( name );
}

void QgsVectorTileProviderMetadata::saveConnection( const QgsAbstractProviderConnection *connection, const QString &name )
{
  saveConnectionProtected( connection, name );
}

QgsProviderMetadata::ProviderCapabilities QgsVectorTileProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QVariantMap QgsVectorTileProviderMetadata::decodeUri( const QString &uri ) const
{
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

QString QgsVectorTileProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
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

QString QgsVectorTileProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  const QString sourceType = dsUri.param( QStringLiteral( "type" ) );
  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( sourceType == QLatin1String( "xyz" ) )
  {
    const QUrl sourceUrl( sourcePath );
    if ( sourceUrl.isLocalFile() )
    {
      // relative path will become "file:./x.txt"
      const QString relSrcUrl = context.pathResolver().writePath( sourceUrl.toLocalFile() );
      dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
      dsUri.setParam( QStringLiteral( "url" ), QUrl::fromLocalFile( relSrcUrl ).toString() );
      return dsUri.encodedUri();
    }
  }
  else if ( sourceType == QLatin1String( "mbtiles" ) )
  {
    sourcePath = context.pathResolver().writePath( sourcePath );
    dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
    dsUri.setParam( QStringLiteral( "url" ), sourcePath );
    return dsUri.encodedUri();
  }

  return uri;
}

QString QgsVectorTileProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  const QString sourceType = dsUri.param( QStringLiteral( "type" ) );
  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( sourceType == QLatin1String( "xyz" ) )
  {
    const QUrl sourceUrl( sourcePath );
    if ( sourceUrl.isLocalFile() )  // file-based URL? convert to relative path
    {
      const QString absSrcUrl = context.pathResolver().readPath( sourceUrl.toLocalFile() );
      dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
      dsUri.setParam( QStringLiteral( "url" ), QUrl::fromLocalFile( absSrcUrl ).toString() );
      return dsUri.encodedUri();
    }
  }
  else if ( sourceType == QLatin1String( "mbtiles" ) )
  {
    sourcePath = context.pathResolver().readPath( sourcePath );
    dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
    dsUri.setParam( QStringLiteral( "url" ), sourcePath );
    return dsUri.encodedUri();
  }

  return uri;
}

QList<Qgis::LayerType> QgsVectorTileProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::VectorTile };
}

///@endcond
