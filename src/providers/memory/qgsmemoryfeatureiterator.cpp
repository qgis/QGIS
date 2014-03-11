/***************************************************************************
    qgsmemoryfeatureiterator.cpp
    ---------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmemoryfeatureiterator.h"
#include "qgsmemoryprovider.h"

#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsspatialindex.h"
#include "qgsmessagelog.h"



QgsMemoryFeatureIterator::QgsMemoryFeatureIterator( QgsMemoryFeatureSource* source, bool ownSource, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIteratorFromSource( source, ownSource, request )
    , mSelectRectGeom( 0 )
{

  if ( mRequest.filterType() == QgsFeatureRequest::FilterRect && mRequest.flags() & QgsFeatureRequest::ExactIntersect )
  {
    mSelectRectGeom = QgsGeometry::fromRect( request.filterRect() );
  }

  // if there's spatial index, use it!
  // (but don't use it when selection rect is not specified)
  if ( mRequest.filterType() == QgsFeatureRequest::FilterRect && mSource->mSpatialIndex )
  {
    mUsingFeatureIdList = true;
    mFeatureIdList = mSource->mSpatialIndex->intersects( mRequest.filterRect() );
    QgsDebugMsg( "Features returned by spatial index: " + QString::number( mFeatureIdList.count() ) );
  }
  else if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    mUsingFeatureIdList = true;
    QgsFeatureMap::const_iterator it = mSource->mFeatures.find( mRequest.filterFid() );
    if ( it != mSource->mFeatures.end() )
      mFeatureIdList.append( mRequest.filterFid() );
  }
  else
  {
    mUsingFeatureIdList = false;
  }

  rewind();
}

QgsMemoryFeatureIterator::~QgsMemoryFeatureIterator()
{
  close();
}


bool QgsMemoryFeatureIterator::fetchFeature( QgsFeature& feature )
{
  feature.setValid( false );

  if ( mClosed )
    return false;

  if ( mUsingFeatureIdList )
    return nextFeatureUsingList( feature );
  else
    return nextFeatureTraverseAll( feature );
}


bool QgsMemoryFeatureIterator::nextFeatureUsingList( QgsFeature& feature )
{
  bool hasFeature = false;

  // option 1: we have a list of features to traverse
  while ( mFeatureIdListIterator != mFeatureIdList.constEnd() )
  {
    if ( mRequest.filterType() == QgsFeatureRequest::FilterRect && mRequest.flags() & QgsFeatureRequest::ExactIntersect )
    {
      // do exact check in case we're doing intersection
      if ( mSource->mFeatures[*mFeatureIdListIterator].geometry() && mSource->mFeatures[*mFeatureIdListIterator].geometry()->intersects( mSelectRectGeom ) )
        hasFeature = true;
    }
    else
      hasFeature = true;

    if ( hasFeature )
      break;

    ++mFeatureIdListIterator;
  }

  // copy feature
  if ( hasFeature )
  {
    feature = mSource->mFeatures[*mFeatureIdListIterator];
    ++mFeatureIdListIterator;
  }
  else
    close();

  if ( hasFeature )
    feature.setFields( &mSource->mFields ); // allow name-based attribute lookups

  return hasFeature;
}


bool QgsMemoryFeatureIterator::nextFeatureTraverseAll( QgsFeature& feature )
{
  bool hasFeature = false;

  // option 2: traversing the whole layer
  while ( mSelectIterator != mSource->mFeatures.constEnd() )
  {
    if ( mRequest.filterType() != QgsFeatureRequest::FilterRect )
    {
      // selection rect empty => using all features
      hasFeature = true;
    }
    else
    {
      if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
      {
        // using exact test when checking for intersection
        if ( mSelectIterator->geometry() && mSelectIterator->geometry()->intersects( mSelectRectGeom ) )
          hasFeature = true;
      }
      else
      {
        // check just bounding box against rect when not using intersection
        if ( mSelectIterator->geometry() && mSelectIterator->geometry()->boundingBox().intersects( mRequest.filterRect() ) )
          hasFeature = true;
      }
    }

    if ( hasFeature )
      break;

    ++mSelectIterator;
  }

  // copy feature
  if ( hasFeature )
  {
    feature = mSelectIterator.value();
    ++mSelectIterator;
    feature.setValid( true );
    feature.setFields( &mSource->mFields ); // allow name-based attribute lookups
  }
  else
    close();

  return hasFeature;
}

bool QgsMemoryFeatureIterator::rewind()
{
  if ( mClosed )
    return false;

  if ( mUsingFeatureIdList )
    mFeatureIdListIterator = mFeatureIdList.constBegin();
  else
    mSelectIterator = mSource->mFeatures.constBegin();

  return true;
}

bool QgsMemoryFeatureIterator::close()
{
  if ( mClosed )
    return false;

  iteratorClosed();

  delete mSelectRectGeom;
  mSelectRectGeom = NULL;

  mClosed = true;
  return true;
}

// -------------------------

QgsMemoryFeatureSource::QgsMemoryFeatureSource( const QgsMemoryProvider* p )
    : mFields( p->mFields )
    , mFeatures( p->mFeatures )
    , mSpatialIndex( p->mSpatialIndex ? new QgsSpatialIndex( *p->mSpatialIndex ) : 0 )  // just shallow copy
{
}

QgsMemoryFeatureSource::~QgsMemoryFeatureSource()
{
  delete mSpatialIndex;
}

QgsFeatureIterator QgsMemoryFeatureSource::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsMemoryFeatureIterator( this, false, request ) );
}
