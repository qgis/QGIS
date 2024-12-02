/***************************************************************************
                         qgsfeaturesink.cpp
                         ------------------
    begin                : April 2017
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

#include "qgsfeaturestore.h"

void QgsFeatureSink::finalize()
{
  flushBuffer();
}

bool QgsFeatureSink::addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags )
{
  QgsFeatureList features;
  features << feature;
  const bool result = addFeatures( features, flags );

  if ( !( flags & FastInsert ) )
  {
    // need to update the passed feature reference to the updated copy from the features list
    feature = features.at( 0 );
  }
  return result;
}

bool QgsFeatureSink::addFeatures( QgsFeatureIterator &iterator, QgsFeatureSink::Flags flags )
{
  QgsFeature f;
  bool result = true;
  while ( iterator.nextFeature( f ) )
  {
    result = result && addFeature( f, flags );
  }
  return result;
}

