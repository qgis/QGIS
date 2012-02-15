/***************************************************************************
                              qgsmaptooloffsetcurve.cpp
    ------------------------------------------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooloffsetcurve.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include <QMouseEvent>

QgsMapToolOffsetCurve::QgsMapToolOffsetCurve( QgsMapCanvas* canvas ): QgsMapToolEdit( canvas ), mRubberBand( 0 ), mOriginalGeometry( 0 ), mGeometryModified( false )
{
}

QgsMapToolOffsetCurve::~QgsMapToolOffsetCurve()
{
  deleteRubberBandAndGeometry();
}

void QgsMapToolOffsetCurve::canvasPressEvent( QMouseEvent * e )
{
  deleteRubberBandAndGeometry();
  mGeometryModified = false;

  //get selected features or snap to nearest feature if no selection
  QgsVectorLayer* layer = currentVectorLayer();
  if ( !layer )
  {
    return;
  }

  QgsFeatureList selectedFeatures = layer->selectedFeatures();
  if ( selectedFeatures.size() > 0 )
  {
    //take the first selected feature
    mOriginalGeometry = selectedFeatures[0].geometryAndOwnership();
    mRubberBand = createRubberBand();
    mRubberBand->setToGeometry( mOriginalGeometry, layer );
    mModifiedFeature = selectedFeatures[0].id();
  }
  else //do a snap to the closest feature
  {
    QList<QgsSnappingResult> snapResults;
    QgsMapCanvasSnapper snapper( mCanvas );
    snapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToSegment );
    if ( snapResults.size() > 0 )
    {
      QgsFeature fet;
      if ( layer->featureAtId( snapResults.at( 0 ).snappedAtGeometry, fet ) )
      {
        mOriginalGeometry = fet.geometryAndOwnership();
        mRubberBand = createRubberBand();
        mRubberBand->setToGeometry( mOriginalGeometry, layer );
        mModifiedFeature = fet.id();
      }
    }
  }
}

void QgsMapToolOffsetCurve::canvasReleaseEvent( QMouseEvent * e )
{
  QgsVectorLayer* vlayer = currentVectorLayer();
  if ( !vlayer || !mGeometryModified )
  {
    deleteRubberBandAndGeometry();
    return;
  }

  vlayer->beginEditCommand( tr( "Offset curve" ) );
  if ( vlayer->changeGeometry( mModifiedFeature, &mModifiedGeometry ) )
  {
    vlayer->endEditCommand();
  }
  else
  {
    vlayer->destroyEditCommand();
  }

  delete mRubberBand;
  mRubberBand = 0;
  mCanvas->refresh();
}

void QgsMapToolOffsetCurve::canvasMoveEvent( QMouseEvent * e )
{
  if ( !mOriginalGeometry || !mRubberBand )
  {
    return;
  }

  QgsVectorLayer* layer = currentVectorLayer();
  if ( !layer )
  {
    return;
  }

  mGeometryModified = true;

  //get offset from current position rectangular to feature
  QgsPoint layerCoords = toLayerCoordinates( layer, e->pos() );
  QgsPoint minDistPoint;
  int beforeVertex;
  double leftOf;
  double offset = sqrt( mOriginalGeometry->closestSegmentWithContext( layerCoords, minDistPoint, beforeVertex, &leftOf ) );
  qWarning( QString::number( offset ).toLocal8Bit().data() );

  //create offset geometry using geos
  QgsGeometry geomCopy( *mOriginalGeometry );
  GEOSGeometry* geosGeom = geomCopy.asGeos();
  if ( geosGeom )
  {
    GEOSGeometry* offsetGeom = GEOSSingleSidedBuffer( geosGeom, offset, 8, 1, 1, ( leftOf < 0 ) ? 1 : 0 );
    if ( offsetGeom )
    {
      mModifiedGeometry.fromGeos( offsetGeom );
      mRubberBand->setToGeometry( &mModifiedGeometry, layer );
    }
  }
}

void QgsMapToolOffsetCurve::deleteRubberBandAndGeometry()
{
  delete mRubberBand;
  mRubberBand = 0;
  delete mOriginalGeometry;
  mOriginalGeometry = 0;
}
