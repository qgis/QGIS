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
#include "qgssphere.h"
#include "qgslogger.h"
#include "qgsorientedbox3d.h"
#include "qgstiledmeshboundingvolume.h"
#include "qgscoordinatetransform.h"

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

void QgsCesiumTilesDataProviderSharedData::setTilesetContent( const QString &tileset, const QgsCoordinateTransformContext &transformContext )
{
  mTileset = json::parse( tileset.toStdString() );

  // parse root
  {
    const auto &root = mTileset[ "root" ];
    // parse root bounding volume

    // TODO -- read crs from metadata tags. Need to find real world examples of this. And can metadata crs override
    // the EPSG:4979 requirement from a region bounding volume??

    {
      // TODO -- on some datasets there is a "boundingVolume" present on the tileset itself, i.e. not the root node.
      // what does this mean? Should we use it instead of the root node bounding volume if it's present?

      const auto &rootBoundingVolume = root[ "boundingVolume" ];

      QgsMatrix4x4 rootTransform;
      if ( root.contains( "transform" ) && !root["transform"].is_null() )
      {
        const auto &transformJson = root["transform"];
        double *ptr = rootTransform.data();
        for ( int i = 0; i < 16; ++i )
          ptr[i] = transformJson[i].get<double>();
      }

      if ( rootBoundingVolume.contains( "region" ) )
      {
        const QgsBox3D rootRegion = QgsCesiumUtils::parseRegion( rootBoundingVolume[ "region" ] );
        if ( !rootRegion.isNull() )
        {
          mBoundingVolume = std::make_unique< QgsTiledMeshNodeBoundingVolumeRegion >( rootRegion );
          mBoundingVolume->setTransform( rootTransform );
          mZRange = QgsDoubleRange( rootRegion.zMinimum(), rootRegion.zMaximum() );
          mLayerCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) );
          mMeshCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) );
          mExtent = rootRegion.toRectangle();
        }
      }
      else if ( rootBoundingVolume.contains( "box" ) )
      {
        const QgsOrientedBox3D bbox = QgsCesiumUtils::parseBox( rootBoundingVolume["box"] );
        if ( !bbox.isNull() )
        {
          // layer must advertise as EPSG:4979, as the various QgsMapLayer
          // methods which utilize QgsMapLayer::crs() (such as layer extent transformation)
          // are all purely 2D and can't handle the cesium data source z value
          // range in EPSG:4978 !
          mLayerCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) );
          mMeshCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) );

          const QgsCoordinateTransform transform( mMeshCrs, mLayerCrs, transformContext );

          mBoundingVolume = std::make_unique< QgsTiledMeshNodeBoundingVolumeBox >( bbox );
          mBoundingVolume->setTransform( rootTransform );
          try
          {
            const QgsBox3D rootRegion = mBoundingVolume->bounds( transform );
            mZRange = QgsDoubleRange( rootRegion.zMinimum(), rootRegion.zMaximum() );

            std::unique_ptr< QgsAbstractGeometry > extent2D( mBoundingVolume->as2DGeometry( transform ) );
            mExtent = extent2D->boundingBox();
          }
          catch ( QgsCsException & )
          {
            QgsDebugError( QStringLiteral( "Caught transform exception when transforming boundingVolume" ) );
          }
        }
      }
      else if ( rootBoundingVolume.contains( "sphere" ) )
      {
        const QgsSphere sphere = QgsCesiumUtils::parseSphere( rootBoundingVolume["sphere"] );
        if ( !sphere.isNull() )
        {
          // layer must advertise as EPSG:4979, as the various QgsMapLayer
          // methods which utilize QgsMapLayer::crs() (such as layer extent transformation)
          // are all purely 2D and can't handle the cesium data source z value
          // range in EPSG:4978 !
          mLayerCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) );
          mMeshCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) );

          const QgsCoordinateTransform transform( mMeshCrs, mLayerCrs, transformContext );

          mBoundingVolume = std::make_unique< QgsTiledMeshNodeBoundingVolumeSphere >( sphere );
          mBoundingVolume->setTransform( rootTransform );
          try
          {
            const QgsBox3D rootRegion = mBoundingVolume->bounds( transform );
            mZRange = QgsDoubleRange( rootRegion.zMinimum(), rootRegion.zMaximum() );

            std::unique_ptr< QgsAbstractGeometry > extent2D( mBoundingVolume->as2DGeometry( transform ) );
            mExtent = extent2D->boundingBox();
          }
          catch ( QgsCsException & )
          {
            QgsDebugError( QStringLiteral( "Caught transform exception when transforming boundingVolume" ) );
          }
        }
      }
      else
      {
        QgsDebugError( QStringLiteral( "unsupported boundingVolume format" ) );
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
    mShared->setTilesetContent( content.content(), transformContext() );
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
        mShared->setTilesetContent( raw, transformContext() );
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

  return mShared->mLayerCrs;
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

  if ( mShared->mTileset.contains( "asset" ) )
  {
    const auto &asset = mShared->mTileset[ "asset" ];
    if ( asset.contains( "version" ) )
    {
      const QString version = QString::fromStdString( asset["version"].get<std::string>() );
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "3D Tiles Version" ) % QStringLiteral( "</td><td>%1</a>" ).arg( version ) % QStringLiteral( "</td></tr>\n" );
    }

    if ( asset.contains( "tilesetVersion" ) )
    {
      const QString tilesetVersion = QString::fromStdString( asset["tilesetVersion"].get<std::string>() );
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Tileset Version" ) % QStringLiteral( "</td><td>%1</a>" ).arg( tilesetVersion ) % QStringLiteral( "</td></tr>\n" );
    }

    if ( asset.contains( "generator" ) )
    {
      const QString generator = QString::fromStdString( asset["generator"].get<std::string>() );
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Tileset Generator" ) % QStringLiteral( "</td><td>%1</a>" ).arg( generator ) % QStringLiteral( "</td></tr>\n" );
    }
  }
  if ( mShared->mTileset.contains( "extensionsRequired" ) )
  {
    QStringList extensions;
    for ( const auto &item : mShared->mTileset["extensionsRequired"] )
    {
      extensions << QString::fromStdString( item.get<std::string>() );
    }
    if ( !extensions.isEmpty() )
    {
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Extensions Required" ) % QStringLiteral( "</td><td><ul><li>%1</li></ul></a>" ).arg( extensions.join( QStringLiteral( "</li><li>" ) ) ) % QStringLiteral( "</td></tr>\n" );
    }
  }
  if ( mShared->mTileset.contains( "extensionsUsed" ) )
  {
    QStringList extensions;
    for ( const auto &item : mShared->mTileset["extensionsUsed"] )
    {
      extensions << QString::fromStdString( item.get<std::string>() );
    }
    if ( !extensions.isEmpty() )
    {
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Extensions Used" ) % QStringLiteral( "</td><td><ul><li>%1</li></ul></a>" ).arg( extensions.join( QStringLiteral( "</li><li>" ) ) ) % QStringLiteral( "</td></tr>\n" );
    }
  }

  if ( !mShared->mZRange.isInfinite() )
  {
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Z Range" ) % QStringLiteral( "</td><td>%1 - %2</a>" ).arg( QLocale().toString( mShared->mZRange.lower() ), QLocale().toString( mShared->mZRange.upper() ) ) % QStringLiteral( "</td></tr>\n" );
  }

  return metadata;
}

const QgsCoordinateReferenceSystem QgsCesiumTilesDataProvider::meshCrs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShared ? mShared->mMeshCrs : QgsCoordinateReferenceSystem();
}

const QgsAbstractTiledMeshNodeBoundingVolume *QgsCesiumTilesDataProvider::boundingVolume() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mShared ? mShared->mBoundingVolume.get() : nullptr;
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
