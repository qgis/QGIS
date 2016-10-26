/***************************************************************************
  qgsvectorlayerutils.cpp
  -----------------------
  Date                 : October 2016
  Copyright            : (C) 2016 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayerutils.h"

bool QgsVectorLayerUtils::valueExists( const QgsVectorLayer* layer, int fieldIndex, const QVariant& value, const QgsFeatureIds& ignoreIds )
{
  if ( !layer )
    return false;

  if ( fieldIndex < 0 || fieldIndex >= layer->fields().count() )
    return false;

  QString fieldName = layer->fields().at( fieldIndex ).name();

  // build up an optimised feature request
  QgsFeatureRequest request;
  request.setSubsetOfAttributes( QgsAttributeList() );
  request.setFlags( QgsFeatureRequest::NoGeometry );

  // at most we need to check ignoreIds.size() + 1 - the feature not in ignoreIds is the one we're interested in
  int limit = ignoreIds.size() + 1;
  request.setLimit( limit );

  request.setFilterExpression( QStringLiteral( "%1=%2" ).arg( QgsExpression::quotedColumnRef( fieldName ),
                               QgsExpression::quotedValue( value ) ) );

  QgsFeature feat;
  QgsFeatureIterator it = layer->getFeatures( request );
  while ( it.nextFeature( feat ) )
  {
    if ( ignoreIds.contains( feat.id() ) )
      continue;

    return true;
  }

  return false;
}

bool QgsVectorLayerUtils::validateAttribute( const QgsVectorLayer* layer, const QgsFeature& feature, int attributeIndex, QStringList& errors )
{
  if ( !layer )
    return false;

  if ( attributeIndex < 0 || attributeIndex >= layer->fields().count() )
    return false;

  QgsField field = layer->fields().at( attributeIndex );
  QVariant value = feature.attribute( attributeIndex );
  bool valid = true;
  errors.clear();

  if ( field.constraints() & QgsField::ConstraintExpression && !field.constraintExpression().isEmpty() )
  {
    QgsExpressionContext context = layer->createExpressionContext();
    context.setFeature( feature );

    QgsExpression expr( field.constraintExpression() );

    valid = expr.evaluate( &context ).toBool();

    if ( expr.hasParserError() )
    {
      errors << QObject::tr( "parser error: %1" ).arg( expr.parserErrorString() );
    }
    else if ( expr.hasEvalError() )
    {
      errors << QObject::tr( "evaluation error: %1" ).arg( expr.evalErrorString() );
    }
    else if ( !valid )
    {
      errors << QObject::tr( "%1 check failed" ).arg( field.constraintDescription() );
    }
  }

  if ( field.constraints() & QgsField::ConstraintNotNull )
  {
    valid = valid && !value.isNull();

    if ( value.isNull() )
    {
      errors << QObject::tr( "value is NULL" );
    }
  }

  if ( field.constraints() & QgsField::ConstraintUnique )
  {
    bool alreadyExists = QgsVectorLayerUtils::valueExists( layer, attributeIndex, value, QgsFeatureIds() << feature.id() );
    valid = valid && !alreadyExists;

    if ( alreadyExists )
    {
      errors << QObject::tr( "value is not unique" );
    }
  }

  return valid;
}

