/***************************************************************************
     qgsgcppoint.h
     --------------------------------------
    Date                 : February 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscoordinatereferencesystem.h"
#include "qgsgcppoint.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include "qgslogger.h"

QgsGcpPoint::QgsGcpPoint( const QgsPointXY &sourcePoint, const QgsPointXY &destinationPoint, const QgsCoordinateReferenceSystem &destinationPointCrs, bool enabled )
  : mSourcePoint( sourcePoint )
  , mDestinationPoint( destinationPoint )
  , mDestinationCrs( destinationPointCrs )
  , mEnabled( enabled )
{

}

QgsCoordinateReferenceSystem QgsGcpPoint::destinationPointCrs() const
{
  return mDestinationCrs;
}

void QgsGcpPoint::setDestinationPointCrs( const QgsCoordinateReferenceSystem &crs )
{
  mDestinationCrs = crs;
}

QgsPointXY QgsGcpPoint::transformedDestinationPoint( const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context ) const
{
  const QgsCoordinateTransform transform( mDestinationCrs, targetCrs, context );
  try
  {
    return transform.transform( mDestinationPoint );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( QStringLiteral( "Error transforming destination point" ) );
    return mDestinationPoint;
  }
}
