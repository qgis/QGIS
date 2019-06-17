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

#include "qgsmessagelog.h"
#include "qgsrelation.h"
#include "qgsexpressioncontext.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"

QString QgsRelationReferenceFieldFormatter::id() const
{
  return QStringLiteral( "RelationReference" );
}

QString QgsRelationReferenceFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  if ( cache.isValid() )
  {
    return cache.value<QMap<QVariant, QString>>().value( value );
  }

  // Some sanity checks
  if ( !config.contains( QStringLiteral( "Relation" ) ) )
  {
    QgsMessageLog::logMessage( QObject::tr( "Missing Relation in configuration" ) );
    return value.toString();
  }
  QgsRelation relation = QgsProject::instance()->relationManager()->relation( config[QStringLiteral( "Relation" )].toString() );
  if ( !relation.isValid() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Invalid relation" ) );
    return value.toString();
  }
  QgsVectorLayer *referencingLayer = relation.referencingLayer();
  if ( layer != referencingLayer )
  {
    QgsMessageLog::logMessage( QObject::tr( "representValue() with inconsistent layer parameter w.r.t relation referencingLayer" ) );
    return value.toString();
  }
  int referencingFieldIdx = referencingLayer->fields().lookupField( relation.fieldPairs().at( 0 ).first );
  if ( referencingFieldIdx != fieldIndex )
  {
    QgsMessageLog::logMessage( QObject::tr( "representValue() with inconsistent fieldIndex parameter w.r.t relation referencingFieldIdx" ) );
    return value.toString();
  }
  QgsVectorLayer *referencedLayer = relation.referencedLayer();
  if ( !referencedLayer )
  {
    QgsMessageLog::logMessage( QObject::tr( "Cannot find referenced layer" ) );
    return value.toString();
  }

  // Attributes from the referencing layer
  QgsAttributes attrs = QgsAttributes( layer->fields().count() );
  // Set the value on the foreign key field of the referencing record
  attrs[ referencingFieldIdx ] = value;

  QgsFeatureRequest request = relation.getReferencedFeatureRequest( attrs );
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
    int referencedFieldIdx = referencedLayer->fields().lookupField( relation.fieldPairs().at( 0 ).second );
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

  // Some sanity checks
  if ( !config.contains( QStringLiteral( "Relation" ) ) )
  {
    QgsMessageLog::logMessage( QObject::tr( "Missing Relation in configuration" ) );
    return QVariant();
  }
  QgsRelation relation = QgsProject::instance()->relationManager()->relation( config[QStringLiteral( "Relation" )].toString() );
  if ( !relation.isValid() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Invalid relation" ) );
    return QVariant();
  }
  QgsVectorLayer *referencingLayer = relation.referencingLayer();
  if ( layer != referencingLayer )
  {
    QgsMessageLog::logMessage( QObject::tr( "representValue() with inconsistent layer parameter w.r.t relation referencingLayer" ) );
    return QVariant();
  }
  QgsVectorLayer *referencedLayer = relation.referencedLayer();
  if ( !referencedLayer )
  {
    QgsMessageLog::logMessage( QObject::tr( "Cannot find referenced layer" ) );
    return QVariant();
  }
  int referencedFieldIdx = referencedLayer->fields().lookupField( relation.fieldPairs().at( 0 ).second );
  if ( referencedFieldIdx == -1 )
  {
    QgsMessageLog::logMessage( QObject::tr( "Invalid referenced field (%1) configured in relation %2" ).arg( relation.fieldPairs().at( 0 ).second, relation.name() ) );
    return QVariant();
  }

  QgsExpression expr( referencedLayer->displayExpression() );

  QgsFeatureRequest request;
  request.setFlags( QgsFeatureRequest::NoGeometry );
  QgsAttributeList requiredAttributes = expr.referencedAttributeIndexes( referencedLayer->fields() ).toList();
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
      int referencedFieldIdx = referencedLayer->fields().lookupField( relation.fieldPairs().at( 0 ).second );
      title = feature.attribute( referencedFieldIdx ).toString();
    }

    cache.insert( feature.attribute( referencedFieldIdx ), title );
  }

  return QVariant::fromValue<QMap<QVariant, QString>>( cache );
}
