/***************************************************************************
                         qgsvirtualpointcloudprovider.cpp
                         ------------------
    begin                : March 2023
    copyright            : (C) 2023 by Stefanos Natsis
    email                : uclaros at gmail dot com
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
#include "qgsvirtualpointcloudprovider.h"
#include "qgscopcpointcloudindex.h"
#include "qgseptpointcloudindex.h"
#include "qgsremotecopcpointcloudindex.h"
#include "qgsremoteeptpointcloudindex.h"
#include "qgspointcloudsubindex.h"
#include "qgspointcloudclassifiedrenderer.h"
#include "qgspointcloudextentrenderer.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"
#include "qgsthreadingutils.h"
#include <nlohmann/json.hpp>
#include "qgsgeometry.h"
#include "qgsmultipolygon.h"
#include "qgscoordinatetransform.h"

#include <QIcon>

///@cond PRIVATE

#define PROVIDER_KEY QStringLiteral( "vpc" )
#define PROVIDER_DESCRIPTION QStringLiteral( "Virtual point cloud data provider" )

QgsVirtualPointCloudProvider::QgsVirtualPointCloudProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsPointCloudDataProvider( uri, options, flags )
{
  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
    profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Open data source" ), QStringLiteral( "projectload" ) );

  mPolygonBounds.reset( new QgsGeometry( new QgsMultiPolygon() ) );

  parseFile();
}

QgsVirtualPointCloudProvider::~QgsVirtualPointCloudProvider() = default;

QgsPointCloudDataProvider::Capabilities QgsVirtualPointCloudProvider::capabilities() const
{
  QgsPointCloudDataProvider::Capabilities c;
  c.setFlag( QgsPointCloudDataProvider::Capability::ContainSubIndexes );
  c.setFlag( QgsPointCloudDataProvider::Capability::CreateRenderer );
  return c;
}

QgsCoordinateReferenceSystem QgsVirtualPointCloudProvider::crs() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCrs;
}

QgsRectangle QgsVirtualPointCloudProvider::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mExtent;
}

QgsPointCloudAttributeCollection QgsVirtualPointCloudProvider::attributes() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mAttributes;
}

bool QgsVirtualPointCloudProvider::isValid() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return !mSubLayers.isEmpty();
}

QString QgsVirtualPointCloudProvider::name() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return PROVIDER_KEY;
}

QString QgsVirtualPointCloudProvider::description() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return PROVIDER_DESCRIPTION;
}

QgsPointCloudIndex *QgsVirtualPointCloudProvider::index() const
{
  // non fatal for now -- 2d rendering of point clouds is not thread safe and calls this
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return nullptr;
}

qint64 QgsVirtualPointCloudProvider::pointCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return static_cast<qint64>( mPointCount );
}

void QgsVirtualPointCloudProvider::loadIndex()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  //no-op, there is no index
}

QVariantMap QgsVirtualPointCloudProvider::originalMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QVariantMap();
}

void QgsVirtualPointCloudProvider::generateIndex()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  //no-op, index is always generated
}

void QgsVirtualPointCloudProvider::parseFile()
{
  QFile file( dataSourceUri() );
  const QFileInfo fInfo( file );

  nlohmann::json data;
  if ( file.open( QFile::ReadOnly ) )
  {
    try
    {
      data = json::parse( file.readAll().toStdString() );
    }
    catch ( std::exception &e )
    {
      appendError( QgsErrorMessage( QStringLiteral( "JSON parsing error: %1" ).arg( QString::fromStdString( e.what() ) ), QString() ) );
      return;
    }
  }

  if ( data["type"] != "FeatureCollection" ||
       !data.contains( "features" ) )
  {
    appendError( QgsErrorMessage( QStringLiteral( "Invalid VPC file" ), QString( "asd" ) ) );
    return;
  }

  QSet<QString> attributeNames;

  for ( const auto &f : data["features"] )
  {
    if ( !f.contains( "type" ) || f["type"] != "Feature" ||
         !f.contains( "stac_version" ) ||
         !f.contains( "assets" ) || !f["assets"].is_object() ||
         !f.contains( "properties" ) || !f["properties"].is_object() ||
         !f.contains( "geometry" ) || !f["geometry"].is_object() )
    {
      QgsDebugMsg( QStringLiteral( "Malformed STAC item: %1" ).arg( QString::fromStdString( f ) ) );
      continue;
    }

    if ( f["stac_version"] != "1.0.0" )
    {
      QgsDebugMsg( QStringLiteral( "Unsupported STAC version: %1" ).arg( QString::fromStdString( f["stac_version"] ) ) );
      continue;
    }

    QgsPointCloudSubIndex si;

    nlohmann::json firstAsset = *f["assets"].begin();

    si.uri = QString::fromStdString( firstAsset["href"] );
    if ( f["properties"].contains( "pc:count" ) )
      si.count = f["properties"]["pc:count"];

    if ( !mCrs.isValid() )
    {
      if ( f["properties"].contains( "proj:epsg" ) )
        mCrs.createFromSrsId( f["properties"]["proj:epsg"].get<long>() );
      else if ( f["properties"].contains( "proj:wkt2" ) )
        mCrs.createFromString( QString::fromStdString( f["properties"]["proj:wkt2"] ) );
    }
    QgsCoordinateTransform transform( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), mCrs, QgsCoordinateTransformContext() );

    if ( f["properties"].contains( "proj:bbox" ) )
    {
      nlohmann::json nativeBbox = f["properties"]["proj:bbox"];
      si.extent = QgsRectangle( nativeBbox[0].get<double>(), nativeBbox[1].get<double>(),
                                nativeBbox[3].get<double>(), nativeBbox[4].get<double>() );
    }
    else if ( f.contains( "bbox" ) && mCrs.isValid() )
    {
      nlohmann::json bboxWgs = f["bbox"];
      QgsRectangle bbox = QgsRectangle( bboxWgs[0].get<double>(), bboxWgs[1].get<double>(),
                                        bboxWgs[2].get<double>(), bboxWgs[3].get<double>() );

      try
      {
        si.extent = transform.transformBoundingBox( bbox );
      }
      catch ( QgsCsException & )
      {
        QgsDebugMsg( QStringLiteral( "Cannot transform bbox to layer crs." ) );
        continue;
      }
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Missing extent information." ) );
      continue;
    }

    const auto geom = f["geometry"];
    try
    {
      QgsMultiPolygonXY geometry;
      if ( geom.at( "type" ) == "Polygon" )
      {
        QgsPolygonXY polygon;
        for ( auto &geomRing : geom.at( "coordinates" ) )
        {
          QgsPolylineXY ring;
          for ( auto &coords : geomRing )
          {
            ring.append( QgsPointXY( coords.at( 0 ), coords.at( 1 ) ) );
          }
          polygon.append( ring );
        }
        geometry.append( polygon );
      }
      else if ( geom.at( "type" ) == "MultiPolygon" )
      {
        for ( auto &geomPart : geom.at( "coordinates" ) )
        {
          QgsPolygonXY part;
          for ( auto &geomRing : geomPart )
          {
            QgsPolylineXY ring;
            for ( auto &coords : geomRing )
            {
              ring.append( QgsPointXY( coords.at( 0 ), coords.at( 1 ) ) );
            }
            part.append( ring );
          }
          geometry.append( part );
        }
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "Unexpected geometry type: %1" ).arg( QString::fromStdString( geom.at( "type" ) ) ) );
      }
      si.geometry = QgsGeometry::fromMultiPolygonXY( geometry );
    }
    catch ( std::exception &e )
    {
      QgsDebugMsg( QStringLiteral( "Malformed geometry item: %1" ).arg( QString::fromStdString( e.what() ) ) );
      return;
    }

    if ( si.uri.startsWith( QLatin1String( "./" ) ) )
    {
      // resolve relative path
      si.uri = fInfo.absoluteDir().absoluteFilePath( si.uri );
    }

    if ( f["properties"].contains( "pc:schemas" ) )
    {
      for ( auto &schemaItem : f["properties"]["pc:schemas"] )
      {
        attributeNames.insert( QString::fromStdString( schemaItem["name"] ) );
      }
    }

    if ( transform.isValid() && !transform.isShortCircuited() )
      try
      {
        si.geometry.transform( transform );
      }
      catch ( QgsCsException & )
      {
        QgsDebugMsg( QStringLiteral( "Cannot transform geometry to layer crs." ) );
        continue;
      }
    mPolygonBounds->addPart( si.geometry );
    mSubLayers.push_back( si );
    mPointCount += si.count;
  }
  mExtent = mPolygonBounds->boundingBox();
  populateAttributeCollection( attributeNames );
}

QgsGeometry QgsVirtualPointCloudProvider::polygonBounds() const
{
  return *mPolygonBounds;
}

void QgsVirtualPointCloudProvider::loadIndex( int i )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( i >= mSubLayers.size() || i < 0 )
    return;

  QgsPointCloudSubIndex &sl = mSubLayers[ i ];
  // Index already loaded -> no need to load
  if ( sl.index )
    return;

  if ( sl.uri.startsWith( QStringLiteral( "http" ), Qt::CaseSensitivity::CaseInsensitive ) )
  {
    if ( sl.uri.endsWith( QStringLiteral( "copc.laz" ), Qt::CaseSensitivity::CaseInsensitive ) )
      sl.index.reset( new QgsRemoteCopcPointCloudIndex() );
    else if ( sl.uri.endsWith( QStringLiteral( "ept.json" ), Qt::CaseSensitivity::CaseInsensitive ) )
      sl.index.reset( new QgsRemoteEptPointCloudIndex() );
  }
  else
  {
    if ( sl.uri.endsWith( QStringLiteral( "copc.laz" ), Qt::CaseSensitivity::CaseInsensitive ) )
      sl.index.reset( new QgsCopcPointCloudIndex() );
    else if ( sl.uri.endsWith( QStringLiteral( "ept.json" ), Qt::CaseSensitivity::CaseInsensitive ) )
      sl.index.reset( new QgsEptPointCloudIndex() );
  }

  if ( !sl.index )
    return;

  sl.index->load( sl.uri );

  // if expression is broken or index is missing a required field, set to "false" so it returns no points
  if ( !sl.index->setSubsetString( mSubsetString ) )
    sl.index->setSubsetString( QStringLiteral( "false" ) );
}

void QgsVirtualPointCloudProvider::populateAttributeCollection( QSet<QString> names )
{
  mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );
  mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Int32 ) );

  if ( names.contains( QLatin1String( "Intensity" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Intensity" ), QgsPointCloudAttribute::UShort ) );
  if ( names.contains( QLatin1String( "ReturnNumber" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "ReturnNumber" ), QgsPointCloudAttribute::Char ) );
  if ( names.contains( QLatin1String( "NumberOfReturns" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "NumberOfReturns" ), QgsPointCloudAttribute::Char ) );
  if ( names.contains( QLatin1String( "ScanDirectionFlag" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "ScanDirectionFlag" ), QgsPointCloudAttribute::Char ) );
  if ( names.contains( QLatin1String( "EdgeOfFlightLine" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "EdgeOfFlightLine" ), QgsPointCloudAttribute::Char ) );
  if ( names.contains( QLatin1String( "Classification" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Classification" ), QgsPointCloudAttribute::UChar ) );
  if ( names.contains( QLatin1String( "ScanAngleRank" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "ScanAngleRank" ), QgsPointCloudAttribute::Short ) );
  if ( names.contains( QLatin1String( "UserData" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "UserData" ), QgsPointCloudAttribute::Char ) );
  if ( names.contains( QLatin1String( "PointSourceId" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "PointSourceId" ), QgsPointCloudAttribute::UShort ) );
  if ( names.contains( QLatin1String( "ScannerChannel" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "ScannerChannel" ), QgsPointCloudAttribute::Char ) );
  if ( names.contains( QLatin1String( "ClassificationFlags" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "ClassificationFlags" ), QgsPointCloudAttribute::Char ) );
  if ( names.contains( QLatin1String( "GpsTime" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "GpsTime" ), QgsPointCloudAttribute::Double ) );
  if ( names.contains( QLatin1String( "Red" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Red" ), QgsPointCloudAttribute::UShort ) );
  if ( names.contains( QLatin1String( "Green" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Green" ), QgsPointCloudAttribute::UShort ) );
  if ( names.contains( QLatin1String( "Blue" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Blue" ), QgsPointCloudAttribute::UShort ) );
  if ( names.contains( QLatin1String( "Infrared" ) ) )
    mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Infrared" ), QgsPointCloudAttribute::UShort ) );
  names.subtract( { QLatin1String( "X" ),
                    QLatin1String( "Y" ),
                    QLatin1String( "Z" ),
                    QLatin1String( "Intensity" ),
                    QLatin1String( "ReturnNumber" ),
                    QLatin1String( "NumberOfReturns" ),
                    QLatin1String( "ScanDirectionFlag" ),
                    QLatin1String( "EdgeOfFlightLine" ),
                    QLatin1String( "Classification" ),
                    QLatin1String( "ScanAngleRank" ),
                    QLatin1String( "UserData" ),
                    QLatin1String( "PointSourceId" ),
                    QLatin1String( "ScannerChannel" ),
                    QLatin1String( "ClassificationFlags" ),
                    QLatin1String( "GpsTime" ),
                    QLatin1String( "Red" ),
                    QLatin1String( "Green" ),
                    QLatin1String( "Blue" ),
                    QLatin1String( "Infrared" ) } );
  for ( const auto &name : names )
    mAttributes.push_back( QgsPointCloudAttribute( name, QgsPointCloudAttribute::Double ) );
}

bool QgsVirtualPointCloudProvider::setSubsetString( const QString &subset, bool updateFeatureCount )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( updateFeatureCount )

  for ( const auto &i : std::as_const( mSubLayers ) )
  {
    if ( !i.index )
      continue;

    // if expression is broken or index is missing a required field, set to "false" so it returns no points
    if ( !i.index->setSubsetString( subset ) )
      i.index->setSubsetString( QStringLiteral( "false" ) );
  }

  mSubsetString = subset;
  emit dataChanged();
  return true;
}

QgsPointCloudRenderer *QgsVirtualPointCloudProvider::createRenderer( const QVariantMap &configuration ) const
{
  Q_UNUSED( configuration )

  if ( mAttributes.indexOf( QStringLiteral( "Classification" ) ) >= 0 )
  {
    return new QgsPointCloudClassifiedRenderer( QStringLiteral( "Classification" ), QgsPointCloudClassifiedRenderer::defaultCategories() );
  }

  return new QgsPointCloudExtentRenderer();
}

QgsVirtualPointCloudProviderMetadata::QgsVirtualPointCloudProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QIcon QgsVirtualPointCloudProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconPointCloudLayer.svg" ) );
}

QgsVirtualPointCloudProvider *QgsVirtualPointCloudProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsVirtualPointCloudProvider( uri, options, flags );
}

QList<QgsProviderSublayerDetails> QgsVirtualPointCloudProviderMetadata::querySublayers( const QString &uri, Qgis::SublayerQueryFlags, QgsFeedback * ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().endsWith( ".vpc", Qt::CaseSensitivity::CaseInsensitive ) )
  {
    QgsProviderSublayerDetails details;
    details.setUri( uri );
    details.setProviderKey( QStringLiteral( "vpc" ) );
    details.setType( Qgis::LayerType::PointCloud );
    details.setName( QgsProviderUtils::suggestLayerNameFromFilePath( uri ) );
    return {details};
  }
  else
  {
    return {};
  }
}

int QgsVirtualPointCloudProviderMetadata::priorityForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().endsWith( ".vpc", Qt::CaseSensitivity::CaseInsensitive ) )
    return 100;

  return 0;
}

QList<Qgis::LayerType> QgsVirtualPointCloudProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  if ( parts.value( QStringLiteral( "file-name" ) ).toString().endsWith( ".vpc", Qt::CaseSensitivity::CaseInsensitive ) )
    return QList< Qgis::LayerType>() << Qgis::LayerType::PointCloud;

  return QList< Qgis::LayerType>();
}

QVariantMap QgsVirtualPointCloudProviderMetadata::decodeUri( const QString &uri ) const
{
  QVariantMap uriComponents;
  QUrl url = QUrl::fromUserInput( uri );
  uriComponents.insert( QStringLiteral( "file-name" ), url.fileName() );
  uriComponents.insert( QStringLiteral( "path" ), uri );
  return uriComponents;
}

QString QgsVirtualPointCloudProviderMetadata::filters( QgsProviderMetadata::FilterType type )
{
  switch ( type )
  {
    case QgsProviderMetadata::FilterType::FilterVector:
    case QgsProviderMetadata::FilterType::FilterRaster:
    case QgsProviderMetadata::FilterType::FilterMesh:
    case QgsProviderMetadata::FilterType::FilterMeshDataset:
      return QString();

    case QgsProviderMetadata::FilterType::FilterPointCloud:
      return QObject::tr( "Virtual Point Clouds" ) + QStringLiteral( " (*.vpc *.VPC)" );
  }
  return QString();
}

QgsProviderMetadata::ProviderCapabilities QgsVirtualPointCloudProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QList<Qgis::LayerType> QgsVirtualPointCloudProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::PointCloud };
}

QString QgsVirtualPointCloudProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  return path;
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsVirtualPointCloudProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::LayerTypesForUri
         | ProviderMetadataCapability::PriorityForUri
         | ProviderMetadataCapability::QuerySublayers;
}
///@endcond

