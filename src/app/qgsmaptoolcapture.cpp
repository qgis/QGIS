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

#include "qgsmaptoolcapture.h"

#include "qgisapp.h"
#include "qgsvertexmarker.h"
#include "qgscursors.h"
#include "qgsrubberband.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsgeometryvalidator.h"

#include <QCursor>
#include <QPixmap>
#include <QMessageBox>
#include <QMouseEvent>
#include <QStatusBar>


QgsMapToolCapture::QgsMapToolCapture( QgsMapCanvas* canvas, enum CaptureMode tool )
    : QgsMapToolEdit( canvas )
    , mCaptureMode( tool )
    , mRubberBand( 0 )
    , mTempRubberBand( 0 )
    , mValidator( 0 )
{
  mCaptureModeFromLayer = tool == CaptureNone;
  mCapturing = false;

  QPixmap mySelectQPixmap = QPixmap(( const char ** ) capture_point_cursor );
  mCursor = QCursor( mySelectQPixmap, 8, 8 );

  connect( QgisApp::instance()->legend(), SIGNAL( currentLayerChanged( QgsMapLayer * ) ),
           this, SLOT( currentLayerChanged( QgsMapLayer * ) ) );
}

QgsMapToolCapture::~QgsMapToolCapture()
{
  while ( !mSnappingMarkers.isEmpty() )
    delete mSnappingMarkers.takeFirst();

  stopCapturing();

  if ( mValidator )
  {
    mValidator->deleteLater();
    mValidator = 0;
  }
}

void QgsMapToolCapture::deactivate()
{
  while ( !mSnappingMarkers.isEmpty() )
    delete mSnappingMarkers.takeFirst();

  QgsMapToolEdit::deactivate();
}

void QgsMapToolCapture::currentLayerChanged( QgsMapLayer *layer )
{
  if ( !mCaptureModeFromLayer )
    return;

  mCaptureMode = CaptureNone;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vlayer )
  {
    return;
  }

  switch ( vlayer->geometryType() )
  {
    case QGis::Point:
      mCaptureMode = CapturePoint;
      break;
    case QGis::Line:
      mCaptureMode = CaptureLine;
      break;
    case QGis::Polygon:
      mCaptureMode = CapturePolygon;
      break;
    default:
      mCaptureMode = CaptureNone;
      break;
  }
}

void QgsMapToolCapture::canvasMoveEvent( QMouseEvent * e )
{
  QgsPoint mapPoint;
  QList<QgsSnappingResult> snapResults;
  if ( mSnapper.snapToBackgroundLayers( e->pos(), snapResults ) == 0 )
  {
    while ( !mSnappingMarkers.isEmpty() )
      delete mSnappingMarkers.takeFirst();

    foreach ( const QgsSnappingResult &r, snapResults )
    {
      QgsVertexMarker *m = new QgsVertexMarker( mCanvas );
      m->setIconType( QgsVertexMarker::ICON_CROSS );
      m->setColor( Qt::magenta );
      m->setPenWidth( 3 );
      m->setCenter( r.snappedVertex );
      mSnappingMarkers << m;
    }

    if ( mCaptureMode != CapturePoint && mTempRubberBand && mCapturing )
    {
      mapPoint = snapPointFromResults( snapResults, e->pos() );
      mTempRubberBand->movePoint( mapPoint );
    }
  }
} // mouseMoveEvent


void QgsMapToolCapture::canvasPressEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
  // nothing to be done
}


void QgsMapToolCapture::renderComplete()
{
}

int QgsMapToolCapture::nextPoint( const QPoint &p, QgsPoint &layerPoint, QgsPoint &mapPoint )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
  {
    QgsDebugMsg( "no vector layer" );
    return 1;
  }

  QgsPoint digitisedPoint;
  try
  {
    digitisedPoint = toLayerCoordinates( vlayer, p );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsDebugMsg( "transformation to layer coordinate failed" );
    return 2;
  }

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
      QgsDebugMsg( "transformation to layer coordinate failed" );
      return 2;
    }
  }

  return 0;
}

int QgsMapToolCapture::addVertex( const QPoint &p )
{
  QgsPoint layerPoint;
  QgsPoint mapPoint;

  if ( mode() == CaptureNone )
  {
    QgsDebugMsg( "invalid capture mode" );
    return 2;
  }

  int res = nextPoint( p, layerPoint, mapPoint );
  if ( res != 0 )
  {
    QgsDebugMsg( "nextPoint failed: " + QString::number( res ) );
    return res;
  }

  if ( !mRubberBand )
  {
    mRubberBand = createRubberBand( mCaptureMode == CapturePolygon ? QGis::Polygon : QGis::Line );
  }
  mRubberBand->addPoint( mapPoint );
  mCaptureList.append( layerPoint );

  if ( !mTempRubberBand )
  {
    mTempRubberBand = createRubberBand( mCaptureMode == CapturePolygon ? QGis::Polygon : QGis::Line , true );
  }
  else
  {
    mTempRubberBand->reset( mCaptureMode == CapturePolygon ? true : false );
  }
  if ( mCaptureMode == CaptureLine )
  {
    mTempRubberBand->addPoint( mapPoint );
  }
  else if ( mCaptureMode == CapturePolygon )
  {
    const QgsPoint *firstPoint = mRubberBand->getPoint( 0 , 0 );
    mTempRubberBand->addPoint( *firstPoint );
    mTempRubberBand->movePoint( mapPoint );
    mTempRubberBand->addPoint( mapPoint );
  }

  validateGeometry();

  return 0;
}

void QgsMapToolCapture::undo()
{
  if ( mRubberBand )
  {
    int rubberBandSize = mRubberBand->numberOfVertices();
    int tempRubberBandSize = mTempRubberBand->numberOfVertices();
    int captureListSize = mCaptureList.size();

    if ( rubberBandSize < 1 || captureListSize < 1 )
    {
      return;
    }

    mRubberBand->removePoint( -1 );

    if ( rubberBandSize > 0 )
    {
      if ( tempRubberBandSize > 1 )
      {
        const QgsPoint *point = mRubberBand->getPoint( 0, rubberBandSize - 1 );
        mTempRubberBand->movePoint( tempRubberBandSize - 2, *point );
      }
    }
    else
    {
      mTempRubberBand->reset( mCaptureMode == CapturePolygon ? true : false );
    }

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

  if ( mTempRubberBand )
  {
    delete mTempRubberBand;
    mTempRubberBand = 0;
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

void QgsMapToolCapture::deleteTempRubberBand()
{
  if ( mTempRubberBand )
  {
    delete mTempRubberBand;
    mTempRubberBand = 0;
  }
}

void QgsMapToolCapture::closePolygon()
{
  mCaptureList.append( mCaptureList[0] );
}

void QgsMapToolCapture::validateGeometry()
{
  QSettings settings;
  if ( settings.value( "/qgis/digitizing/validate_geometries", 1 ).toInt() == 0 )
    return;

  if ( mValidator )
  {
    mValidator->deleteLater();
    mValidator = 0;
  }

  mTip = "";
  mGeomErrors.clear();
  while ( !mGeomErrorMarkers.isEmpty() )
  {
    delete mGeomErrorMarkers.takeFirst();
  }

  QgsGeometry *g = 0;

  switch ( mCaptureMode )
  {
    case CaptureNone:
    case CapturePoint:
      return;
    case CaptureLine:
      if ( mCaptureList.size() < 2 )
        return;
      g = QgsGeometry::fromPolyline( mCaptureList.toVector() );
      break;
    case CapturePolygon:
      if ( mCaptureList.size() < 3 )
        return;
      g = QgsGeometry::fromPolygon( QgsPolygon() << ( QgsPolyline() << mCaptureList.toVector() << mCaptureList[0] ) );
      break;
  }

  if ( !g )
    return;

  mValidator = new QgsGeometryValidator( g );
  connect( mValidator, SIGNAL( errorFound( QgsGeometry::Error ) ), this, SLOT( addError( QgsGeometry::Error ) ) );
  connect( mValidator, SIGNAL( finished() ), this, SLOT( validationFinished() ) );
  mValidator->start();

  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( tr( "Validation started." ) );
}

void QgsMapToolCapture::addError( QgsGeometry::Error e )
{
  mGeomErrors << e;
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
    return;

  if ( !mTip.isEmpty() )
    mTip += "\n";

  mTip += e.what();

  if ( e.hasWhere() )
  {
    QgsVertexMarker *vm =  new QgsVertexMarker( mCanvas );
    vm->setCenter( mCanvas->mapRenderer()->layerToMapCoordinates( vlayer, e.where() ) );
    vm->setIconType( QgsVertexMarker::ICON_X );
    vm->setPenWidth( 2 );
    vm->setToolTip( e.what() );
    vm->setColor( Qt::green );
    vm->setZValue( vm->zValue() + 1 );
    mGeomErrorMarkers << vm;
  }

  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( e.what() );
  if ( !mTip.isEmpty() )
    sb->setToolTip( mTip );
}

void QgsMapToolCapture::validationFinished()
{
  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( tr( "Validation finished." ) );
}
