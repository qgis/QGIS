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
}

QgsAfsFeatureIterator::~QgsAfsFeatureIterator()
{
  close();
}

bool QgsAfsFeatureIterator::fetchFeature( QgsFeature &f )
{
  if ( mClosed )
    return false;

  if ( mFeatureIterator >= mSource->sharedData()->featureCount() )
    return false;

  bool fetchGeometries = ( mRequest.flags() & QgsFeatureRequest::NoGeometry ) == 0;
  QgsAttributeList fetchAttribures;
  if ( ( mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes ) != 0 )
    fetchAttribures = mRequest.subsetOfAttributes();
  else
  {
    for ( int i = 0; i < mSource->sharedData()->fields().size(); ++i )
      fetchAttribures.append( i );
  }

  if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    bool result = mSource->sharedData()->getFeature( mRequest.filterFid(), f, fetchGeometries, fetchAttribures );
    geometryToDestinationCrs( f, mTransform );
    return result;
  }
  else
  {
    QgsRectangle filterRect = mSource->sharedData()->extent();
    if ( !mRequest.filterRect().isNull() )
      filterRect = filterRect.intersect( &mFilterRect );
    while ( mFeatureIterator < mSource->sharedData()->featureCount() )
    {
      bool success = mSource->sharedData()->getFeature( mFeatureIterator, f, fetchGeometries, fetchAttribures, filterRect );
      ++mFeatureIterator;
      if ( !success )
        continue;
      geometryToDestinationCrs( f, mTransform );
      return true;
    }
  }
  return false;
}

bool QgsAfsFeatureIterator::rewind()
{
  if ( mClosed )
    return false;
  mFeatureIterator = 0;
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
