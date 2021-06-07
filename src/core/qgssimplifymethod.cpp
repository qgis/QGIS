/***************************************************************************
    qgssimplifymethod.cpp
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Matthias Kuhn / Alvaro Huarte
    email                :
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
  QgsSimplifyMethod::MethodType methodType = simplifyMethod.methodType();

  // returns a geometry simplifier according to specified method
  if ( methodType == QgsSimplifyMethod::OptimizeForRendering )
  {
    int simplifyFlags = QgsMapToPixelSimplifier::SimplifyGeometry | QgsMapToPixelSimplifier::SimplifyEnvelope;
    return new QgsMapToPixelSimplifier( simplifyFlags, simplifyMethod.tolerance(), QgsMapToPixelSimplifier::Distance );
  }
  else if ( methodType == QgsSimplifyMethod::PreserveTopology )
  {
    return new QgsTopologyPreservingSimplifier( simplifyMethod.tolerance() );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Simplification method type (%1) is not recognised" ).arg( methodType ) );
    return nullptr;
  }
}
