/***************************************************************************
                              qgsspatialfilter.cpp
                              -----------------------
  begin                : Oct 19, 2012
  copyright            : (C) 2012 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsspatialfilter.h"
#include "qgsgeometry.h"
#include <QDomElement>

QgsSpatialFilter::QgsSpatialFilter(): QgsFilter(), mSpatialType( QgsSpatialFilter::UNKNOWN ), mGeom( 0 )
{
}

QgsSpatialFilter::QgsSpatialFilter( SPATIAL_TYPE st, QgsGeometry* geom ): QgsFilter(), mSpatialType( st ), mGeom( geom )
{
}

QgsSpatialFilter::~QgsSpatialFilter()
{
  delete mGeom;
}

bool QgsSpatialFilter::evaluate( const QgsFeature& f ) const
{
  if ( !mGeom )
  {
    return true;
  }

  QgsGeometry* geom = ( new QgsFeature( f ) )->geometry();
  switch ( mSpatialType )
  {
    case BBOX:
      return geom->intersects( mGeom->boundingBox() );
      break;
    case CONTAINS:
      return geom->contains( mGeom );
      break;
    case CROSSES:
      return geom->crosses( mGeom );
      break;
    case DISJOINT:
      return geom->disjoint( mGeom );
      break;
    case EQUALS:
      return geom->equals( mGeom );
      break;
    case INTERSECTS:
      return geom->intersects( mGeom );
      break;
    case OVERLAPS:
      return geom->overlaps( mGeom );
      break;
    case TOUCHES:
      return geom->touches( mGeom );
      break;
    case WITHIN:
      return geom->within( mGeom );
      break;
    case UNKNOWN:
    default:
      break;
  }
  return false;
}
