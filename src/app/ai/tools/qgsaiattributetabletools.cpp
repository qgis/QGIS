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
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgsfield.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

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
