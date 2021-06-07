/***************************************************************************
                         qgslinesegment.cpp
                         -----------------
    begin                : April 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgslinesegment.h"
#include "qgsgeometryutils.h"

int QgsLineSegment2D::pointLeftOfLine( const QgsPointXY &point ) const
{
  return QgsGeometryUtils::leftOfLine( point.x(), point.y(), mStart.x(), mStart.y(), mEnd.x(), mEnd.y() );
}

