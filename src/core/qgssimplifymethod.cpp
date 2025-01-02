/***************************************************************************
    qgssimplifymethod.cpp
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Matthias Kuhn / Alvaro Huarte
    email                :
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssimplifymethod.h"
#include "qgslogger.h"
#include "qgsgeometrysimplifier.h"
#include "qgsmaptopixelgeometrysimplifier.h"


void QgsSimplifyMethod::setMethodType( MethodType methodType )
{
  mMethodType = methodType;
}

void QgsSimplifyMethod::setTolerance( double tolerance )
{
  mTolerance = tolerance;
}

void QgsSimplifyMethod::setForceLocalOptimization( bool localOptimization )
{
  mForceLocalOptimization = localOptimization;
}

QgsAbstractGeometrySimplifier *QgsSimplifyMethod::createGeometrySimplifier( const QgsSimplifyMethod &simplifyMethod )
{
  const QgsSimplifyMethod::MethodType methodType = simplifyMethod.methodType();

  // returns a geometry simplifier according to specified method
  if ( methodType == QgsSimplifyMethod::OptimizeForRendering )
  {
    const int simplifyFlags = QgsMapToPixelSimplifier::SimplifyGeometry | QgsMapToPixelSimplifier::SimplifyEnvelope;
    return new QgsMapToPixelSimplifier( simplifyFlags, simplifyMethod.tolerance(), Qgis::VectorSimplificationAlgorithm::Distance );
  }
  else if ( methodType == QgsSimplifyMethod::PreserveTopology )
  {
    return new QgsTopologyPreservingSimplifier( simplifyMethod.tolerance() );
  }
  else
  {
    QgsDebugError( QStringLiteral( "Simplification method type (%1) is not recognised" ).arg( methodType ) );
    return nullptr;
  }
}

bool QgsSimplifyMethod::operator==( const QgsSimplifyMethod &v ) const
{
  return
    mMethodType == v.mMethodType &&
    mTolerance == v.mTolerance &&
    mThreshold == v.mThreshold &&
    mForceLocalOptimization == v.mForceLocalOptimization;
}

bool QgsSimplifyMethod::operator!=( const QgsSimplifyMethod &v ) const
{
  return !( v == *this );
}
