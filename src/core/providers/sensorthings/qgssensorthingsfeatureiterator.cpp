/***************************************************************************
      qgssensorthingsfeatureiterator.cpp
      ----------------
    begin                : November 2023
    copyright            : (C) 2013 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgssensorthingsfeatureiterator.h"
#include "geometry/qgsgeometry.h"
#include "qgsexception.h"
#include "qgsfeedback.h"
#include "qgsgeometryengine.h"

///@cond PRIVATE

//
// QgsSensorThingsFeatureSource
//

QgsSensorThingsFeatureSource::QgsSensorThingsFeatureSource( const std::shared_ptr<QgsSensorThingsSharedData> &sharedData )
  : mSharedData( sharedData )
{
}

QgsFeatureIterator QgsSensorThingsFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsSensorThingsFeatureIterator( this, false, request ) );
}

QgsSensorThingsSharedData *QgsSensorThingsFeatureSource::sharedData() const
{
  return mSharedData.get();
}


//
// QgsSensorThingsFeatureIterator
//

QgsSensorThingsFeatureIterator::QgsSensorThingsFeatureIterator( QgsSensorThingsFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsSensorThingsFeatureSource>( source, ownSource, request )
  , mInterruptionChecker( request.feedback() )
{
  mTransform = mRequest.calculateTransform( mSource->sharedData()->crs() );
  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
    if ( !mRequest.filterRect().isNull() && mFilterRect.isNull() )
    {
      close();
      return;
    }
  }
  catch ( QgsCsException & )
  {
    // can't reproject mFilterRect
    close();
    return;
  }

  QgsFeatureIds requestIds;
  if ( mRequest.filterType() == Qgis::FeatureRequestFilterType::Fids )
  {
    requestIds = mRequest.filterFids();
  }
  else if ( mRequest.filterType() == Qgis::FeatureRequestFilterType::Fid )
  {
    requestIds.insert( mRequest.filterFid() );
  }

  if ( !mFilterRect.isNull() )
  {
    // defer request to find features in filter rect until first feature is requested
    // this allows time for a interruption checker to be installed on the iterator
    // and avoids performing this expensive check in the main thread when just
    // preparing iterators
    mDeferredFeaturesInFilterRectCheck = true;
  }

  // prepare spatial filter geometries for optimal speed
  switch ( mRequest.spatialFilterType() )
  {
    case Qgis::SpatialFilterType::NoFilter:
    case Qgis::SpatialFilterType::BoundingBox:
      break;

    case Qgis::SpatialFilterType::DistanceWithin:
      if ( !mRequest.referenceGeometry().isEmpty() )
      {
        mDistanceWithinGeom = mRequest.referenceGeometry();
        mDistanceWithinEngine.reset( QgsGeometry::createGeometryEngine( mDistanceWithinGeom.constGet() ) );
        mDistanceWithinEngine->prepareGeometry();
      }
      break;
  }

  mRequestFeatureIdList = qgis::setToList( requestIds );
  std::sort( mRequestFeatureIdList.begin(), mRequestFeatureIdList.end() );
  mRemainingFeatureIds = mRequestFeatureIdList;
  if ( !mRemainingFeatureIds.empty() )
    mFeatureIterator = mRemainingFeatureIds.at( 0 );

  mGeometryTestFilterRect = mFilterRect;
}

QgsSensorThingsFeatureIterator::~QgsSensorThingsFeatureIterator()
{
  QgsSensorThingsFeatureIterator::close();
}

bool QgsSensorThingsFeatureIterator::fetchFeature( QgsFeature &f )
{
  f.setValid( false );

  if ( mClosed )
    return false;

  if ( mInterruptionChecker && mInterruptionChecker->isCanceled() )
    return false;

  if ( mDeferredFeaturesInFilterRectCheck || !mCurrentPage.isEmpty() )
  {
    const QgsFeatureIds featuresInRect = mSource->sharedData()->getFeatureIdsInExtent( mFilterRect, mInterruptionChecker, mCurrentPage, mNextPage, mAlreadyFetchedIds );
    mCurrentPage.clear();

    if ( !mRequestFeatureIdList.isEmpty() )
    {
      QgsFeatureIds requestIds = qgis::listToSet( mRequestFeatureIdList );
      requestIds.intersect( featuresInRect );
      mCurrentPageFeatureIdList = qgis::setToList( requestIds );
    }
    else
    {
      mCurrentPageFeatureIdList = qgis::setToList( featuresInRect );
    }
    if ( mCurrentPageFeatureIdList.empty() )
    {
      if ( mNextPage.isEmpty() )
      {
        return false;
      }
      else
      {
        mCurrentPage = mNextPage;
        return fetchFeature( f );
      }
    }

    std::sort( mCurrentPageFeatureIdList.begin(), mCurrentPageFeatureIdList.end() );
    mRemainingFeatureIds = mCurrentPageFeatureIdList;
    if ( !mRemainingFeatureIds.empty() )
      mFeatureIterator = mRemainingFeatureIds.at( 0 );

    mDeferredFeaturesInFilterRectCheck = false;

    if ( !( mRequest.flags() & Qgis::FeatureRequestFlag::ExactIntersect ) )
    {
      // discard the filter rect - we know that the features in mRemainingFeatureIds are guaranteed
      // to be intersecting the rect, so avoid any extra unnecessary checks
      mGeometryTestFilterRect = QgsRectangle();
    }
  }

  if ( !mCurrentPageFeatureIdList.empty() && mRemainingFeatureIds.empty() )
  {
    if ( mNextPage.isEmpty() )
    {
      return false;
    }
    else
    {
      // request next page
      mCurrentPage = mNextPage;
      return fetchFeature( f );
    }
  }

  switch ( mRequest.filterType() )
  {
    case Qgis::FeatureRequestFilterType::Fid:
    {
      if ( mRemainingFeatureIds.empty() )
        return false;

      bool result = mSource->sharedData()->getFeature( mRequest.filterFid(), f, mInterruptionChecker );
      mAlreadyFetchedIds.insert( mRequest.filterFid() );
      if ( mInterruptionChecker && mInterruptionChecker->isCanceled() )
        return false;

      geometryToDestinationCrs( f, mTransform );
      if ( mDistanceWithinEngine && mDistanceWithinEngine->distance( f.geometry().constGet() ) > mRequest.distanceWithin() )
      {
        result = false;
      }
      f.setValid( result );

      mRemainingFeatureIds.removeAll( f.id() );
      return result;
    }

    case Qgis::FeatureRequestFilterType::Fids:
    case Qgis::FeatureRequestFilterType::Expression:
    case Qgis::FeatureRequestFilterType::NoFilter:
    {
      while ( true )
      {
        if ( mInterruptionChecker && mInterruptionChecker->isCanceled() )
          return false;

        if ( !mCurrentPageFeatureIdList.empty() && mRemainingFeatureIds.empty() )
        {
          if ( mNextPage.isEmpty() )
          {
            return false;
          }
          else
          {
            // fetch next page
            mCurrentPage = mNextPage;
            return fetchFeature( f );
          }
        }

        bool success = mSource->sharedData()->getFeature( mFeatureIterator, f, mInterruptionChecker );
        mAlreadyFetchedIds.insert( mFeatureIterator );

        if ( !mCurrentPageFeatureIdList.empty() )
        {
          mRemainingFeatureIds.removeAll( mFeatureIterator );
          if ( !mRemainingFeatureIds.empty() )
            mFeatureIterator = mRemainingFeatureIds.at( 0 );
        }
        else
        {
          if ( !success )
            return false;
          ++mFeatureIterator;
        }

        if ( success && !mGeometryTestFilterRect.isNull() )
        {
          if ( !f.hasGeometry() )
            success = false;
          else
          {
            if ( mRequest.spatialFilterType() == Qgis::SpatialFilterType::BoundingBox && mRequest.flags() & Qgis::FeatureRequestFlag::ExactIntersect )
            {
              // exact intersection check requested
              if ( !f.geometry().intersects( mGeometryTestFilterRect ) )
                success = false;
            }
            else
            {
              // bounding box intersect check only
              if ( !f.geometry().boundingBoxIntersects( mGeometryTestFilterRect ) )
                success = false;
            }
          }
        }

        if ( !success )
          continue;
        geometryToDestinationCrs( f, mTransform );

        bool result = true;
        if ( mDistanceWithinEngine && mDistanceWithinEngine->distance( f.geometry().constGet() ) > mRequest.distanceWithin() )
          result = false;

        if ( result )
          return true;
      }
      return false;
    }
  }

  return false;
}

bool QgsSensorThingsFeatureIterator::rewind()
{
  if ( mClosed )
    return false;
  mFeatureIterator = 0;
  mCurrentPage.clear();
  mAlreadyFetchedIds.clear();
  mRemainingFeatureIds = mRequestFeatureIdList;
  if ( !mRemainingFeatureIds.empty() )
    mFeatureIterator = mRemainingFeatureIds.at( 0 );
  return true;
}

bool QgsSensorThingsFeatureIterator::close()
{
  if ( mClosed )
    return false;
  iteratorClosed();
  mClosed = true;
  return true;
}

void QgsSensorThingsFeatureIterator::setInterruptionChecker( QgsFeedback *interruptionChecker )
{
  mInterruptionChecker = interruptionChecker;
}

///@endcond PRIVATE
