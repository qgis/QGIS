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

#include "qgsapplication.h"
#include "qgsreadwritecontext.h"
#include "qgsvectortileconnection.h"
#include "qgsvectortiledataitems.h"

#include <QUrl>

#include "moc_qgsvectortileprovidermetadata.cpp"

///@cond PRIVATE

#define PROVIDER_KEY u"vectortile"_s
#define PROVIDER_DESCRIPTION u"Vector tile provider"_s

QgsVectorTileProviderMetadata::QgsVectorTileProviderMetadata()
  : QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QIcon QgsVectorTileProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconVectorTileLayer.svg"_s );
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
  uriComponents.insert( u"type"_s, dsUri.param( u"type"_s ) );
  if ( dsUri.hasParam( u"serviceType"_s ) )
    uriComponents.insert( u"serviceType"_s, dsUri.param( u"serviceType"_s ) );

  if ( uriComponents[ u"type"_s ] == "mbtiles"_L1 ||
       ( uriComponents[ u"type"_s ] == "xyz"_L1 &&
         !dsUri.param( u"url"_s ).startsWith( "http"_L1 ) ) )
  {
    uriComponents.insert( u"path"_s, dsUri.param( u"url"_s ) );
  }
  else
  {
    uriComponents.insert( u"url"_s, dsUri.param( u"url"_s ) );
  }

  if ( dsUri.hasParam( u"zmin"_s ) )
    uriComponents.insert( u"zmin"_s, dsUri.param( u"zmin"_s ) );
  if ( dsUri.hasParam( u"zmax"_s ) )
    uriComponents.insert( u"zmax"_s, dsUri.param( u"zmax"_s ) );

  dsUri.httpHeaders().updateMap( uriComponents );

  if ( dsUri.hasParam( u"styleUrl"_s ) )
    uriComponents.insert( u"styleUrl"_s, dsUri.param( u"styleUrl"_s ) );

  const QString authcfg = dsUri.authConfigId();
  if ( !authcfg.isEmpty() )
    uriComponents.insert( u"authcfg"_s, authcfg );

  return uriComponents;
}

QString QgsVectorTileProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( u"type"_s, parts.value( u"type"_s ).toString() );
  if ( parts.contains( u"serviceType"_s ) )
    dsUri.setParam( u"serviceType"_s, parts[ u"serviceType"_s ].toString() );
  dsUri.setParam( u"url"_s, parts.value( parts.contains( u"path"_s ) ? u"path"_s : u"url"_s ).toString() );

  if ( parts.contains( u"zmin"_s ) )
    dsUri.setParam( u"zmin"_s, parts[ u"zmin"_s ].toString() );
  if ( parts.contains( u"zmax"_s ) )
    dsUri.setParam( u"zmax"_s, parts[ u"zmax"_s ].toString() );

  dsUri.httpHeaders().setFromMap( parts );

  if ( parts.contains( u"styleUrl"_s ) )
    dsUri.setParam( u"styleUrl"_s, parts[ u"styleUrl"_s ].toString() );

  if ( parts.contains( u"authcfg"_s ) )
    dsUri.setAuthConfigId( parts[ u"authcfg"_s ].toString() );

  return dsUri.encodedUri();
}

QString QgsVectorTileProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  const QString sourceType = dsUri.param( u"type"_s );
  QString sourcePath = dsUri.param( u"url"_s );
  if ( sourceType == "xyz"_L1 )
  {
    const QUrl sourceUrl( sourcePath );
    if ( sourceUrl.isLocalFile() )
    {
      // relative path will become "file:./x.txt"
      const QString relSrcUrl = context.pathResolver().writePath( sourceUrl.toLocalFile() );
      dsUri.removeParam( u"url"_s );  // needed because setParam() would insert second "url" key
      dsUri.setParam( u"url"_s, QUrl::fromLocalFile( relSrcUrl ).toString( QUrl::DecodeReserved ) );
      return dsUri.encodedUri();
    }
  }
  else if ( sourceType == "mbtiles"_L1 )
  {
    sourcePath = context.pathResolver().writePath( sourcePath );
    dsUri.removeParam( u"url"_s );  // needed because setParam() would insert second "url" key
    dsUri.setParam( u"url"_s, sourcePath );
    return dsUri.encodedUri();
  }

  return uri;
}

QString QgsVectorTileProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  const QString sourceType = dsUri.param( u"type"_s );
  QString sourcePath = dsUri.param( u"url"_s );
  if ( sourceType == "xyz"_L1 )
  {
    const QUrl sourceUrl( sourcePath );
    if ( sourceUrl.isLocalFile() )  // file-based URL? convert to relative path
    {
      const QString absSrcUrl = context.pathResolver().readPath( sourceUrl.toLocalFile() );
      dsUri.removeParam( u"url"_s );  // needed because setParam() would insert second "url" key
      dsUri.setParam( u"url"_s, QUrl::fromLocalFile( absSrcUrl ).toString( QUrl::DecodeReserved ) );
      return dsUri.encodedUri();
    }
  }
  else if ( sourceType == "mbtiles"_L1 )
  {
    sourcePath = context.pathResolver().readPath( sourcePath );
    dsUri.removeParam( u"url"_s );  // needed because setParam() would insert second "url" key
    dsUri.setParam( u"url"_s, sourcePath );
    return dsUri.encodedUri();
  }

  return uri;
}

QList<Qgis::LayerType> QgsVectorTileProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::VectorTile };
}

///@endcond
