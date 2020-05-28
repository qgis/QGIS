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
    mSubsetExpression = qgis::make_unique< QgsExpression >( mSource->mSubsetString );
    mSubsetExpression->prepare( &mSource->mExpressionContext );
  }

  if ( !mFilterRect.isNull() && mRequest.flags() & QgsFeatureRequest::ExactIntersect )
  {
    mSelectRectGeom = QgsGeometry::fromRect( mFilterRect );
    mSelectRectEngine.reset( QgsGeometry::createGeometryEngine( mSelectRectGeom.constGet() ) );
    mSelectRectEngine->prepareGeometry();
  }

  // if there's spatial index, use it!
  // (but don't use it when selection rect is not specified)
  if ( !mFilterRect.isNull() && mSource->mSpatialIndex )
  {
    mUsingFeatureIdList = true;
    mFeatureIdList = mSource->mSpatialIndex->intersects( mFilterRect );
    QgsDebugMsg( "Features returned by spatial index: " + QString::number( mFeatureIdList.count() ) );
  }
  else if ( mRequest.filterType() == QgsFeatureRequest::FilterFid )
  {
    mUsingFeatureIdList = true;
    QgsFeatureMap::const_iterator it = mSource->mFeatures.constFind( mRequest.filterFid() );
    if ( it != mSource->mFeatures.constEnd() )
      mFeatureIdList.append( mRequest.filterFid() );
  }
  else if ( mRequest.filterType() == QgsFeatureRequest::FilterFids )
  {
    mUsingFeatureIdList = true;
    mFeatureIdList = mRequest.filterFids().toList();
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
  QgsFeature candidate;
  while ( mFeatureIdListIterator != mFeatureIdList.constEnd() )
  {
    candidate = mSource->mFeatures.value( *mFeatureIdListIterator );
    if ( !mFilterRect.isNull() )
    {
      if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
      {
        // do exact check in case we're doing intersection
        if ( candidate.hasGeometry() && mSelectRectEngine->intersects( candidate.geometry().constGet() ) )
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
        if ( candidate.hasGeometry() && candidate.geometry().boundingBoxIntersects( mFilterRect ) )
          hasFeature = true;
      }
    }
    else
      hasFeature = true;

    if ( hasFeature && mSubsetExpression )
    {
      mSource->mExpressionContext.setFeature( candidate );
      if ( !mSubsetExpression->evaluate( &mSource->mExpressionContext ).toBool() )
        hasFeature = false;
    }

    if ( hasFeature )
      break;

    ++mFeatureIdListIterator;
  }

  // copy feature
  if ( hasFeature )
  {
    feature = candidate;
    ++mFeatureIdListIterator;
  }
  else
    close();

  if ( hasFeature )
  {
    feature.setFields( mSource->mFields ); // allow name-based attribute lookups
    geometryToDestinationCrs( feature, mTransform );
  }

  return hasFeature;
}


bool QgsMemoryFeatureIterator::nextFeatureTraverseAll( QgsFeature &feature )
{
  bool hasFeature = false;

  // option 2: traversing the whole layer
  while ( mSelectIterator != mSource->mFeatures.constEnd() )
  {
    if ( mFilterRect.isNull() )
    {
      // selection rect empty => using all features
      hasFeature = true;
    }
    else
    {
      if ( mRequest.flags() & QgsFeatureRequest::ExactIntersect )
      {
        // using exact test when checking for intersection
        if ( mSelectIterator->hasGeometry() && mSelectRectEngine->intersects( mSelectIterator->geometry().constGet() ) )
          hasFeature = true;
      }
      else
      {
        // check just bounding box against rect when not using intersection
        if ( mSelectIterator->hasGeometry() && mSelectIterator->geometry().boundingBox().intersects( mFilterRect ) )
          hasFeature = true;
      }
    }

    if ( mSubsetExpression )
    {
      mSource->mExpressionContext.setFeature( *mSelectIterator );
      if ( !mSubsetExpression->evaluate( &mSource->mExpressionContext ).toBool() )
        hasFeature = false;
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
    feature.setFields( mSource->mFields ); // allow name-based attribute lookups
    geometryToDestinationCrs( feature, mTransform );
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

  mClosed = true;
  return true;
}

// -------------------------

QgsMemoryFeatureSource::QgsMemoryFeatureSource( const QgsMemoryProvider *p )
  : mFields( p->mFields )
  , mFeatures( p->mFeatures )
  , mSpatialIndex( p->mSpatialIndex ? qgis::make_unique< QgsSpatialIndex >( *p->mSpatialIndex ) : nullptr ) // just shallow copy
  , mSubsetString( p->mSubsetString )
  , mCrs( p->mCrs )
{
  mExpressionContext << QgsExpressionContextUtils::globalScope()
                     << QgsExpressionContextUtils::projectScope( QgsProject::instance() );
  mExpressionContext.setFields( mFields );
}

QgsFeatureIterator QgsMemoryFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsMemoryFeatureIterator( this, false, request ) );
}

///@endcond PRIVATE
