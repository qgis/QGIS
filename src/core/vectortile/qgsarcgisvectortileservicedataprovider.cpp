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

#include "qgsapplication.h"
#include "qgsarcgisrestutils.h"
#include "qgsblockingnetworkrequest.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsthreadingutils.h"
#include "qgsvectortileutils.h"

#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QUrl>
#include <QUrlQuery>

#include "moc_qgsarcgisvectortileservicedataprovider.cpp"

///@cond PRIVATE

QString QgsArcGisVectorTileServiceDataProvider::ARCGIS_VT_SERVICE_DATA_PROVIDER_KEY = u"arcgisvectortileservice"_s;
QString QgsArcGisVectorTileServiceDataProvider::ARCGIS_VT_SERVICE_DATA_PROVIDER_DESCRIPTION = QObject::tr( "ArcGIS Vector Tile Service data provider" );


QgsArcGisVectorTileServiceDataProvider::QgsArcGisVectorTileServiceDataProvider( const QString &uri, const ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags )
  : QgsXyzVectorTileDataProviderBase( uri, providerOptions, flags )
{
  mIsValid = setupArcgisVectorTileServiceConnection();

  if ( !mIsValid )
    return;

  // populate default metadata
  mLayerMetadata.setIdentifier( mArcgisLayerConfiguration.value( u"serviceUri"_s ).toString() );
  const QString parentIdentifier = mArcgisLayerConfiguration.value( u"serviceItemId"_s ).toString();
  if ( !parentIdentifier.isEmpty() )
  {
    mLayerMetadata.setParentIdentifier( parentIdentifier );
  }
  mLayerMetadata.setType( u"dataset"_s );
  mLayerMetadata.setTitle( mArcgisLayerConfiguration.value( u"name"_s ).toString() );
  const QString copyright = mArcgisLayerConfiguration.value( u"copyrightText"_s ).toString();
  if ( !copyright.isEmpty() )
    mLayerMetadata.setRights( QStringList() << copyright );
  mLayerMetadata.addLink( QgsAbstractMetadataBase::Link( tr( "Source" ), u"WWW:LINK"_s, mArcgisLayerConfiguration.value( u"serviceUri"_s ).toString() ) );
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
  return mArcgisLayerConfiguration.value( u"serviceUri"_s ).toString()
         + '/' + mArcgisLayerConfiguration.value( u"defaultStyles"_s ).toString();
}

QString QgsArcGisVectorTileServiceDataProvider::htmlMetadata() const
{
  QString metadata;

  if ( !mTileMapUrl.isEmpty() )
    metadata += u"<tr><td class=\"highlight\">"_s % tr( "Tilemap" ) % u"</td><td><a href=\"%1\">%1</a>"_s.arg( mTileMapUrl ) % u"</td></tr>\n"_s;

  return metadata;
}

bool QgsArcGisVectorTileServiceDataProvider::setupArcgisVectorTileServiceConnection()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );
  QString tileServiceUri = dsUri.param( u"url"_s );

  QUrl url( tileServiceUri );
  // some services don't default to json format, while others do... so let's explicitly request it!
  // (refs https://github.com/qgis/QGIS/issues/4231)
  QUrlQuery query;
  query.addQueryItem( u"f"_s, u"pjson"_s );
  url.setQuery( query );

  QNetworkRequest request = QNetworkRequest( url );

  QgsSetRequestInitiatorClass( request, u"QgsVectorTileLayer"_s )

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
  if ( mArcgisLayerConfiguration.contains( u"error"_s ) )
  {
    return false;
  }

  if ( !mArcgisLayerConfiguration.value( u"tiles"_s ).isValid() )
  {
    // maybe url is pointing to a resources/styles/root.json type url, that's ok too!
    const QString sourceUri = mArcgisLayerConfiguration.value( u"sources"_s ).toMap().value( u"esri"_s ).toMap().value( u"url"_s ).toString();
    if ( !sourceUri.isEmpty() )
    {
      QUrl url( sourceUri );
      // some services don't default to json format, while others do... so let's explicitly request it!
      // (refs https://github.com/qgis/QGIS/issues/4231)
      QUrlQuery query;
      query.addQueryItem( u"f"_s, u"pjson"_s );
      url.setQuery( query );

      QNetworkRequest request = QNetworkRequest( url );

      QgsSetRequestInitiatorClass( request, u"QgsVectorTileLayer"_s )

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
      if ( mArcgisLayerConfiguration.contains( u"error"_s ) )
      {
        return false;
      }
    }
  }

  // read tileMap if available
  QVariantMap tileMap;
  const QString tileMapEndpoint = mArcgisLayerConfiguration.value( u"tileMap"_s ).toString();
  if ( !tileMapEndpoint.isEmpty() )
  {
    mTileMapUrl = tileServiceUri + '/' + tileMapEndpoint;
    QUrl tilemapUrl( mTileMapUrl );
    tilemapUrl.setQuery( query );

    QNetworkRequest tileMapRequest = QNetworkRequest( tilemapUrl );
    QgsSetRequestInitiatorClass( tileMapRequest, u"QgsVectorTileLayer"_s )

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

  mSourcePath = tileServiceUri + '/' + mArcgisLayerConfiguration.value( u"tiles"_s ).toList().value( 0 ).toString();
  if ( !QgsVectorTileUtils::checkXYZUrlTemplate( mSourcePath ) )
  {
    QgsDebugError( u"Invalid format of URL for XYZ source: "_s + tileServiceUri );
    return false;
  }

  mArcgisLayerConfiguration.insert( u"serviceUri"_s, tileServiceUri );

  mMatrixSet.fromEsriJson( mArcgisLayerConfiguration, tileMap );
  mCrs = mMatrixSet.crs();

  // if hardcoded zoom limits aren't specified, take them from the server
  if ( dsUri.hasParam( u"zmin"_s ) )
    mMatrixSet.dropMatricesOutsideZoomRange( dsUri.param( u"zmin"_s ).toInt(), 99 );

  if ( dsUri.hasParam( u"zmax"_s ) )
    mMatrixSet.dropMatricesOutsideZoomRange( 0, dsUri.param( u"zmax"_s ).toInt() );

  const QVariantMap fullExtent = mArcgisLayerConfiguration.value( u"fullExtent"_s ).toMap();
  if ( !fullExtent.isEmpty() )
  {
    const QgsRectangle fullExtentRect(
      fullExtent.value( u"xmin"_s ).toDouble(),
      fullExtent.value( u"ymin"_s ).toDouble(),
      fullExtent.value( u"xmax"_s ).toDouble(),
      fullExtent.value( u"ymax"_s ).toDouble()
    );

    const QgsCoordinateReferenceSystem fullExtentCrs = QgsArcGisRestUtils::convertSpatialReference( fullExtent.value( u"spatialReference"_s ).toMap() );
    const QgsCoordinateTransform extentTransform( fullExtentCrs, mCrs, transformContext() );
    try
    {
      mExtent = extentTransform.transformBoundingBox( fullExtentRect );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( u"Could not transform layer fullExtent to layer CRS"_s );
    }
  }
  else
  {
    // if no fullExtent specified in JSON, default to web mercator specs full extent
    const QgsCoordinateTransform extentTransform( QgsCoordinateReferenceSystem( u"EPSG:3857"_s ), mCrs, transformContext() );
    try
    {
      mExtent = extentTransform.transformBoundingBox( QgsRectangle( -20037508.3427892, -20037508.3427892, 20037508.3427892, 20037508.3427892 ) );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( u"Could not transform layer extent to layer CRS"_s );
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
  return QgsApplication::getThemeIcon( u"mIconVectorTileLayer.svg"_s );
}

QgsProviderMetadata::ProviderCapabilities QgsArcGisVectorTileServiceDataProviderMetadata::providerCapabilities() const
{
  return QgsProviderMetadata::ProviderCapabilities();
}

QgsArcGisVectorTileServiceDataProvider *QgsArcGisVectorTileServiceDataProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  return new QgsArcGisVectorTileServiceDataProvider( uri, options, flags );
}

QVariantMap QgsArcGisVectorTileServiceDataProviderMetadata::decodeUri( const QString &uri ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QVariantMap uriComponents;
  uriComponents.insert( u"type"_s, u"xyz"_s );
  uriComponents.insert( u"serviceType"_s, u"arcgis"_s );
  uriComponents.insert( u"url"_s, dsUri.param( u"url"_s ) );

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

QString QgsArcGisVectorTileServiceDataProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setParam( u"type"_s, u"xyz"_s );
  dsUri.setParam( u"serviceType"_s, u"arcgis"_s );
  dsUri.setParam( u"url"_s, parts.value( u"url"_s ).toString() );

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


