/***************************************************************************
 *  qgsgeometryselfcontactcheck.cpp                                        *
 *  -------------------                                                    *
 *  copyright            : (C) 2017 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

#include "qgsgeometryselfcontactcheck.h"
#include "qgsgeometryutils.h"
#include "qgsfeaturepool.h"

void QgsGeometrySelfContactCheck::collectErrors( QList<QgsGeometryCheckError *> &errors, QStringList &/*messages*/, QAtomicInt *progressCounter, const QMap<QString, QgsFeatureIds> &ids ) const
{
  QMap<QString, QgsFeatureIds> featureIds = ids.isEmpty() ? allLayerFeatureIds() : ids;
  QgsGeometryCheckerUtils::LayerFeatures layerFeatures( mContext->featurePools, featureIds, mCompatibleGeometryTypes, progressCounter );
  for ( const QgsGeometryCheckerUtils::LayerFeature &layerFeature : layerFeatures )
  {
    const QgsAbstractGeometry *geom = layerFeature.geometry();
    for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
    {
      for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        // Test for self-contacts
        int n = geom->vertexCount( iPart, iRing );
        bool isClosed = geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) ) == geom->vertexAt( QgsVertexId( iPart, iRing, n - 1 ) );

        // Geometry ring without duplicate nodes
        QVector<int> vtxMap;
        QVector<QgsPoint> ring;
        vtxMap.append( 0 );
        ring.append( geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) ) );
        for ( int i = 1; i < n; ++i )
        {
          QgsPoint p = geom->vertexAt( QgsVertexId( iPart, iRing, i ) );
          if ( QgsGeometryUtils::sqrDistance2D( p, ring.last() ) > mContext->tolerance * mContext->tolerance )
          {
            vtxMap.append( i );
            ring.append( p );
          }
        }
        while ( QgsGeometryUtils::sqrDistance2D( ring.front(), ring.back() ) < mContext->tolerance * mContext->tolerance )
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
          const QgsPoint &p = ring[iVert];
          for ( int i = 0, j = 1; j < n; i = j++ )
          {
            if ( iVert == i || iVert == j || ( isClosed && iVert == 0 && j == n - 1 ) )
            {
              continue;
            }
            const QgsPoint &si = ring[i];
            const QgsPoint &sj = ring[j];
            QgsPoint q = QgsGeometryUtils::projPointOnSegment( p, si, sj );
            if ( QgsGeometryUtils::sqrDistance2D( p, q ) < mContext->tolerance * mContext->tolerance )
            {
              errors.append( new QgsGeometryCheckError( this, layerFeature, p, QgsVertexId( iPart, iRing, vtxMap[iVert] ) ) );
              break; // No need to report same contact on different segments multiple times
            }
          }
        }
      }
    }
  }
}

void QgsGeometrySelfContactCheck::fixError( QgsGeometryCheckError *error, int method, const QMap<QString, int> & /*mergeAttributeIndices*/, Changes & /*changes*/ ) const
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
