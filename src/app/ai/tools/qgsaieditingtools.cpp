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
#include "qgsexpression.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgsfield.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgspoint.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QUuid>
#include <QVariant>

using namespace Qt::StringLiterals;

namespace
{
  enum class EditingRollbackType
  {
    RestoreGeometry,
    RestoreAttributes,
    RestoreFieldCalculation
  };

  struct EditingRollbackEntry
  {
      EditingRollbackType type = EditingRollbackType::RestoreGeometry;
      QString layerId;
      QgsFeatureId featureId = FID_NULL;
      QString geometryWkt;
      QgsFeatureIds addedFeatureIds;
      QStringList addedGeometryWkts;
      QgsAttributeMap oldAttributes;
      QHash<QgsFeatureId, QVariant> oldFieldValues;
      bool createdField = false;
      QString fieldName;
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

  QJsonObject editingRollbackJson( const QString &token, const QString &action )
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

  bool pointSequenceFromJson( const QJsonValue &value, QgsPointSequence &points, QString &error )
  {
    if ( !value.isArray() )
    {
      error = u"split_feature requires split_line as an array of at least two points."_s;
      return false;
    }

    const QJsonArray array = value.toArray();
    if ( array.size() < 2 )
    {
      error = u"split_line must contain at least two points."_s;
      return false;
    }

    for ( const QJsonValue &pointValue : array )
    {
      double x = 0;
      double y = 0;
      if ( pointValue.isObject() )
      {
        const QJsonObject pointObject = pointValue.toObject();
        if ( !pointObject.value( u"x"_s ).isDouble() || !pointObject.value( u"y"_s ).isDouble() )
        {
          error = u"Each split_line point object must include numeric x and y."_s;
          return false;
        }
        x = pointObject.value( u"x"_s ).toDouble();
        y = pointObject.value( u"y"_s ).toDouble();
      }
      else if ( pointValue.isArray() )
      {
        const QJsonArray pointArray = pointValue.toArray();
        if ( pointArray.size() < 2 || !pointArray.at( 0 ).isDouble() || !pointArray.at( 1 ).isDouble() )
        {
          error = u"Each split_line point array must be [x, y]."_s;
          return false;
        }
        x = pointArray.at( 0 ).toDouble();
        y = pointArray.at( 1 ).toDouble();
      }
      else
      {
        error = u"Each split_line point must be an object {x,y} or array [x,y]."_s;
        return false;
      }
      points << QgsPoint( x, y );
    }

    return true;
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
    QgsFeatureIds featureIdsToDelete;
    for ( const QgsFeatureId id : entry.addedFeatureIds )
    {
      if ( !FID_IS_NEW( id ) )
        featureIdsToDelete.insert( id );
    }
    if ( !entry.addedGeometryWkts.isEmpty() )
    {
      QgsFeature addedFeature;
      QgsFeatureIterator it = layer->getFeatures();
      while ( it.nextFeature( addedFeature ) )
      {
        if ( entry.addedGeometryWkts.contains( addedFeature.geometry().asWkt() ) )
          featureIdsToDelete.insert( addedFeature.id() );
      }
    }
    if ( !featureIdsToDelete.isEmpty() && !layer->deleteFeatures( featureIdsToDelete ) )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( u"Could not remove features added by split rollback."_s );
    }
    if ( !layer->changeGeometry( entry.featureId, original ) )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( u"Could not restore geometry for feature_id %1."_s.arg( entry.featureId ) );
    }
    layer->endEditCommand();

    if ( startedEditing && !layer->commitChanges() )
      return QgsAiToolResult::error( u"Could not commit geometry rollback: %1"_s.arg( layer->commitErrors().join( "; "_L1 ) ) );

    QJsonObject diff;
    diff.insert( u"summary"_s, u"Restored previous feature geometry."_s );
    diff.insert( u"layer_id"_s, entry.layerId );
    diff.insert( u"feature_id"_s, static_cast<qint64>( entry.featureId ) );
    diff.insert( u"removed_added_features"_s, featureIdsToDelete.size() );
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
      return QgsAiToolResult::error( u"Could not commit attribute rollback: %1"_s.arg( layer->commitErrors().join( "; "_L1 ) ) );

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

  QgsAiToolResult rollbackFieldCalculation( QgsProject *project, const QString &token )
  {
    if ( !editingRollbackStore().contains( token ) )
      return QgsAiToolResult::error( u"Unknown or expired rollback token."_s );
    const EditingRollbackEntry entry = editingRollbackStore().take( token );
    if ( entry.type != EditingRollbackType::RestoreFieldCalculation )
      return QgsAiToolResult::error( u"Rollback token does not belong to a field calculation operation."_s );
    if ( !project )
      return QgsAiToolResult::error( u"No active QgsProject available."_s );

    QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( project->mapLayer( entry.layerId ) );
    if ( !layer )
      return QgsAiToolResult::error( u"Cannot rollback field calculation: layer no longer exists: %1"_s.arg( entry.layerId ) );

    const bool startedEditing = !layer->isEditable();
    if ( startedEditing && !layer->startEditing() )
      return QgsAiToolResult::error( u"Cannot start editing session for layer: %1"_s.arg( layer->name() ) );

    layer->beginEditCommand( u"AI field calculation rollback"_s );
    if ( entry.createdField )
    {
      const int fieldIndex = layer->fields().lookupField( entry.fieldName );
      if ( fieldIndex >= 0 && !layer->deleteAttribute( fieldIndex ) )
      {
        layer->destroyEditCommand();
        if ( startedEditing )
          layer->rollBack();
        return QgsAiToolResult::error( u"Could not delete calculated field: %1"_s.arg( entry.fieldName ) );
      }
    }
    else
    {
      const int fieldIndex = layer->fields().lookupField( entry.fieldName );
      if ( fieldIndex < 0 )
      {
        layer->destroyEditCommand();
        if ( startedEditing )
          layer->rollBack();
        return QgsAiToolResult::error( u"Cannot rollback field calculation: field no longer exists: %1"_s.arg( entry.fieldName ) );
      }

      for ( auto it = entry.oldFieldValues.constBegin(); it != entry.oldFieldValues.constEnd(); ++it )
      {
        if ( !layer->changeAttributeValue( it.key(), fieldIndex, it.value(), QVariant(), true ) )
        {
          layer->destroyEditCommand();
          if ( startedEditing )
            layer->rollBack();
          return QgsAiToolResult::error( u"Could not restore calculated value for feature_id %1."_s.arg( it.key() ) );
        }
      }
    }
    layer->endEditCommand();

    if ( startedEditing && !layer->commitChanges() )
      return QgsAiToolResult::error( u"Could not commit field calculation rollback: %1"_s.arg( layer->commitErrors().join( "; "_L1 ) ) );

    QJsonObject diff;
    diff.insert( u"summary"_s, entry.createdField ? u"Removed field created by field calculation."_s : u"Restored previous field values."_s );
    diff.insert( u"layer_id"_s, entry.layerId );
    diff.insert( u"field_name"_s, entry.fieldName );
    diff.insert( u"restored_feature_count"_s, entry.oldFieldValues.size() );

    QJsonObject output;
    output.insert( u"status"_s, u"rolled_back"_s );
    output.insert( u"rollback_token"_s, token );
    output.insert( u"diff"_s, diff );
    return QgsAiToolResult::ok( output );
  }

  QMetaType::Type fieldTypeFromName( const QString &name )
  {
    const QString normalized = name.toLower().trimmed();
    if ( normalized == "int"_L1 || normalized == "integer"_L1 )
      return QMetaType::Type::Int;
    if ( normalized == "longlong"_L1 || normalized == "long"_L1 || normalized == "integer64"_L1 )
      return QMetaType::Type::LongLong;
    if ( normalized == "string"_L1 || normalized == "text"_L1 )
      return QMetaType::Type::QString;
    if ( normalized == "bool"_L1 || normalized == "boolean"_L1 )
      return QMetaType::Type::Bool;
    return QMetaType::Type::Double;
  }
} //namespace

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
    "Supported operations are move_vertex, insert_vertex, delete_vertex and split_feature. "
    "The tool validates the resulting geometry and returns a rollback token."
  );
}

QJsonObject QgsAiEditFeatureGeometryTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"layer_id"_s, prop( u"string"_s, u"Target vector layer id."_s ) );
  properties.insert( u"feature_id"_s, prop( u"integer"_s, u"Target feature id."_s ) );
  properties.insert( u"operation"_s, prop( u"string"_s, u"One of: move_vertex, insert_vertex, delete_vertex, split_feature."_s ) );
  properties.insert( u"vertex_index"_s, prop( u"integer"_s, u"Vertex index for move_vertex/delete_vertex."_s ) );
  properties.insert( u"before_vertex"_s, prop( u"integer"_s, u"Insert before this vertex index for insert_vertex."_s ) );
  properties.insert( u"x"_s, prop( u"number"_s, u"New vertex x coordinate."_s ) );
  properties.insert( u"y"_s, prop( u"number"_s, u"New vertex y coordinate."_s ) );
  {
    QJsonObject pointSchema;
    pointSchema.insert( u"type"_s, u"object"_s );
    QJsonObject pointProps;
    pointProps.insert( u"x"_s, prop( u"number"_s, u"x coordinate."_s ) );
    pointProps.insert( u"y"_s, prop( u"number"_s, u"y coordinate."_s ) );
    pointSchema.insert( u"properties"_s, pointProps );
    properties.insert( u"split_line"_s, propArray( pointSchema, u"Line for split_feature as [{x,y}, ...] or [[x,y], ...]."_s ) );
  }
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
  QgsFeatureIds addedFeatureIds;
  QStringList addedGeometryWkts;
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
    QgsPointSequence splitLine;
    QString error;
    if ( !pointSequenceFromJson( args.value( u"split_line"_s ), splitLine, error ) )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( error );
    }

    QgsGeometry splitGeometry = originalGeometry;
    QVector<QgsGeometry> newGeometries;
    QgsPointSequence topologyTestPoints;
    const Qgis::GeometryOperationResult splitResult = splitGeometry.splitGeometry( splitLine, newGeometries, false, topologyTestPoints, true );
    if ( splitResult != Qgis::GeometryOperationResult::Success || newGeometries.isEmpty() )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( u"split_feature failed or did not create a new geometry."_s );
    }

    ok = layer->changeGeometry( featureId, splitGeometry );
    if ( ok )
    {
      QgsFeatureList newFeatures;
      for ( const QgsGeometry &newGeometry : newGeometries )
      {
        QgsFeature newFeature( layer->fields() );
        newFeature.setAttributes( feature.attributes() );
        newFeature.setGeometry( newGeometry );
        newFeatures << newFeature;
        addedGeometryWkts << newGeometry.asWkt();
      }
      ok = layer->addFeatures( newFeatures );
      for ( const QgsFeature &newFeature : newFeatures )
      {
        if ( newFeature.id() != FID_NULL )
          addedFeatureIds.insert( newFeature.id() );
      }
    }
    appliedOperation = u"split_feature"_s;
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
    return QgsAiToolResult::error( u"Could not commit geometry edit: %1"_s.arg( layer->commitErrors().join( "; "_L1 ) ) );

  EditingRollbackEntry rollback;
  rollback.type = EditingRollbackType::RestoreGeometry;
  rollback.layerId = layerId;
  rollback.featureId = featureId;
  rollback.geometryWkt = originalGeometry.asWkt();
  rollback.addedFeatureIds = addedFeatureIds;
  rollback.addedGeometryWkts = addedGeometryWkts;
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
  output.insert( u"rollback"_s, editingRollbackJson( token, u"restore_feature_geometry"_s ) );
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
        conversionError.isEmpty() ? u"Value for field '%1' is not compatible with field type %2."_s.arg( it.key(), fields.at( index ).typeName() )
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
    return QgsAiToolResult::error( u"Could not commit attribute update: %1"_s.arg( layer->commitErrors().join( "; "_L1 ) ) );

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
  output.insert( u"rollback"_s, editingRollbackJson( token, u"restore_feature_attributes"_s ) );
  return QgsAiToolResult::ok( output );
}

// ---------------------------------------------------------------------------
// calculate_field
// ---------------------------------------------------------------------------

QgsAiCalculateFieldTool::QgsAiCalculateFieldTool( QgsProject *project )
  : mProject( project )
{}

QString QgsAiCalculateFieldTool::description() const
{
  return QStringLiteral(
    "Evaluates a QGIS expression into a new or existing field for all features, "
    "or for a subset selected by filter_expression. Invalid expressions fail before editing."
  );
}

QJsonObject QgsAiCalculateFieldTool::schema() const
{
  QJsonObject properties;
  properties.insert( u"layer_id"_s, prop( u"string"_s, u"Target vector layer id."_s ) );
  properties.insert( u"field_name"_s, prop( u"string"_s, u"Target field name."_s ) );
  properties.insert( u"expression"_s, prop( u"string"_s, u"QGIS expression to evaluate for each feature."_s ) );
  properties.insert( u"create_field"_s, prop( u"boolean"_s, u"If true, creates field_name before writing values."_s ) );
  properties.insert( u"field_type"_s, prop( u"string"_s, u"Type for new fields: double, integer, longlong, string or boolean. Defaults to double."_s ) );
  properties.insert( u"filter_expression"_s, prop( u"string"_s, u"Optional QGIS expression limiting which features are updated."_s ) );
  properties.insert( u"rollback_token"_s, prop( u"string"_s, u"Optional token returned by a previous calculate_field call."_s ) );
  return schemaObject( properties );
}

QgsAiToolResult QgsAiCalculateFieldTool::execute( const QJsonObject &args )
{
  QgsProject *project = mProject ? mProject : QgsProject::instance();
  if ( !project )
    return QgsAiToolResult::error( u"No active QgsProject available."_s );

  const QString rollbackToken = args.value( u"rollback_token"_s ).toString().trimmed();
  if ( !rollbackToken.isEmpty() )
    return rollbackFieldCalculation( project, rollbackToken );

  const QString layerId = args.value( u"layer_id"_s ).toString().trimmed();
  if ( layerId.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'layer_id' is required."_s );

  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( project->mapLayer( layerId ) );
  if ( !layer )
    return QgsAiToolResult::error( u"No vector layer with id: %1"_s.arg( layerId ) );

  const QString fieldName = args.value( u"field_name"_s ).toString().trimmed();
  if ( fieldName.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'field_name' is required."_s );

  const QString expressionText = args.value( u"expression"_s ).toString().trimmed();
  if ( expressionText.isEmpty() )
    return QgsAiToolResult::error( u"Argument 'expression' is required."_s );

  const bool createField = args.value( u"create_field"_s ).toBool( false );
  const int existingFieldIndex = layer->fields().lookupField( fieldName );
  if ( createField && existingFieldIndex >= 0 )
    return QgsAiToolResult::error( u"Field already exists: %1"_s.arg( fieldName ) );
  if ( !createField && existingFieldIndex < 0 )
    return QgsAiToolResult::error( u"No field named '%1' on layer: %2"_s.arg( fieldName, layer->name() ) );

  QgsField targetField = createField ? QgsField( fieldName, fieldTypeFromName( args.value( u"field_type"_s ).toString( u"double"_s ) ) ) : layer->fields().at( existingFieldIndex );

  QgsExpression expression( expressionText );
  if ( expression.hasParserError() )
    return QgsAiToolResult::error( u"Expression parser error: %1"_s.arg( expression.parserErrorString() ) );

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  context.setFields( layer->fields() );
  expression.prepare( &context );

  QgsFeatureRequest request;
  const QString filterExpression = args.value( u"filter_expression"_s ).toString().trimmed();
  if ( !filterExpression.isEmpty() )
  {
    QgsExpression filter( filterExpression );
    if ( filter.hasParserError() )
      return QgsAiToolResult::error( u"Filter expression parser error: %1"_s.arg( filter.parserErrorString() ) );
    request.setFilterExpression( filterExpression );
    request.setExpressionContext( context );
  }

  struct PendingValue
  {
      QgsFeatureId featureId = FID_NULL;
      QVariant oldValue;
      QVariant newValue;
  };
  QList<PendingValue> pendingValues;

  QgsFeatureIterator it = layer->getFeatures( request );
  QgsFeature feature;
  while ( it.nextFeature( feature ) )
  {
    context.setFeature( feature );
    QVariant value = expression.evaluate( &context );
    if ( expression.hasEvalError() )
      return QgsAiToolResult::error( u"Expression evaluation error: %1"_s.arg( expression.evalErrorString() ) );

    QString conversionError;
    if ( !targetField.convertCompatible( value, &conversionError ) )
    {
      return QgsAiToolResult::error(
        conversionError.isEmpty() ? u"Expression result is not compatible with field '%1'."_s.arg( fieldName )
                                  : u"Expression result is not compatible with field '%1': %2"_s.arg( fieldName, conversionError )
      );
    }

    PendingValue pending;
    pending.featureId = feature.id();
    pending.oldValue = existingFieldIndex >= 0 ? feature.attribute( existingFieldIndex ) : QVariant();
    pending.newValue = value;
    pendingValues.push_back( pending );
  }

  const bool startedEditing = !layer->isEditable();
  if ( startedEditing && !layer->startEditing() )
    return QgsAiToolResult::error( u"Cannot start editing session for layer: %1"_s.arg( layer->name() ) );

  layer->beginEditCommand( u"AI field calculation"_s );

  int targetFieldIndex = existingFieldIndex;
  if ( createField )
  {
    if ( !layer->addAttribute( targetField ) )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( u"Could not create field: %1"_s.arg( fieldName ) );
    }
    layer->updateFields();
    targetFieldIndex = layer->fields().lookupField( fieldName );
    if ( targetFieldIndex < 0 )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( u"Created field was not found: %1"_s.arg( fieldName ) );
    }
  }

  QHash<QgsFeatureId, QVariant> oldFieldValues;
  for ( const PendingValue &pending : std::as_const( pendingValues ) )
  {
    if ( !layer->changeAttributeValue( pending.featureId, targetFieldIndex, pending.newValue, pending.oldValue, true ) )
    {
      layer->destroyEditCommand();
      if ( startedEditing )
        layer->rollBack();
      return QgsAiToolResult::error( u"Could not write calculated value for feature_id %1."_s.arg( pending.featureId ) );
    }
    oldFieldValues.insert( pending.featureId, pending.oldValue );
  }
  layer->endEditCommand();

  if ( startedEditing && !layer->commitChanges() )
    return QgsAiToolResult::error( u"Could not commit field calculation: %1"_s.arg( layer->commitErrors().join( "; "_L1 ) ) );

  EditingRollbackEntry rollback;
  rollback.type = EditingRollbackType::RestoreFieldCalculation;
  rollback.layerId = layerId;
  rollback.fieldName = fieldName;
  rollback.createdField = createField;
  rollback.oldFieldValues = oldFieldValues;
  rollback.description = createField ? u"Remove field created by AI calculation."_s : u"Restore field values before AI calculation."_s;
  const QString token = storeEditingRollback( rollback );

  QJsonObject diff;
  diff.insert( u"summary"_s, u"Calculated field values from QGIS expression."_s );
  diff.insert( u"layer_id"_s, layerId );
  diff.insert( u"field_name"_s, fieldName );
  diff.insert( u"expression"_s, expressionText );
  diff.insert( u"updated_feature_count"_s, pendingValues.size() );
  diff.insert( u"created_field"_s, createField );

  QJsonObject output;
  output.insert( u"layer_id"_s, layerId );
  output.insert( u"field_name"_s, fieldName );
  output.insert( u"updated_feature_count"_s, pendingValues.size() );
  output.insert( u"created_field"_s, createField );
  output.insert( u"diff"_s, diff );
  output.insert( u"rollback_token"_s, token );
  output.insert( u"rollback"_s, editingRollbackJson( token, createField ? u"remove_calculated_field"_s : u"restore_calculated_field_values"_s ) );
  return QgsAiToolResult::ok( output );
}
