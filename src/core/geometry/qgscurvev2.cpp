/***************************************************************************
                         qgscurvev2.cpp
                         --------------
    begin                : November 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscurvev2.h"
#include "qgslinestringv2.h"

QgsCurveV2::QgsCurveV2(): QgsAbstractGeometryV2()
{}

QgsCurveV2::~QgsCurveV2()
{}

bool QgsCurveV2::isClosed() const
{
  return ( numPoints() > 0 && ( startPoint() == endPoint() ) );
}

bool QgsCurveV2::isRing() const
{
  return ( isClosed() && numPoints() >= 4 );
}

void QgsCurveV2::coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const
{
  coord.clear();
  QList<QgsPointV2> pts;
  points( pts );
  QList< QList<QgsPointV2> > ptsList;
  ptsList.append( pts );
  coord.append( ptsList );
}

bool QgsCurveV2::nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const
{
  if ( id.vertex < 0 )
  {
    id.vertex = 0;
    if ( id.part < 0 )
    {
      id.part = 0;
    }
    if ( id.ring < 0 )
    {
      id.ring = 0;
    }
  }
  else
  {
    if ( id.vertex + 1 >= numPoints() )
    {
      return false;
    }
    ++id.vertex;
  }
  return pointAt( id.vertex, vertex, id.type );
}

double QgsCurveV2::area() const
{
  if ( !isClosed() )
  {
    return 0.0;
  }

  double area = 0.0;
  sumUpArea( area );
  return qAbs( area );
}

QgsAbstractGeometryV2* QgsCurveV2::segmentize() const
{
  return curveToLine();
}
