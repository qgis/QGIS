/***************************************************************************
                         qgsfeaturesink.cpp
                         ------------------
    begin                : April 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsfeaturestore.h"

bool QgsFeatureSink::addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags )
{
  QgsFeatureList features;
  features << feature;
  bool result = addFeatures( features, flags );

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

