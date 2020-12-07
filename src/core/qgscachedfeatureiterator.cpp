/***************************************************************************
    qgscachedfeatureiterator.cpp
     --------------------------------------
    Date                 : 12.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
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
#include "qgsexception.h"
#include "qgsvectorlayer.h"

QgsCachedFeatureIterator::QgsCachedFeatureIterator( QgsVectorLayerCache *vlCache, const QgsFeatureRequest &featureRequest )
  : QgsAbstractFeatureIterator( featureRequest )
  , mVectorLayerCache( vlCache )
{
  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mVectorLayerCache->sourceCrs() )
  {
    mTransform = QgsCoordinateTransform( mVectorLayerCache->sourceCrs(), mRequest.destinationCrs(), mRequest.transformContext() );
  }
  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
  }
  catch ( QgsCsException & )
  {
    // can't reproject mFilterRect
    close();
    return;
  }
  if ( !mFilterRect.isNull() )
  {
    // update request to be the unprojected filter rect
    mRequest.setFilterRect( mFilterRect );
  }

  switch ( featureRequest.filterType() )
  {
    case QgsFeatureRequest::FilterFids:
      mFeatureIds = featureRequest.filterFids();
      break;

    case QgsFeatureRequest::FilterFid:
      mFeatureIds = QgsFeatureIds() << featureRequest.filterFid();
      break;

    default:
      mFeatureIds = qgis::listToSet( mVectorLayerCache->mCache.keys() );
      break;
  }

  mFeatureIdIterator = mFeatureIds.constBegin();

  if ( mFeatureIdIterator == mFeatureIds.constEnd() )
    close();
}

bool QgsCachedFeatureIterator::fetchFeature( QgsFeature &f )
{
  f.setValid( false );

  if ( mClosed )
    return false;

  while ( mFeatureIdIterator != mFeatureIds.constEnd() )
  {
    if ( !mVectorLayerCache->mCache.contains( *mFeatureIdIterator ) )
    {
      ++mFeatureIdIterator;
      continue;
    }

    f = QgsFeature( *mVectorLayerCache->mCache[*mFeatureIdIterator]->feature() );
    ++mFeatureIdIterator;
    if ( mRequest.acceptFeature( f ) )
    {
      f.setValid( true );
      geometryToDestinationCrs( f, mTransform );
      return true;
    }
  }
  close();
  return false;
}

bool QgsCachedFeatureIterator::rewind()
{
  mFeatureIdIterator = mFeatureIds.constBegin();
  return true;
}

bool QgsCachedFeatureIterator::close()
{
  mClosed = true;
  mFeatureIds.clear();
  return true;
}

QgsCachedFeatureWriterIterator::QgsCachedFeatureWriterIterator( QgsVectorLayerCache *vlCache, const QgsFeatureRequest &featureRequest )
  : QgsAbstractFeatureIterator( featureRequest )
  , mVectorLayerCache( vlCache )
{
  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mVectorLayerCache->sourceCrs() )
  {
    mTransform = QgsCoordinateTransform( mVectorLayerCache->sourceCrs(), mRequest.destinationCrs(), mRequest.transformContext() );
  }
  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
  }
  catch ( QgsCsException & )
  {
    // can't reproject mFilterRect
    close();
    return;
  }
  if ( !mFilterRect.isNull() )
  {
    // update request to be the unprojected filter rect
    mRequest.setFilterRect( mFilterRect );
  }

  mFeatIt = vlCache->layer()->getFeatures( mRequest );
}

bool QgsCachedFeatureWriterIterator::fetchFeature( QgsFeature &f )
{
  if ( mClosed )
  {
    f.setValid( false );
    return false;
  }
  if ( mFeatIt.nextFeature( f ) )
  {
    // As long as features can be fetched from the provider: Write them to cache
    mVectorLayerCache->cacheFeature( f );
    mFids.insert( f.id() );
    geometryToDestinationCrs( f, mTransform );
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
  mClosed = true;
  return mFeatIt.close();
}
