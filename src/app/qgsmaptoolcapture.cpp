/***************************************************************************
    qgsmaptoolcapture.cpp  -  map tool for capturing points, lines, polygons
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsapplication.h"
#include "qgsattributedialog.h"
#include "qgscoordinatetransform.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsgeometry.h"
#include "qgsmaptoolcapture.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsmaptopixel.h"
#include "qgsfeature.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgscursors.h"
#include <QCursor>
#include <QPixmap>
#include <QMessageBox>
#include <QMouseEvent>


QgsMapToolCapture::QgsMapToolCapture( QgsMapCanvas* canvas, enum CaptureTool tool )
    : QgsMapToolEdit( canvas ), mTool( tool ), mRubberBand( 0 )
{
  mCapturing = FALSE;

  QPixmap mySelectQPixmap = QPixmap(( const char ** ) capture_point_cursor );
  mCursor = QCursor( mySelectQPixmap, 8, 8 );

  mSnapper.setMapCanvas( canvas );
}

QgsMapToolCapture::~QgsMapToolCapture()
{
  delete mRubberBand;
  mRubberBand = 0;
}

void QgsMapToolCapture::canvasMoveEvent( QMouseEvent * e )
{
  if ( mRubberBand && mCapturing )
  {
    QgsPoint mapPoint;
    QList<QgsSnappingResult> snapResults;
    if ( mSnapper.snapToBackgroundLayers( e->pos(), snapResults ) == 0 )
    {
      mapPoint = snapPointFromResults( snapResults, e->pos() );
      mRubberBand->movePoint( mapPoint );
    }
  }
} // mouseMoveEvent


void QgsMapToolCapture::canvasPressEvent( QMouseEvent * e )
{
  // nothing to be done
}


void QgsMapToolCapture::renderComplete()
{
}

void QgsMapToolCapture::deactivate()
{
  delete mRubberBand;
  mRubberBand = 0;
  mCaptureList.clear();

  QgsMapTool::deactivate();
}

int QgsMapToolCapture::addVertex( const QPoint& p )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );

  if ( !vlayer )
  {
    return 1;
  }

  if ( !mRubberBand )
  {
    mRubberBand = createRubberBand( mTool == CapturePolygon );
  }

  QgsPoint digitisedPoint;
  try
  {
    digitisedPoint = toLayerCoordinates( vlayer, p );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    return 2;
  }

  QgsPoint mapPoint;
  QgsPoint layerPoint;

  QList<QgsSnappingResult> snapResults;
  if ( mSnapper.snapToBackgroundLayers( p, snapResults ) == 0 )
  {
    mapPoint = snapPointFromResults( snapResults, p );
    try
    {
      layerPoint = toLayerCoordinates( vlayer, mapPoint ); //transform snapped point back to layer crs
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
      return 2;
    }
    mRubberBand->addPoint( mapPoint );
    mCaptureList.push_back( layerPoint );
  }

  return 0;
}

void QgsMapToolCapture::undo()
{
  if ( mRubberBand )
  {
    int rubberBandSize = mRubberBand->numberOfVertices();
    int captureListSize = mCaptureList.size();

    if ( rubberBandSize < 1 || captureListSize < 1 )
    {
      return;
    }

    mRubberBand->removeLastPoint();
    mCaptureList.pop_back();
  }
}

void QgsMapToolCapture::keyPressEvent( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_Backspace )
  {
    undo();
  }
}
