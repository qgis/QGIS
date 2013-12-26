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
#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgssimplifymethod.h"

QgsAbstractFeatureIterator::QgsAbstractFeatureIterator( const QgsFeatureRequest& request )
    : mRequest( request )
    , mClosed( false )
    , refs( 0 )
    , mGeometrySimplifier( NULL )
{
}

QgsAbstractFeatureIterator::~QgsAbstractFeatureIterator()
{
  if ( mGeometrySimplifier )
  {
    delete mGeometrySimplifier;
    mGeometrySimplifier = NULL;
  }
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

  // simplify locally the geometry using the simplifier defined in constructor
  if ( dataOk && mGeometrySimplifier )
  {
    QgsGeometry* geometry = f.geometry();

    if ( geometry ) 
    {
      QGis::GeometryType geometryType = geometry->type();
      if ( geometryType == QGis::Line || geometryType == QGis::Polygon ) mGeometrySimplifier->simplifyGeometry( geometry );
    }
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
  refs++;
}

void QgsAbstractFeatureIterator::deref()
{
  refs--;
  if ( !refs )
    delete this;
}

bool QgsAbstractFeatureIterator::prepareLocalSimplification()
{
  const QgsSimplifyMethod& simplifyMethod = mRequest.simplifyMethod();

  if ( mGeometrySimplifier )
  {
    delete mGeometrySimplifier;
    mGeometrySimplifier = NULL;
  }

  // setup the local simplification of geometries to fetch, it uses the settings of current FeatureRequest
  if ( simplifyMethod.methodType() != QgsSimplifyMethod::NoSimplification && simplifyMethod.forceLocalOptimization() && !( mRequest.flags() & QgsFeatureRequest::NoGeometry ) )
  {
    QgsSimplifyMethod::MethodType methodType = simplifyMethod.methodType();

    if ( methodType == QgsSimplifyMethod::OptimizeForRendering )
    {
      int simplifyFlags = QgsMapToPixelSimplifier::SimplifyGeometry | QgsMapToPixelSimplifier::SimplifyEnvelope;
      mGeometrySimplifier = new QgsMapToPixelSimplifier( simplifyFlags, simplifyMethod.tolerance() );
      return true;
    }
    else
    if ( methodType == QgsSimplifyMethod::PreserveTopology )
    {
      mGeometrySimplifier = new QgsTopologyPreservingSimplifier( simplifyMethod.tolerance() );
      return true;
    }
    else
    {
      QgsDebugMsg( QString( "Simplification method type (%1) is not recognised" ).arg( methodType ) );
    }
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
