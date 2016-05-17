/***************************************************************************
 *  qgsgeometrysnapper.cpp                                                 *
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

#include <QtConcurrentMap>
#include <qmath.h>

#include "qgsgeometry.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrysnapper.h"
#include "qgsvectordataprovider.h"
#include "qgsgeometryutils.h"
#include "qgssnapindex.h"

QgsGeometrySnapper::QgsGeometrySnapper( QgsVectorLayer *adjustLayer, QgsVectorLayer *referenceLayer, bool selectedOnly, double snapToleranceMapUnits, const QgsMapSettings *mapSettings )
    : mAdjustLayer( adjustLayer )
    , mReferenceLayer( referenceLayer )
    , mSnapToleranceMapUnits( snapToleranceMapUnits )
    , mMapSettings( mapSettings )
{
  if ( selectedOnly )
  {
    mFeatures = mAdjustLayer->selectedFeaturesIds();
  }
  else
  {
    mFeatures = mAdjustLayer->allFeatureIds();
  }

  // Build spatial index
  QgsFeature feature;
  QgsFeatureRequest req;
  req.setSubsetOfAttributes( QgsAttributeList() );
  QgsFeatureIterator it = mReferenceLayer->getFeatures( req );
  while ( it.nextFeature( feature ) )
  {
    mIndex.insertFeature( feature );
  }
}

QFuture<void> QgsGeometrySnapper::processFeatures()
{
  emit progressRangeChanged( 0, mFeatures.size() );
  return QtConcurrent::map( mFeatures, ProcessFeatureWrapper( this ) );
}

void QgsGeometrySnapper::processFeature( QgsFeatureId id )
{
  emit progressStep();
  // Get current feature
  QgsFeature feature;
  if ( !getFeature( mAdjustLayer, mAdjustLayerMutex, id, feature ) )
  {
    mErrorMutex.lock();
    mErrors.append( tr( "Failed to read feature %1 of input layer." ).arg( id ) );
    mErrorMutex.unlock();
    return;
  }
  QgsPointV2 center = QgsPointV2( feature.geometry()->geometry()->boundingBox().center() );

  // Compute snap tolerance
  double layerToMapUnits = mMapSettings->layerToMapUnits( mAdjustLayer, feature.geometry()->boundingBox() );
  double snapTolerance = mSnapToleranceMapUnits / layerToMapUnits;


  // Get potential reference features and construct snap index
  QList<QgsAbstractGeometryV2*> refGeometries;
  mIndexMutex.lock();
  QList<QgsFeatureId> refFeatureIds = mIndex.intersects( feature.geometry()->boundingBox() );
  mIndexMutex.unlock();
  Q_FOREACH ( QgsFeatureId refId, refFeatureIds )
  {
    QgsFeature refFeature;
    if ( getFeature( mReferenceLayer, mReferenceLayerMutex, refId, refFeature ) )
    {
      refGeometries.append( refFeature.geometry()->geometry()->clone() );
    }
    else
    {
      mErrorMutex.lock();
      mErrors.append( tr( "Failed to read feature %1 of input layer." ).arg( refId ) );
      mErrorMutex.unlock();
    }
  }
  QgsSnapIndex refSnapIndex( center, 10 * snapTolerance );
  Q_FOREACH ( const QgsAbstractGeometryV2* geom, refGeometries )
  {
    refSnapIndex.addGeometry( geom );
  }

  // Snap geometries
  QgsAbstractGeometryV2* subjGeom = feature.geometry()->geometry();
  QList < QList< QList<PointFlag> > > subjPointFlags;

  // Pass 1: snap vertices of subject geometry to reference vertices
  for ( int iPart = 0, nParts = subjGeom->partCount(); iPart < nParts; ++iPart )
  {
    subjPointFlags.append( QList< QList<PointFlag> >() );

    for ( int iRing = 0, nRings = subjGeom->ringCount( iPart ); iRing < nRings; ++iRing )
    {
      subjPointFlags[iPart].append( QList<PointFlag>() );

      for ( int iVert = 0, nVerts = polyLineSize( subjGeom, iPart, iRing ); iVert < nVerts; ++iVert )
      {

        QgsSnapIndex::PointSnapItem* snapPoint = nullptr;
        QgsSnapIndex::SegmentSnapItem* snapSegment = nullptr;
        QgsVertexId vidx( iPart, iRing, iVert );
        QgsPointV2 p = subjGeom->vertexAt( vidx );
        if ( !refSnapIndex.getSnapItem( p, snapTolerance, &snapPoint, &snapSegment ) )
        {
          subjPointFlags[iPart][iRing].append( Unsnapped );
        }
        else
        {
          // Prefer snapping to point
          if ( snapPoint )
          {
            subjGeom->moveVertex( vidx, snapPoint->getSnapPoint( p ) );
            subjPointFlags[iPart][iRing].append( SnappedToRefNode );
          }
          else if ( snapSegment )
          {
            subjGeom->moveVertex( vidx, snapSegment->getSnapPoint( p ) );
            subjPointFlags[iPart][iRing].append( SnappedToRefSegment );
          }
        }
      }
    }
  }

  // SnapIndex for subject feature
  QgsSnapIndex* subjSnapIndex = new QgsSnapIndex( center, 10 * snapTolerance );
  subjSnapIndex->addGeometry( subjGeom );

  QgsAbstractGeometryV2* origSubjGeom = subjGeom->clone();
  QgsSnapIndex* origSubjSnapIndex = new QgsSnapIndex( center, 10 * snapTolerance );
  origSubjSnapIndex->addGeometry( origSubjGeom );

  // Pass 2: add missing vertices to subject geometry
  Q_FOREACH ( const QgsAbstractGeometryV2* refGeom, refGeometries )
  {
    for ( int iPart = 0, nParts = refGeom->partCount(); iPart < nParts; ++iPart )
    {
      for ( int iRing = 0, nRings = refGeom->ringCount( iPart ); iRing < nRings; ++iRing )
      {
        for ( int iVert = 0, nVerts = polyLineSize( refGeom, iPart, iRing ); iVert < nVerts; ++iVert )
        {

          QgsSnapIndex::PointSnapItem* snapPoint = nullptr;
          QgsSnapIndex::SegmentSnapItem* snapSegment = nullptr;
          QgsPointV2 point = refGeom->vertexAt( QgsVertexId( iPart, iRing, iVert ) );
          if ( subjSnapIndex->getSnapItem( point, snapTolerance, &snapPoint, &snapSegment ) )
          {
            // Snap to segment, unless a subject point was already snapped to the reference point
            if ( snapPoint && QgsGeometryUtils::sqrDistance2D( snapPoint->getSnapPoint( point ), point ) < 1E-16 )
            {
              continue;
            }
            else if ( snapSegment )
            {
              // Look if there is a closer reference segment, if so, ignore this point
              QgsPointV2 pProj = snapSegment->getSnapPoint( point );
              QgsPointV2 closest = refSnapIndex.getClosestSnapToPoint( point, pProj );
              if ( QgsGeometryUtils::sqrDistance2D( pProj, point ) > QgsGeometryUtils::sqrDistance2D( pProj, closest ) )
              {
                continue;
              }

              // If we are too far away from the original geometry, do nothing
              if ( !origSubjSnapIndex->getSnapItem( point, snapTolerance ) )
              {
                continue;
              }

              const QgsSnapIndex::CoordIdx* idx = snapSegment->idxFrom;
              subjGeom->insertVertex( QgsVertexId( idx->vidx.part, idx->vidx.ring, idx->vidx.vertex + 1 ), point );
              subjPointFlags[idx->vidx.part][idx->vidx.ring].insert( idx->vidx.vertex + 1, SnappedToRefNode );
              delete subjSnapIndex;
              subjSnapIndex = new QgsSnapIndex( center, 10 * snapTolerance );
              subjSnapIndex->addGeometry( subjGeom );
            }
          }
        }
      }
    }
  }
  delete subjSnapIndex;
  delete origSubjSnapIndex;

  // Pass 3: remove superfluous vertices: all vertices which are snapped to a segment and not preceded or succeeded by an unsnapped vertex
  for ( int iPart = 0, nParts = subjGeom->partCount(); iPart < nParts; ++iPart )
  {
    for ( int iRing = 0, nRings = subjGeom->ringCount( iPart ); iRing < nRings; ++iRing )
    {
      for ( int iVert = 0, nVerts = polyLineSize( subjGeom, iPart, iRing ); iVert < nVerts; ++iVert )
      {
        int iPrev = ( iVert - 1 + nVerts ) % nVerts;
        int iNext = ( iVert + 1 ) % nVerts;
        QgsPointV2 pMid = subjGeom->vertexAt( QgsVertexId( iPart, iRing, iVert ) );
        QgsPointV2 pPrev = subjGeom->vertexAt( QgsVertexId( iPart, iRing, iPrev ) );
        QgsPointV2 pNext = subjGeom->vertexAt( QgsVertexId( iPart, iRing, iNext ) );

        if ( subjPointFlags[iPart][iRing][iVert] == SnappedToRefSegment &&
             subjPointFlags[iPart][iRing][iPrev] != Unsnapped &&
             subjPointFlags[iPart][iRing][iNext] != Unsnapped &&
             QgsGeometryUtils::sqrDistance2D( QgsGeometryUtils::projPointOnSegment( pMid, pPrev, pNext ), pMid ) < 1E-12 )
        {
          subjGeom->deleteVertex( QgsVertexId( iPart, iRing, iVert ) );
          subjPointFlags[iPart][iRing].removeAt( iVert );
          iVert -= 1;
          nVerts -= 1;
        }
      }
    }
  }

  feature.setGeometry( new QgsGeometry( feature.geometry()->geometry()->clone() ) ); // force refresh
  QgsGeometryMap geometryMap;
  geometryMap.insert( id, *feature.geometry() );
  qDeleteAll( refGeometries );
  mAdjustLayerMutex.lock();
  mAdjustLayer->dataProvider()->changeGeometryValues( geometryMap );
  mAdjustLayerMutex.unlock();
}

bool QgsGeometrySnapper::getFeature( QgsVectorLayer *layer, QMutex &mutex, QgsFeatureId id, QgsFeature &feature )
{
  QMutexLocker locker( &mutex );
  QgsFeatureRequest req( id );
  req.setSubsetOfAttributes( QgsAttributeList() );
  return layer->getFeatures( req ).nextFeature( feature );
}


int QgsGeometrySnapper::polyLineSize( const QgsAbstractGeometryV2* geom, int iPart, int iRing ) const
{
  int nVerts = geom->vertexCount( iPart, iRing );
  QgsPointV2 front = geom->vertexAt( QgsVertexId( iPart, iRing, 0 ) );
  QgsPointV2 back = geom->vertexAt( QgsVertexId( iPart, iRing, nVerts - 1 ) );
  return back == front ? nVerts - 1 : nVerts;
}
