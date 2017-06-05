/***************************************************************************
                         qgsprocessingutils.cpp
                         ------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsprocessingutils.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgscsexception.h"
#include "qgsprocessingcontext.h"
#include "qgsvectorlayerexporter.h"
#include "qgsvectorfilewriter.h"
#include "qgsmemoryproviderutils.h"

QList<QgsRasterLayer *> QgsProcessingUtils::compatibleRasterLayers( QgsProject *project, bool sort )
{
  if ( !project )
    return QList<QgsRasterLayer *>();

  QList<QgsRasterLayer *> layers;
  Q_FOREACH ( QgsRasterLayer *l, project->layers<QgsRasterLayer *>() )
  {
    if ( canUseLayer( l ) )
      layers << l;
  }

  if ( sort )
  {
    std::sort( layers.begin(), layers.end(), []( const QgsRasterLayer * a, const QgsRasterLayer * b ) -> bool
    {
      return QString::localeAwareCompare( a->name(), b->name() ) < 0;
    } );
  }
  return layers;
}

QList<QgsVectorLayer *> QgsProcessingUtils::compatibleVectorLayers( QgsProject *project, const QList<QgsWkbTypes::GeometryType> &geometryTypes, bool sort )
{
  if ( !project )
    return QList<QgsVectorLayer *>();

  QList<QgsVectorLayer *> layers;
  Q_FOREACH ( QgsVectorLayer *l, project->layers<QgsVectorLayer *>() )
  {
    if ( canUseLayer( l, geometryTypes ) )
      layers << l;
  }

  if ( sort )
  {
    std::sort( layers.begin(), layers.end(), []( const QgsVectorLayer * a, const QgsVectorLayer * b ) -> bool
    {
      return QString::localeAwareCompare( a->name(), b->name() ) < 0;
    } );
  }
  return layers;
}

QList<QgsMapLayer *> QgsProcessingUtils::compatibleLayers( QgsProject *project, bool sort )
{
  if ( !project )
    return QList<QgsMapLayer *>();

  QList<QgsMapLayer *> layers;
  Q_FOREACH ( QgsRasterLayer *rl, compatibleRasterLayers( project, false ) )
    layers << rl;
  Q_FOREACH ( QgsVectorLayer *vl, compatibleVectorLayers( project, QList< QgsWkbTypes::GeometryType >(), false ) )
    layers << vl;

  if ( sort )
  {
    std::sort( layers.begin(), layers.end(), []( const QgsMapLayer * a, const QgsMapLayer * b ) -> bool
    {
      return QString::localeAwareCompare( a->name(), b->name() ) < 0;
    } );
  }
  return layers;
}

QgsMapLayer *QgsProcessingUtils::mapLayerFromStore( const QString &string, QgsMapLayerStore *store )
{
  if ( !store || string.isEmpty() )
    return nullptr;

  QList< QgsMapLayer * > layers = store->mapLayers().values();

  layers.erase( std::remove_if( layers.begin(), layers.end(), []( QgsMapLayer * layer )
  {
    switch ( layer->type() )
    {
      case QgsMapLayer::VectorLayer:
        return !canUseLayer( qobject_cast< QgsVectorLayer * >( layer ) );
      case QgsMapLayer::RasterLayer:
        return !canUseLayer( qobject_cast< QgsRasterLayer * >( layer ) );
      case QgsMapLayer::PluginLayer:
        return true;
    }
    return true;
  } ), layers.end() );

  Q_FOREACH ( QgsMapLayer *l, layers )
  {
    if ( l->id() == string )
      return l;
  }
  Q_FOREACH ( QgsMapLayer *l, layers )
  {
    if ( l->name() == string )
      return l;
  }
  Q_FOREACH ( QgsMapLayer *l, layers )
  {
    if ( normalizeLayerSource( l->source() ) == normalizeLayerSource( string ) )
      return l;
  }
  return nullptr;
}

///@cond PRIVATE
class ProjectionSettingRestorer
{
  public:

    ProjectionSettingRestorer()
    {
      QgsSettings settings;
      previousSetting = settings.value( QStringLiteral( "/Projections/defaultBehavior" ) ).toString();
      settings.setValue( QStringLiteral( "/Projections/defaultBehavior" ), QString() );
    }

    ~ProjectionSettingRestorer()
    {
      QgsSettings settings;
      settings.setValue( QStringLiteral( "/Projections/defaultBehavior" ), previousSetting );
    }

    QString previousSetting;
};
///@endcond PRIVATE

QgsMapLayer *QgsProcessingUtils::loadMapLayerFromString( const QString &string )
{
  if ( QFileInfo::exists( string ) )
  {
    // TODO - remove when there is a cleaner way to block the unknown projection dialog!
    ProjectionSettingRestorer restorer;
    ( void )restorer; // no warnings

    // brute force attempt to load a matching layer
    std::unique_ptr< QgsVectorLayer > layer( new QgsVectorLayer( string, QStringLiteral( "temp" ), QStringLiteral( "ogr" ), false ) );
    if ( layer->isValid() )
    {
      return layer.release();
    }
    std::unique_ptr< QgsRasterLayer > rasterLayer( new QgsRasterLayer( string, QStringLiteral( "temp" ), QStringLiteral( "gdal" ), false ) );
    if ( rasterLayer->isValid() )
    {
      return rasterLayer.release();
    }
  }
  return nullptr;
}

QgsMapLayer *QgsProcessingUtils::mapLayerFromString( const QString &string, QgsProcessingContext &context, bool allowLoadingNewLayers )
{
  if ( string.isEmpty() )
    return nullptr;

  // prefer project layers
  QgsMapLayer *layer = nullptr;
  if ( context.project() )
  {
    QgsMapLayer *layer = mapLayerFromStore( string, context.project()->layerStore() );
    if ( layer )
      return layer;
  }

  layer = mapLayerFromStore( string, context.temporaryLayerStore() );
  if ( layer )
    return layer;

  if ( !allowLoadingNewLayers )
    return nullptr;

  layer = loadMapLayerFromString( string );
  if ( layer )
  {
    context.temporaryLayerStore()->addMapLayer( layer );
    return layer;
  }
  else
  {
    return nullptr;
  }
}

bool QgsProcessingUtils::canUseLayer( const QgsRasterLayer *layer )
{
  // only gdal file-based layers
  return layer && layer->providerType() == QStringLiteral( "gdal" );
}

bool QgsProcessingUtils::canUseLayer( const QgsVectorLayer *layer, const QList<QgsWkbTypes::GeometryType> &geometryTypes )
{
  return layer &&
         ( geometryTypes.isEmpty() || geometryTypes.contains( layer->geometryType() ) );
}

QString QgsProcessingUtils::normalizeLayerSource( const QString &source )
{
  QString normalized = source;
  normalized.replace( '\\', '/' );
  normalized.replace( '"', "'" );
  return normalized.trimmed();
}

void parseDestinationString( QString &destination, QString &providerKey, QString &uri, QString &format, QMap<QString, QVariant> &options )
{
  QRegularExpression splitRx( "^(.*?):(.*)$" );
  QRegularExpressionMatch match = splitRx.match( destination );
  if ( match.hasMatch() )
  {
    providerKey = match.captured( 1 );
    if ( providerKey == QStringLiteral( "postgis" ) ) // older processing used "postgis" instead of "postgres"
    {
      providerKey = QStringLiteral( "postgres" );
    }
    uri = match.captured( 2 );
  }
  else
  {
    providerKey = QStringLiteral( "ogr" );
    QRegularExpression splitRx( "^(.*)\\.(.*?)$" );
    QRegularExpressionMatch match = splitRx.match( destination );
    QString extension;
    if ( match.hasMatch() )
    {
      extension = match.captured( 2 );
      format = QgsVectorFileWriter::driverForExtension( extension );
    }

    if ( format.isEmpty() )
    {
      format = QStringLiteral( "ESRI Shapefile" );
      destination = destination + QStringLiteral( ".shp" );
    }

    options.insert( QStringLiteral( "driverName" ), format );
    uri = destination;
  }
}

QgsFeatureSink *QgsProcessingUtils::createFeatureSink( QString &destination, const QString &encoding, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs, QgsProcessingContext &context )
{
  QString destEncoding = encoding;
  QgsVectorLayer *layer = nullptr;
  if ( destEncoding.isEmpty() )
  {
    // no destination encoding specified, use default
    destEncoding = context.defaultEncoding().isEmpty() ? QStringLiteral( "system" ) : context.defaultEncoding();
  }

  if ( destination.isEmpty() || destination.startsWith( QStringLiteral( "memory:" ) ) )
  {
    // memory provider cannot be used with QgsVectorLayerImport - so create layer manually
    layer = QgsMemoryProviderUtils::createMemoryLayer( destination, fields, geometryType, crs );
  }
  else
  {
    QMap<QString, QVariant> options;
    options.insert( QStringLiteral( "fileEncoding" ), destEncoding );

    QString providerKey;
    QString uri;
    QString format;
    parseDestinationString( destination, providerKey, uri, format, options );

    if ( providerKey == "ogr" )
    {
      // use QgsVectorFileWriter for OGR destinations instead of QgsVectorLayerImport, as that allows
      // us to use any OGR format which supports feature addition
      QString finalFileName;
      QgsVectorFileWriter *writer = new QgsVectorFileWriter( destination, destEncoding, fields, geometryType, crs, format, QgsVectorFileWriter::defaultDatasetOptions( format ),
          QgsVectorFileWriter::defaultLayerOptions( format ), &finalFileName );
      destination = finalFileName;
      return writer;
    }
    else
    {
      //create empty layer
      {
        QgsVectorLayerExporter import( uri, providerKey, fields, geometryType, crs, false, &options );
        if ( import.errorCode() )
          return nullptr;
      }

      // use destination string as layer name (eg "postgis:..." )
      layer = new QgsVectorLayer( uri, destination, providerKey );
    }
  }

  if ( !layer )
    return nullptr;

  if ( !layer->isValid() )
  {
    delete layer;
    return nullptr;
  }

  // update destination to layer ID
  destination = layer->id();

  context.temporaryLayerStore()->addMapLayer( layer );

  // this is a factory, so we need to return a proxy
  return new QgsProxyFeatureSink( layer->dataProvider() );
}

void QgsProcessingUtils::createFeatureSinkPython( QgsFeatureSink **sink, QString &destination, const QString &encoding, const QgsFields &fields, QgsWkbTypes::Type geometryType, const QgsCoordinateReferenceSystem &crs, QgsProcessingContext &context )
{
  *sink = createFeatureSink( destination, encoding, fields, geometryType, crs, context );
}


QgsRectangle QgsProcessingUtils::combineLayerExtents( const QList<QgsMapLayer *> layers, const QgsCoordinateReferenceSystem &crs )
{
  QgsRectangle extent;
  Q_FOREACH ( QgsMapLayer *layer, layers )
  {
    if ( !layer )
      continue;

    if ( crs.isValid() )
    {
      //transform layer extent to target CRS
      QgsCoordinateTransform ct( layer->crs(), crs );
      try
      {
        QgsRectangle reprojExtent = ct.transformBoundingBox( layer->extent() );
        extent.combineExtentWith( reprojExtent );
      }
      catch ( QgsCsException & )
      {
        // can't reproject... what to do here? hmmm?
        // let's ignore this layer for now, but maybe we should just use the original extent?
      }
    }
    else
    {
      extent.combineExtentWith( layer->extent() );
    }

  }
  return extent;
}


//
// QgsProcessingFeatureSource
//

QgsProcessingFeatureSource::QgsProcessingFeatureSource( QgsFeatureSource *originalSource, const QgsProcessingContext &context, bool ownsOriginalSource )
  : mSource( originalSource )
  , mOwnsSource( ownsOriginalSource )
  , mInvalidGeometryCheck( context.invalidGeometryCheck() )
  , mInvalidGeometryCallback( context.invalidGeometryCallback() )
{}

QgsProcessingFeatureSource::~QgsProcessingFeatureSource()
{
  if ( mOwnsSource )
    delete mSource;
}

QgsFeatureIterator QgsProcessingFeatureSource::getFeatures( const QgsFeatureRequest &request ) const
{
  QgsFeatureRequest req( request );
  req.setInvalidGeometryCheck( mInvalidGeometryCheck );
  req.setInvalidGeometryCallback( mInvalidGeometryCallback );
  return mSource->getFeatures( req );
}

QgsCoordinateReferenceSystem QgsProcessingFeatureSource::sourceCrs() const
{
  return mSource->sourceCrs();
}

QgsFields QgsProcessingFeatureSource::fields() const
{
  return mSource->fields();
}

QgsWkbTypes::Type QgsProcessingFeatureSource::wkbType() const
{
  return mSource->wkbType();
}

long QgsProcessingFeatureSource::featureCount() const
{
  return mSource->featureCount();
}









