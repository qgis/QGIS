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

#include "qgsvectortileconnection.h"
#include "qgsvectortiledataitems.h"

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "vectortile" )
#define PROVIDER_DESCRIPTION QStringLiteral( "Vector tile provider" )

QgsVectorTileProviderMetadata::QgsVectorTileProviderMetadata()
  : QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
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
  if ( uriComponents[ QStringLiteral( "type" ) ] == QStringLiteral( "mbtiles" ) )
  {
    uriComponents.insert( QStringLiteral( "path" ), dsUri.param( QStringLiteral( "url" ) ) );
  }
  else
  {
    uriComponents.insert( QStringLiteral( "url" ), dsUri.param( QStringLiteral( "url" ) ) );
  }
  return uriComponents;
}

QString QgsVectorTileProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "type" ), parts.value( QStringLiteral( "type" ) ).toString() );
  dsUri.setParam( QStringLiteral( "url" ), parts.value( parts.contains( QStringLiteral( "path" ) ) ? QStringLiteral( "path" ) : QStringLiteral( "url" ) ).toString() );
  return dsUri.encodedUri();
}

///@endcond
