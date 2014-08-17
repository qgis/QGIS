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

#include "qgsgeometrysimplifier.h"
#include "qgssimplifymethod.h"

QgsAbstractFeatureIterator::QgsAbstractFeatureIterator( const QgsFeatureRequest& request )
    : mRequest( request )
    , mClosed( false )
    , refs( 0 )
    , mGeometrySimplifier( NULL )
    , mLocalSimplification( false )
{
}

QgsAbstractFeatureIterator::~QgsAbstractFeatureIterator()
{
  delete mGeometrySimplifier;
  mGeometrySimplifier = NULL;
}

bool QgsAbstractFeatureIterator::nextFeature( QgsFeature& f )
{
  bool dataOk = false;

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

  // simplify the geometry using the simplifier configured
  if ( dataOk && mLocalSimplification )
  {
    QgsGeometry* geometry = f.geometry();
    if ( geometry )
      simplify( f );
  }
  return dataOk;
}

bool QgsAbstractFeatureIterator::nextFeatureFilterExpression( QgsFeature& f )
{
  while ( fetchFeature( f ) )
  {
    if ( mRequest.filterExpression()->evaluate( f ).toBool() )
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
  mLocalSimplification = false;

  delete mGeometrySimplifier;
  mGeometrySimplifier = NULL;

  // setup the simplification of geometries to fetch
  if ( !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) && simplifyMethod.methodType() != QgsSimplifyMethod::NoSimplification && ( simplifyMethod.forceLocalOptimization() || !providerCanSimplify( simplifyMethod.methodType() ) ) )
  {
    mGeometrySimplifier = QgsSimplifyMethod::createGeometrySimplifier( simplifyMethod );
    mLocalSimplification = mGeometrySimplifier != NULL;
    return mLocalSimplification;
  }
  return false;
}

bool QgsAbstractFeatureIterator::providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const
{
  Q_UNUSED( methodType )
  return false;
}

bool QgsAbstractFeatureIterator::simplify( QgsFeature& feature )
{
  // simplify locally the geometry using the configured simplifier
  if ( mGeometrySimplifier )
  {
    QgsGeometry* geometry = feature.geometry();

    QGis::GeometryType geometryType = geometry->type();
    if ( geometryType == QGis::Line || geometryType == QGis::Polygon )
      return mGeometrySimplifier->simplifyGeometry( geometry );
  }
  return false;
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
