/***************************************************************************
                               qgsexpressionutils.cpp
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionutils.h"
#include "qgsexpressionnode.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QgsExpressionUtils::TVL QgsExpressionUtils::AND[3][3] =
{
  // false  true    unknown
  { False, False,   False },   // false
  { False, True,    Unknown }, // true
  { False, Unknown, Unknown }  // unknown
};
QgsExpressionUtils::TVL QgsExpressionUtils::OR[3][3] =
{
  { False,   True, Unknown },  // false
  { True,    True, True },     // true
  { Unknown, True, Unknown }   // unknown
};

QgsExpressionUtils::TVL QgsExpressionUtils::NOT[3] = { True, False, Unknown };

///@endcond

std::tuple<QVariant::Type, int> QgsExpressionUtils::determineResultType( const QString &expression, const QgsVectorLayer *layer, QgsFeatureRequest request, QgsExpressionContext context, bool *foundFeatures )
{
  QgsExpression exp( expression );
  request.setFlags( ( exp.needsGeometry() ) ?
                    QgsFeatureRequest::NoFlags :
                    QgsFeatureRequest::NoGeometry );
  request.setLimit( 10 );
  request.setExpressionContext( context );

  // avoid endless recursion by removing virtual fields while going through features
  // to determine result type
  QgsAttributeList attributes;
  const QgsFields fields = layer->fields();
  for ( int i = 0; i < fields.count(); i++ )
  {
    if ( fields.fieldOrigin( i ) != QgsFields::OriginExpression )
      attributes << i;
  }
  request.setSubsetOfAttributes( attributes );

  QVariant value;
  QgsFeature f;
  QgsFeatureIterator it = layer->getFeatures( request );
  bool hasFeature = it.nextFeature( f );
  if ( foundFeatures )
    *foundFeatures = hasFeature;
  while ( hasFeature )
  {
    context.setFeature( f );
    const QVariant value = exp.evaluate( &context );
    if ( !value.isNull() )
    {
      return std::make_tuple( value.type(), value.userType() );
    }
    hasFeature = it.nextFeature( f );
  }
  value = QVariant();
  return std::make_tuple( value.type(), value.userType() );
}
