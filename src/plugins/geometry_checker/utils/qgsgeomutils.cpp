/***************************************************************************
 *  qgsgeomutils.cpp                                                       *
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

#include "qgsgeomutils.h"
#include <qmath.h>
#include "qgsgeos.h"
#include "qgsgeometryutils.h"
#include "qgsgeometrycollectionv2.h"

namespace QgsGeomUtils
{

  QgsGeometryEngine* createGeomEngine( QgsAbstractGeometryV2* geometry, double tolerance )
  {
    return new QgsGeos( geometry, tolerance );
  }

  QgsAbstractGeometryV2* getGeomPart( QgsAbstractGeometryV2* geom, int partIdx )
  {
    if ( dynamic_cast<QgsGeometryCollectionV2*>( geom ) )
    {
      return static_cast<QgsGeometryCollectionV2*>( geom )->geometryN( partIdx );
    }
    return geom;
  }

  void filter1DTypes( QgsAbstractGeometryV2* geom )
  {
    if ( dynamic_cast<QgsGeometryCollectionV2*>( geom ) )
    {
      QgsGeometryCollectionV2* geomCollection = static_cast<QgsGeometryCollectionV2*>( geom );
      for ( int nParts = geom->partCount(), iPart = nParts - 1; iPart >= 0; --iPart )
      {
        if ( !dynamic_cast<QgsCurveV2*>( geomCollection->geometryN( iPart ) ) )
        {
          geomCollection->removeGeometry( iPart );
        }
      }
    }
  }

  static inline double pointLineDist( const QgsPointV2& p1, const QgsPointV2& p2, const QgsPointV2& q )
  {
    double nom = qAbs(( p2.y() - p1.y() ) * q.x() - ( p2.x() - p1.x() ) * q.y() + p2.x() * p1.y() - p2.y() * p1.x() );
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return nom / qSqrt( dx * dx + dy * dy );
  }

  double sharedEdgeLength( const QgsAbstractGeometryV2* geom1, const QgsAbstractGeometryV2* geom2, double tol )
  {
    double len = 0;

    // Test every pair of segments for shared edges
    for ( int iPart1 = 0, nParts1 = geom1->partCount(); iPart1 < nParts1; ++iPart1 )
    {
      for ( int iRing1 = 0, nRings1 = geom1->ringCount( iPart1 ); iRing1 < nRings1; ++iRing1 )
      {
        for ( int iVert1 = 0, jVert1 = 1, nVerts1 = geom1->vertexCount( iPart1, iRing1 ); jVert1 < nVerts1; iVert1 = jVert1++ )
        {
          QgsPointV2 p1 = geom1->vertexAt( QgsVertexId( iPart1, iRing1, iVert1 ) );
          QgsPointV2 p2 = geom1->vertexAt( QgsVertexId( iPart1, iRing1, jVert1 ) );
          double lambdap1 = 0.;
          double lambdap2 = qSqrt( QgsGeometryUtils::sqrDistance2D( p1, p2 ) );
          QgsVector d = QgsVector( p2.x() - p1.x(), p2.y() - p1.y() ).normalized();

          for ( int iPart2 = 0, nParts2 = geom2->partCount(); iPart2 < nParts2; ++iPart2 )
          {
            for ( int iRing2 = 0, nRings2 = geom2->ringCount( iPart2 ); iRing2 < nRings2; ++iRing2 )
            {
              for ( int iVert2 = 0, jVert2 = 1, nVerts2 = geom2->vertexCount( iPart2, iRing2 ); jVert2 < nVerts2; iVert2 = jVert2++ )
              {
                QgsPointV2 q1 = geom2->vertexAt( QgsVertexId( iPart2, iRing2, iVert2 ) );
                QgsPointV2 q2 = geom2->vertexAt( QgsVertexId( iPart2, iRing2, jVert2 ) );

                // Check whether q1 and q2 are on the line p1, p
                if ( pointLineDist( p1, p2, q1 ) <= tol && pointLineDist( p1, p2, q2 ) <= tol )
                {
                  // Get length common edge
                  double lambdaq1 = QgsVector( q1.x() - p1.x(), q1.y() - p1.y() ) * d;
                  double lambdaq2 = QgsVector( q2.x() - p1.x(), q2.y() - p1.y() ) * d;
                  if ( lambdaq1 > lambdaq2 )
                  {
                    qSwap( lambdaq1, lambdaq2 );
                  }
                  double lambda1 = qMax( lambdaq1, lambdap1 );
                  double lambda2 = qMin( lambdaq2, lambdap2 );
                  len += qMax( 0., lambda2 - lambda1 );
                }
              }
            }
          }
        }
      }
    }
    return len;
  }

}
