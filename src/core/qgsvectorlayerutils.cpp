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
#include "qgsvectordataprovider.h"
#include <QRegularExpression>

bool QgsVectorLayerUtils::valueExists( const QgsVectorLayer* layer, int fieldIndex, const QVariant& value, const QgsFeatureIds& ignoreIds )
{
  if ( !layer )
    return false;

  QgsFields fields = layer->fields();

  if ( fieldIndex < 0 || fieldIndex >= fields.count() )
    return false;

  // check - if value is a provider side defaultValueClause then we exclude it from the check
  if ( fields.fieldOrigin( fieldIndex ) == QgsFields::OriginProvider )
  {
    int providerIdx = fields.fieldOriginIndex( fieldIndex );
    QString providerDefaultClause = layer->dataProvider()->defaultValueClause( providerIdx );
    if ( !providerDefaultClause.isEmpty() && value.toString() == providerDefaultClause )
    {
      // exempt from check
      return false;
    }
  }

  QString fieldName = fields.at( fieldIndex ).name();

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

QVariant QgsVectorLayerUtils::createUniqueValue( const QgsVectorLayer* layer, int fieldIndex, const QVariant& seed )
{
  if ( !layer )
    return QVariant();

  QgsFields fields = layer->fields();

  if ( fieldIndex < 0 || fieldIndex >= fields.count() )
    return QVariant();

  QgsField field = fields.at( fieldIndex );

  if ( field.isNumeric() )
  {
    QVariant maxVal = layer->maximumValue( fieldIndex );
    QVariant newVar( maxVal.toLongLong() + 1 );
    if ( field.convertCompatible( newVar ) )
      return newVar;
    else
      return QVariant();
  }
  else
  {
    switch ( field.type() )
    {
      case QVariant::String:
      {
        QString base;
        if ( seed.isValid() )
          base = seed.toString();

        if ( !base.isEmpty() )
        {
          // strip any existing _1, _2 from the seed
          QRegularExpression rx( "(.*)_\\d+" );
          QRegularExpressionMatch match = rx.match( base );
          if ( match.hasMatch() )
          {
            base = match.captured( 1 );
          }
        }
        else
        {
          // no base seed - fetch first value from layer
          QgsFeatureRequest req;
          req.setLimit( 1 );
          req.setSubsetOfAttributes( QgsAttributeList() << fieldIndex );
          req.setFlags( QgsFeatureRequest::NoGeometry );
          QgsFeature f;
          layer->getFeatures( req ).nextFeature( f );
          base = f.attribute( fieldIndex ).toString();
        }

        // try variants like base_1, base_2, etc until a new value found
        QStringList vals = layer->uniqueStringsMatching( fieldIndex, base );

        // might already be unique
        if ( !base.isEmpty() && !vals.contains( base ) )
          return base;

        for ( int i = 1; i < 10000; ++i )
        {
          QString testVal = base + '_' + QString::number( i );
          if ( !vals.contains( testVal ) )
            return testVal;
        }

        // failed
        return QVariant();
      }

      default:
        // todo other types - dates? times?
        return QVariant();
    }
  }

  return QVariant();
}

bool QgsVectorLayerUtils::validateAttribute( const QgsVectorLayer* layer, const QgsFeature& feature, int attributeIndex, QStringList& errors,
    QgsFieldConstraints::ConstraintStrength strength, QgsFieldConstraints::ConstraintOrigin origin )
{
  if ( !layer )
    return false;

  if ( attributeIndex < 0 || attributeIndex >= layer->fields().count() )
    return false;

  QgsField field = layer->fields().at( attributeIndex );
  QVariant value = feature.attribute( attributeIndex );
  bool valid = true;
  errors.clear();

  QgsFieldConstraints constraints = field.constraints();

  if ( constraints.constraints() & QgsFieldConstraints::ConstraintExpression && !constraints.constraintExpression().isEmpty()
       && ( strength == QgsFieldConstraints::ConstraintStrengthNotSet || strength == constraints.constraintStrength( QgsFieldConstraints::ConstraintExpression ) )
       && ( origin == QgsFieldConstraints::ConstraintOriginNotSet || origin == constraints.constraintOrigin( QgsFieldConstraints::ConstraintExpression ) ) )
  {
    QgsExpressionContext context = layer->createExpressionContext();
    context.setFeature( feature );

    QgsExpression expr( constraints.constraintExpression() );

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
      errors << QObject::tr( "%1 check failed" ).arg( constraints.constraintDescription() );
    }
  }

  if ( constraints.constraints() & QgsFieldConstraints::ConstraintNotNull
       && ( strength == QgsFieldConstraints::ConstraintStrengthNotSet || strength == constraints.constraintStrength( QgsFieldConstraints::ConstraintNotNull ) )
       && ( origin == QgsFieldConstraints::ConstraintOriginNotSet || origin == constraints.constraintOrigin( QgsFieldConstraints::ConstraintNotNull ) ) )
  {
    valid = valid && !value.isNull();

    if ( value.isNull() )
    {
      errors << QObject::tr( "value is NULL" );
    }
  }

  if ( constraints.constraints() & QgsFieldConstraints::ConstraintUnique
       && ( strength == QgsFieldConstraints::ConstraintStrengthNotSet || strength == constraints.constraintStrength( QgsFieldConstraints::ConstraintUnique ) )
       && ( origin == QgsFieldConstraints::ConstraintOriginNotSet || origin == constraints.constraintOrigin( QgsFieldConstraints::ConstraintUnique ) ) )
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

