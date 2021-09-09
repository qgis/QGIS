/***************************************************************************
    qgsgeometryselfcontactcheck.cpp
    ---------------------
    begin                : September 2017
    copyright            : (C) 2017 by Sandro Mani / Sourcepole AG
    email                : smani at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckcontext.h"
#include "qgsgeometryselfcontactcheck.h"
#include "qgsgeometryutils.h"
#include "qgsfeaturepool.h"

QList<QgsSingleGeometryCheckError *> QgsGeometrySelfContactCheck::processGeometry( const QgsGeometry &geometry ) const
{
  QList<QgsSingleGeometryCheckError *> errors;
  const QgsAbstractGeometry *geom = geometry.constGet();
  for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
  {
    for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
    {
      // Test for self-contacts
      int n = geom->vertexCount( iPart, iRing );
      const bool isClosed = geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) ) == geom->vertexAt( QgsVertexId( iPart, iRing, n - 1 ) );

      // Geometry ring without duplicate nodes
      QVector<int> vtxMap;
      QVector<QgsPoint> ring;
      vtxMap.append( 0 );
      ring.append( geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) ) );
      for ( int i = 1; i < n; ++i )
      {
        const QgsPoint p = geom->vertexAt( QgsVertexId( iPart, iRing, i ) );
        if ( QgsGeometryUtils::sqrDistance2D( p, ring.last() ) > mContext->tolerance * mContext->tolerance )
        {
          vtxMap.append( i );
          ring.append( p );
        }
      }
      while ( !ring.empty() && QgsGeometryUtils::sqrDistance2D( ring.front(), ring.back() ) < mContext->tolerance * mContext->tolerance )
      {
        vtxMap.pop_back();
        ring.pop_back();
      }
      if ( !ring.empty() && isClosed )
      {
        vtxMap.append( n - 1 );
        ring.append( ring.front() );
      }
      n = ring.size();

      // For each vertex, check whether it lies on a segment
      for ( int iVert = 0, nVerts = n - isClosed; iVert < nVerts; ++iVert )
      {
        const QgsPoint &p = ring[iVert];
        for ( int i = 0, j = 1; j < n; i = j++ )
        {
          if ( iVert == i || iVert == j || ( isClosed && iVert == 0 && j == n - 1 ) )
          {
            continue;
          }
          const QgsPoint &si = ring[i];
          const QgsPoint &sj = ring[j];
          const QgsPoint q = QgsGeometryUtils::projectPointOnSegment( p, si, sj );
          if ( QgsGeometryUtils::sqrDistance2D( p, q ) < mContext->tolerance * mContext->tolerance )
          {
            errors.append( new QgsSingleGeometryCheckError( this, geometry, QgsGeometry( p.clone() ), QgsVertexId( iPart, iRing, vtxMap[iVert] ) ) );
            break; // No need to report same contact on different segments multiple times
          }
        }
      }
    }
  }
  return errors;
}

void QgsGeometrySelfContactCheck::fixError( const QMap<QString, QgsFeaturePool *> &featurePools, QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes & /*changes*/ ) const
{
  Q_UNUSED( featurePools )
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometrySelfContactCheck::resolutionMethods() const
{
  static const QStringList methods = QStringList() << tr( "No action" );
  return methods;
}

QgsGeometryCheck::CheckType QgsGeometrySelfContactCheck::factoryCheckType()
{
  return QgsGeometryCheck::FeatureNodeCheck;
}
