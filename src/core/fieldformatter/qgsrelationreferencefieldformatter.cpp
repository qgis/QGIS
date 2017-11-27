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

QString QgsRelationReferenceFieldFormatter::id() const
{
  return QStringLiteral( "RelationReference" );
}

QString QgsRelationReferenceFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( cache );

  // Some sanity checks
  if ( !config.contains( QStringLiteral( "Relation" ) ) )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Missing Relation in configuration" ) );
    return value.toString();
  }
  QgsRelation relation = QgsProject::instance()->relationManager()->relation( config[QStringLiteral( "Relation" )].toString() );
  if ( !relation.isValid() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Invalid relation" ) );
    return value.toString();
  }
  QgsVectorLayer *referencingLayer = relation.referencingLayer();
  if ( layer != referencingLayer )
  {
    QgsMessageLog::logMessage( QStringLiteral( "representValue() with inconsistent layer parameter w.r.t relation referencingLayer" ) );
    return value.toString();
  }
  int referencingFieldIdx = referencingLayer->fields().lookupField( relation.fieldPairs().at( 0 ).first );
  if ( referencingFieldIdx != fieldIndex )
  {
    QgsMessageLog::logMessage( QStringLiteral( "representValue() with inconsistent fieldIndex parameter w.r.t relation referencingFieldIdx" ) );
    return value.toString();
  }
  QgsVectorLayer *referencedLayer = relation.referencedLayer();
  if ( !referencedLayer )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Cannot find referenced layer" ) );
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
