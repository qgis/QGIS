/***************************************************************************
     qgsgeorefconfigdialog.h
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointxy.h"
#include "qgsgeorefdatapoint.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsproject.h"

#include "qgsgcplist.h"

void QgsGCPList::createGCPVectors( QVector<QgsPointXY> &sourcePoints, QVector<QgsPointXY> &destinationPoints, const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context ) const
{
  const int targetSize = countEnabledPoints();
  sourcePoints.clear();
  sourcePoints.reserve( targetSize );
  destinationPoints.clear();
  destinationPoints.reserve( targetSize );

  for ( const QgsGeorefDataPoint *pt : std::as_const( *this ) )
  {
    if ( !pt->isEnabled() )
      continue;

    sourcePoints.push_back( pt->sourcePoint() );
    if ( targetCrs.isValid() )
    {
      destinationPoints.push_back( pt->transformedDestinationPoint( targetCrs, context ) );
    }
    else
    {
      destinationPoints.push_back( pt->destinationPoint() );
    }
  }
}

int QgsGCPList::countEnabledPoints() const
{
  if ( isEmpty() )
    return 0;

  int s = 0;
  const_iterator it = begin();
  while ( it != end() )
  {
    if ( ( *it )->isEnabled() )
      s++;
    ++it;
  }
  return s;
}

QList<QgsGcpPoint> QgsGCPList::asPoints() const
{
  QList<QgsGcpPoint> res;
  res.reserve( size() );
  for ( QgsGeorefDataPoint *pt : *this )
  {
    res.append( QgsGcpPoint( pt->point() ) );
  }
  return res;
}
