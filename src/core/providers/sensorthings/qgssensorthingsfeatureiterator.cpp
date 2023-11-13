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
  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->sharedData()->crs() )
  {
    mTransform = QgsCoordinateTransform( mSource->sharedData()->crs(), mRequest.destinationCrs(), mRequest.transformContext() );
  }
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
  if ( mRequest.filterType() == QgsFeatureRequest::FilterFids )
  {
    requestIds = mRequest.filterFids();
  }
  else if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    requestIds.insert( mRequest.filterFid() );
  }

  if ( !mFilterRect.isNull() && !mSource->sharedData()->hasCachedAllFeatures() )
  {
    // defer request to find features in filter rect until first feature is requested
    // this allows time for a interruption checker to be installed on the iterator
    // and avoids performing this expensive check in the main thread when just
    // preparing iterators

    // (but if we've already cached ALL the features, we skip this -- there's no need for
    // firing off another request to the server)
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

  mFeatureIdList = qgis::setToList( requestIds );
  std::sort( mFeatureIdList.begin(), mFeatureIdList.end() );
  mRemainingFeatureIds = mFeatureIdList;
  if ( !mRemainingFeatureIds.empty() )
    mFeatureIterator = mRemainingFeatureIds.at( 0 );
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

  if ( mFeatureIterator >= mSource->sharedData()->objectIdCount() )
    return false;

  if ( mDeferredFeaturesInFilterRectCheck )
  {
    const QgsFeatureIds featuresInRect = mSource->sharedData()->getFeatureIdsInExtent( mFilterRect, mInterruptionChecker );
    if ( !mFeatureIdList.isEmpty() )
    {
      QgsFeatureIds requestIds = qgis::listToSet( mFeatureIdList );
      requestIds.intersect( featuresInRect );
      mFeatureIdList = qgis::setToList( requestIds );
    }
    else
    {
      mFeatureIdList = qgis::setToList( featuresInRect );
    }
    if ( mFeatureIdList.empty() )
    {
      return false;
    }

    std::sort( mFeatureIdList.begin(), mFeatureIdList.end() );
    mRemainingFeatureIds = mFeatureIdList;
    if ( !mRemainingFeatureIds.empty() )
      mFeatureIterator = mRemainingFeatureIds.at( 0 );

    mDeferredFeaturesInFilterRectCheck = false;

    if ( !( mRequest.flags() & QgsFeatureRequest::ExactIntersect ) )
    {
      // discard the filter rect - we know that the features in mRemainingFeatureIds are guaranteed
      // to be intersecting the rect, so avoid any extra unnecessary checks
      mFilterRect = QgsRectangle();
    }
  }

  if ( !mFeatureIdList.empty() && mRemainingFeatureIds.empty() )
    return false;

  switch ( mRequest.filterType() )
  {
    case QgsFeatureRequest::FilterFid:
    {
      if ( mRemainingFeatureIds.empty() )
        return false;

      bool result = mSource->sharedData()->getFeature( mRequest.filterFid(), f, mInterruptionChecker );
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

    case QgsFeatureRequest::FilterFids:
    case QgsFeatureRequest::FilterExpression:
    case QgsFeatureRequest::FilterNone:
    {
      while ( mFeatureIterator < mSource->sharedData()->objectIdCount() )
      {
        if ( mInterruptionChecker && mInterruptionChecker->isCanceled() )
          return false;

        if ( !mFeatureIdList.empty() && mRemainingFeatureIds.empty() )
          return false;

        bool success = mSource->sharedData()->getFeature( mFeatureIterator, f, mInterruptionChecker );

        if ( !mFeatureIdList.empty() )
        {
          mRemainingFeatureIds.removeAll( mFeatureIterator );
          if ( !mRemainingFeatureIds.empty() )
            mFeatureIterator = mRemainingFeatureIds.at( 0 );
        }
        else
        {
          ++mFeatureIterator;
        }

        if ( success && !mFilterRect.isNull() )
        {
          if ( !f.hasGeometry() )
            success = false;
          else
          {
            if ( mRequest.spatialFilterType() == Qgis::SpatialFilterType::BoundingBox && mRequest.flags() & QgsFeatureRequest::ExactIntersect )
            {
              // exact intersection check requested
              if ( !f.geometry().intersects( mFilterRect ) )
                success = false;
            }
            else
            {
              // bounding box intersect check only
              if ( !f.geometry().boundingBoxIntersects( mFilterRect ) )
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
  mRemainingFeatureIds = mFeatureIdList;
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
