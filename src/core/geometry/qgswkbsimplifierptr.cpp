/***************************************************************************
    qgswkbsimplifierptr.cpp
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Alvaro Huarte
    email                : http://wiki.osgeo.org/wiki/Alvaro_Huarte
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswkbsimplifierptr.h"
#include "qgsgeometry.h"
#include "qgssimplifymethod.h"
#include "qgsmaptopixelgeometrysimplifier.h"

QgsConstWkbSimplifierPtr::QgsConstWkbSimplifierPtr( const unsigned char *p, int size, const QgsVectorSimplifyMethod &simplifyMethod )
    : QgsConstWkbPtr( p, size )
    , mSimplifyMethod( simplifyMethod )
{
}

const QgsConstWkbPtr &QgsConstWkbSimplifierPtr::operator>>( QPointF &point ) const
{
  return QgsConstWkbPtr::operator>>( point );
}

const QgsConstWkbPtr &QgsConstWkbSimplifierPtr::operator>>( QPolygonF &points ) const
{
  if ( mSimplifyMethod.simplifyHints() != QgsVectorSimplifyMethod::NoSimplification && mSimplifyMethod.forceLocalOptimization() )
  {
    int simplifyHints = mSimplifyMethod.simplifyHints() | QgsMapToPixelSimplifier::SimplifyEnvelope;
    QgsMapToPixelSimplifier::SimplifyAlgorithm simplifyAlgorithm = static_cast< QgsMapToPixelSimplifier::SimplifyAlgorithm >( mSimplifyMethod.simplifyAlgorithm() );

    QgsConstWkbPtr wkbPtr = *this;

    if ( QgsMapToPixelSimplifier::simplifyPoints( mWkbType, wkbPtr, points, simplifyHints, mSimplifyMethod.tolerance(), simplifyAlgorithm ) )
    {
      mP = const_cast< unsigned char * >(( const unsigned char * ) wkbPtr );
      return *this;
    }
  }
  QgsConstWkbPtr::operator>>( points );
  return *this;
}
