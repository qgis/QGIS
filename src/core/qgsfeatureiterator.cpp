/***************************************************************************
    qgsfeatureiterator.cpp
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
#include "qgsfeatureiterator.h"
#include "qgslogger.h"

#include "qgssimplifymethod.h"
#include "qgsexception.h"
#include "qgslinestring.h"
#include "qgsexpressionsorter_p.h"
#include "qgsfeedback.h"
#include "qgscoordinatetransform.h"

QgsAbstractFeatureIterator::QgsAbstractFeatureIterator( const QgsFeatureRequest &request )
  : mRequest( request )
{
}

bool QgsAbstractFeatureIterator::nextFeature( QgsFeature &f )
{
  bool dataOk = false;
  if ( mRequest.limit() >= 0 && mFetchedCount >= mRequest.limit() )
  {
    return false;
  }

  if ( mRequest.feedback() && mRequest.feedback()->isCanceled() )
    return false;

  if ( mUseCachedFeatures )
  {
    if ( mFeatureIterator != mCachedFeatures.constEnd() )
    {
      f = mFeatureIterator->mFeature;
      ++mFeatureIterator;
      dataOk = true;
    }
    else
    {
      dataOk = false;
      // even the zombie dies at this point...
      mZombie = false;
    }
  }
  else
  {
    switch ( mRequest.filterType() )
    {
      case QgsFeatureRequest::FilterExpression:
        dataOk = nextFeatureFilterExpression( f );
        break;

      case QgsFeatureRequest::FilterFids:
        dataOk = nextFeatureFilterFids( f );
        break;

      default:
        dataOk = fetchFeature( f );
        break;
    }
  }

  if ( dataOk )
    mFetchedCount++;

  return dataOk;
}

bool QgsAbstractFeatureIterator::nextFeatureFilterExpression( QgsFeature &f )
{
  while ( fetchFeature( f ) )
  {
    mRequest.expressionContext()->setFeature( f );
    if ( mRequest.filterExpression()->evaluate( mRequest.expressionContext() ).toBool() )
      return true;
  }
  return false;
}

bool QgsAbstractFeatureIterator::nextFeatureFilterFids( QgsFeature &f )
{
  while ( fetchFeature( f ) )
  {
    if ( mRequest.filterFids().contains( f.id() ) )
      return true;
  }
  return false;
}

void QgsAbstractFeatureIterator::geometryToDestinationCrs( QgsFeature &feature, const QgsCoordinateTransform &transform ) const
{
  if ( transform.isValid() && feature.hasGeometry() )
  {
    try
    {
      QgsGeometry g = feature.geometry();
      g.transform( transform );
      feature.setGeometry( g );
    }
    catch ( QgsCsException & )
    {
      // transform error
      if ( mRequest.transformErrorCallback() )
      {
        mRequest.transformErrorCallback()( feature );
      }
      // remove geometry - we can't reproject so better not return a geometry in a different crs
      feature.clearGeometry();
    }
  }
}

QgsAbstractFeatureIterator::RequestToSourceCrsResult QgsAbstractFeatureIterator::updateRequestToSourceCrs( QgsFeatureRequest &request, const QgsCoordinateTransform &transform ) const
{
  if ( transform.isShortCircuited() )
    return RequestToSourceCrsResult::Success; // nothing to do

  switch ( request.spatialFilterType() )
  {
    case Qgis::SpatialFilterType::NoFilter:
      return RequestToSourceCrsResult::Success;

    case Qgis::SpatialFilterType::BoundingBox:
    {
      QgsRectangle newRect = transform.transformBoundingBox( request.filterRect(), Qgis::TransformDirection::Reverse );
      request.setFilterRect( newRect );
      return RequestToSourceCrsResult::Success;
    }
    case Qgis::SpatialFilterType::DistanceWithin:
    {
      // we can't safely handle a distance within query, as we cannot transform the
      // static within tolerance distance from one CRS to a static distance in a different CRS.

      // in this case we transform the request's distance within requirement to a "worst case" bounding box filter, so
      // that the request itself can still take advantage of spatial indices even when we have to do the distance within check locally
      QgsRectangle newRect = transform.transformBoundingBox( request.filterRect(), Qgis::TransformDirection::Reverse );
      request.setFilterRect( newRect );

      return RequestToSourceCrsResult::DistanceWithinMustBeCheckedManually;
    }
  }

  BUILTIN_UNREACHABLE
}

QgsRectangle QgsAbstractFeatureIterator::filterRectToSourceCrs( const QgsCoordinateTransform &transform ) const
{
  if ( mRequest.filterRect().isNull() )
    return QgsRectangle();

  QgsCoordinateTransform extentTransform = transform;
  extentTransform.setBallparkTransformsAreAppropriate( true );
  return extentTransform.transformBoundingBox( mRequest.filterRect(), Qgis::TransformDirection::Reverse );
}

void QgsAbstractFeatureIterator::ref()
{
  // Prepare if required the simplification of geometries to fetch:
  // This code runs here because of 'prepareSimplification()' is virtual and it can be overridden
  // in inherited iterators who change the default behavior.
  // It would be better to call this method in the constructor enabling virtual-calls as it is described by example at:
  // http://www.parashift.com/c%2B%2B-faq-lite/calling-virtuals-from-ctor-idiom.html
  if ( refs == 0 )
  {
    prepareSimplification( mRequest.simplifyMethod() );

    // Should be called as last preparation step since it possibly will already fetch all features
    setupOrderBy( mRequest.orderBy() );
  }
  refs++;
}

void QgsAbstractFeatureIterator::deref()
{
  refs--;
  if ( !refs )
    delete this;
}

bool QgsAbstractFeatureIterator::compileFailed() const
{
  return mCompileFailed;
}

bool QgsAbstractFeatureIterator::prepareSimplification( const QgsSimplifyMethod &simplifyMethod )
{
  Q_UNUSED( simplifyMethod )
  return false;
}

void QgsAbstractFeatureIterator::setupOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys )
{
  // Let the provider try using an efficient order by strategy first
  if ( !orderBys.isEmpty() && !prepareOrderBy( orderBys ) )
  {
    // No success from the provider

    // Prepare the expressions
    QList<QgsFeatureRequest::OrderByClause> preparedOrderBys( orderBys );
    QList<QgsFeatureRequest::OrderByClause>::iterator orderByIt( preparedOrderBys.begin() );

    QgsExpressionContext *expressionContext( mRequest.expressionContext() );
    do
    {
      orderByIt->prepare( expressionContext );
    }
    while ( ++orderByIt != preparedOrderBys.end() );

    // Fetch all features
    QgsIndexedFeature indexedFeature;
    indexedFeature.mIndexes.resize( preparedOrderBys.size() );

    while ( nextFeature( indexedFeature.mFeature ) )
    {
      expressionContext->setFeature( indexedFeature.mFeature );
      int i = 0;
      const auto constPreparedOrderBys = preparedOrderBys;
      for ( const QgsFeatureRequest::OrderByClause &orderBy : constPreparedOrderBys )
      {
        indexedFeature.mIndexes.replace( i++, orderBy.expression().evaluate( expressionContext ) );
      }

      // We need all features, to ignore the limit for this pre-fetch
      // keep the fetched count at 0.
      mFetchedCount = 0;
      mCachedFeatures.append( indexedFeature );
    }

    std::sort( mCachedFeatures.begin(), mCachedFeatures.end(), QgsExpressionSorter( preparedOrderBys ) );

    mFeatureIterator = mCachedFeatures.constBegin();
    mUseCachedFeatures = true;
    // The real iterator is closed, we are only serving cached features
    mZombie = true;
  }
}

bool QgsAbstractFeatureIterator::providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const
{
  Q_UNUSED( methodType )
  return false;
}

bool QgsAbstractFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys )
{
  Q_UNUSED( orderBys )
  return false;
}

void QgsAbstractFeatureIterator::setInterruptionChecker( QgsFeedback * )
{
}

///////

QgsFeatureIterator &QgsFeatureIterator::operator=( const QgsFeatureIterator &other )
{
  if ( this != &other )
  {
    if ( mIter )
      mIter->deref();
    mIter = other.mIter;
    if ( mIter )
      mIter->ref();
  }
  return *this;
}

bool QgsFeatureIterator::isValid() const
{
  return mIter && mIter->isValid();
}
