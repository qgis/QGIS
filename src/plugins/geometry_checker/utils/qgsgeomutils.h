/***************************************************************************
 *  qgsgeomutils.h                                                         *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GEOM_UTILS_H
#define QGS_GEOM_UTILS_H

#include "geometry/qgsabstractgeometryv2.h"
#include "geometry/qgspointv2.h"
#include <qmath.h>

class QgsGeometryEngine;

namespace QgsGeomUtils
{

  QgsGeometryEngine *createGeomEngine( QgsAbstractGeometryV2* geometry, double tolerance );

  QgsAbstractGeometryV2* getGeomPart( QgsAbstractGeometryV2* geom, int partIdx );

  void filter1DTypes( QgsAbstractGeometryV2* geom );

  /**
   * @brief Return the number of points in a polyline, accounting for duplicate start and end point if the polyline is closed
   * @param polyLine The polyline
   * @return The number of distinct points of the polyline
   */
  inline int polyLineSize( const QgsAbstractGeometryV2* geom, int iPart, int iRing, bool* isClosed = nullptr )
  {
    if ( !geom->isEmpty() )
    {
      int nVerts = geom->vertexCount( iPart, iRing );
      QgsPointV2 front = geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) );
      QgsPointV2 back = geom->vertexAt( QgsVertexId( iPart, iRing, nVerts - 1 ) );
      bool closed = back == front;
      if ( isClosed )
        *isClosed = closed;
      return closed ? nVerts - 1 : nVerts;
    }
    else
    {
      if ( isClosed )
        *isClosed = true;
      return 0;
    }
  }

  double sharedEdgeLength( const QgsAbstractGeometryV2* geom1, const QgsAbstractGeometryV2* geom2, double tol );

  /**
     * @brief Determine whether two points are equal up to the specified tolerance
     * @param p1 The first point
     * @param p2 The second point
     * @param tol The tolerance
     * @return Whether the points are equal
     */
  inline bool pointsFuzzyEqual( const QgsPointV2& p1, const QgsPointV2& p2, double tol )
  {
    double dx = p1.x() - p2.x(), dy = p1.y() - p2.y();
    return ( dx * dx + dy * dy ) < tol * tol;
  }

}

#endif // QGS_GEOM_UTILS_H
