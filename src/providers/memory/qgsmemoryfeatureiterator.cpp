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


QgsMemoryFeatureIterator::QgsMemoryFeatureIterator( QgsMemoryProvider* p, const QgsFeatureRequest& request )
    : QgsAbstractFeatureIterator( request ), P( p ), mSelectRectGeom( NULL )
{
  // make sure that only one iterator is active
  if ( P->mActiveIterator )
  {
    QgsMessageLog::logMessage( QObject::tr( "Already active iterator on this provider was closed." ), QObject::tr( "Memory provider" ) );
    P->mActiveIterator->close();
  }
  P->mActiveIterator = this;

  if ( mRequest.filterType() == QgsFeatureRequest::FilterRect && mRequest.flags() & QgsFeatureRequest::ExactIntersect )
  {
    mSelectRectGeom = QgsGeometry::fromRect( request.filterRect() );
  }

  // if there's spatial index, use it!
  // (but don't use it when selection rect is not specified)
  if ( mRequest.filterType() == QgsFeatureRequest::FilterRect && P->mSpatialIndex )
  {
    mUsingFeatureIdList = true;
    mFeatureIdList = P->mSpatialIndex->intersects( mRequest.filterRect() );
    QgsDebugMsg( "Features returned by spatial index: " + QString::number( mFeatureIdList.count() ) );
  }
  else if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    mUsingFeatureIdList = true;
    QgsFeatureMap::iterator it = P->mFeatures.find( mRequest.filterFid() );
    if ( it != P->mFeatures.end() )
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


bool QgsMemoryFeatureIterator::nextFeature( QgsFeature& feature )
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
  while ( mFeatureIdListIterator != mFeatureIdList.end() )
  {
    if ( mRequest.filterType() == QgsFeatureRequest::FilterRect && mRequest.flags() & QgsFeatureRequest::ExactIntersect )
    {
      // do exact check in case we're doing intersection
      if ( P->mFeatures[*mFeatureIdListIterator].geometry()->intersects( mSelectRectGeom ) )
        hasFeature = true;
    }
    else
      hasFeature = true;

    if ( hasFeature )
      break;

    mFeatureIdListIterator++;
  }

  // copy feature
  if ( hasFeature )
  {
    feature = P->mFeatures[*mFeatureIdListIterator];
    mFeatureIdListIterator++;
  }
  else
    close();

  return hasFeature;
}


bool QgsMemoryFeatureIterator::nextFeatureTraverseAll( QgsFeature& feature )
{
  bool hasFeature = false;

  // option 2: traversing the whole layer
  while ( mSelectIterator != P->mFeatures.end() )
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
        if ( mSelectIterator->geometry()->intersects( mSelectRectGeom ) )
          hasFeature = true;
      }
      else
      {
        // check just bounding box against rect when not using intersection
        if ( mSelectIterator->geometry()->boundingBox().intersects( mRequest.filterRect() ) )
          hasFeature = true;
      }
    }

    if ( hasFeature )
      break;

    mSelectIterator++;
  }

  // copy feature
  if ( hasFeature )
  {
    feature = mSelectIterator.value();
    mSelectIterator++;
    feature.setValid( true );
    feature.setFields( &P->mFields ); // allow name-based attribute lookups
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
    mFeatureIdListIterator = mFeatureIdList.begin();
  else
    mSelectIterator = P->mFeatures.begin();

  return true;
}

bool QgsMemoryFeatureIterator::close()
{
  if ( mClosed )
    return false;

  delete mSelectRectGeom;
  mSelectRectGeom = NULL;

  // tell provider that this iterator is not active anymore
  P->mActiveIterator = 0;

  mClosed = true;
  return true;
}
