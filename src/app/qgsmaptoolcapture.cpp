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

#include "qgsmaptoolcapture.h"

#include "qgisapp.h"
#include "qgsvertexmarker.h"
#include "qgscursors.h"
#include "qgsrubberband.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsvectorlayer.h"

#include <QCursor>
#include <QPixmap>
#include <QMessageBox>
#include <QMouseEvent>
#include <QStatusBar>


QgsMapToolCapture::QgsMapToolCapture( QgsMapCanvas* canvas, enum CaptureMode tool )
    : QgsMapToolEdit( canvas ), mCaptureMode( tool ), mRubberBand( 0 )
{
  mCapturing = false;

  QPixmap mySelectQPixmap = QPixmap(( const char ** ) capture_point_cursor );
  mCursor = QCursor( mySelectQPixmap, 8, 8 );

  mSnapper.setMapCanvas( canvas );
}

QgsMapToolCapture::~QgsMapToolCapture()
{
  stopCapturing();
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
  stopCapturing();
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
    mRubberBand = createRubberBand( mCaptureMode == CapturePolygon );
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
    mCaptureList.append( layerPoint );

    validateGeometry();
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
    mCaptureList.removeLast();

    validateGeometry();
  }
}

void QgsMapToolCapture::keyPressEvent( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
  {
    undo();
  }
}

void QgsMapToolCapture::startCapturing()
{
  mCapturing = true;
}

void QgsMapToolCapture::stopCapturing()
{
  if ( mRubberBand )
  {
    delete mRubberBand;
    mRubberBand = 0;
  }

  while ( !mGeomErrorMarkers.isEmpty() )
  {
    delete mGeomErrorMarkers.takeFirst();
  }

  mGeomErrors.clear();

#ifdef Q_OS_WIN
  // hope your wearing your peril sensitive sunglasses.
  QgisApp::instance()->skipNextContextMenuEvent();
#endif

  mCapturing = false;
  mCaptureList.clear();
  mCanvas->refresh();
}

void QgsMapToolCapture::closePolygon()
{
  mCaptureList.append( mCaptureList[0] );
}

void QgsMapToolCapture::validateGeometry()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );

  mGeomErrors.clear();

  while ( !mGeomErrorMarkers.isEmpty() )
  {
    delete mGeomErrorMarkers.takeFirst();
  }

  switch ( mCaptureMode )
  {
    case CapturePoint:
      return;
    case CaptureLine:
      if ( mCaptureList.size() < 2 )
        return;
      QgsGeometry::validatePolyline( mGeomErrors, 0, mCaptureList.toVector() );
      break;
    case CapturePolygon:
      if ( mCaptureList.size() < 3 )
        return;
      QgsGeometry::validatePolyline( mGeomErrors, 0, mCaptureList.toVector() << mCaptureList[0], true );
      break;
  }

  QString tip;
  for ( int i = 0; i < mGeomErrors.size(); i++ )
  {
    tip += mGeomErrors[i].what() + "\n";
    if ( !mGeomErrors[i].hasWhere() )
      continue;

    QgsVertexMarker *vm =  new QgsVertexMarker( mCanvas );
    vm->setCenter( mCanvas->mapRenderer()->layerToMapCoordinates( vlayer, mGeomErrors[i].where() ) );
    vm->setIconType( QgsVertexMarker::ICON_X );
    vm->setPenWidth( 2 );
    vm->setToolTip( mGeomErrors[i].what() );
    vm->setColor( Qt::green );
    vm->setZValue( vm->zValue() + 1 );
    mGeomErrorMarkers << vm;
  }

  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( QObject::tr( "%n geometry error(s) found.", "number of geometry errors", mGeomErrors.size() ) );
  if ( !tip.isEmpty() )
    sb->setToolTip( tip );
}
