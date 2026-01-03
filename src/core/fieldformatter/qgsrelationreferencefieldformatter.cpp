/***************************************************************************
  qgsrelationreferencefieldformatter.cpp - QgsRelationReferenceFieldFormatter

 ---------------------
 begin                : 3.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrelationreferencefieldformatter.h"

#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgsrelation.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"

QgsRelationReferenceFieldFormatter::QgsRelationReferenceFieldFormatter()
{
  setFlags( flags() | QgsFieldFormatter::CanProvideAvailableValues );
}

QString QgsRelationReferenceFieldFormatter::id() const
{
  return u"RelationReference"_s;
}

QString QgsRelationReferenceFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  if ( cache.isValid() )
  {
    return cache.value<QMap<QVariant, QString>>().value( value );
  }

  const QString fieldName = fieldIndex < layer->fields().size() ? layer->fields().at( fieldIndex ).name() : QObject::tr( "<unknown>" );

  // Some sanity checks
  if ( !config.contains( u"Relation"_s ) )
  {
    QgsMessageLog::logMessage( QObject::tr( "Layer %1, field %2: Missing Relation in configuration" ).arg( layer->name(), fieldName ) );
    return value.toString();
  }

  const QString relationName = config[u"Relation"_s].toString();
  const QgsRelation relation = QgsProject::instance()->relationManager()->relation( relationName ); // skip-keyword-check
  if ( !relation.isValid() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Layer %1, field %2: Invalid relation %3" ).arg( layer->name(), fieldName, relationName ) );
    return value.toString();
  }
  QgsVectorLayer *referencingLayer = relation.referencingLayer();
  if ( layer != referencingLayer )
  {
    QgsMessageLog::logMessage( QObject::tr( "Layer %1, field %2: representValue() with inconsistent layer parameter w.r.t relation referencingLayer" ).arg( layer->name(), fieldName ) );
    return value.toString();
  }
  const int referencingFieldIdx = referencingLayer->fields().lookupField( relation.fieldPairs().at( 0 ).first );
  if ( referencingFieldIdx != fieldIndex )
  {
    QgsMessageLog::logMessage( QObject::tr( "Layer %1, field %2: representValue() with inconsistent fieldIndex parameter w.r.t relation referencingFieldIdx" ).arg( layer->name(), fieldName ) );
    return value.toString();
  }
  QgsVectorLayer *referencedLayer = relation.referencedLayer();
  if ( !referencedLayer )
  {
    QgsMessageLog::logMessage( QObject::tr( "Layer %1, field %2: Cannot find referenced layer" ).arg( layer->name(), fieldName ) );
    return value.toString();
  }

  // Attributes from the referencing layer
  QgsAttributes attrs = QgsAttributes( layer->fields().count() );
  // Set the value on the foreign key field of the referencing record
  attrs[ referencingFieldIdx ] = value;

  const QgsFeatureRequest request = relation.getReferencedFeatureRequest( attrs );
  QgsFeature feature;
  referencedLayer->getFeatures( request ).nextFeature( feature );
  if ( !feature.isValid() )
    return value.toString();

  QgsExpression expr( referencedLayer->displayExpression() );
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( referencedLayer ) );
  context.setFeature( feature );
  QString title = expr.evaluate( &context ).toString();
  if ( expr.hasEvalError() )
  {
    const int referencedFieldIdx = referencedLayer->fields().lookupField( relation.fieldPairs().at( 0 ).second );
    title = feature.attribute( referencedFieldIdx ).toString();
  }
  return title;
}

QVariant QgsRelationReferenceFieldFormatter::sortValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  return representValue( layer, fieldIndex, config, cache, value );
}

QVariant QgsRelationReferenceFieldFormatter::createCache( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config ) const
{
  Q_UNUSED( fieldIndex )
  QMap<QVariant, QString> cache;

  const QString fieldName = fieldIndex < layer->fields().size() ? layer->fields().at( fieldIndex ).name() : QObject::tr( "<unknown>" );

  // Some sanity checks
  if ( !config.contains( u"Relation"_s ) )
  {
    QgsMessageLog::logMessage( QObject::tr( "Layer %1, field %2: Missing Relation in configuration" ).arg( layer->name(), fieldName ) );
    return QVariant();
  }
  const QString relationName = config[u"Relation"_s].toString();
  const QgsRelation relation = QgsProject::instance()->relationManager()->relation( config[u"Relation"_s].toString() ); // skip-keyword-check
  if ( !relation.isValid() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Layer %1, field %2: Invalid relation %3" ).arg( layer->name(), fieldName, relationName ) );
    return QVariant();
  }
  QgsVectorLayer *referencingLayer = relation.referencingLayer();
  if ( layer != referencingLayer )
  {
    QgsMessageLog::logMessage( QObject::tr( "Layer %1, field %2: representValue() with inconsistent layer parameter w.r.t relation referencingLayer" ).arg( layer->name(), fieldName ) );
    return QVariant();
  }
  QgsVectorLayer *referencedLayer = relation.referencedLayer();
  if ( !referencedLayer )
  {
    QgsMessageLog::logMessage( QObject::tr( "Layer %1, field %2: Cannot find referenced layer" ).arg( layer->name(), fieldName ) );
    return QVariant();
  }

  const int referencedFieldIdx = referencedLayer->fields().lookupField( relation.fieldPairs().at( 0 ).second );
  if ( referencedFieldIdx == -1 )
  {
    QgsMessageLog::logMessage( QObject::tr( "Layer %1, field %2: Invalid referenced field (%3) configured in relation %4" ).arg( layer->name(), fieldName, relation.fieldPairs().at( 0 ).second, relation.name() ) );
    return QVariant();
  }

  QgsExpression expr( referencedLayer->displayExpression() );

  QgsFeatureRequest request;
  request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  QgsAttributeList requiredAttributes = qgis::setToList( expr.referencedAttributeIndexes( referencedLayer->fields() ) );
  requiredAttributes << referencedFieldIdx;
  request.setSubsetOfAttributes( requiredAttributes );
  QgsFeature feature;
  auto iterator = referencedLayer->getFeatures( request );

  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( referencedLayer ) );

  expr.prepare( &context );

  while ( iterator.nextFeature( feature ) )
  {
    context.setFeature( feature );
    QString title = expr.evaluate( &context ).toString();

    if ( expr.hasEvalError() )
    {
      title = feature.attribute( referencedFieldIdx ).toString();
    }

    cache.insert( feature.attribute( referencedFieldIdx ), title );
  }

  return QVariant::fromValue<QMap<QVariant, QString>>( cache );
}


QList<QgsVectorLayerRef> QgsRelationReferenceFieldFormatter::layerDependencies( const QVariantMap &config ) const
{
  // Old projects, create before the weak relations were introduced and stored with the
  // widget configuration do not have the referenced layer details but only the "Relation" id,
  // for these projects automatic loading of broken references is not supported.
  if ( config.value( u"ReferencedLayerId"_s ).toString().isEmpty() )
  {
    return {};
  }

  const QList<QgsVectorLayerRef> result {{
      QgsVectorLayerRef(
        config.value( u"ReferencedLayerId"_s ).toString(),
        config.value( u"ReferencedLayerName"_s ).toString(),
        config.value( u"ReferencedLayerDataSource"_s ).toString(),
        config.value( u"ReferencedLayerProviderKey"_s ).toString() )
    }};
  return result;
}

QVariantList QgsRelationReferenceFieldFormatter::availableValues( const QVariantMap &config, int countLimit, const QgsFieldFormatterContext &context ) const
{
  QVariantList values;
  if ( auto *lProject = context.project() )
  {
    const QgsVectorLayer *referencedLayer = lProject->relationManager()->relation( config[u"Relation"_s].toString() ).referencedLayer();
    if ( referencedLayer )
    {
      const int fieldIndex =  lProject->relationManager()->relation( config[u"Relation"_s].toString() ).referencedFields().first();
      values = qgis::setToList( referencedLayer->uniqueValues( fieldIndex, countLimit ) );
    }
  }
  return values;
}
