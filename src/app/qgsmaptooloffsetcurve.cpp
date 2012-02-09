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
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include <QMouseEvent>

QgsMapToolOffsetCurve::QgsMapToolOffsetCurve( QgsMapCanvas* canvas ): QgsMapToolEdit( canvas ), mRubberBand( 0 ), mGeometry( 0 )
{
}

QgsMapToolOffsetCurve::~QgsMapToolOffsetCurve()
{
  deleteRubberBandAndGeometry();
}

void QgsMapToolOffsetCurve::canvasPressEvent( QMouseEvent * e )
{
  deleteRubberBandAndGeometry();

  //get selected features or snap to nearest feature if no selection
  QgsVectorLayer* layer = currentVectorLayer();
  if ( !layer )
  {
    return;
  }

  //selection or take closest feature
  //QgsPoint layerCoords = toLayerCoordinates( layer, e->pos() );

  //optionally merge the features together

  //create rubberband from feature(s)

  //for now, just take the first selected feature
  QgsFeatureList selectedFeatures = layer->selectedFeatures();
  if ( selectedFeatures.size() > 0 )
  {
    mGeometry = selectedFeatures[0].geometryAndOwnership();
    mRubberBand = createRubberBand();
    mRubberBand->setToGeometry( mGeometry, layer );
  }
}

void QgsMapToolOffsetCurve::canvasReleaseEvent( QMouseEvent * e )
{
  deleteRubberBandAndGeometry();
}

void QgsMapToolOffsetCurve::canvasMoveEvent( QMouseEvent * e )
{
  if ( !mGeometry || !mRubberBand )
  {
    return;
  }

  QgsVectorLayer* layer = currentVectorLayer();
  if ( !layer )
  {
    return;
  }

  //get offset from current position rectangular to feature
  QgsPoint layerCoords = toLayerCoordinates( layer, e->pos() );
  QgsPoint minDistPoint;
  int beforeVertex;
  double offset = sqrt( mGeometry->closestSegmentWithContext( layerCoords, minDistPoint, beforeVertex ) );
  qWarning( QString::number( offset ).toLocal8Bit().data() );

  //create offset geometry using geos
  QgsGeometry geomCopy( *mGeometry );
  GEOSGeometry* geosGeom = geomCopy.asGeos();
  if ( geosGeom )
  {
    //GEOSGeometry* offsetGeom = GEOSOffsetCurve( geosGeom, offset, 8, 1, 1 );
    GEOSGeometry* offsetGeom = GEOSSingleSidedBuffer( geosGeom, offset, 8, 1, 1, 0 );
    if ( offsetGeom )
    {
      QgsGeometry rubberBandGeometry;
      rubberBandGeometry.fromGeos( offsetGeom );
      mRubberBand->setToGeometry( &rubberBandGeometry, layer );
    }
  }
}

void QgsMapToolOffsetCurve::deleteRubberBandAndGeometry()
{
  delete mRubberBand;
  mRubberBand = 0;
  delete mGeometry;
  mGeometry = 0;
}
