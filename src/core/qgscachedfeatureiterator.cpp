/***************************************************************************
    qgscachedfeatureiterator.cpp
     --------------------------------------
    Date                 : 12.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscachedfeatureiterator.h"
#include "qgsvectorlayercache.h"
#include "qgsvectordataprovider.h"

QgsCachedFeatureIterator::QgsCachedFeatureIterator( QgsVectorLayerCache *vlCache, QgsFeatureRequest featureRequest, QgsFeatureIds featureIds )
    : QgsAbstractFeatureIterator( featureRequest )
    , mFeatureIds( featureIds )
    , mVectorLayerCache( vlCache )
{
  mFeatureIdIterator = featureIds.begin();
}

bool QgsCachedFeatureIterator::nextFeature( QgsFeature& f )
{
  mFeatureIdIterator++;

  if ( mFeatureIdIterator == mFeatureIds.end() )
  {
    return false;
  }
  else
  {
    f = QgsFeature( *mVectorLayerCache->mCache[*mFeatureIdIterator]->feature() );
    return true;
  }
}

bool QgsCachedFeatureIterator::rewind()
{
  mFeatureIdIterator = mFeatureIds.begin();
  return true;
}

bool QgsCachedFeatureIterator::close()
{
  // Nothing to clean...
  return true;
}

QgsCachedFeatureWriterIterator::QgsCachedFeatureWriterIterator( QgsVectorLayerCache *vlCache, QgsFeatureRequest featureRequest )
    : QgsAbstractFeatureIterator( featureRequest )
    , mVectorLayerCache( vlCache )
{
  mFeatIt = vlCache->layer()->dataProvider()->getFeatures( featureRequest );
}

bool QgsCachedFeatureWriterIterator::nextFeature( QgsFeature& f )
{
  if ( mFeatIt.nextFeature( f ) )
  {
    // As long as features can be fetched from the provider: Write them to cache
    mVectorLayerCache->cacheFeature( f );
    mFids.insert( f.id() );
    return true;
  }
  else
  {
    // Once no more features can be fetched: Inform the cache, that
    // the request has been completed
    mVectorLayerCache->requestCompleted( mRequest, mFids );
    return false;
  }
}

bool QgsCachedFeatureWriterIterator::rewind()
{
  mFids.clear();
  return mFeatIt.rewind();
}

bool QgsCachedFeatureWriterIterator::close()
{
  return mFeatIt.close();
}
