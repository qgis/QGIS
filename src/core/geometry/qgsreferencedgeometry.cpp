/***************************************************************************
                             qgsreferencedgeometry.cpp
                             ------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsreferencedgeometry.h"

QgsReferencedGeometryBase::QgsReferencedGeometryBase( const QgsCoordinateReferenceSystem &crs )
  : mCrs( crs )
{}

QgsReferencedRectangle::QgsReferencedRectangle( const QgsRectangle &rect, const QgsCoordinateReferenceSystem &crs )
  : QgsRectangle( rect )
  , QgsReferencedGeometryBase( crs )
{}

QgsReferencedPointXY::QgsReferencedPointXY( const QgsPointXY &point, const QgsCoordinateReferenceSystem &crs )
  : QgsPointXY( point )
  , QgsReferencedGeometryBase( crs )
{}
