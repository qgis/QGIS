/***************************************************************************
 *  qgsgeometryselfcontactcheck.cpp                                        *
 *  -------------------                                                    *
 *  copyright            : (C) 2017 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#include "qgsgeometryselfcontactcheck.h"
#include "qgsgeometryutils.h"
#include "../utils/qgsfeaturepool.h"

void QgsGeometrySelfContactCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QgsFeatureIds &ids ) const
{
  const QgsFeatureIds &featureIds = ids.isEmpty() ? mFeaturePool->getFeatureIds() : ids;
  double tolerance = QgsGeometryCheckPrecision::tolerance();
  foreach ( const QgsFeatureId &featureid, featureIds )
  {
    if ( progressCounter ) progressCounter->fetchAndAddRelaxed( 1 );
    QgsFeature feature;
    if ( !mFeaturePool->get( featureid, feature ) )
    {
      continue;
    }
    QgsAbstractGeometry *geom = feature.geometry().geometry();

    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        // Test for self-contacts
        int n = geom->vertexCount( iPart, iRing );
        bool isClosed = geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) ) == geom->vertexAt( QgsVertexId( iPart, iRing, n - 1 ) );

        // Geometry ring without duplicate nodes
        QVector<int> vtxMap;
        QVector<QgsPointV2> ring;
        vtxMap.append( 0 );
        ring.append( geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) ) );
        for ( int i = 1; i < n; ++i )
        {
          QgsPointV2 p = geom->vertexAt( QgsVertexId( iPart, iRing, i ) );
          if ( QgsGeometryUtils::sqrDistance2D( p, ring.last() ) > tolerance * tolerance )
          {
            vtxMap.append( i );
            ring.append( p );
          }
        }
        while ( QgsGeometryUtils::sqrDistance2D( ring.front(), ring.back() ) < tolerance * tolerance )
        {
          vtxMap.pop_back();
          ring.pop_back();
        }
        if ( isClosed )
        {
          vtxMap.append( n - 1 );
          ring.append( ring.front() );
        }
        n = ring.size();

        // For each vertex, check whether it lies on a segment
        for ( int iVert = 0, nVerts = n - isClosed; iVert < nVerts; ++iVert )
        {
          const QgsPointV2 &p = ring[iVert];
          for ( int i = 0, j = 1; j < n; i = j++ )
          {
            if ( iVert == i || iVert == j || ( isClosed && iVert == 0 && j == n - 1 ) )
            {
              continue;
            }
            const QgsPointV2 &si = ring[i];
            const QgsPointV2 &sj = ring[j];
            QgsPointV2 q = QgsGeometryUtils::projPointOnSegment( p, si, sj );
            if ( QgsGeometryUtils::sqrDistance2D( p, q ) < tolerance * tolerance )
            {
              errors.append( new QgsGeometryCheckError( this, featureid, p, QgsVertexId( iPart, iRing, vtxMap[iVert] ) ) );
              break; // No need to report same contact on different segments multiple times
            }
          }
        }
      }
    }
  }
}

void QgsGeometrySelfContactCheck::fixError( QgsGeometryCheckError *error, int method, int /*mergeAttributeIndex*/, Changes & /*changes*/ ) const
{
  if ( method == NoChange )
  {
    error->setFixed( method );
  }
  else
  {
    error->setFixFailed( tr( "Unknown method" ) );
  }
}

QStringList QgsGeometrySelfContactCheck::getResolutionMethods() const
{
  static QStringList methods = QStringList() << tr( "No action" );
  return methods;
}
