#include "qgsagentplugin.h"

#include "qgis.h"
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgsfield.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerfactory.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterfilewriter.h"
#include "qgsrasterhistogram.h"
#include "qgsrasterlayer.h"
#include "qgsrasterpipe.h"
#include "qgsstatisticalsummary.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"

#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#include <algorithm>
#include <cmath>

using namespace Qt::StringLiterals;

namespace
{
  QStringList jsonStringList( const QJsonValue &value )
  {
    QStringList values;
    for ( const QJsonValue &entry : value.toArray() )
    {
      if ( entry.isString() )
        values.append( entry.toString() );
    }
    return values;
  }

  QStringList featureIdStrings( const QgsFeatureIds &ids )
  {
    QStringList result;
    result.reserve( ids.size() );
  for ( QgsFeatureId id : ids )
    result.append( QString::number( id ) );
  return result;
}
  Qgis::SelectBehavior selectionBehavior( const QString &name, bool &ok )
  {
    ok = true;
    if ( name.isEmpty() || name == u"replace"_s )
      return Qgis::SelectBehavior::SetSelection;
    if ( name == u"add"_s )
      return Qgis::SelectBehavior::AddToSelection;
    if ( name == u"remove"_s )
      return Qgis::SelectBehavior::RemoveFromSelection;
    if ( name == u"intersect"_s )
      return Qgis::SelectBehavior::IntersectSelection;
    ok = false;
    return Qgis::SelectBehavior::SetSelection;
  }
  QString rasterDataTypeName( Qgis::DataType type )
  {
    switch ( type )
    {
      case Qgis::DataType::Byte:
        return u"Byte"_s;
      case Qgis::DataType::Int8:
        return u"Int8"_s;
      case Qgis::DataType::UInt16:
        return u"UInt16"_s;
      case Qgis::DataType::Int16:
        return u"Int16"_s;
      case Qgis::DataType::UInt32:
        return u"UInt32"_s;
      case Qgis::DataType::Int32:
        return u"Int32"_s;
      case Qgis::DataType::Float32:
        return u"Float32"_s;
      case Qgis::DataType::Float64:
        return u"Float64"_s;
      case Qgis::DataType::CInt16:
        return u"CInt16"_s;
      case Qgis::DataType::CInt32:
        return u"CInt32"_s;
      case Qgis::DataType::CFloat32:
        return u"CFloat32"_s;
      case Qgis::DataType::CFloat64:
        return u"CFloat64"_s;
      case Qgis::DataType::ARGB32:
        return u"ARGB32"_s;
      case Qgis::DataType::ARGB32_Premultiplied:
        return u"ARGB32_Premultiplied"_s;
      case Qgis::DataType::UnknownDataType:
        return u"Unknown"_s;
    }
    return u"Unknown"_s;
  }
}

QgsMapLayer *QgsAgentServer::findLayer( const QString &reference, QString &error ) const
{
  QgsProject *project = QgsProject::instance();
  QgsMapLayer *layer = project->mapLayer( reference );
  if ( layer )
    return layer;

  const QList<QgsMapLayer *> matches = project->mapLayersByName( reference );
  if ( matches.size() == 1 )
    return matches.constFirst();
  error = matches.isEmpty() ? tr( "Layer not found: %1" ).arg( reference )
                            : tr( "Multiple layers are named '%1'; use a layer ID." ).arg( reference );
  return nullptr;
}
QJsonObject QgsAgentServer::listDataSourceLayers( const QJsonObject &arguments ) const
{
  const QString path = arguments.value( u"path"_s ).toString();
  if ( path.isEmpty() || !QFileInfo::exists( path ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "An existing data source path is required." ) } };

  const QList<QgsProviderSublayerDetails> details = QgsProviderRegistry::instance()->querySublayers(
    path,
    Qgis::SublayerQueryFlag::ResolveGeometryType
  );
  QJsonArray layers;
  for ( const QgsProviderSublayerDetails &detail : details )
  {
    layers.append( QJsonObject{
      { u"name"_s, detail.name() },
      { u"description"_s, detail.description() },
      { u"provider"_s, detail.providerKey() },
      { u"type"_s, QgsMapLayerFactory::typeToString( detail.type() ) },
      { u"uri"_s, detail.uri() },
      { u"path"_s, QJsonArray::fromStringList( detail.path() ) },
      { u"feature_count"_s, detail.featureCount() },
      { u"geometry_type"_s, QgsWkbTypes::displayString( detail.wkbType() ) },
    } );
  }
  return QJsonObject{
    { u"ok"_s, true },
    { u"source"_s, path },
    { u"sublayers"_s, layers },
  };
}
bool QgsAgentServer::confirmWriteAction( const QString &title, const QString &details ) const
{
  return !mConfirmActions
         || QMessageBox::question( mInterface->mainWindow(), title, details, QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes;
}

QJsonObject QgsAgentServer::loadLayer( const QJsonObject &arguments )
{
  QStringList paths = jsonStringList( arguments.value( u"paths"_s ) );
  if ( paths.isEmpty() && arguments.value( u"path"_s ).isString() )
    paths.append( arguments.value( u"path"_s ).toString() );
  if ( paths.isEmpty() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "At least one path is required." ) } };

  const QString requestedType = arguments.value( u"type"_s ).toString( u"auto"_s ).toLower();
  const QString requestedProvider = arguments.value( u"provider"_s ).toString();
  const QString sublayer = arguments.value( u"sublayer"_s ).toString();
  QJsonArray loaded;
  QJsonArray errors;

  for ( const QString &originalPath : std::as_const( paths ) )
  {
    QString uri = originalPath;
    const QString filePath = originalPath.section( QLatin1Char( '|' ), 0, 0 );
    const QFileInfo fileInfo( filePath );
    if ( !fileInfo.exists() )
    {
      errors.append( QJsonObject{ { u"path"_s, originalPath }, { u"error"_s, tr( "File does not exist." ) } } );
      continue;
    }
    if ( !sublayer.isEmpty() && !uri.contains( u"|layername="_s ) )
      uri += u"|layername=%1"_s.arg( sublayer );

    const QString suffix = fileInfo.suffix().toLower();
    const bool isRaster = requestedType == u"raster"_s
                          || ( requestedType == u"auto"_s
                               && QStringList{ u"tif"_s, u"tiff"_s, u"vrt"_s, u"img"_s, u"asc"_s, u"jp2"_s, u"png"_s, u"jpg"_s, u"jpeg"_s, u"nc"_s, u"grib"_s, u"grb"_s }.contains( suffix ) );
    const QString name = arguments.value( u"name"_s ).toString( fileInfo.completeBaseName() );
    QgsMapLayer *layer = nullptr;
    if ( isRaster )
      layer = mInterface->addRasterLayer( uri, name, requestedProvider.isEmpty() ? u"gdal"_s : requestedProvider );
    else
      layer = mInterface->addVectorLayer( uri, name, requestedProvider.isEmpty() ? u"ogr"_s : requestedProvider );

    if ( !layer || !layer->isValid() )
    {
      errors.append( QJsonObject{ { u"path"_s, originalPath }, { u"error"_s, tr( "QGIS could not load this data source." ) } } );
      continue;
    }
    loaded.append( layerDetails( layer ) );
    emit layersLoaded( QStringList{ layer->id() }, tr( "Loaded %1" ).arg( layer->name() ) );
  }

  return QJsonObject{
    { u"ok"_s, errors.isEmpty() },
    { u"loaded"_s, loaded },
    { u"errors"_s, errors },
  };
}

QJsonObject QgsAgentServer::exportLayer( const QJsonObject &arguments )
{
  QString error;
  QgsMapLayer *layer = findLayer( arguments.value( u"layer"_s ).toString(), error );
  if ( !layer )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error } };

  const QString path = arguments.value( u"path"_s ).toString();
  if ( path.isEmpty() || !QDir::isAbsolutePath( path ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "An absolute output path is required." ) } };
  const QFileInfo outputInfo( path );
  if ( !outputInfo.dir().exists() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Output directory does not exist: %1" ).arg( outputInfo.absolutePath() ) } };
  if ( outputInfo.exists() && !arguments.value( u"overwrite"_s ).toBool( false ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Output already exists. Set overwrite=true to replace it." ) } };
  if ( !confirmWriteAction( tr( "Export QGIS layer?" ), tr( "Layer: %1\nOutput: %2" ).arg( layer->name(), path ) ) )
    return QJsonObject{ { u"ok"_s, false }, { u"cancelled"_s, true } };

  if ( QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer ) )
  {
    QgsVectorFileWriter::SaveVectorOptions options;
    options.driverName = arguments.value( u"format"_s ).toString();
    if ( options.driverName.isEmpty() )
      options.driverName = QgsVectorFileWriter::driverForExtension( outputInfo.suffix() );
    if ( options.driverName.isEmpty() )
      return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Could not determine a vector driver for %1." ).arg( path ) } };
    options.fileEncoding = arguments.value( u"encoding"_s ).toString( u"UTF-8"_s );
    options.onlySelectedFeatures = arguments.value( u"selected_only"_s ).toBool( false );
    options.actionOnExistingFile = QgsVectorFileWriter::CreateOrOverwriteFile;
    QString errorMessage;
    QString newFilename;
    QString newLayer;
    const QgsVectorFileWriter::WriterError result = QgsVectorFileWriter::writeAsVectorFormatV3(
      vectorLayer,
      path,
      QgsProject::instance()->transformContext(),
      options,
      &errorMessage,
      &newFilename,
      &newLayer
    );
    return QJsonObject{
      { u"ok"_s, result == QgsVectorFileWriter::NoError },
      { u"path"_s, newFilename.isEmpty() ? path : newFilename },
      { u"layer_name"_s, newLayer },
      { u"driver"_s, options.driverName },
      { u"error"_s, errorMessage },
    };
  }

  QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( layer );
  if ( !rasterLayer || !rasterLayer->pipe() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Only vector and raster layers can be exported." ) } };

  QgsRasterPipe pipe;
  if ( !pipe.set( rasterLayer->dataProvider()->clone() ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Could not clone the raster data provider for export." ) } };
  QgsRasterFileWriter writer( path );
  writer.setOutputFormat( arguments.value( u"format"_s ).toString( u"GTiff"_s ) );
  const Qgis::RasterFileWriterResult result = writer.writeRaster(
    &pipe,
    rasterLayer->width(),
    rasterLayer->height(),
    rasterLayer->extent(),
    rasterLayer->crs(),
    QgsProject::instance()->transformContext()
  );
  return QJsonObject{
    { u"ok"_s, result == Qgis::RasterFileWriterResult::Success },
    { u"path"_s, path },
    { u"error_code"_s, static_cast<int>( result ) },
  };
}

QJsonObject QgsAgentServer::saveProject( const QJsonObject &arguments )
{
  QgsProject *project = QgsProject::instance();
  const QString requestedPath = arguments.value( u"path"_s ).toString();
  const QString path = requestedPath.isEmpty() ? project->fileName() : requestedPath;
  if ( path.isEmpty() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "The project has no path. Provide an absolute .qgz path." ) } };
  if ( !QDir::isAbsolutePath( path ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "An absolute project path is required." ) } };
  if ( !QFileInfo( path ).dir().exists() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Project directory does not exist." ) } };
  if ( QFileInfo::exists( path ) && path != project->fileName() && !arguments.value( u"overwrite"_s ).toBool( false ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Project file already exists. Set overwrite=true to replace it." ) } };
  if ( !confirmWriteAction( tr( "Save QGIS project?" ), tr( "Project path: %1" ).arg( path ) ) )
    return QJsonObject{ { u"ok"_s, false }, { u"cancelled"_s, true } };

  const bool ok = requestedPath.isEmpty() ? project->write() : project->write( path );
  return QJsonObject{
    { u"ok"_s, ok },
    { u"path"_s, project->fileName() },
    { u"error"_s, ok ? QString() : project->error() },
  };
}

QJsonObject QgsAgentServer::queryFeatures( const QJsonObject &arguments ) const
{
  QString error;
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( findLayer( arguments.value( u"layer"_s ).toString(), error ) );
  if ( !layer )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error.isEmpty() ? tr( "The requested layer is not a vector layer." ) : error } };

  const QString expressionText = arguments.value( u"expression"_s ).toString();
  if ( !expressionText.isEmpty() )
  {
    QgsExpression expression( expressionText );
    if ( expression.hasParserError() )
      return QJsonObject{ { u"ok"_s, false }, { u"error"_s, expression.parserErrorString() } };
  }

  QgsFeatureRequest request;
  if ( !expressionText.isEmpty() )
  {
    request.setFilterExpression( expressionText );
    request.setExpressionContext( QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) ) );
  }
  const bool selectedOnly = arguments.value( u"selected_only"_s ).toBool( false );
  const QgsFeatureIds selectedIds = selectedOnly ? layer->selectedFeatureIds() : QgsFeatureIds();
  const QStringList requestedFields = jsonStringList( arguments.value( u"fields"_s ) );
  if ( !requestedFields.isEmpty() )
    request.setSubsetOfAttributes( requestedFields, layer->fields() );
  const int limit = std::clamp( arguments.value( u"limit"_s ).toInt( 20 ), 1, 200 );
  request.setLimit( limit );

  QJsonArray features;
  QgsFeature feature;
  QgsFeatureIterator iterator = layer->getFeatures( request );
  const bool includeGeometry = arguments.value( u"include_geometry"_s ).toBool( false );
  while ( iterator.nextFeature( feature ) )
  {
    if ( selectedOnly && !selectedIds.contains( feature.id() ) )
      continue;
    QJsonObject attributes;
    const QVariantMap attributeMap = feature.attributeMap();
    if ( requestedFields.isEmpty() )
    {
      attributes = QJsonObject::fromVariantMap( attributeMap );
    }
    else
    {
      for ( const QString &fieldName : requestedFields )
        attributes.insert( fieldName, QJsonValue::fromVariant( attributeMap.value( fieldName ) ) );
    }
    QJsonObject item{
      { u"id"_s, QString::number( feature.id() ) },
      { u"attributes"_s, attributes },
    };
    if ( includeGeometry && feature.hasGeometry() )
      item.insert( u"geometry_wkt"_s, feature.geometry().asWkt( 8 ) );
    features.append( item );
  }
  return QJsonObject{
    { u"ok"_s, true },
    { u"layer_id"_s, layer->id() },
    { u"returned"_s, features.size() },
    { u"limit"_s, limit },
    { u"features"_s, features },
  };
}

QJsonObject QgsAgentServer::fieldStatistics( const QJsonObject &arguments ) const
{
  QString error;
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( findLayer( arguments.value( u"layer"_s ).toString(), error ) );
  if ( !layer )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error.isEmpty() ? tr( "The requested layer is not a vector layer." ) : error } };
  const QString fieldName = arguments.value( u"field"_s ).toString();
  const int index = layer->fields().indexFromName( fieldName );
  if ( index < 0 )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Field not found: %1" ).arg( fieldName ) } };

  const int uniqueLimit = std::clamp( arguments.value( u"unique_limit"_s ).toInt( 50 ), 1, 500 );
  QJsonArray uniqueValues;
  const QSet<QVariant> values = layer->uniqueValues( index, uniqueLimit );
  for ( const QVariant &value : values )
    uniqueValues.append( QJsonValue::fromVariant( value ) );

  qint64 nullCount = 0;
  qint64 nonNullCount = 0;
  QgsStatisticalSummary summary;
  QgsFeatureRequest request;
  request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  request.setSubsetOfAttributes( QgsAttributeList{ index } );
  QgsFeature feature;
  QgsFeatureIterator iterator = layer->getFeatures( request );
  while ( iterator.nextFeature( feature ) )
  {
    const QVariant value = feature.attribute( index );
    if ( value.isNull() )
      ++nullCount;
    else
    {
      ++nonNullCount;
      bool numeric = false;
      const double numericValue = value.toDouble( &numeric );
      if ( numeric )
        summary.addValue( numericValue );
    }
  }
  summary.finalize();
  const double outlierZScore = std::max( 0.0, arguments.value( u"outlier_z_score"_s ).toDouble( 3.0 ) );
  const int outlierLimit = std::clamp( arguments.value( u"outlier_limit"_s ).toInt( 50 ), 1, 500 );
  QJsonArray outliers;
  if ( summary.count() > 1 && std::isfinite( summary.stDev() ) && summary.stDev() > 0 )
  {
    QgsFeatureIterator outlierIterator = layer->getFeatures( request );
    while ( outlierIterator.nextFeature( feature ) && outliers.size() < outlierLimit )
    {
      bool numeric = false;
      const double numericValue = feature.attribute( index ).toDouble( &numeric );
      if ( !numeric )
        continue;
      const double zScore = std::abs( numericValue - summary.mean() ) / summary.stDev();
      if ( zScore >= outlierZScore )
      {
        outliers.append( QJsonObject{
          { u"id"_s, QString::number( feature.id() ) },
          { u"value"_s, numericValue },
          { u"z_score"_s, zScore },
        } );
      }
    }
  }
  return QJsonObject{
    { u"ok"_s, true },
    { u"layer_id"_s, layer->id() },
    { u"field"_s, fieldName },
    { u"type"_s, layer->fields().at( index ).typeName() },
    { u"minimum"_s, QJsonValue::fromVariant( layer->minimumValue( index ) ) },
    { u"maximum"_s, QJsonValue::fromVariant( layer->maximumValue( index ) ) },
    { u"null_count"_s, nullCount },
    { u"non_null_count"_s, nonNullCount },
    { u"numeric_count"_s, summary.count() },
    { u"mean"_s, summary.count() > 0 ? QJsonValue( summary.mean() ) : QJsonValue() },
    { u"standard_deviation"_s, summary.count() > 1 ? QJsonValue( summary.stDev() ) : QJsonValue() },
    { u"unique_values"_s, uniqueValues },
    { u"unique_values_truncated"_s, values.size() >= uniqueLimit },
    { u"outlier_z_score"_s, outlierZScore },
    { u"outliers"_s, outliers },
    { u"outliers_truncated"_s, outliers.size() >= outlierLimit },
  };
}

QJsonObject QgsAgentServer::validateExpression( const QJsonObject &arguments ) const
{
  QString error;
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( findLayer( arguments.value( u"layer"_s ).toString(), error ) );
  if ( !layer )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error.isEmpty() ? tr( "The requested layer is not a vector layer." ) : error } };
  const QString expressionText = arguments.value( u"expression"_s ).toString();
  QgsExpression expression( expressionText );
  if ( expression.hasParserError() )
    return QJsonObject{ { u"ok"_s, false }, { u"valid"_s, false }, { u"error"_s, expression.parserErrorString() } };

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  if ( !expression.prepare( &context ) )
    return QJsonObject{ { u"ok"_s, false }, { u"valid"_s, false }, { u"error"_s, expression.evalErrorString() } };
  qint64 matchedCount = 0;
  QgsFeatureRequest request;
  request.setFilterExpression( expressionText );
  request.setExpressionContext( context );
  request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  QgsFeature feature;
  QgsFeatureIterator iterator = layer->getFeatures( request );
  while ( iterator.nextFeature( feature ) )
    ++matchedCount;
  return QJsonObject{
    { u"ok"_s, true },
    { u"valid"_s, true },
    { u"matched_count"_s, matchedCount },
    { u"referenced_fields"_s, QJsonArray::fromStringList( expression.referencedColumns().values() ) },
  };
}

QJsonObject QgsAgentServer::selectFeatures( const QJsonObject &arguments )
{
  QString error;
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( findLayer( arguments.value( u"layer"_s ).toString(), error ) );
  if ( !layer )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error.isEmpty() ? tr( "The requested layer is not a vector layer." ) : error } };
  const QString expressionText = arguments.value( u"expression"_s ).toString();
  QgsExpression expression( expressionText );
  if ( expressionText.isEmpty() || expression.hasParserError() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, expressionText.isEmpty() ? tr( "An expression is required." ) : expression.parserErrorString() } };

  bool behaviorOk = false;
  const Qgis::SelectBehavior behavior = selectionBehavior( arguments.value( u"behavior"_s ).toString(), behaviorOk );
  if ( !behaviorOk )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Selection behavior must be replace, add, remove, or intersect." ) } };
  emit selectionChanged( layer->id(), featureIdStrings( layer->selectedFeatureIds() ) );
  layer->selectByExpression( expressionText, behavior );
  return QJsonObject{
    { u"ok"_s, true },
    { u"layer_id"_s, layer->id() },
    { u"selected_count"_s, layer->selectedFeatureCount() },
    { u"selected_feature_ids"_s, QJsonArray::fromStringList( featureIdStrings( layer->selectedFeatureIds() ) ) },
  };
}

QJsonObject QgsAgentServer::clearSelection( const QJsonObject &arguments )
{
  QString error;
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( findLayer( arguments.value( u"layer"_s ).toString(), error ) );
  if ( !layer )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error.isEmpty() ? tr( "The requested layer is not a vector layer." ) : error } };
  const QStringList previousIds = featureIdStrings( layer->selectedFeatureIds() );
  emit selectionChanged( layer->id(), previousIds );
  layer->removeSelection();
  return QJsonObject{ { u"ok"_s, true }, { u"layer_id"_s, layer->id() }, { u"cleared_count"_s, previousIds.size() } };
}

QJsonObject QgsAgentServer::mapCanvasState() const
{
  QgsMapCanvas *canvas = mInterface->mapCanvas();
  const QgsRectangle extent = canvas->extent();
  return QJsonObject{
    { u"ok"_s, true },
    { u"crs"_s, canvas->mapSettings().destinationCrs().authid() },
    { u"extent"_s, QJsonArray{ extent.xMinimum(), extent.yMinimum(), extent.xMaximum(), extent.yMaximum() } },
    { u"scale"_s, canvas->scale() },
    { u"map_units_per_pixel"_s, canvas->mapUnitsPerPixel() },
  };
}

QJsonObject QgsAgentServer::zoomToLayer( const QJsonObject &arguments )
{
  QString error;
  QgsMapLayer *layer = findLayer( arguments.value( u"layer"_s ).toString(), error );
  if ( !layer )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error } };
  QgsMapCanvas *canvas = mInterface->mapCanvas();
  const QgsRectangle previousExtent = canvas->extent();

  const QString expressionText = arguments.value( u"expression"_s ).toString();
  if ( !expressionText.isEmpty() )
  {
    QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( !vectorLayer )
      return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Feature expressions require a vector layer." ) } };
    QgsExpression expression( expressionText );
    if ( expression.hasParserError() )
      return QJsonObject{ { u"ok"_s, false }, { u"error"_s, expression.parserErrorString() } };
    QgsFeatureIds ids;
    QgsFeatureRequest request;
    request.setFilterExpression( expressionText );
    request.setExpressionContext( QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( vectorLayer ) ) );
    request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
    QgsFeature feature;
    QgsFeatureIterator iterator = vectorLayer->getFeatures( request );
    while ( iterator.nextFeature( feature ) )
      ids.insert( feature.id() );
    if ( ids.isEmpty() )
      return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "The expression did not match any features." ) } };
    emit canvasExtentChanged( previousExtent.xMinimum(), previousExtent.yMinimum(), previousExtent.xMaximum(), previousExtent.yMaximum() );
    canvas->zoomToFeatureIds( vectorLayer, ids );
  }
  else if ( arguments.value( u"selected_only"_s ).toBool( false ) )
  {
    QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( !vectorLayer || vectorLayer->selectedFeatureIds().isEmpty() )
      return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "The layer has no selected features." ) } };
    emit canvasExtentChanged( previousExtent.xMinimum(), previousExtent.yMinimum(), previousExtent.xMaximum(), previousExtent.yMaximum() );
    canvas->zoomToSelected( vectorLayer );
  }
  else
  {
    emit canvasExtentChanged( previousExtent.xMinimum(), previousExtent.yMinimum(), previousExtent.xMaximum(), previousExtent.yMaximum() );
    canvas->zoomToLayers( QList<QgsMapLayer *>{ layer } );
  }
  mInterface->setActiveLayer( layer );
  canvas->refresh();
  const QgsRectangle extent = canvas->extent();
  return QJsonObject{
    { u"ok"_s, true },
    { u"layer_id"_s, layer->id() },
    { u"extent"_s, QJsonArray{ extent.xMinimum(), extent.yMinimum(), extent.xMaximum(), extent.yMaximum() } },
  };
}

QJsonObject QgsAgentServer::inspectRaster( const QJsonObject &arguments ) const
{
  QString error;
  QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( findLayer( arguments.value( u"layer"_s ).toString(), error ) );
  if ( !layer || !layer->dataProvider() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error.isEmpty() ? tr( "The requested layer is not a raster layer." ) : error } };
  const int sampleSize = std::clamp( arguments.value( u"sample_size"_s ).toInt( 250000 ), 1000, 2000000 );
  QJsonArray bands;
  for ( int band = 1; band <= layer->bandCount(); ++band )
  {
    const QgsRasterBandStats stats = layer->dataProvider()->bandStatistics( band, Qgis::RasterBandStatistic::All, QgsRectangle(), sampleSize );
    QJsonObject bandDetails{
      { u"band"_s, band },
      { u"name"_s, layer->bandName( band ) },
      { u"data_type"_s, rasterDataTypeName( layer->dataProvider()->sourceDataType( band ) ) },
      { u"minimum"_s, stats.minimumValue },
      { u"maximum"_s, stats.maximumValue },
      { u"mean"_s, stats.mean },
      { u"standard_deviation"_s, stats.stdDev },
      { u"sample_count"_s, static_cast<double>( stats.elementCount ) },
    };
    if ( layer->dataProvider()->sourceHasNoDataValue( band ) )
      bandDetails.insert( u"source_nodata"_s, layer->dataProvider()->sourceNoDataValue( band ) );
    bands.append( bandDetails );
  }
  return QJsonObject{
    { u"ok"_s, true },
    { u"layer_id"_s, layer->id() },
    { u"width"_s, layer->width() },
    { u"height"_s, layer->height() },
    { u"band_count"_s, layer->bandCount() },
    { u"crs"_s, layer->crs().authid() },
    { u"bands"_s, bands },
  };
}

QJsonObject QgsAgentServer::suggestRasterThreshold( const QJsonObject &arguments ) const
{
  QString error;
  QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( findLayer( arguments.value( u"layer"_s ).toString(), error ) );
  if ( !layer || !layer->dataProvider() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error.isEmpty() ? tr( "The requested layer is not a raster layer." ) : error } };
  const int band = arguments.value( u"band"_s ).toInt( 1 );
  if ( band < 1 || band > layer->bandCount() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Band must be between 1 and %1." ).arg( layer->bandCount() ) } };
  const int binCount = std::clamp( arguments.value( u"bins"_s ).toInt( 256 ), 16, 4096 );
  const int sampleSize = std::clamp( arguments.value( u"sample_size"_s ).toInt( 500000 ), 1000, 5000000 );
  const QgsRasterHistogram histogram = layer->dataProvider()->histogram(
    band,
    binCount,
    std::numeric_limits<double>::quiet_NaN(),
    std::numeric_limits<double>::quiet_NaN(),
    QgsRectangle(),
    sampleSize
  );
  if ( !histogram.valid || histogram.histogramVector.isEmpty() || histogram.maximum <= histogram.minimum )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "QGIS could not calculate a valid raster histogram." ) } };

  double total = 0;
  double weightedTotal = 0;
  for ( int index = 0; index < histogram.histogramVector.size(); ++index )
  {
    const double count = histogram.histogramVector.at( index );
    total += count;
    weightedTotal += index * count;
  }

  double backgroundWeight = 0;
  double backgroundSum = 0;
  double bestVariance = -1;
  int bestIndex = 0;
  for ( int index = 0; index < histogram.histogramVector.size(); ++index )
  {
    const double count = histogram.histogramVector.at( index );
    backgroundWeight += count;
    if ( backgroundWeight <= 0 )
      continue;
    const double foregroundWeight = total - backgroundWeight;
    if ( foregroundWeight <= 0 )
      break;
    backgroundSum += index * count;
    const double backgroundMean = backgroundSum / backgroundWeight;
    const double foregroundMean = ( weightedTotal - backgroundSum ) / foregroundWeight;
    const double difference = backgroundMean - foregroundMean;
    const double variance = backgroundWeight * foregroundWeight * difference * difference;
    if ( variance > bestVariance )
    {
      bestVariance = variance;
      bestIndex = index;
    }
  }
  const double binWidth = ( histogram.maximum - histogram.minimum ) / histogram.histogramVector.size();
  const double threshold = histogram.minimum + ( bestIndex + 0.5 ) * binWidth;
  return QJsonObject{
    { u"ok"_s, true },
    { u"method"_s, u"otsu"_s },
    { u"layer_id"_s, layer->id() },
    { u"band"_s, band },
    { u"threshold"_s, threshold },
    { u"minimum"_s, histogram.minimum },
    { u"maximum"_s, histogram.maximum },
    { u"bins"_s, histogram.histogramVector.size() },
    { u"sample_count"_s, histogram.nonNullCount },
    { u"guidance"_s, tr( "Review the threshold against the raster histogram and map preview before classification." ) },
  };
}

QJsonObject QgsAgentServer::qualityCheck( const QJsonObject &arguments ) const
{
  QString error;
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( findLayer( arguments.value( u"layer"_s ).toString(), error ) );
  if ( !layer )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error.isEmpty() ? tr( "The requested layer is not a vector layer." ) : error } };
  const QStringList requiredFields = jsonStringList( arguments.value( u"required_fields"_s ) );
  const int issueLimit = std::clamp( arguments.value( u"issue_limit"_s ).toInt( 100 ), 1, 1000 );
  const int featureLimit = std::clamp( arguments.value( u"feature_limit"_s ).toInt( 100000 ), 1, 1000000 );
  QJsonArray missingFields;
  QHash<int, qint64> nullCounts;
  for ( const QString &field : requiredFields )
  {
    const int index = layer->fields().indexFromName( field );
    if ( index < 0 )
      missingFields.append( field );
    else
      nullCounts.insert( index, 0 );
  }

  qint64 emptyGeometryCount = 0;
  qint64 invalidGeometryCount = 0;
  qint64 duplicateGeometryCount = 0;
  QJsonArray invalidFeatureIds;
  QJsonArray duplicateFeatureIds;
  QHash<QByteArray, QgsFeatureId> seenGeometries;
  qint64 scannedCount = 0;
  QgsFeature feature;
  QgsFeatureIterator iterator = layer->getFeatures( QgsFeatureRequest().setLimit( featureLimit ) );
  while ( iterator.nextFeature( feature ) )
  {
    ++scannedCount;
    for ( auto it = nullCounts.begin(); it != nullCounts.end(); ++it )
    {
      if ( feature.attribute( it.key() ).isNull() )
        ++it.value();
    }
    if ( !feature.hasGeometry() || feature.geometry().isEmpty() )
    {
      ++emptyGeometryCount;
      continue;
    }
    if ( !feature.geometry().isGeosValid() )
    {
      ++invalidGeometryCount;
      if ( invalidFeatureIds.size() < issueLimit )
        invalidFeatureIds.append( QString::number( feature.id() ) );
    }
    const QByteArray wkb = feature.geometry().asWkb();
    if ( seenGeometries.contains( wkb ) )
    {
      ++duplicateGeometryCount;
      if ( duplicateFeatureIds.size() < issueLimit )
        duplicateFeatureIds.append( QString::number( feature.id() ) );
    }
    else
    {
      seenGeometries.insert( wkb, feature.id() );
    }
  }

  QJsonObject nullFields;
  for ( auto it = nullCounts.constBegin(); it != nullCounts.constEnd(); ++it )
    nullFields.insert( layer->fields().at( it.key() ).name(), it.value() );
  const QString projectCrs = QgsProject::instance()->crs().authid();
  return QJsonObject{
    { u"ok"_s, true },
    { u"layer_id"_s, layer->id() },
    { u"feature_count"_s, layer->featureCount() },
    { u"scanned_feature_count"_s, scannedCount },
    { u"scan_truncated"_s, layer->featureCount() > featureLimit },
    { u"empty_geometry_count"_s, emptyGeometryCount },
    { u"invalid_geometry_count"_s, invalidGeometryCount },
    { u"duplicate_geometry_count"_s, duplicateGeometryCount },
    { u"invalid_feature_ids"_s, invalidFeatureIds },
    { u"duplicate_feature_ids"_s, duplicateFeatureIds },
    { u"missing_fields"_s, missingFields },
    { u"null_counts"_s, nullFields },
    { u"layer_crs"_s, layer->crs().authid() },
    { u"project_crs"_s, projectCrs },
    { u"crs_matches_project"_s, projectCrs.isEmpty() || layer->crs().authid() == projectCrs },
  };
}

QJsonObject QgsAgentServer::runProcessingAlgorithmInternal( const QJsonObject &arguments, bool confirm )
{
  const bool previousConfirm = mConfirmActions;
  if ( !confirm )
    mConfirmActions = false;
  const QJsonObject result = runProcessingAlgorithm( arguments );
  mConfirmActions = previousConfirm;
  return result;
}
