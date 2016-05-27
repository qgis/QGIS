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

#include "qgsexpressionsorter.h"

QgsAbstractFeatureIterator::QgsAbstractFeatureIterator( const QgsFeatureRequest& request )
    : mRequest( request )
    , mClosed( false )
    , mZombie( false )
    , refs( 0 )
    , mFetchedCount( 0 )
    , mCompileStatus( NoCompilation )
    , mUseCachedFeatures( false )
{
}

QgsAbstractFeatureIterator::~QgsAbstractFeatureIterator()
{
}

bool QgsAbstractFeatureIterator::nextFeature( QgsFeature& f )
{
  bool dataOk = false;
  if ( mRequest.limit() >= 0 && mFetchedCount >= mRequest.limit() )
  {
    return false;
  }

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

bool QgsAbstractFeatureIterator::nextFeatureFilterExpression( QgsFeature& f )
{
  while ( fetchFeature( f ) )
  {
    mRequest.expressionContext()->setFeature( f );
    if ( mRequest.filterExpression()->evaluate( mRequest.expressionContext() ).toBool() )
      return true;
  }
  return false;
}

bool QgsAbstractFeatureIterator::nextFeatureFilterFids( QgsFeature& f )
{
  while ( fetchFeature( f ) )
  {
    if ( mRequest.filterFids().contains( f.id() ) )
      return true;
  }
  return false;
}

void QgsAbstractFeatureIterator::ref()
{
  // Prepare if required the simplification of geometries to fetch:
  // This code runs here because of 'prepareSimplification()' is virtual and it can be overrided
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

bool QgsAbstractFeatureIterator::prepareSimplification( const QgsSimplifyMethod& simplifyMethod )
{
  Q_UNUSED( simplifyMethod );
  return false;
}

void QgsAbstractFeatureIterator::setupOrderBy( const QList<QgsFeatureRequest::OrderByClause>& orderBys )
{
  // Let the provider try using an efficient order by strategy first
  if ( !orderBys.isEmpty() && !prepareOrderBy( orderBys ) )
  {
    // No success from the provider

    // Prepare the expressions
    QList<QgsFeatureRequest::OrderByClause> preparedOrderBys( orderBys );
    QList<QgsFeatureRequest::OrderByClause>::iterator orderByIt( preparedOrderBys.begin() );

    QgsExpressionContext* expressionContext( mRequest.expressionContext() );
    do
    {
      orderByIt->expression().prepare( expressionContext );
    }
    while ( ++orderByIt != preparedOrderBys.end() );

    // Fetch all features
    QgsIndexedFeature indexedFeature;
    indexedFeature.mIndexes.resize( preparedOrderBys.size() );

    while ( nextFeature( indexedFeature.mFeature ) )
    {
      expressionContext->setFeature( indexedFeature.mFeature );
      int i = 0;
      Q_FOREACH ( const QgsFeatureRequest::OrderByClause& orderBy, preparedOrderBys )
      {
        indexedFeature.mIndexes.replace( i++, orderBy.expression().evaluate( expressionContext ) );
      }

      // We need all features, to ignore the limit for this pre-fetch
      // keep the fetched count at 0.
      mFetchedCount = 0;
      mCachedFeatures.append( indexedFeature );
    }

    qSort( mCachedFeatures.begin(), mCachedFeatures.end(), QgsExpressionSorter( preparedOrderBys ) );

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

bool QgsAbstractFeatureIterator::prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause>& orderBys )
{
  Q_UNUSED( orderBys )
  return false;
}

void QgsAbstractFeatureIterator::setInterruptionChecker( QgsInterruptionChecker* )
{
}

///////

QgsFeatureIterator& QgsFeatureIterator::operator=( const QgsFeatureIterator & other )
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
