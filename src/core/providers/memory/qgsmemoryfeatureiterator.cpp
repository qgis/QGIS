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
#include "qgsgeometryengine.h"
#include "qgslogger.h"
#include "qgsspatialindex.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgsexception.h"
#include "qgsexpressioncontextutils.h"

///@cond PRIVATE

QgsMemoryFeatureIterator::QgsMemoryFeatureIterator( QgsMemoryFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsMemoryFeatureSource>( source, ownSource, request )
{
  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs )
  {
    mTransform = QgsCoordinateTransform( mSource->mCrs, mRequest.destinationCrs(), mRequest.transformContext() );
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

  if ( !mSource->mSubsetString.isEmpty() )
  {
    mSubsetExpression = std::make_unique< QgsExpression >( mSource->mSubsetString );
    mSubsetExpression->prepare( mSource->expressionContext() );
  }

  // prepare spatial filter geometries for optimal speed
  switch ( mRequest.spatialFilterType() )
  {
    case Qgis::SpatialFilterType::NoFilter:
      break;

    case Qgis::SpatialFilterType::BoundingBox:
      if ( !mFilterRect.isNull() && mRequest.flags() & QgsFeatureRequest::ExactIntersect )
      {
        mSelectRectGeom = QgsGeometry::fromRect( mFilterRect );
        mSelectRectEngine.reset( QgsGeometry::createGeometryEngine( mSelectRectGeom.constGet() ) );
        mSelectRectEngine->prepareGeometry();
      }
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

  // if there's spatial index, use it!
  // (but don't use it when selection rect is not specified)
  if ( !mFilterRect.isNull() && mSource->mSpatialIndex )
  {
    mUsingFeatureIdList = true;
    mFeatureIdList = mSource->mSpatialIndex->intersects( mFilterRect );
    QgsDebugMsgLevel( "Features returned by spatial index: " + QString::number( mFeatureIdList.count() ), 2 );
  }
  else if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    mUsingFeatureIdList = true;
    const QgsFeatureMap::const_iterator it = mSource->mFeatures.constFind( mRequest.filterFid() );
    if ( it != mSource->mFeatures.constEnd() )
      mFeatureIdList.append( mRequest.filterFid() );
  }
  else if ( mRequest.filterType() == QgsFeatureRequest::FilterFids )
  {
    mUsingFeatureIdList = true;
    mFeatureIdList = qgis::setToList( mRequest.filterFids() );
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

bool QgsMemoryFeatureIterator::fetchFeature( QgsFeature &feature )
{
  feature.setValid( false );

  if ( mClosed )
    return false;

  if ( mUsingFeatureIdList )
    return nextFeatureUsingList( feature );
  else
    return nextFeatureTraverseAll( feature );
}


bool QgsMemoryFeatureIterator::nextFeatureUsingList( QgsFeature &feature )
{
  bool hasFeature = false;

  // option 1: we have a list of features to traverse
  while ( mFeatureIdListIterator != mFeatureIdList.constEnd() )
  {
    feature = mSource->mFeatures.value( *mFeatureIdListIterator );
    if ( !mFilterRect.isNull() )
    {
      if ( mRequest.spatialFilterType() == Qgis::SpatialFilterType::BoundingBox && mRequest.flags() & QgsFeatureRequest::ExactIntersect )
      {
        // do exact check in case we're doing intersection
        if ( feature.hasGeometry() && mSelectRectEngine->intersects( feature.geometry().constGet() ) )
          hasFeature = true;
      }
      else if ( mSource->mSpatialIndex )
      {
        // using a spatial index - so we already know that the bounding box intersects correctly
        hasFeature = true;
      }
      else
      {
        // do bounding box check if we aren't using a spatial index
        if ( feature.hasGeometry() && feature.geometry().boundingBoxIntersects( mFilterRect ) )
          hasFeature = true;
      }
    }
    else
      hasFeature = true;

    if ( hasFeature )
      feature.setFields( mSource->mFields ); // allow name-based attribute lookups

    if ( hasFeature && mSubsetExpression )
    {
      mSource->expressionContext()->setFeature( feature );
      if ( !mSubsetExpression->evaluate( mSource->expressionContext() ).toBool() )
        hasFeature = false;
    }

    if ( hasFeature )
    {
      // geometry must be in destination crs before we can perform distance within check
      geometryToDestinationCrs( feature, mTransform );
    }

    if ( hasFeature && mRequest.spatialFilterType() == Qgis::SpatialFilterType::DistanceWithin )
    {
      hasFeature = mDistanceWithinEngine->distance( feature.geometry().constGet() ) <= mRequest.distanceWithin();
    }

    ++mFeatureIdListIterator;
    if ( hasFeature )
      break;
  }

  feature.setValid( hasFeature );
  if ( !hasFeature )
  {
    close();
  }

  return hasFeature;
}


bool QgsMemoryFeatureIterator::nextFeatureTraverseAll( QgsFeature &feature )
{
  bool hasFeature = false;

  // option 2: traversing the whole layer
  while ( mSelectIterator != mSource->mFeatures.constEnd() )
  {
    hasFeature = false;
    feature = *mSelectIterator;
    if ( mFilterRect.isNull() )
    {
      // selection rect empty => using all features
      hasFeature = true;
    }
    else
    {
      if ( mRequest.spatialFilterType() == Qgis::SpatialFilterType::BoundingBox && mRequest.flags() & QgsFeatureRequest::ExactIntersect )
      {
        // using exact test when checking for intersection
        if ( feature.hasGeometry() && mSelectRectEngine->intersects( feature.geometry().constGet() ) )
          hasFeature = true;
      }
      else
      {
        // check just bounding box against rect when not using intersection
        if ( feature.hasGeometry() && feature.geometry().boundingBox().intersects( mFilterRect ) )
          hasFeature = true;
      }
    }

    if ( hasFeature && mSubsetExpression )
    {
      mSource->expressionContext()->setFeature( feature );
      if ( !mSubsetExpression->evaluate( mSource->expressionContext() ).toBool() )
        hasFeature = false;
    }

    if ( hasFeature )
    {
      // geometry must be in destination crs before we can perform distance within check
      geometryToDestinationCrs( feature, mTransform );
    }

    if ( hasFeature && mRequest.spatialFilterType() == Qgis::SpatialFilterType::DistanceWithin )
    {
      hasFeature = mDistanceWithinEngine->distance( feature.geometry().constGet() ) <= mRequest.distanceWithin();
    }

    ++mSelectIterator;
    if ( hasFeature )
      break;
  }

  // copy feature
  if ( hasFeature )
  {
    feature.setValid( true );
    feature.setFields( mSource->mFields ); // allow name-based attribute lookups
  }
  else
  {
    feature.setValid( false );
    close();
  }

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

  mClosed = true;
  return true;
}

// -------------------------

QgsMemoryFeatureSource::QgsMemoryFeatureSource( const QgsMemoryProvider *p )
  : mFields( p->mFields )
  , mFeatures( p->mFeatures )
  , mSpatialIndex( p->mSpatialIndex ? std::make_unique< QgsSpatialIndex >( *p->mSpatialIndex ) : nullptr ) // just shallow copy
  , mSubsetString( p->mSubsetString )
  , mCrs( p->mCrs )
{
}

QgsFeatureIterator QgsMemoryFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsMemoryFeatureIterator( this, false, request ) );
}

QgsExpressionContext *QgsMemoryFeatureSource::expressionContext()
{
  // lazy construct expression context -- it's not free to calculate, and is only used when
  // iterating over a memory layer with a subset string set
  if ( !mExpressionContext )
  {
    mExpressionContext = std::make_unique< QgsExpressionContext >(
                           QList<QgsExpressionContextScope *>()
                           << QgsExpressionContextUtils::globalScope()
                           << QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );
    mExpressionContext->setFields( mFields );
  }
  return mExpressionContext.get();
}

///@endcond PRIVATE
