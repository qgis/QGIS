/***************************************************************************
    qgsaieditingtools.cpp
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

#include "qgsaieditingtools.h"

#include "qgsaitoolschemautil.h"
#include "qgsfield.h"
#include "qgsfields.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
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
  enum class EditingRollbackType
  {
    RestoreGeometry,
    RestoreAttributes
  };

  struct EditingRollbackEntry
  {
      EditingRollbackType type = EditingRollbackType::RestoreGeometry;
      QString layerId;
      QgsFeatureId featureId = FID_NULL;
      QString geometryWkt;
      QgsAttributeMap oldAttributes;
      QString description;
  };

  QHash<QString, EditingRollbackEntry> &editingRollbackStore()
  {
    static QHash<QString, EditingRollbackEntry> store;
    return store;
  }

  QString storeEditingRollback( const EditingRollbackEntry &entry )
  {
    const QString token = QUuid::createUuid().toString( QUuid::WithoutBraces );
    editingRollbackStore().insert( token, entry );
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

  QJsonObject geometrySummary( const QgsGeometry &geometry )
  {
    QJsonObject output;
    output.insert( u"wkt"_s, geometry.asWkt() );
    output.insert( u"is_empty"_s, geometry.isEmpty() );
    output.insert( u"is_geos_valid"_s, geometry.isNull() || geometry.isEmpty() ? false : geometry.isGeosValid() );
    return output;
  }

  QgsFeatureId featureIdFromJson( const QJsonObject &args )
  {
    return args.value( u"feature_id"_s ).toVariant().toLongLong();
  }

  QgsAiToolResult rollbackGeometry( QgsProject *project, const QString &token )
  {
    if ( !editingRollbackStore().contains( token ) )
      return QgsAiToolResult::error( u"Unknown or expired rollback token."_s );
    const EditingRollbackEntry entry = editingRollbackStore().take( token );
    if ( entry.type != EditingRollbackType::RestoreGeometry )
      return QgsAiToolResult::error( u"Rollback token does not belong to a geometry edit operation."_s );
    if ( !project )
      return QgsAiToolResult::error( u"No active QgsProject available."_s );

    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( project->mapLayer( entry.layerId ) );
    if ( !layer )
      return QgsAiToolResult::error( u"Cannot rollback geometry: layer no longer exists: %1"_s.arg( entry.layerId ) );

    const bool startedEditing = !layer->isEditable();
    if ( startedEditing && !layer->startEditing() )
      return QgsAiToolResult::error( u"Cannot start editing session for layer: %1"_s.arg( layer->name() ) );

    QgsFeature beforeFeature;
    const bool hadBeforeFeature = layer->getFeatures( QgsFeatureRequest().setFilterFid( entry.featureId ) ).nextFeature( beforeFeature );

    QgsGeometry original = QgsGeometry::fromWkt( entry.geometryWkt );
    layer->beginEditCommand( u"AI geometry rollback"_s );
    if ( !layer->changeGeometry( entry.featureId, original ) )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( u"Could not restore geometry for feature_id %1."_s.arg( entry.featureId ) );
    }
    layer->endEditCommand();

    if ( startedEditing && !layer->commitChanges() )
      return QgsAiToolResult::error( u"Could not commit geometry rollback: %1"_s.arg( layer->commitErrors().join( u"; "_s ) ) );

    QJsonObject diff;
    diff.insert( u"summary"_s, u"Restored previous feature geometry."_s );
    diff.insert( u"layer_id"_s, entry.layerId );
    diff.insert( u"feature_id"_s, static_cast<qint64>( entry.featureId ) );
    if ( hadBeforeFeature )
      diff.insert( u"before"_s, geometrySummary( beforeFeature.geometry() ) );
    diff.insert( u"after"_s, geometrySummary( original ) );

    QJsonObject output;
    output.insert( u"status"_s, u"rolled_back"_s );
    output.insert( u"rollback_token"_s, token );
    output.insert( u"diff"_s, diff );
    return QgsAiToolResult::ok( output );
  }

  QJsonObject attributesJson( const QgsFields &fields, const QgsAttributeMap &attributes )
  {
    QJsonObject output;
    for ( auto it = attributes.constBegin(); it != attributes.constEnd(); ++it )
    {
      const int index = it.key();
      const QString name = fields.exists( index ) ? fields.at( index ).name() : QString::number( index );
      output.insert( name, QJsonValue::fromVariant( it.value() ) );
    }
    return output;
  }

  QgsAiToolResult rollbackAttributes( QgsProject *project, const QString &token )
  {
    if ( !editingRollbackStore().contains( token ) )
      return QgsAiToolResult::error( u"Unknown or expired rollback token."_s );
    const EditingRollbackEntry entry = editingRollbackStore().take( token );
    if ( entry.type != EditingRollbackType::RestoreAttributes )
      return QgsAiToolResult::error( u"Rollback token does not belong to an attribute update operation."_s );
    if ( !project )
      return QgsAiToolResult::error( u"No active QgsProject available."_s );

    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( project->mapLayer( entry.layerId ) );
    if ( !layer )
      return QgsAiToolResult::error( u"Cannot rollback attributes: layer no longer exists: %1"_s.arg( entry.layerId ) );

    QgsFeature beforeFeature;
    if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( entry.featureId ) ).nextFeature( beforeFeature ) )
      return QgsAiToolResult::error( u"Cannot rollback attributes: feature no longer exists: %1"_s.arg( entry.featureId ) );

    QgsAttributeMap beforeAttributes;
    for ( auto it = entry.oldAttributes.constBegin(); it != entry.oldAttributes.constEnd(); ++it )
      beforeAttributes.insert( it.key(), beforeFeature.attribute( it.key() ) );

    const bool startedEditing = !layer->isEditable();
    if ( startedEditing && !layer->startEditing() )
      return QgsAiToolResult::error( u"Cannot start editing session for layer: %1"_s.arg( layer->name() ) );

    layer->beginEditCommand( u"AI attribute rollback"_s );
    if ( !layer->changeAttributeValues( entry.featureId, entry.oldAttributes, beforeAttributes, true ) )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( u"Could not restore attributes for feature_id %1."_s.arg( entry.featureId ) );
    }
    layer->endEditCommand();

    if ( startedEditing && !layer->commitChanges() )
      return QgsAiToolResult::error( u"Could not commit attribute rollback: %1"_s.arg( layer->commitErrors().join( u"; "_s ) ) );

    QJsonObject diff;
    diff.insert( u"summary"_s, u"Restored previous feature attributes."_s );
    diff.insert( u"layer_id"_s, entry.layerId );
    diff.insert( u"feature_id"_s, static_cast<qint64>( entry.featureId ) );
    diff.insert( u"before"_s, attributesJson( layer->fields(), beforeAttributes ) );
    diff.insert( u"after"_s, attributesJson( layer->fields(), entry.oldAttributes ) );

    QJsonObject output;
    output.insert( u"status"_s, u"rolled_back"_s );
    output.insert( u"rollback_token"_s, token );
    output.insert( u"diff"_s, diff );
    return QgsAiToolResult::ok( output );
  }
}

// ---------------------------------------------------------------------------
// edit_feature_geometry
// ---------------------------------------------------------------------------

QgsAiEditFeatureGeometryTool::QgsAiEditFeatureGeometryTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiEditFeatureGeometryTool::description() const
{
  return QStringLiteral(
    "Edits the geometry of an existing vector feature using native QGIS APIs. "
    "Supported operations are move_vertex, insert_vertex and delete_vertex. "
    "The tool validates the resulting geometry and returns a rollback token."
  );
}

QJsonObject QgsAiEditFeatureGeometryTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"layer_id"_s, prop( u"string"_s, u"Target vector layer id."_s ) );
  properties.insert( u"feature_id"_s, prop( u"integer"_s, u"Target feature id."_s ) );
  properties.insert( u"operation"_s, prop( u"string"_s, u"One of: move_vertex, insert_vertex, delete_vertex."_s ) );
  properties.insert( u"vertex_index"_s, prop( u"integer"_s, u"Vertex index for move_vertex/delete_vertex."_s ) );
  properties.insert( u"before_vertex"_s, prop( u"integer"_s, u"Insert before this vertex index for insert_vertex."_s ) );
  properties.insert( u"x"_s, prop( u"number"_s, u"New vertex x coordinate."_s ) );
  properties.insert( u"y"_s, prop( u"number"_s, u"New vertex y coordinate."_s ) );
  properties.insert( u"rollback_token"_s, prop( u"string"_s, u"Optional token returned by a previous edit_feature_geometry call. If set, restores the previous geometry."_s ) );
  return schemaObject( properties );
}

QgsAiToolResult QgsAiEditFeatureGeometryTool::execute( const QJsonObject &args )
{
  QgsProject *project = mProject ? mProject : QgsProject::instance();
  if ( !project )
    return QgsAiToolResult::error( u"No active QgsProject available."_s );

  const QString rollbackToken = args.value( u"rollback_token"_s ).toString().trimmed();
  if ( !rollbackToken.isEmpty() )
  {
    if ( editingRollbackStore().contains( rollbackToken ) && editingRollbackStore().value( rollbackToken ).type == EditingRollbackType::RestoreAttributes )
      return rollbackAttributes( project, rollbackToken );
    return rollbackGeometry( project, rollbackToken );
  }

  const QString layerId = args.value( u"layer_id"_s ).toString().trimmed();
  if ( layerId.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'layer_id' is required."_s );

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( project->mapLayer( layerId ) );
  if ( !layer )
    return QgsAiToolResult::error( u"No vector layer with id: %1"_s.arg( layerId ) );
  if ( !layer->isSpatial() )
    return QgsAiToolResult::error( u"Layer is not spatial: %1"_s.arg( layer->name() ) );

  const QgsFeatureId featureId = featureIdFromJson( args );
  QgsFeature feature;
  if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ) ).nextFeature( feature ) )
    return QgsAiToolResult::error( u"No feature with feature_id: %1"_s.arg( featureId ) );
  if ( !feature.hasGeometry() || feature.geometry().isNull() || feature.geometry().isEmpty() )
    return QgsAiToolResult::error( u"Feature has no editable geometry: %1"_s.arg( featureId ) );

  const QString operation = args.value( u"operation"_s ).toString().toLower().trimmed();
  if ( operation.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'operation' is required."_s );

  const QgsGeometry originalGeometry = feature.geometry();
  const QJsonObject before = geometrySummary( originalGeometry );
  const bool startedEditing = !layer->isEditable();
  if ( startedEditing && !layer->startEditing() )
    return QgsAiToolResult::error( u"Cannot start editing session for layer: %1"_s.arg( layer->name() ) );

  bool ok = false;
  QString appliedOperation;
  layer->beginEditCommand( u"AI geometry edit"_s );

  if ( operation == "move_vertex"_L1 )
  {
    const int vertexIndex = args.value( u"vertex_index"_s ).toInt( -1 );
    if ( vertexIndex < 0 || !args.contains( u"x"_s ) || !args.contains( u"y"_s ) )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( u"move_vertex requires vertex_index, x and y."_s );
    }
    ok = layer->moveVertex( args.value( u"x"_s ).toDouble(), args.value( u"y"_s ).toDouble(), featureId, vertexIndex );
    appliedOperation = u"move_vertex"_s;
  }
  else if ( operation == "insert_vertex"_L1 )
  {
    const int beforeVertex = args.value( u"before_vertex"_s ).toInt( -1 );
    if ( beforeVertex < 0 || !args.contains( u"x"_s ) || !args.contains( u"y"_s ) )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( u"insert_vertex requires before_vertex, x and y."_s );
    }
    ok = layer->insertVertex( args.value( u"x"_s ).toDouble(), args.value( u"y"_s ).toDouble(), featureId, beforeVertex );
    appliedOperation = u"insert_vertex"_s;
  }
  else if ( operation == "delete_vertex"_L1 )
  {
    const int vertexIndex = args.value( u"vertex_index"_s ).toInt( -1 );
    if ( vertexIndex < 0 )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( u"delete_vertex requires vertex_index."_s );
    }
    ok = layer->deleteVertex( featureId, vertexIndex ) == Qgis::VectorEditResult::Success;
    appliedOperation = u"delete_vertex"_s;
  }
  else if ( operation == "split_feature"_L1 )
  {
    layer->destroyEditCommand();
    if ( startedEditing )
      layer->rollBack();
    return QgsAiToolResult::error( u"split_feature is not supported yet by edit_feature_geometry."_s );
  }
  else
  {
    layer->destroyEditCommand();
    if ( startedEditing )
      layer->rollBack();
    return QgsAiToolResult::error( u"Unknown operation: %1."_s.arg( operation ) );
  }

  if ( !ok )
  {
    layer->destroyEditCommand();
    if ( startedEditing )
      layer->rollBack();
    return QgsAiToolResult::error( u"Geometry edit failed for operation: %1."_s.arg( operation ) );
  }

  QgsFeature editedFeature;
  if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ) ).nextFeature( editedFeature ) )
  {
    layer->destroyEditCommand();
    if ( startedEditing )
      layer->rollBack();
    return QgsAiToolResult::error( u"Could not fetch edited feature for validation: %1"_s.arg( featureId ) );
  }

  const QgsGeometry editedGeometry = editedFeature.geometry();
  QVector<QgsGeometry::Error> validationErrors;
  editedGeometry.validateGeometry( validationErrors, Qgis::GeometryValidationEngine::Geos );
  if ( editedGeometry.isNull() || editedGeometry.isEmpty() || !editedGeometry.isGeosValid() || !validationErrors.isEmpty() )
  {
    layer->destroyEditCommand();
    if ( startedEditing )
      layer->rollBack();
    return QgsAiToolResult::error( u"Geometry edit would create an invalid geometry; no changes were committed."_s );
  }

  layer->endEditCommand();
  if ( startedEditing && !layer->commitChanges() )
    return QgsAiToolResult::error( u"Could not commit geometry edit: %1"_s.arg( layer->commitErrors().join( u"; "_s ) ) );

  EditingRollbackEntry rollback;
  rollback.type = EditingRollbackType::RestoreGeometry;
  rollback.layerId = layerId;
  rollback.featureId = featureId;
  rollback.geometryWkt = originalGeometry.asWkt();
  rollback.description = u"Restore feature geometry before AI edit."_s;
  const QString token = storeEditingRollback( rollback );

  QJsonObject diff;
  diff.insert( u"summary"_s, u"Edited feature geometry."_s );
  diff.insert( u"operation"_s, appliedOperation );
  diff.insert( u"layer_id"_s, layerId );
  diff.insert( u"feature_id"_s, static_cast<qint64>( featureId ) );
  diff.insert( u"before"_s, before );
  diff.insert( u"after"_s, geometrySummary( editedGeometry ) );

  QJsonObject output;
  output.insert( u"layer_id"_s, layerId );
  output.insert( u"feature_id"_s, static_cast<qint64>( featureId ) );
  output.insert( u"operation"_s, appliedOperation );
  output.insert( u"diff"_s, diff );
  output.insert( u"rollback_token"_s, token );
  output.insert( u"rollback"_s, rollbackJson( token, u"restore_feature_geometry"_s ) );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// update_feature_attributes
// ---------------------------------------------------------------------------

QgsAiUpdateFeatureAttributesTool::QgsAiUpdateFeatureAttributesTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiUpdateFeatureAttributesTool::description() const
{
  return QStringLiteral(
    "Updates one or more attributes of an existing vector feature. "
    "Values are converted through QgsField::convertCompatible before editing, "
    "and the previous values are returned as a rollback token."
  );
}

QJsonObject QgsAiUpdateFeatureAttributesTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"layer_id"_s, prop( u"string"_s, u"Target vector layer id."_s ) );
  properties.insert( u"feature_id"_s, prop( u"integer"_s, u"Target feature id."_s ) );
  properties.insert( u"attributes"_s, prop( u"object"_s, u"Object mapping field names to new values."_s ) );
  properties.insert( u"rollback_token"_s, prop( u"string"_s, u"Optional token returned by a previous update_feature_attributes call. If set, restores the previous values."_s ) );
  return schemaObject( properties );
}

QgsAiToolResult QgsAiUpdateFeatureAttributesTool::execute( const QJsonObject &args )
{
  QgsProject *project = mProject ? mProject : QgsProject::instance();
  if ( !project )
    return QgsAiToolResult::error( u"No active QgsProject available."_s );

  const QString rollbackToken = args.value( u"rollback_token"_s ).toString().trimmed();
  if ( !rollbackToken.isEmpty() )
    return rollbackAttributes( project, rollbackToken );

  const QString layerId = args.value( u"layer_id"_s ).toString().trimmed();
  if ( layerId.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'layer_id' is required."_s );

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( project->mapLayer( layerId ) );
  if ( !layer )
    return QgsAiToolResult::error( u"No vector layer with id: %1"_s.arg( layerId ) );

  const QgsFeatureId featureId = featureIdFromJson( args );
  QgsFeature feature;
  if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ) ).nextFeature( feature ) )
    return QgsAiToolResult::error( u"No feature with feature_id: %1"_s.arg( featureId ) );

  if ( !args.value( u"attributes"_s ).isObject() )
    return QgsAiToolResult::error( u"Argument 'attributes' must be an object."_s );

  const QJsonObject requestedAttributes = args.value( u"attributes"_s ).toObject();
  if ( requestedAttributes.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'attributes' must include at least one field."_s );

  QgsAttributeMap newValues;
  QgsAttributeMap oldValues;
  const QgsFields fields = layer->fields();
  for ( auto it = requestedAttributes.constBegin(); it != requestedAttributes.constEnd(); ++it )
  {
    const int index = fields.lookupField( it.key() );
    if ( index < 0 )
      return QgsAiToolResult::error( u"No field named '%1' on layer: %2"_s.arg( it.key(), layer->name() ) );

    QVariant value = it.value().toVariant();
    QString conversionError;
    if ( !fields.at( index ).convertCompatible( value, &conversionError ) )
    {
      return QgsAiToolResult::error(
        conversionError.isEmpty()
          ? u"Value for field '%1' is not compatible with field type %2."_s.arg( it.key(), fields.at( index ).typeName() )
          : u"Value for field '%1' is not compatible: %2"_s.arg( it.key(), conversionError )
      );
    }

    newValues.insert( index, value );
    oldValues.insert( index, feature.attribute( index ) );
  }

  const bool startedEditing = !layer->isEditable();
  if ( startedEditing && !layer->startEditing() )
    return QgsAiToolResult::error( u"Cannot start editing session for layer: %1"_s.arg( layer->name() ) );

  layer->beginEditCommand( u"AI attribute update"_s );
  if ( !layer->changeAttributeValues( featureId, newValues, oldValues, true ) )
  {
    layer->destroyEditCommand();
    if ( startedEditing )
      layer->rollBack();
    return QgsAiToolResult::error( u"Could not update attributes for feature_id %1."_s.arg( featureId ) );
  }
  layer->endEditCommand();

  if ( startedEditing && !layer->commitChanges() )
    return QgsAiToolResult::error( u"Could not commit attribute update: %1"_s.arg( layer->commitErrors().join( u"; "_s ) ) );

  EditingRollbackEntry rollback;
  rollback.type = EditingRollbackType::RestoreAttributes;
  rollback.layerId = layerId;
  rollback.featureId = featureId;
  rollback.oldAttributes = oldValues;
  rollback.description = u"Restore feature attributes before AI update."_s;
  const QString token = storeEditingRollback( rollback );

  QJsonObject diff;
  diff.insert( u"summary"_s, u"Updated feature attributes."_s );
  diff.insert( u"layer_id"_s, layerId );
  diff.insert( u"feature_id"_s, static_cast<qint64>( featureId ) );
  diff.insert( u"before"_s, attributesJson( fields, oldValues ) );
  diff.insert( u"after"_s, attributesJson( fields, newValues ) );

  QJsonObject output;
  output.insert( u"layer_id"_s, layerId );
  output.insert( u"feature_id"_s, static_cast<qint64>( featureId ) );
  output.insert( u"updated_attributes"_s, attributesJson( fields, newValues ) );
  output.insert( u"diff"_s, diff );
  output.insert( u"rollback_token"_s, token );
  output.insert( u"rollback"_s, rollbackJson( token, u"restore_feature_attributes"_s ) );
  return QgsAiToolResult::ok( output );
}
