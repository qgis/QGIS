#include "qgsailayertools.h"

#include "qgsaifilecontextprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h"
#include "qgsfields.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerfactory.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QStringList>
#include <QVariant>

namespace
{
  QJsonObject schemaObject( const QJsonObject &properties, const QJsonArray &required = QJsonArray() )
  {
    QJsonObject schema;
    schema.insert( QStringLiteral( "type" ), QStringLiteral( "object" ) );
    schema.insert( QStringLiteral( "properties" ), properties );
    if ( !required.isEmpty() )
      schema.insert( QStringLiteral( "required" ), required );
    return schema;
  }

  QJsonObject prop( const QString &type, const QString &description )
  {
    QJsonObject p;
    p.insert( QStringLiteral( "type" ), type );
    p.insert( QStringLiteral( "description" ), description );
    return p;
  }

  // Vector and raster file extensions we auto-detect. Lowercase, no leading dot.
  const QSet<QString> &vectorExts()
  {
    static const QSet<QString> set { QStringLiteral( "shp" ), QStringLiteral( "geojson" ), QStringLiteral( "json" ),
                                     QStringLiteral( "gpkg" ), QStringLiteral( "kml" ), QStringLiteral( "kmz" ),
                                     QStringLiteral( "tab" ), QStringLiteral( "mif" ), QStringLiteral( "gml" ),
                                     QStringLiteral( "gpx" ), QStringLiteral( "csv" ), QStringLiteral( "fgb" ),
                                     QStringLiteral( "sqlite" ), QStringLiteral( "db" ) };
    return set;
  }
  const QSet<QString> &rasterExts()
  {
    static const QSet<QString> set { QStringLiteral( "tif" ), QStringLiteral( "tiff" ), QStringLiteral( "asc" ),
                                     QStringLiteral( "img" ), QStringLiteral( "jp2" ), QStringLiteral( "vrt" ),
                                     QStringLiteral( "nc" ), QStringLiteral( "hdf" ), QStringLiteral( "hgt" ),
                                     QStringLiteral( "ecw" ), QStringLiteral( "mbtiles" ), QStringLiteral( "dem" ),
                                     QStringLiteral( "bil" ), QStringLiteral( "png" ), QStringLiteral( "jpg" ),
                                     QStringLiteral( "jpeg" ) };
    return set;
  }

  QString detectKind( const QString &path )
  {
    const QString ext = QFileInfo( path ).suffix().toLower();
    if ( vectorExts().contains( ext ) )
      return QStringLiteral( "vector" );
    if ( rasterExts().contains( ext ) )
      return QStringLiteral( "raster" );
    return QString();
  }

  // Resolve a user-supplied path: try workspace first, then accept absolute path
  // if the file exists on disk. Returns empty QString if unresolvable.
  QString resolvePath( QgsAiFileContextProvider *provider, const QString &path )
  {
    if ( provider )
    {
      const QString resolved = provider->resolveWorkspaceFile( path );
      if ( !resolved.isEmpty() && QFileInfo::exists( resolved ) )
        return resolved;
    }
    const QFileInfo info( path );
    if ( info.exists() && info.isFile() )
      return info.absoluteFilePath();
    return QString();
  }

  QJsonObject extentJson( const QgsRectangle &extent )
  {
    QJsonObject e;
    e.insert( QStringLiteral( "xmin" ), extent.xMinimum() );
    e.insert( QStringLiteral( "ymin" ), extent.yMinimum() );
    e.insert( QStringLiteral( "xmax" ), extent.xMaximum() );
    e.insert( QStringLiteral( "ymax" ), extent.yMaximum() );
    return e;
  }
}

// ---------------------------------------------------------------------------
// add_layer_from_file
// ---------------------------------------------------------------------------

QgsAiAddLayerFromFileTool::QgsAiAddLayerFromFileTool( QgsAiFileContextProvider *contextProvider, QgsProject *project )
  : mContextProvider( contextProvider )
  , mProject( project )
{}

QString QgsAiAddLayerFromFileTool::description() const
{
  return QStringLiteral(
    "Loads a vector or raster file as a new map layer in the active QGIS project. "
    "Path may be workspace-relative or an absolute path to a file on disk. "
    "The layer kind is auto-detected from the file extension unless 'kind' is set explicitly. "
    "Returns the created layer's id, feature_count (vector) or band info (raster), CRS authid and extent. "
    "Use this instead of writing run_python with QgsVectorLayer/QgsRasterLayer when the user wants to "
    "load a single file — it's a single-shot, typed call with no Python quoting concerns."
  );
}

QJsonObject QgsAiAddLayerFromFileTool::schema() const
{
  QJsonObject properties;
  properties.insert( QStringLiteral( "path" ), prop( QStringLiteral( "string" ), QStringLiteral( "Workspace-relative or absolute path to the source file." ) ) );
  properties.insert( QStringLiteral( "name" ), prop( QStringLiteral( "string" ), QStringLiteral( "Optional display name. Defaults to the file stem." ) ) );
  properties.insert( QStringLiteral( "kind" ), prop( QStringLiteral( "string" ), QStringLiteral( "Optional. One of: 'vector', 'raster', 'auto'. Default 'auto' (detect by extension)." ) ) );
  return schemaObject( properties, QJsonArray { QStringLiteral( "path" ) } );
}

QgsAiToolResult QgsAiAddLayerFromFileTool::execute( const QJsonObject &args )
{
  const QString rawPath = args.value( QStringLiteral( "path" ) ).toString().trimmed();
  if ( rawPath.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Argument 'path' is required." ) );

  const QString path = resolvePath( mContextProvider, rawPath );
  if ( path.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Cannot resolve path to an existing file: %1" ).arg( rawPath ) );

  QgsProject *project = mProject ? mProject : QgsProject::instance();
  if ( !project )
    return QgsAiToolResult::error( QStringLiteral( "No active QgsProject available." ) );

  QString kind = args.value( QStringLiteral( "kind" ) ).toString().toLower().trimmed();
  if ( kind.isEmpty() || kind == QStringLiteral( "auto" ) )
    kind = detectKind( path );
  if ( kind.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Cannot auto-detect layer kind from extension. Set 'kind' to 'vector' or 'raster'. Path: %1" ).arg( path ) );

  const QString name = args.value( QStringLiteral( "name" ) ).toString().trimmed().isEmpty()
                         ? QFileInfo( path ).completeBaseName()
                         : args.value( QStringLiteral( "name" ) ).toString().trimmed();

  QgsMapLayer *added = nullptr;
  QJsonObject output;
  output.insert( QStringLiteral( "kind" ), kind );

  if ( kind == QStringLiteral( "vector" ) )
  {
    auto layer = std::make_unique<QgsVectorLayer>( path, name, QStringLiteral( "ogr" ) );
    if ( !layer->isValid() )
      return QgsAiToolResult::error( QStringLiteral( "Vector layer is invalid: %1 (provider error: %2)" ).arg( path, layer->error().summary() ) );

    output.insert( QStringLiteral( "feature_count" ), static_cast<qint64>( layer->featureCount() ) );
    output.insert( QStringLiteral( "geometry_type" ), QgsWkbTypes::geometryDisplayString( layer->geometryType() ) );
    added = layer.release();
  }
  else if ( kind == QStringLiteral( "raster" ) )
  {
    auto layer = std::make_unique<QgsRasterLayer>( path, name, QStringLiteral( "gdal" ) );
    if ( !layer->isValid() )
      return QgsAiToolResult::error( QStringLiteral( "Raster layer is invalid: %1 (provider error: %2)" ).arg( path, layer->error().summary() ) );

    output.insert( QStringLiteral( "width" ), layer->width() );
    output.insert( QStringLiteral( "height" ), layer->height() );
    output.insert( QStringLiteral( "bands" ), layer->bandCount() );
    added = layer.release();
  }
  else
  {
    return QgsAiToolResult::error( QStringLiteral( "Unknown 'kind': %1 (expected 'vector' or 'raster')." ).arg( kind ) );
  }

  project->addMapLayer( added );

  output.insert( QStringLiteral( "layer_id" ), added->id() );
  output.insert( QStringLiteral( "name" ), added->name() );
  output.insert( QStringLiteral( "crs" ), added->crs().authid() );
  output.insert( QStringLiteral( "source" ), added->publicSource() );
  output.insert( QStringLiteral( "extent" ), extentJson( added->extent() ) );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// describe_layer
// ---------------------------------------------------------------------------

QgsAiDescribeLayerTool::QgsAiDescribeLayerTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiDescribeLayerTool::description() const
{
  return QStringLiteral(
    "Describes a layer already loaded in the project. For vectors: id, name, geometry_type, "
    "feature_count, CRS, extent, fields[{name,type}], and (optionally) a small sample of "
    "feature attributes. For rasters: dimensions and band count. "
    "Use this BEFORE writing run_python that filters or aggregates by attribute — it lets you "
    "discover field names and types in one read-only call instead of two run_python rounds."
  );
}

QJsonObject QgsAiDescribeLayerTool::schema() const
{
  QJsonObject properties;
  properties.insert( QStringLiteral( "layer_id" ), prop( QStringLiteral( "string" ), QStringLiteral( "Id of the layer (as returned by list_project_layers / add_layer_from_file)." ) ) );
  properties.insert( QStringLiteral( "sample_features" ), prop( QStringLiteral( "integer" ), QStringLiteral( "Optional number of feature attribute samples to include (default 0, hard cap 10)." ) ) );
  return schemaObject( properties, QJsonArray { QStringLiteral( "layer_id" ) } );
}

QgsAiToolResult QgsAiDescribeLayerTool::execute( const QJsonObject &args )
{
  const QString layerId = args.value( QStringLiteral( "layer_id" ) ).toString().trimmed();
  if ( layerId.isEmpty() )
    return QgsAiToolResult::error( QStringLiteral( "Argument 'layer_id' is required." ) );

  QgsProject *project = mProject ? mProject : QgsProject::instance();
  if ( !project )
    return QgsAiToolResult::error( QStringLiteral( "No active QgsProject available." ) );

  QgsMapLayer *layer = project->mapLayer( layerId );
  if ( !layer )
    return QgsAiToolResult::error( QStringLiteral( "No layer with id: %1" ).arg( layerId ) );

  const int requestedSamples = args.value( QStringLiteral( "sample_features" ) ).toInt( 0 );
  const int maxSamples = std::clamp( requestedSamples, 0, 10 );

  QJsonObject output;
  output.insert( QStringLiteral( "id" ), layer->id() );
  output.insert( QStringLiteral( "name" ), layer->name() );
  output.insert( QStringLiteral( "type" ), QgsMapLayerFactory::typeToString( layer->type() ) );
  output.insert( QStringLiteral( "crs" ), layer->crs().authid() );
  output.insert( QStringLiteral( "source" ), layer->publicSource() );
  output.insert( QStringLiteral( "extent" ), extentJson( layer->extent() ) );

  if ( QgsVectorLayer *vector = qobject_cast<QgsVectorLayer *>( layer ) )
  {
    output.insert( QStringLiteral( "geometry_type" ), QgsWkbTypes::geometryDisplayString( vector->geometryType() ) );
    output.insert( QStringLiteral( "feature_count" ), static_cast<qint64>( vector->featureCount() ) );

    QJsonArray fields;
    const QgsFields layerFields = vector->fields();
    for ( const QgsField &field : layerFields )
    {
      QJsonObject f;
      f.insert( QStringLiteral( "name" ), field.name() );
      f.insert( QStringLiteral( "type" ), field.typeName() );
      fields.push_back( f );
    }
    output.insert( QStringLiteral( "fields" ), fields );

    if ( maxSamples > 0 )
    {
      QJsonArray samples;
      QgsFeatureRequest request;
      request.setLimit( maxSamples );
      request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
      QgsFeatureIterator it = vector->getFeatures( request );
      QgsFeature feature;
      while ( it.nextFeature( feature ) && samples.size() < maxSamples )
      {
        QJsonObject row;
        for ( const QgsField &field : layerFields )
        {
          const QVariant value = feature.attribute( field.name() );
          row.insert( field.name(), QJsonValue::fromVariant( value ) );
        }
        samples.push_back( row );
      }
      output.insert( QStringLiteral( "sample_features" ), samples );
    }
  }
  else if ( QgsRasterLayer *raster = qobject_cast<QgsRasterLayer *>( layer ) )
  {
    output.insert( QStringLiteral( "width" ), raster->width() );
    output.insert( QStringLiteral( "height" ), raster->height() );
    output.insert( QStringLiteral( "bands" ), raster->bandCount() );
  }

  return QgsAiToolResult::ok( output );
}
