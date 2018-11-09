/***************************************************************************
                         qgslinesegment.cpp
                         -----------------
    begin                : April 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgslinesegment.h"
#include "qgsgeometryutils.h"

int QgsLineSegment2D::pointLeftOfLine( const QgsPointXY &point ) const
{
  return QgsGeometryUtils::leftOfLine( point.x(), point.y(), mStart.x(), mStart.y(), mEnd.x(), mEnd.y() );
}

