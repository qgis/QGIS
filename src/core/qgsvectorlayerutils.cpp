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
