/***************************************************************************
      qgsafsfeatureiterator.cpp
      -------------------------
    begin                : Jun 03, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsafsfeatureiterator.h"
#include "qgsspatialindex.h"
#include "qgsafsshareddata.h"
#include "qgsmessagelog.h"
#include "geometry/qgsgeometry.h"
#include "qgsexception.h"

QgsAfsFeatureSource::QgsAfsFeatureSource( const std::shared_ptr<QgsAfsSharedData> &sharedData )
  : mSharedData( sharedData )
{
}

QgsFeatureIterator QgsAfsFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsAfsFeatureIterator( this, false, request ) );
}

QgsAfsSharedData *QgsAfsFeatureSource::sharedData() const
{
  return mSharedData.get();
}

///////////////////////////////////////////////////////////////////////////////

QgsAfsFeatureIterator::QgsAfsFeatureIterator( QgsAfsFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsAfsFeatureSource>( source, ownSource, request )
{
  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->sharedData()->crs() )
  {
    mTransform = QgsCoordinateTransform( mSource->sharedData()->crs(), mRequest.destinationCrs(), mRequest.transformContext() );
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

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFids )
  {
    mUsingFeatureIdList = true;
    mFeatureIdList = mRequest.filterFids();
    mRemainingFeatureIds = mFeatureIdList;
  }
  else if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    mFeatureIdList.insert( mRequest.filterFid() );
    mRemainingFeatureIds = mFeatureIdList;
  }
}

QgsAfsFeatureIterator::~QgsAfsFeatureIterator()
{
  close();
}

bool QgsAfsFeatureIterator::fetchFeature( QgsFeature &f )
{
  f.setValid( false );

  if ( mClosed )
    return false;

  if ( mFeatureIterator >= mSource->sharedData()->featureCount() )
    return false;

  switch ( mRequest.filterType() )
  {
    case QgsFeatureRequest::FilterFid:
    {
      if ( mRemainingFeatureIds.empty() )
        return false;

      bool result = mSource->sharedData()->getFeature( mRequest.filterFid(), f );
      geometryToDestinationCrs( f, mTransform );
      f.setValid( result );
      mRemainingFeatureIds.remove( f.id() );
      return result;
    }

    case QgsFeatureRequest::FilterFids:
    case QgsFeatureRequest::FilterExpression:
    case QgsFeatureRequest::FilterNone:
    {
      QgsRectangle filterRect;
      if ( !mRequest.filterRect().isNull() )
      {
        filterRect = mSource->sharedData()->extent();
        filterRect = filterRect.intersect( &mFilterRect );
      }
      while ( mFeatureIterator < mSource->sharedData()->featureCount() )
      {
        bool success = mSource->sharedData()->getFeature( mFeatureIterator, f, filterRect );
        ++mFeatureIterator;
        if ( !success )
          continue;
        if ( mUsingFeatureIdList )
        {
          if ( !mRemainingFeatureIds.contains( f.id() ) )
            continue;
          else
            mRemainingFeatureIds.remove( f.id() );
        }
        geometryToDestinationCrs( f, mTransform );
        f.setValid( true );
        return true;
      }
      return false;
    }
  }
  return false;
}

bool QgsAfsFeatureIterator::rewind()
{
  if ( mClosed )
    return false;
  mFeatureIterator = 0;
  mRemainingFeatureIds = mFeatureIdList;
  return true;
}

bool QgsAfsFeatureIterator::close()
{
  if ( mClosed )
    return false;
  iteratorClosed();
  mClosed = true;
  return true;
}
