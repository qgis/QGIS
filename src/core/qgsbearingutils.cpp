/***************************************************************************
                             qgsbearingutils.cpp
                             -------------------
    begin                : October 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbearingutils.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgspointxy.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"

double QgsBearingUtils::bearingTrueNorth( const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext  &transformContext, const QgsPointXY &point )
{
  // step 1 - transform point into WGS84 geographic crs
  const QgsCoordinateTransform transform( crs, QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), transformContext );

  if ( !transform.isValid() )
  {
    //raise
    throw QgsException( QObject::tr( "Could not create transform to calculate true north" ) );
  }

  if ( transform.isShortCircuited() )
    return 0.0;

  const QgsPointXY p1 = transform.transform( point );

  // shift point a tiny bit north
  QgsPointXY p2 = p1;
  p2.setY( p2.y() + 0.000001 );

  //transform back
  const QgsPointXY p3 = transform.transform( p2, Qgis::TransformDirection::Reverse );

  // find bearing from point to p3
  return point.azimuth( p3 );
}
