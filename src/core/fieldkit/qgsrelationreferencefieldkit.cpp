/***************************************************************************
  qgsrelationreferencefieldkit.cpp - QgsRelationReferenceFieldKit

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
#include "qgsrelationreferencefieldkit.h"

#include "qgsmessagelog.h"
#include "qgsrelation.h"
#include "qgsexpressioncontext.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"

QgsRelationReferenceFieldKit::QgsRelationReferenceFieldKit()
{

}

QString QgsRelationReferenceFieldKit::representValue( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( cache );

  // Some sanity checks
  if ( !config.contains( QStringLiteral( "Relation" ) ) )
  {
    QgsMessageLog::logMessage( "Missing Relation in configuration" );
    return value.toString();
  }
  QgsRelation relation = QgsProject::instance()->relationManager()->relation( config[QStringLiteral( "Relation" )].toString() );
  if ( !relation.isValid() )
  {
    QgsMessageLog::logMessage( "Invalid relation" );
    return value.toString();
  }
  QgsVectorLayer* referencingLayer = relation.referencingLayer();
  if ( vl != referencingLayer )
  {
    QgsMessageLog::logMessage( "representValue() with inconsistent vl parameter w.r.t relation referencingLayer" );
    return value.toString();
  }
  int referencingFieldIdx = referencingLayer->fields().lookupField( relation.fieldPairs().at( 0 ).first );
  if ( referencingFieldIdx != fieldIdx )
  {
    QgsMessageLog::logMessage( "representValue() with inconsistent fieldIdx parameter w.r.t relation referencingFieldIdx" );
    return value.toString();
  }
  QgsVectorLayer* referencedLayer = relation.referencedLayer();
  if ( !referencedLayer )
  {
    QgsMessageLog::logMessage( "Cannot find referenced layer" );
    return value.toString();
  }

  // Attributes from the referencing layer
  QgsAttributes attrs = QgsAttributes( vl->fields().count() );
  // Set the value on the foreign key field of the referencing record
  attrs[ referencingFieldIdx ] = value;

  QgsFeatureRequest request = relation.getReferencedFeatureRequest( attrs );
  QgsFeature feature;
  referencedLayer->getFeatures( request ).nextFeature( feature );
  if ( !feature.isValid() )
    return value.toString();

  QgsExpression expr( referencedLayer->displayExpression() );
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( referencedLayer );
  context.setFeature( feature );
  QString title = expr.evaluate( &context ).toString();
  if ( expr.hasEvalError() )
  {
    int referencedFieldIdx = referencedLayer->fields().lookupField( relation.fieldPairs().at( 0 ).second );
    title = feature.attribute( referencedFieldIdx ).toString();
  }
  return title;
}

QVariant QgsRelationReferenceFieldKit::sortValue( QgsVectorLayer* vl, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  return representValue( vl, fieldIdx, config, cache, value );
}
