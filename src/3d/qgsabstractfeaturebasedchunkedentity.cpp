/***************************************************************************
  qgsabstractfeaturebasedchunkedentity.cpp
  --------------------------------------
  Date                 : March 2026
  Copyright            : (C) 2026 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractfeaturebasedchunkedentity.h"

#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgschunknode.h"
#include "qgsfeatureid.h"
#include "qgsgeotransform.h"
#include "qgsraycastcontext.h"
#include "qgsraycastingutils.h"
#include "qgstessellatedpolygongeometry.h"

#include <QString>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QGeometryRenderer>

#include "moc_qgsabstractfeaturebasedchunkedentity.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE

QgsAbstractFeatureBasedChunkedEntity::QgsAbstractFeatureBasedChunkedEntity(
  Qgs3DMapSettings *mapSettings, float tau, QgsChunkLoaderFactory *loaderFactory, bool ownsFactory, int primitivesBudget, Qt3DCore::QNode *parent
)
  : QgsChunkedEntity( mapSettings, tau, loaderFactory, ownsFactory, primitivesBudget, parent )
{
  mTransform = new Qt3DCore::QTransform;
  this->addComponent( mTransform );

  connect( mapSettings, &Qgs3DMapSettings::terrainSettingsChanged, this, &QgsAbstractFeatureBasedChunkedEntity::onTerrainElevationOffsetChanged );
}

void QgsAbstractFeatureBasedChunkedEntity::onTerrainElevationOffsetChanged()
{
  QgsDebugMsgLevel( u"QgsAbstractFeatureBasedChunkedEntity::onTerrainElevationOffsetChanged"_s, 2 );

  float newOffset = static_cast<float>( mMapSettings->terrainSettings()->elevationOffset() );
  if ( !applyTerrainOffset() )
  {
    newOffset = 0.0;
  }
  mTransform->setTranslation( QVector3D( 0.0f, 0.0f, newOffset ) );
}

QList<QgsRayCastHit> QgsAbstractFeatureBasedChunkedEntity::rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const
{
  return rayIntersection( activeNodes(), mTransform->matrix(), ray, context, mMapSettings->origin() );
}

QList<QgsRayCastHit> QgsAbstractFeatureBasedChunkedEntity::rayIntersection(
  const QList<QgsChunkNode *> &activeNodes, const QMatrix4x4 &transformMatrix, const QgsRay3D &ray, const QgsRayCastContext &context, const QgsVector3D &origin
) const
{
  QgsDebugMsgLevel( u"Ray cast on vector layer"_s, 2 );
#ifdef QGISDEBUG
  int nodeUsed = 0;
  int nodesAll = 0;
  int hits = 0;
  int ignoredGeometries = 0;
#endif
  QList<QgsRayCastHit> result;

  float minDist = -1;
  QVector3D intersectionPoint;
  QgsFeatureId nearestFid = FID_NULL;

  for ( QgsChunkNode *node : activeNodes )
  {
#ifdef QGISDEBUG
    nodesAll++;
#endif

    QgsAABB nodeBbox = Qgs3DUtils::mapToWorldExtent( node->box3D(), origin );

    if ( node->entity() && ( minDist < 0 || nodeBbox.distanceFromPoint( ray.origin() ) < minDist ) && QgsRayCastingUtils::rayBoxIntersection( ray, nodeBbox ) )
    {
#ifdef QGISDEBUG
      nodeUsed++;
#endif
      const QList<Qt3DRender::QGeometryRenderer *> rendLst = node->entity()->findChildren<Qt3DRender::QGeometryRenderer *>();
      for ( const auto &rend : rendLst )
      {
        auto *geom = rend->geometry();
        QgsTessellatedPolygonGeometry *polygonGeom = qobject_cast<QgsTessellatedPolygonGeometry *>( geom );
        if ( !polygonGeom )
        {
#ifdef QGISDEBUG
          ignoredGeometries++;
#endif
          continue; // other QGeometry types are not supported for now
        }

        QVector3D nodeIntPoint;
        int triangleIndex = -1;

        // the node geometry has been translated by chunkOrigin
        // This translation is stored in the QTransform component
        // this needs to be taken into account to get the whole transformation
        const QMatrix4x4 nodeTransformMatrix = node->entity()->findChild<QgsGeoTransform *>()->matrix();
        const QMatrix4x4 fullTransformMatrix = transformMatrix * nodeTransformMatrix;
        if ( QgsRayCastingUtils::rayMeshIntersection( rend, ray, context.maximumDistance(), fullTransformMatrix, nodeIntPoint, triangleIndex ) )
        {
#ifdef QGISDEBUG
          hits++;
#endif
          float dist = ( ray.origin() - nodeIntPoint ).length();
          if ( minDist < 0 || dist < minDist )
          {
            minDist = dist;
            intersectionPoint = nodeIntPoint;
            nearestFid = polygonGeom->triangleIndexToFeatureId( triangleIndex );
          }
        }
      }
    }
  }
  if ( !FID_IS_NULL( nearestFid ) )
  {
    QgsRayCastHit hit;
    hit.setDistance( minDist );
    hit.setMapCoordinates( Qgs3DUtils::worldToMapCoordinates( intersectionPoint, origin ) );
    hit.setProperties( { { u"fid"_s, nearestFid } } );
    result.append( hit );
  }
  QgsDebugMsgLevel( u"Active Nodes: %1, checked nodes: %2, hits found: %3, incompatible geometries: %4"_s.arg( nodesAll ).arg( nodeUsed ).arg( hits ).arg( ignoredGeometries ), 2 );
  return result;
}

/// @endcond
