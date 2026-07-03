/***************************************************************************
    qgsaiattributetabletools.cpp
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaiattributetabletools.h"

#include "qgsaitoolschemautil.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgsfield.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgsrectangle.h"
#include "qgsvectorlayer.h"

#include <algorithm>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>
#include <QVariant>

using namespace Qt::StringLiterals;

namespace
{
  struct AttributeTableRollbackEntry
  {
      QString layerId;
      QString fieldName;
      QHash<QgsFeatureId, QVariant> oldValues;
  };

  QHash<QString, AttributeTableRollbackEntry> &attributeTableRollbackStore()
  {
    static QHash<QString, AttributeTableRollbackEntry> store;
    return store;
  }

  QString storeRollback( const AttributeTableRollbackEntry &entry )
  {
    const QString token = QUuid::createUuid().toString( QUuid::WithoutBraces );
    attributeTableRollbackStore().insert( token, entry );
    return token;
  }

  QJsonObject rollbackJson( const QString &token, const QString &action )
  {
    QJsonObject rollback;
    rollback.insert( u"token"_s, token );
    rollback.insert( u"action"_s, action );
    rollback.insert( u"volatile"_s, true );
    return rollback;
  }

  QJsonObject featureJson( const QgsFeature &feature, const QgsFields &fields, bool includeGeometry )
  {
    QJsonObject output;
    output.insert( u"feature_id"_s, static_cast<qint64>( feature.id() ) );

    QJsonObject attributes;
    for ( int i = 0; i < fields.count(); ++i )
      attributes.insert( fields.at( i ).name(), QJsonValue::fromVariant( feature.attribute( i ) ) );
    output.insert( u"attributes"_s, attributes );

    if ( includeGeometry && feature.hasGeometry() )
      output.insert( u"geometry_wkt"_s, feature.geometry().asWkt() );
    return output;
  }

  QgsVectorLayer *vectorLayerFromArgs( QgsProject *project, const QJsonObject &args, QString &error )
  {
    if ( !project )
    {
      error = u"No active QgsProject available."_s;
      return nullptr;
    }
    const QString layerId = args.value( u"layer_id"_s ).toString().trimmed();
    if ( layerId.isEmpty() )
    {
      error = u"Argument 'layer_id' is required."_s;
      return nullptr;
    }
    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( project->mapLayer( layerId ) );
    if ( !layer )
      error = u"No vector layer with id: %1"_s.arg( layerId );
    return layer;
  }

  bool validateFilterExpression( const QString &filterExpression, QString &error )
  {
    if ( filterExpression.trimmed().isEmpty() )
      return true;
    QgsExpression expression( filterExpression );
    if ( expression.hasParserError() )
    {
      error = u"Filter expression parser error: %1"_s.arg( expression.parserErrorString() );
      return false;
    }
    return true;
  }

  bool selectBehaviorFromString( const QString &value, Qgis::SelectBehavior &behavior )
  {
    const QString normalized = value.toLower().trimmed();
    if ( normalized.isEmpty() || normalized == "replace"_L1 || normalized == "set"_L1 )
    {
      behavior = Qgis::SelectBehavior::SetSelection;
      return true;
    }
    if ( normalized == "add"_L1 || normalized == "add_to_selection"_L1 )
    {
      behavior = Qgis::SelectBehavior::AddToSelection;
      return true;
    }
    if ( normalized == "remove"_L1 || normalized == "remove_from_selection"_L1 )
    {
      behavior = Qgis::SelectBehavior::RemoveFromSelection;
      return true;
    }
    if ( normalized == "intersect"_L1 || normalized == "intersect_selection"_L1 )
    {
      behavior = Qgis::SelectBehavior::IntersectSelection;
      return true;
    }
    return false;
  }

  bool rectangleFromJson( const QJsonValue &value, QgsRectangle &rectangle, QString &error )
  {
    if ( value.isUndefined() || value.isNull() )
      return false;
    if ( value.isArray() )
    {
      const QJsonArray array = value.toArray();
      if ( array.size() != 4 )
      {
        error = u"bbox array must contain [xmin, ymin, xmax, ymax]."_s;
        return false;
      }
      rectangle = QgsRectangle( array.at( 0 ).toDouble(), array.at( 1 ).toDouble(), array.at( 2 ).toDouble(), array.at( 3 ).toDouble() );
    }
    else if ( value.isObject() )
    {
      const QJsonObject object = value.toObject();
      rectangle = QgsRectangle(
        object.value( u"xmin"_s ).toDouble(),
        object.value( u"ymin"_s ).toDouble(),
        object.value( u"xmax"_s ).toDouble(),
        object.value( u"ymax"_s ).toDouble()
      );
    }
    else
    {
      error = u"bbox must be an object or [xmin, ymin, xmax, ymax] array."_s;
      return false;
    }

    if ( rectangle.isNull() || !rectangle.isFinite() )
    {
      error = u"bbox is not a valid finite rectangle."_s;
      return false;
    }
    rectangle.normalize();
    return true;
  }

  QJsonArray featureIdsJson( const QgsFeatureIds &ids )
  {
    QList<QgsFeatureId> values = ids.values();
    std::sort( values.begin(), values.end() );
    QJsonArray array;
    for ( QgsFeatureId id : values )
      array.push_back( static_cast<qint64>( id ) );
    return array;
  }

  QgsAiToolResult rollbackBatchUpdate( QgsProject *project, const QString &token )
  {
    if ( !attributeTableRollbackStore().contains( token ) )
      return QgsAiToolResult::error( u"Unknown or expired rollback token."_s );
    const AttributeTableRollbackEntry entry = attributeTableRollbackStore().take( token );
    if ( !project )
      return QgsAiToolResult::error( u"No active QgsProject available."_s );

    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( project->mapLayer( entry.layerId ) );
    if ( !layer )
      return QgsAiToolResult::error( u"Cannot rollback batch update: layer no longer exists: %1"_s.arg( entry.layerId ) );
    const int fieldIndex = layer->fields().lookupField( entry.fieldName );
    if ( fieldIndex < 0 )
      return QgsAiToolResult::error( u"Cannot rollback batch update: field no longer exists: %1"_s.arg( entry.fieldName ) );

    const bool startedEditing = !layer->isEditable();
    if ( startedEditing && !layer->startEditing() )
      return QgsAiToolResult::error( u"Cannot start editing session for layer: %1"_s.arg( layer->name() ) );

    layer->beginEditCommand( u"AI batch attribute rollback"_s );
    for ( auto it = entry.oldValues.constBegin(); it != entry.oldValues.constEnd(); ++it )
    {
      if ( !layer->changeAttributeValue( it.key(), fieldIndex, it.value(), QVariant(), true ) )
      {
        layer->destroyEditCommand();
        if ( startedEditing )
          layer->rollBack();
        return QgsAiToolResult::error( u"Could not restore value for feature_id %1."_s.arg( it.key() ) );
      }
    }
    layer->endEditCommand();

    if ( startedEditing && !layer->commitChanges() )
      return QgsAiToolResult::error( u"Could not commit batch update rollback: %1"_s.arg( layer->commitErrors().join( u"; "_s ) ) );

    QJsonObject diff;
    diff.insert( u"summary"_s, u"Restored previous batch attribute values."_s );
    diff.insert( u"layer_id"_s, entry.layerId );
    diff.insert( u"field_name"_s, entry.fieldName );
    diff.insert( u"restored_feature_count"_s, entry.oldValues.size() );

    QJsonObject output;
    output.insert( u"status"_s, u"rolled_back"_s );
    output.insert( u"rollback_token"_s, token );
    output.insert( u"diff"_s, diff );
    return QgsAiToolResult::ok( output );
  }
}

// ---------------------------------------------------------------------------
// query_features
// ---------------------------------------------------------------------------

QgsAiQueryFeaturesTool::QgsAiQueryFeaturesTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiQueryFeaturesTool::description() const
{
  return QStringLiteral(
    "Queries vector layer features by optional QGIS filter_expression and returns paginated attributes."
  );
}

QJsonObject QgsAiQueryFeaturesTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"layer_id"_s, prop( u"string"_s, u"Target vector layer id."_s ) );
  properties.insert( u"filter_expression"_s, prop( u"string"_s, u"Optional QGIS expression used to filter features."_s ) );
  properties.insert( u"offset"_s, prop( u"integer"_s, u"Zero-based result offset. Defaults to 0."_s ) );
  properties.insert( u"limit"_s, prop( u"integer"_s, u"Maximum number of rows to return. Defaults to 50, max 500."_s ) );
  properties.insert( u"include_geometry"_s, prop( u"boolean"_s, u"If true, includes feature geometry WKT."_s ) );
  return schemaObject( properties, QJsonArray { u"layer_id"_s } );
}

QgsAiToolResult QgsAiQueryFeaturesTool::execute( const QJsonObject &args )
{
  QgsProject *project = mProject ? mProject : QgsProject::instance();
  QString error;
  QgsVectorLayer *layer = vectorLayerFromArgs( project, args, error );
  if ( !layer )
    return QgsAiToolResult::error( error );

  const QString filterExpression = args.value( u"filter_expression"_s ).toString().trimmed();
  if ( !validateFilterExpression( filterExpression, error ) )
    return QgsAiToolResult::error( error );

  QgsFeatureRequest request;
  if ( !filterExpression.isEmpty() )
    request.setFilterExpression( filterExpression );

  const int offset = std::max( 0, args.value( u"offset"_s ).toInt( 0 ) );
  const int limit = std::clamp( args.value( u"limit"_s ).toInt( 50 ), 1, 500 );
  const bool includeGeometry = args.value( u"include_geometry"_s ).toBool( false );

  QJsonArray rows;
  int matchedCount = 0;
  QgsFeatureIterator it = layer->getFeatures( request );
  QgsFeature feature;
  while ( it.nextFeature( feature ) )
  {
    if ( matchedCount >= offset && rows.size() < limit )
      rows.push_back( featureJson( feature, layer->fields(), includeGeometry ) );
    matchedCount++;
  }

  QJsonObject output;
  output.insert( u"layer_id"_s, layer->id() );
  output.insert( u"feature_count"_s, matchedCount );
  output.insert( u"offset"_s, offset );
  output.insert( u"limit"_s, limit );
  output.insert( u"features"_s, rows );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// batch_update_attributes
// ---------------------------------------------------------------------------

QgsAiBatchUpdateAttributesTool::QgsAiBatchUpdateAttributesTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiBatchUpdateAttributesTool::description() const
{
  return QStringLiteral(
    "Updates one field for every feature matching filter_expression and returns a rollback token."
  );
}

QJsonObject QgsAiBatchUpdateAttributesTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"layer_id"_s, prop( u"string"_s, u"Target vector layer id."_s ) );
  properties.insert( u"filter_expression"_s, prop( u"string"_s, u"QGIS expression selecting features to update."_s ) );
  properties.insert( u"field_name"_s, prop( u"string"_s, u"Field to update."_s ) );
  properties.insert( u"value"_s, prop( u"string"_s, u"New value. Converted to the target field type before writing."_s ) );
  properties.insert( u"rollback_token"_s, prop( u"string"_s, u"Optional token returned by a previous batch_update_attributes call."_s ) );
  return schemaObject( properties );
}

QgsAiToolResult QgsAiBatchUpdateAttributesTool::execute( const QJsonObject &args )
{
  QgsProject *project = mProject ? mProject : QgsProject::instance();
  const QString rollbackToken = args.value( u"rollback_token"_s ).toString().trimmed();
  if ( !rollbackToken.isEmpty() )
    return rollbackBatchUpdate( project, rollbackToken );

  QString error;
  QgsVectorLayer *layer = vectorLayerFromArgs( project, args, error );
  if ( !layer )
    return QgsAiToolResult::error( error );

  const QString filterExpression = args.value( u"filter_expression"_s ).toString().trimmed();
  if ( filterExpression.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'filter_expression' is required for batch updates."_s );
  if ( !validateFilterExpression( filterExpression, error ) )
    return QgsAiToolResult::error( error );

  const QString fieldName = args.value( u"field_name"_s ).toString().trimmed();
  const int fieldIndex = layer->fields().lookupField( fieldName );
  if ( fieldIndex < 0 )
    return QgsAiToolResult::error( u"No field named '%1' on layer: %2"_s.arg( fieldName, layer->name() ) );

  QVariant convertedValue = args.value( u"value"_s ).toVariant();
  QString conversionError;
  if ( !layer->fields().at( fieldIndex ).convertCompatible( convertedValue, &conversionError ) )
    return QgsAiToolResult::error( conversionError.isEmpty() ? u"Value is not compatible with field: %1"_s.arg( fieldName ) : conversionError );

  QgsFeatureRequest request;
  request.setFilterExpression( filterExpression );

  QList<QgsFeature> matchingFeatures;
  QgsFeatureIterator it = layer->getFeatures( request );
  QgsFeature feature;
  while ( it.nextFeature( feature ) )
    matchingFeatures.push_back( feature );

  const bool startedEditing = !layer->isEditable();
  if ( startedEditing && !layer->startEditing() )
    return QgsAiToolResult::error( u"Cannot start editing session for layer: %1"_s.arg( layer->name() ) );

  QHash<QgsFeatureId, QVariant> oldValues;
  layer->beginEditCommand( u"AI batch attribute update"_s );
  for ( const QgsFeature &matchingFeature : std::as_const( matchingFeatures ) )
  {
    oldValues.insert( matchingFeature.id(), matchingFeature.attribute( fieldIndex ) );
    if ( !layer->changeAttributeValue( matchingFeature.id(), fieldIndex, convertedValue, matchingFeature.attribute( fieldIndex ), true ) )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( u"Could not update feature_id %1."_s.arg( matchingFeature.id() ) );
    }
  }
  layer->endEditCommand();

  if ( startedEditing && !layer->commitChanges() )
    return QgsAiToolResult::error( u"Could not commit batch update: %1"_s.arg( layer->commitErrors().join( u"; "_s ) ) );

  AttributeTableRollbackEntry rollback;
  rollback.layerId = layer->id();
  rollback.fieldName = fieldName;
  rollback.oldValues = oldValues;
  const QString token = storeRollback( rollback );

  QJsonObject diff;
  diff.insert( u"summary"_s, u"Batch-updated feature attributes."_s );
  diff.insert( u"layer_id"_s, layer->id() );
  diff.insert( u"field_name"_s, fieldName );
  diff.insert( u"updated_feature_count"_s, matchingFeatures.size() );
  diff.insert( u"filter_expression"_s, filterExpression );

  QJsonObject output;
  output.insert( u"layer_id"_s, layer->id() );
  output.insert( u"field_name"_s, fieldName );
  output.insert( u"updated_feature_count"_s, matchingFeatures.size() );
  output.insert( u"diff"_s, diff );
  output.insert( u"rollback_token"_s, token );
  output.insert( u"rollback"_s, rollbackJson( token, u"restore_batch_attribute_values"_s ) );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// select_features
// ---------------------------------------------------------------------------

QgsAiSelectFeaturesTool::QgsAiSelectFeaturesTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiSelectFeaturesTool::description() const
{
  return QStringLiteral(
    "Selects vector layer features by QGIS expression and/or bounding box, updating the layer selection state."
  );
}

QJsonObject QgsAiSelectFeaturesTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"layer_id"_s, prop( u"string"_s, u"Target vector layer id."_s ) );
  properties.insert( u"mode"_s, prop( u"string"_s, u"Selection mode: replace, add, remove or intersect. Default replace."_s ) );
  properties.insert( u"filter_expression"_s, prop( u"string"_s, u"Optional QGIS expression selecting features."_s ) );
  properties.insert( u"bbox"_s, prop( u"object"_s, u"Optional rectangle object {xmin,ymin,xmax,ymax} or array [xmin,ymin,xmax,ymax] in layer CRS."_s ) );
  return schemaObject( properties, QJsonArray { u"layer_id"_s } );
}

QgsAiToolResult QgsAiSelectFeaturesTool::execute( const QJsonObject &args )
{
  QgsProject *project = mProject ? mProject : QgsProject::instance();
  QString error;
  QgsVectorLayer *layer = vectorLayerFromArgs( project, args, error );
  if ( !layer )
    return QgsAiToolResult::error( error );

  Qgis::SelectBehavior behavior = Qgis::SelectBehavior::SetSelection;
  if ( !selectBehaviorFromString( args.value( u"mode"_s ).toString(), behavior ) )
    return QgsAiToolResult::error( u"Invalid selection mode. Use replace, add, remove or intersect."_s );

  const QString filterExpression = args.value( u"filter_expression"_s ).toString().trimmed();
  if ( !validateFilterExpression( filterExpression, error ) )
    return QgsAiToolResult::error( error );

  QgsFeatureRequest request;
  if ( !filterExpression.isEmpty() )
    request.setFilterExpression( filterExpression );

  QgsRectangle bbox;
  if ( args.contains( u"bbox"_s ) )
  {
    if ( !rectangleFromJson( args.value( u"bbox"_s ), bbox, error ) )
      return QgsAiToolResult::error( error );
    request.setFilterRect( bbox );
  }

  QgsFeatureIds ids;
  QgsFeature feature;
  QgsFeatureIterator it = layer->getFeatures( request );
  while ( it.nextFeature( feature ) )
    ids.insert( feature.id() );

  const int beforeSelectedCount = layer->selectedFeatureCount();
  layer->selectByIds( ids, behavior, true );

  QJsonObject diff;
  diff.insert( u"summary"_s, u"Updated vector layer selection."_s );
  diff.insert( u"before_selected_count"_s, beforeSelectedCount );
  diff.insert( u"after_selected_count"_s, layer->selectedFeatureCount() );
  diff.insert( u"matched_feature_count"_s, ids.size() );

  QJsonObject output;
  output.insert( u"layer_id"_s, layer->id() );
  output.insert( u"selected_count"_s, layer->selectedFeatureCount() );
  output.insert( u"matched_feature_count"_s, ids.size() );
  output.insert( u"selected_feature_ids"_s, featureIdsJson( layer->selectedFeatureIds() ) );
  output.insert( u"diff"_s, diff );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// identify_features_at
// ---------------------------------------------------------------------------

QgsAiIdentifyFeaturesAtTool::QgsAiIdentifyFeaturesAtTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiIdentifyFeaturesAtTool::description() const
{
  return QStringLiteral(
    "Identifies vector layer features at a coordinate, using an optional source CRS and layer-unit tolerance."
  );
}

QJsonObject QgsAiIdentifyFeaturesAtTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"layer_id"_s, prop( u"string"_s, u"Target vector layer id."_s ) );
  properties.insert( u"x"_s, prop( u"number"_s, u"X coordinate."_s ) );
  properties.insert( u"y"_s, prop( u"number"_s, u"Y coordinate."_s ) );
  properties.insert( u"crs"_s, prop( u"string"_s, u"Optional CRS for x/y, e.g. EPSG:4326. Defaults to layer CRS."_s ) );
  properties.insert( u"tolerance"_s, prop( u"number"_s, u"Optional search tolerance in layer CRS units. Defaults to 0."_s ) );
  return schemaObject( properties, QJsonArray { u"layer_id"_s, u"x"_s, u"y"_s } );
}

QgsAiToolResult QgsAiIdentifyFeaturesAtTool::execute( const QJsonObject &args )
{
  QgsProject *project = mProject ? mProject : QgsProject::instance();
  QString error;
  QgsVectorLayer *layer = vectorLayerFromArgs( project, args, error );
  if ( !layer )
    return QgsAiToolResult::error( error );
  if ( !args.contains( u"x"_s ) || !args.contains( u"y"_s ) )
    return QgsAiToolResult::error( u"Arguments 'x' and 'y' are required."_s );

  QgsPointXY point( args.value( u"x"_s ).toDouble(), args.value( u"y"_s ).toDouble() );
  const QString crsDefinition = args.value( u"crs"_s ).toString().trimmed();
  if ( !crsDefinition.isEmpty() )
  {
    const QgsCoordinateReferenceSystem sourceCrs( crsDefinition );
    if ( !sourceCrs.isValid() )
      return QgsAiToolResult::error( u"Invalid CRS definition: %1"_s.arg( crsDefinition ) );
    if ( sourceCrs != layer->crs() )
    {
      try
      {
        QgsCoordinateTransform transform( sourceCrs, layer->crs(), project );
        point = transform.transform( point );
      }
      catch ( QgsCsException & )
      {
        return QgsAiToolResult::error( u"Could not transform identify point to layer CRS."_s );
      }
    }
  }

  const double tolerance = std::max( 0.0, args.value( u"tolerance"_s ).toDouble( 0.0 ) );
  const double searchTolerance = tolerance > 0.0 ? tolerance : 1e-9;
  const QgsRectangle searchRect( point.x() - searchTolerance, point.y() - searchTolerance, point.x() + searchTolerance, point.y() + searchTolerance );
  const QgsGeometry pointGeometry = QgsGeometry::fromPointXY( point );
  const QgsGeometry searchGeometry = tolerance > 0.0 ? pointGeometry.buffer( tolerance, 8 ) : pointGeometry;

  QgsFeatureRequest request;
  request.setFilterRect( searchRect );

  QJsonArray features;
  QgsFeature feature;
  QgsFeatureIterator it = layer->getFeatures( request );
  while ( it.nextFeature( feature ) )
  {
    if ( !feature.hasGeometry() )
      continue;
    const QgsGeometry geometry = feature.geometry();
    const bool matches = tolerance > 0.0
                           ? ( geometry.intersects( searchGeometry ) || geometry.distance( pointGeometry ) <= tolerance )
                           : ( geometry.intersects( pointGeometry ) || geometry.distance( pointGeometry ) <= 1e-9 );
    if ( matches )
      features.push_back( featureJson( feature, layer->fields(), true ) );
  }

  QJsonObject output;
  output.insert( u"layer_id"_s, layer->id() );
  output.insert( u"x"_s, point.x() );
  output.insert( u"y"_s, point.y() );
  output.insert( u"crs"_s, layer->crs().authid() );
  output.insert( u"tolerance"_s, tolerance );
  output.insert( u"feature_count"_s, features.size() );
  output.insert( u"features"_s, features );
  return QgsAiToolResult::ok( output );
}
