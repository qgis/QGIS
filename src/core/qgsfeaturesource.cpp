/***************************************************************************
                         qgsfeaturesource.cpp
                         -------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfeaturesource.h"
#include "qgsfeaturerequest.h"
#include "qgsfeatureiterator.h"

QSet<QVariant> QgsFeatureSource::uniqueValues( int fieldIndex, int limit ) const
{
  if ( fieldIndex < 0 || fieldIndex >= fields().count() )
    return QSet<QVariant>();

  QgsFeatureRequest req;
  req.setFlags( QgsFeatureRequest::NoGeometry );
  req.setSubsetOfAttributes( QgsAttributeList() << fieldIndex );

  QSet<QVariant> values;
  QgsFeatureIterator it = getFeatures( req );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    values.insert( f.attribute( fieldIndex ) );
    if ( limit > 0 && values.size() >= limit )
      return values;
  }
  return values;
}

