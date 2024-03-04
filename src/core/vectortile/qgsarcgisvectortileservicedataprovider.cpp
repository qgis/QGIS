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
#include "qgsblockingnetworkrequest.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsvectortileutils.h"
#include "qgsarcgisrestutils.h"
#include "qgslogger.h"
#include "qgscoordinatetransform.h"

#include <QIcon>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>

///@cond PRIVATE

QString QgsArcGisVectorTileServiceDataProvider::ARCGIS_VT_SERVICE_DATA_PROVIDER_KEY = QStringLiteral( "arcgisvectortileservice" );
QString QgsArcGisVectorTileServiceDataProvider::ARCGIS_VT_SERVICE_DATA_PROVIDER_DESCRIPTION = QObject::tr( "ArcGIS Vector Tile Service data provider" );


QgsArcGisVectorTileServiceDataProvider::QgsArcGisVectorTileServiceDataProvider( const QString &uri, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsXyzVectorTileDataProviderBase( uri, providerOptions, flags )
{
  mIsValid = setupArcgisVectorTileServiceConnection();

  if ( !mIsValid )
    return;

  // populate default metadata
  mLayerMetadata.setIdentifier( mArcgisLayerConfiguration.value( QStringLiteral( "serviceUri" ) ).toString() );
  const QString parentIdentifier = mArcgisLayerConfiguration.value( QStringLiteral( "serviceItemId" ) ).toString();
  if ( !parentIdentifier.isEmpty() )
  {
    mLayerMetadata.setParentIdentifier( parentIdentifier );
  }
  mLayerMetadata.setType( QStringLiteral( "dataset" ) );
  mLayerMetadata.setTitle( mArcgisLayerConfiguration.value( QStringLiteral( "name" ) ).toString() );
  const QString copyright = mArcgisLayerConfiguration.value( QStringLiteral( "copyrightText" ) ).toString();
  if ( !copyright.isEmpty() )
    mLayerMetadata.setRights( QStringList() << copyright );
  mLayerMetadata.addLink( QgsAbstractMetadataBase::Link( tr( "Source" ), QStringLiteral( "WWW:LINK" ), mArcgisLayerConfiguration.value( QStringLiteral( "serviceUri" ) ).toString() ) );
}

QgsArcGisVectorTileServiceDataProvider::QgsArcGisVectorTileServiceDataProvider( const QgsArcGisVectorTileServiceDataProvider &other )
  : QgsXyzVectorTileDataProviderBase( other )
{
  mIsValid = other.mIsValid;
  mExtent = other.mExtent;
  mMatrixSet = other.mMatrixSet;
  mSourcePath = other.mSourcePath;
  mArcgisLayerConfiguration = other.mArcgisLayerConfiguration;
  mArcgisStyleConfiguration = other.mArcgisStyleConfiguration;
  mCrs = other.mCrs;
  mLayerMetadata = other.mLayerMetadata;
}

Qgis::DataProviderFlags QgsArcGisVectorTileServiceDataProvider::flags() const
{
  return Qgis::DataProviderFlag::FastExtent2D;
}

Qgis::VectorTileProviderFlags QgsArcGisVectorTileServiceDataProvider::providerFlags() const
{
  return QgsXyzVectorTileDataProviderBase::providerFlags() | Qgis::VectorTileProviderFlag::AlwaysUseTileMatrixSetFromProvider;
}

Qgis::VectorTileProviderCapabilities QgsArcGisVectorTileServiceDataProvider::providerCapabilities() const
{
  return Qgis::VectorTileProviderCapability::ReadLayerMetadata;
}

QString QgsArcGisVectorTileServiceDataProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return ARCGIS_VT_SERVICE_DATA_PROVIDER_KEY;
}

QString QgsArcGisVectorTileServiceDataProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return ARCGIS_VT_SERVICE_DATA_PROVIDER_DESCRIPTION;
}

QgsVectorTileDataProvider *QgsArcGisVectorTileServiceDataProvider::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return new QgsArcGisVectorTileServiceDataProvider( *this );
}

QString QgsArcGisVectorTileServiceDataProvider::sourcePath() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSourcePath;
}

bool QgsArcGisVectorTileServiceDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIsValid;
}

QgsRectangle QgsArcGisVectorTileServiceDataProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mExtent;
}

const QgsVectorTileMatrixSet &QgsArcGisVectorTileServiceDataProvider::tileMatrixSet() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMatrixSet;
}

QgsCoordinateReferenceSystem QgsArcGisVectorTileServiceDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCrs;
}

QgsLayerMetadata QgsArcGisVectorTileServiceDataProvider::layerMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLayerMetadata;
}

QVariantMap QgsArcGisVectorTileServiceDataProvider::styleDefinition() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mArcgisStyleConfiguration;
}

QString QgsArcGisVectorTileServiceDataProvider::styleUrl() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // for ArcMap VectorTileServices we default to the defaultStyles URL from the layer configuration
  return mArcgisLayerConfiguration.value( QStringLiteral( "serviceUri" ) ).toString()
         + '/' + mArcgisLayerConfiguration.value( QStringLiteral( "defaultStyles" ) ).toString();
}

QString QgsArcGisVectorTileServiceDataProvider::htmlMetadata() const
{
  QString metadata;

  if ( !mTileMapUrl.isEmpty() )
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Tilemap" ) % QStringLiteral( "</td><td><a href=\"%1\">%1</a>" ).arg( mTileMapUrl ) % QStringLiteral( "</td></tr>\n" );

  return metadata;
}

bool QgsArcGisVectorTileServiceDataProvider::setupArcgisVectorTileServiceConnection()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  QString tileServiceUri = dsUri.param( QStringLiteral( "url" ) );

  QUrl url( tileServiceUri );
  // some services don't default to json format, while others do... so let's explicitly request it!
  // (refs https://github.com/qgis/QGIS/issues/4231)
  QUrlQuery query;
  query.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "pjson" ) );
  url.setQuery( query );

  QNetworkRequest request = QNetworkRequest( url );

  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsVectorTileLayer" ) )

  QgsBlockingNetworkRequest networkRequest;
  switch ( networkRequest.get( request ) )
  {
    case QgsBlockingNetworkRequest::NoError:
      break;

    case QgsBlockingNetworkRequest::NetworkError:
    case QgsBlockingNetworkRequest::TimeoutError:
    case QgsBlockingNetworkRequest::ServerExceptionError:
      return false;
  }

  const QgsNetworkReplyContent content = networkRequest.reply();
  const QByteArray raw = content.content();

  // Parse data
  QJsonParseError err;
  const QJsonDocument doc = QJsonDocument::fromJson( raw, &err );
  if ( doc.isNull() )
  {
    return false;
  }

  mArcgisLayerConfiguration = doc.object().toVariantMap();
  if ( mArcgisLayerConfiguration.contains( QStringLiteral( "error" ) ) )
  {
    return false;
  }

  if ( !mArcgisLayerConfiguration.value( QStringLiteral( "tiles" ) ).isValid() )
  {
    // maybe url is pointing to a resources/styles/root.json type url, that's ok too!
    const QString sourceUri = mArcgisLayerConfiguration.value( QStringLiteral( "sources" ) ).toMap().value( QStringLiteral( "esri" ) ).toMap().value( QStringLiteral( "url" ) ).toString();
    if ( !sourceUri.isEmpty() )
    {
      QUrl url( sourceUri );
      // some services don't default to json format, while others do... so let's explicitly request it!
      // (refs https://github.com/qgis/QGIS/issues/4231)
      QUrlQuery query;
      query.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "pjson" ) );
      url.setQuery( query );

      QNetworkRequest request = QNetworkRequest( url );

      QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsVectorTileLayer" ) )

      QgsBlockingNetworkRequest networkRequest;
      switch ( networkRequest.get( request ) )
      {
        case QgsBlockingNetworkRequest::NoError:
          break;

        case QgsBlockingNetworkRequest::NetworkError:
        case QgsBlockingNetworkRequest::TimeoutError:
        case QgsBlockingNetworkRequest::ServerExceptionError:
          return false;
      }

      const QgsNetworkReplyContent content = networkRequest.reply();
      const QByteArray raw = content.content();

      // Parse data
      QJsonParseError err;
      const QJsonDocument doc = QJsonDocument::fromJson( raw, &err );
      if ( doc.isNull() )
      {
        return false;
      }

      tileServiceUri = sourceUri;

      // the resources/styles/root.json configuration is actually our style definition
      mArcgisStyleConfiguration = mArcgisLayerConfiguration;
      mArcgisLayerConfiguration = doc.object().toVariantMap();
      if ( mArcgisLayerConfiguration.contains( QStringLiteral( "error" ) ) )
      {
        return false;
      }
    }
  }

  // read tileMap if available
  QVariantMap tileMap;
  const QString tileMapEndpoint = mArcgisLayerConfiguration.value( QStringLiteral( "tileMap" ) ).toString();
  if ( !tileMapEndpoint.isEmpty() )
  {
    mTileMapUrl = tileServiceUri + '/' + tileMapEndpoint;
    QUrl tilemapUrl( mTileMapUrl );
    tilemapUrl.setQuery( query );

    QNetworkRequest tileMapRequest = QNetworkRequest( tilemapUrl );
    QgsSetRequestInitiatorClass( tileMapRequest, QStringLiteral( "QgsVectorTileLayer" ) )

    QgsBlockingNetworkRequest tileMapNetworkRequest;
    switch ( tileMapNetworkRequest.get( tileMapRequest ) )
    {
      case QgsBlockingNetworkRequest::NoError:
        break;

      case QgsBlockingNetworkRequest::NetworkError:
      case QgsBlockingNetworkRequest::TimeoutError:
      case QgsBlockingNetworkRequest::ServerExceptionError:
        return false;
    }

    const QgsNetworkReplyContent tileMapContent = tileMapNetworkRequest.reply();
    const QByteArray tileMapRaw = tileMapContent.content();

    const QJsonDocument tileMapDoc = QJsonDocument::fromJson( tileMapRaw, &err );
    if ( !tileMapDoc.isNull() )
    {
      tileMap = tileMapDoc.object().toVariantMap();
    }
  }

  mSourcePath = tileServiceUri + '/' + mArcgisLayerConfiguration.value( QStringLiteral( "tiles" ) ).toList().value( 0 ).toString();
  if ( !QgsVectorTileUtils::checkXYZUrlTemplate( mSourcePath ) )
  {
    QgsDebugError( QStringLiteral( "Invalid format of URL for XYZ source: " ) + tileServiceUri );
    return false;
  }

  mArcgisLayerConfiguration.insert( QStringLiteral( "serviceUri" ), tileServiceUri );

  mMatrixSet.fromEsriJson( mArcgisLayerConfiguration, tileMap );
  mCrs = mMatrixSet.crs();

  // if hardcoded zoom limits aren't specified, take them from the server
  if ( dsUri.hasParam( QStringLiteral( "zmin" ) ) )
    mMatrixSet.dropMatricesOutsideZoomRange( dsUri.param( QStringLiteral( "zmin" ) ).toInt(), 99 );

  if ( dsUri.hasParam( QStringLiteral( "zmax" ) ) )
    mMatrixSet.dropMatricesOutsideZoomRange( 0, dsUri.param( QStringLiteral( "zmax" ) ).toInt() );

  const QVariantMap fullExtent = mArcgisLayerConfiguration.value( QStringLiteral( "fullExtent" ) ).toMap();
  if ( !fullExtent.isEmpty() )
  {
    const QgsRectangle fullExtentRect(
      fullExtent.value( QStringLiteral( "xmin" ) ).toDouble(),
      fullExtent.value( QStringLiteral( "ymin" ) ).toDouble(),
      fullExtent.value( QStringLiteral( "xmax" ) ).toDouble(),
      fullExtent.value( QStringLiteral( "ymax" ) ).toDouble()
    );

    const QgsCoordinateReferenceSystem fullExtentCrs = QgsArcGisRestUtils::convertSpatialReference( fullExtent.value( QStringLiteral( "spatialReference" ) ).toMap() );
    const QgsCoordinateTransform extentTransform( fullExtentCrs, mCrs, transformContext() );
    try
    {
      mExtent = extentTransform.transformBoundingBox( fullExtentRect );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "Could not transform layer fullExtent to layer CRS" ) );
    }
  }
  else
  {
    // if no fullExtent specified in JSON, default to web mercator specs full extent
    const QgsCoordinateTransform extentTransform( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), mCrs, transformContext() );
    try
    {
      mExtent = extentTransform.transformBoundingBox( QgsRectangle( -20037508.3427892, -20037508.3427892, 20037508.3427892, 20037508.3427892 ) );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "Could not transform layer extent to layer CRS" ) );
    }
  }

  return true;
}


//
// QgsArcGisVectorTileServiceDataProviderMetadata
//

QgsArcGisVectorTileServiceDataProviderMetadata::QgsArcGisVectorTileServiceDataProviderMetadata()
  : QgsProviderMetadata( QgsArcGisVectorTileServiceDataProvider::ARCGIS_VT_SERVICE_DATA_PROVIDER_KEY,
                         QgsArcGisVectorTileServiceDataProvider::ARCGIS_VT_SERVICE_DATA_PROVIDER_DESCRIPTION )
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

QgsArcGisVectorTileServiceDataProvider *QgsArcGisVectorTileServiceDataProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsArcGisVectorTileServiceDataProvider( uri, options, flags );
}

QVariantMap QgsArcGisVectorTileServiceDataProviderMetadata::decodeUri( const QString &uri ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  uriComponents.insert( QStringLiteral( "serviceType" ), QStringLiteral( "arcgis" ) );
  uriComponents.insert( QStringLiteral( "url" ), dsUri.param( QStringLiteral( "url" ) ) );

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
  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  dsUri.setParam( QStringLiteral( "serviceType" ), QStringLiteral( "arcgis" ) );
  dsUri.setParam( QStringLiteral( "url" ), parts.value( QStringLiteral( "url" ) ).toString() );

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


