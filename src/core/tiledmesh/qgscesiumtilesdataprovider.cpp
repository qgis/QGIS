/***************************************************************************
                         qgscesiumtilesdataprovider.cpp
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscesiumtilesdataprovider.h"
#include "qgsproviderutils.h"
#include "qgsapplication.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsthreadingutils.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsblockingnetworkrequest.h"
#include "qgscesiumutils.h"

#include <QUrl>
#include <QIcon>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "cesiumtiles" )
#define PROVIDER_DESCRIPTION QStringLiteral( "Cesium 3D Tiles data provider" )

//
// QgsCesiumTilesDataProviderSharedData
//

QgsCesiumTilesDataProviderSharedData::QgsCesiumTilesDataProviderSharedData() = default;

void QgsCesiumTilesDataProviderSharedData::setTilesetContent( const QVariantMap &tileset )
{
  mTileset = tileset;
  mCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) );

  // parse root
  {
    const QVariantMap root = tileset.value( QStringLiteral( "root" ) ).toMap();
    // parse root bounding volume
    {
      const QVariantMap rootBoundingVolume = root.value( QStringLiteral( "boundingVolume" ) ).toMap();
      if ( rootBoundingVolume.contains( QStringLiteral( "region" ) ) )
      {
        const QgsBox3d rootRegion = QgsCesiumUtils::parseRegion( rootBoundingVolume.value( QStringLiteral( "region" ) ).toList() );
        if ( !rootRegion.isNull() )
        {
          mZRange = QgsDoubleRange( rootRegion.zMinimum(), rootRegion.zMaximum() );
          // The latitude and longitude values are given in radians!

          const double xMin = rootRegion.xMinimum() * 180 / M_PI;
          const double xMax = rootRegion.xMaximum() * 180 / M_PI;
          const double yMin = rootRegion.yMinimum() * 180 / M_PI;
          const double yMax = rootRegion.yMaximum() * 180 / M_PI;
          mExtent = QgsRectangle( xMin, yMin, xMax, yMax );
        }
      }
      else
      {
        // TODO: handle box, sphere bounding volumes
      }
    }
  }
}


//
// QgsCesiumTilesDataProvider
//

QgsCesiumTilesDataProvider::QgsCesiumTilesDataProvider( const QString &uri, const ProviderOptions &providerOptions, ReadFlags flags )
  : QgsTiledMeshDataProvider( uri, providerOptions, flags )
  , mShared( std::make_shared< QgsCesiumTilesDataProviderSharedData >() )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );
  mAuthCfg = dsUri.authConfigId();
  mHeaders = dsUri.httpHeaders();

  mIsValid = init();
}

QgsCesiumTilesDataProvider::QgsCesiumTilesDataProvider( const QgsCesiumTilesDataProvider &other )
  : QgsTiledMeshDataProvider( other )
  , mIsValid( other.mIsValid )
  , mAuthCfg( other.mAuthCfg )
  , mHeaders( other.mHeaders )
  , mShared( other.mShared )
{
}

QgsCesiumTilesDataProvider::~QgsCesiumTilesDataProvider() = default;

QgsCesiumTilesDataProvider *QgsCesiumTilesDataProvider::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  return new QgsCesiumTilesDataProvider( *this );
}

bool QgsCesiumTilesDataProvider::init()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( dataSourceUri() );

  const QString tileSetUri = dsUri.param( QStringLiteral( "url" ) );
  if ( !tileSetUri.isEmpty() )
  {
    const QUrl url( tileSetUri );

    QNetworkRequest request = QNetworkRequest( url );
    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsCesiumTilesDataProvider" ) )
    mHeaders.updateNetworkRequest( request );

    QgsBlockingNetworkRequest networkRequest;
    networkRequest.setAuthCfg( mAuthCfg );

    switch ( networkRequest.get( request ) )
    {
      case QgsBlockingNetworkRequest::NoError:
        break;

      case QgsBlockingNetworkRequest::NetworkError:
      case QgsBlockingNetworkRequest::TimeoutError:
      case QgsBlockingNetworkRequest::ServerExceptionError:
        // TODO -- error reporting
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
    mShared->setTilesetContent( doc.object().toVariantMap() );
  }
  else
  {
    // try uri as a local file
    if ( QFileInfo::exists( dataSourceUri( ) ) )
    {
      QFile file( dataSourceUri( ) );
      if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
        const QByteArray raw = file.readAll();
        // Parse data
        QJsonParseError err;
        const QJsonDocument doc = QJsonDocument::fromJson( raw, &err );
        if ( doc.isNull() )
        {
          return false;
        }
        mShared->setTilesetContent( doc.object().toVariantMap() );
      }
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }

  return true;
}

QgsCoordinateReferenceSystem QgsCesiumTilesDataProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShared->mCrs;
}

QgsRectangle QgsCesiumTilesDataProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShared->mExtent;
}

bool QgsCesiumTilesDataProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mIsValid;
}

QString QgsCesiumTilesDataProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return PROVIDER_KEY;
}

QString QgsCesiumTilesDataProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QObject::tr( "Cesium 3D Tiles" );
}

QString QgsCesiumTilesDataProvider::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QString metadata;

  {
    const QVariantMap asset = mShared->mTileset.value( QStringLiteral( "asset" ) ).toMap();
    const QString version = asset.value( QStringLiteral( "version" ) ).toString();
    if ( !version.isEmpty() )
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "3D Tiles Version" ) % QStringLiteral( "</td><td>%1</a>" ).arg( version ) % QStringLiteral( "</td></tr>\n" );

    const QString tilesetVersion = asset.value( QStringLiteral( "tilesetVersion" ) ).toString();
    if ( !tilesetVersion.isEmpty() )
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Tileset Version" ) % QStringLiteral( "</td><td>%1</a>" ).arg( tilesetVersion ) % QStringLiteral( "</td></tr>\n" );

    const QString generator = asset.value( QStringLiteral( "generator" ) ).toString();
    if ( !generator.isEmpty() )
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Tileset Generator" ) % QStringLiteral( "</td><td>%1</a>" ).arg( generator ) % QStringLiteral( "</td></tr>\n" );
  }

  if ( !mShared->mZRange.isInfinite() )
  {
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Z Range" ) % QStringLiteral( "</td><td>%1 - %2</a>" ).arg( QLocale().toString( mShared->mZRange.lower() ), QLocale().toString( mShared->mZRange.upper() ) ) % QStringLiteral( "</td></tr>\n" );
  }

  return metadata;
}


//
// QgsCesiumTilesProviderMetadata
//

QgsCesiumTilesProviderMetadata::QgsCesiumTilesProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QIcon QgsCesiumTilesProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "xxxcesiumxxx.svg" ) );
}

QgsCesiumTilesDataProvider *QgsCesiumTilesProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsCesiumTilesDataProvider( uri, options, flags );
}

QList<QgsProviderSublayerDetails> QgsCesiumTilesProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags, QgsFeedback * ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().compare( QLatin1String( "tileset.json" ), Qt::CaseInsensitive ) == 0 )
  {
    QgsProviderSublayerDetails details;
    details.setUri( uri );
    details.setProviderKey( PROVIDER_KEY );
    details.setType( Qgis::LayerType::TiledMesh );
    details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( uri ) );
    return {details};
  }
  else
  {
    return {};
  }
}

int QgsCesiumTilesProviderMetadata::priorityForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().compare( QLatin1String( "tileset.json" ), Qt::CaseInsensitive ) == 0 )
    return 100;

  return 0;
}

QList<Qgis::LayerType> QgsCesiumTilesProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().compare( QLatin1String( "tileset.json" ), Qt::CaseInsensitive ) == 0 )
    return QList< Qgis::LayerType>() << Qgis::LayerType::TiledMesh;

  return QList< Qgis::LayerType>();
}

QVariantMap QgsCesiumTilesProviderMetadata::decodeUri( const QString &uri ) const
{
  QVariantMap uriComponents;
  QUrl url = QUrl::fromUserInput( uri );
  uriComponents.insert( QStringLiteral( "file-name" ), url.fileName() );
  uriComponents.insert( QStringLiteral( "path" ), uri );
  return uriComponents;
}

QString QgsCesiumTilesProviderMetadata::filters( Qgis::FileFilterType type )
{
  switch ( type )
  {
    case Qgis::FileFilterType::Vector:
    case Qgis::FileFilterType::Raster:
    case Qgis::FileFilterType::Mesh:
    case Qgis::FileFilterType::MeshDataset:
    case Qgis::FileFilterType::VectorTile:
    case Qgis::FileFilterType::PointCloud:
      return QString();

    case Qgis::FileFilterType::TiledMesh:
      return QObject::tr( "Cesium 3D Tiles" ) + QStringLiteral( " (tileset.json TILESET.JSON)" );
  }
  return QString();
}

QgsProviderMetadata::ProviderCapabilities QgsCesiumTilesProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QList<Qgis::LayerType> QgsCesiumTilesProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::TiledMesh };
}

QString QgsCesiumTilesProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  return path;
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsCesiumTilesProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::LayerTypesForUri
         | ProviderMetadataCapability::PriorityForUri
         | ProviderMetadataCapability::QuerySublayers;
}

///@endcond
