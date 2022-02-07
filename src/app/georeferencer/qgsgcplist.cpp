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

QgsGCPList::QgsGCPList( const QgsGCPList &list )
  :  QList<QgsGeorefDataPoint *>()
{
  clear();
  QgsGCPList::const_iterator it = list.constBegin();
  for ( ; it != list.constEnd(); ++it )
  {
    QgsGeorefDataPoint *pt = new QgsGeorefDataPoint( **it );
    append( pt );
  }
}

void QgsGCPList::createGCPVectors( QVector<QgsPointXY> &sourceCoordinates, QVector<QgsPointXY> &destinationCoordinates, const QgsCoordinateReferenceSystem &targetCrs )
{
  const int targetSize = countEnabledPoints();
  sourceCoordinates.clear();
  sourceCoordinates.reserve( targetSize );
  destinationCoordinates.clear();
  destinationCoordinates.reserve( targetSize );

  for ( QgsGeorefDataPoint *pt : std::as_const( *this ) )
  {
    if ( !pt->isEnabled() )
      continue;

    sourceCoordinates.push_back( pt->sourceCoords() );
    if ( targetCrs.isValid() )
    {
      try
      {
        QgsPointXY transCoords = QgsCoordinateTransform( pt->destinationCrs(), targetCrs,
                                 QgsProject::instance() ).transform( pt->destinationMapCoords() );
        destinationCoordinates.push_back( transCoords );
        pt->setTransCoords( transCoords );
      }
      catch ( const QgsException & )
      {
        destinationCoordinates.push_back( pt->destinationMapCoords() );
      }
    }
    else
      destinationCoordinates.push_back( pt->destinationMapCoords() );
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

QgsGCPList &QgsGCPList::operator =( const QgsGCPList &list )
{
  clear();
  QgsGCPList::const_iterator it = list.constBegin();
  for ( ; it != list.constEnd(); ++it )
  {
    QgsGeorefDataPoint *pt = new QgsGeorefDataPoint( **it );
    append( pt );
  }
  return *this;
}
