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

#include "qgscursors.h"
#include "qgsgeometryvalidator.h"
#include "qgslayertreeview.h"
#include "qgslinestringv2.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmaprenderer.h"
#include "qgspolygonv2.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvertexmarker.h"

#include <QCursor>
#include <QPixmap>
#include <QMouseEvent>
#include <QStatusBar>


QgsMapToolCapture::QgsMapToolCapture( QgsMapCanvas* canvas, QgsAdvancedDigitizingDockWidget* cadDockWidget, CaptureMode mode )
    : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
    , mRubberBand( 0 )
    , mTempRubberBand( 0 )
    , mValidator( 0 )
    , mSnappingMarker( 0 )
#ifdef Q_OS_WIN
    , mSkipNextContextMenuEvent( 0 )
#endif
{
  mCaptureMode = mode;

  // enable the snapping on mouse move / release
  mSnapOnMove = true;
  mSnapOnRelease = true;
  mSnapOnDoubleClick = false;
  mSnapOnPress = false;

  mCaptureModeFromLayer = mode == CaptureNone;
  mCapturing = false;

  QPixmap mySelectQPixmap = QPixmap(( const char ** ) capture_point_cursor );
  setCursor( QCursor( mySelectQPixmap, 8, 8 ) );

  connect( canvas, SIGNAL( currentLayerChanged( QgsMapLayer * ) ),
           this, SLOT( currentLayerChanged( QgsMapLayer * ) ) );
}

QgsMapToolCapture::~QgsMapToolCapture()
{
  delete mSnappingMarker;

  stopCapturing();

  if ( mValidator )
  {
    mValidator->deleteLater();
    mValidator = 0;
  }
}

void QgsMapToolCapture::deactivate()
{
  delete mSnappingMarker;
  mSnappingMarker = 0;

  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolCapture::validationFinished()
{
  emit messageDiscarded();
  QString msgFinished = tr( "Validation finished" );
  if ( mValidationWarnings.count() )
  {
    emit messageEmitted( mValidationWarnings.join( "\n" ).append( "\n" ).append( msgFinished ), QgsMessageBar::WARNING );
  }
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

void QgsMapToolCapture::cadCanvasMoveEvent( QgsMapMouseEvent * e )
{
  QgsMapToolAdvancedDigitizing::cadCanvasMoveEvent( e );
  bool snapped = e->isSnapped();
  QgsPoint point = e->mapPoint();

  if ( !snapped )
  {
    delete mSnappingMarker;
    mSnappingMarker = 0;
  }
  else
  {
    if ( !mSnappingMarker )
    {
      mSnappingMarker = new QgsVertexMarker( mCanvas );
      mSnappingMarker->setIconType( QgsVertexMarker::ICON_CROSS );
      mSnappingMarker->setColor( Qt::magenta );
      mSnappingMarker->setPenWidth( 3 );
    }
    mSnappingMarker->setCenter( point );
  }

  if ( !mTempRubberBand && mCaptureCurve.numPoints() > 0 )
  {
    mTempRubberBand = createRubberBand( mCaptureMode == CapturePolygon ? QGis::Polygon : QGis::Line, true );
    QgsPointV2 pt = mCaptureCurve.endPoint();
    mTempRubberBand->addPoint( QgsPoint( pt.x(), pt.y() ) );
    mTempRubberBand->addPoint( point );
  }

  if ( mCaptureMode != CapturePoint && mTempRubberBand && mCapturing )
  {
    mTempRubberBand->movePoint( point );
  }
} // mouseMoveEvent

int QgsMapToolCapture::nextPoint( const QgsPoint& mapPoint, QgsPoint& layerPoint )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
  {
    QgsDebugMsg( "no vector layer" );
    return 1;
  }
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

  return 0;
}

int QgsMapToolCapture::nextPoint( const QPoint &p, QgsPoint &layerPoint, QgsPoint &mapPoint )
{

  mapPoint = toMapCoordinates( p );
  return nextPoint( mapPoint, layerPoint );
}

int QgsMapToolCapture::addVertex( const QgsPoint& point )
{
  if ( mode() == CaptureNone )
  {
    QgsDebugMsg( "invalid capture mode" );
    return 2;
  }

  int res;
  QgsPoint layerPoint;
  res = nextPoint( point, layerPoint );
  if ( res != 0 )
  {
    return res;
  }

  if ( !mRubberBand )
  {
    mRubberBand = createRubberBand( mCaptureMode == CapturePolygon ? QGis::Polygon : QGis::Line );
  }
  mRubberBand->addPoint( point );
  mCaptureCurve.addVertex( QgsPointV2( layerPoint.x(), layerPoint.y() ) );

  if ( !mTempRubberBand )
  {
    mTempRubberBand = createRubberBand( mCaptureMode == CapturePolygon ? QGis::Polygon : QGis::Line, true );
  }
  else
  {
    mTempRubberBand->reset( mCaptureMode == CapturePolygon ? true : false );
  }
  if ( mCaptureMode == CaptureLine )
  {
    mTempRubberBand->addPoint( point );
  }
  else if ( mCaptureMode == CapturePolygon )
  {
    const QgsPoint *firstPoint = mRubberBand->getPoint( 0, 0 );
    mTempRubberBand->addPoint( *firstPoint );
    mTempRubberBand->movePoint( point );
    mTempRubberBand->addPoint( point );
  }

  validateGeometry();

  return 0;
}

int QgsMapToolCapture::addCurve( QgsCurveV2* c )
{
  if ( !c )
  {
    return 1;
  }

  if ( !mRubberBand )
  {
    mRubberBand = createRubberBand( mCaptureMode == CapturePolygon ? QGis::Polygon : QGis::Line );
  }

  QgsLineStringV2* lineString = c->curveToLine();
  QList<QgsPointV2> linePoints;
  lineString->points( linePoints );
  delete lineString;
  QList<QgsPointV2>::const_iterator ptIt = linePoints.constBegin();
  for ( ; ptIt != linePoints.constEnd(); ++ptIt )
  {
    mRubberBand->addPoint( QgsPoint( ptIt->x(), ptIt->y() ) );
  }

  if ( !mTempRubberBand )
  {
    mTempRubberBand = createRubberBand( mCaptureMode == CapturePolygon ? QGis::Polygon : QGis::Line, true );
  }
  else
  {
    mTempRubberBand->reset();
  }
  QgsPointV2 endPt = c->endPoint();
  mTempRubberBand->addPoint( QgsPoint( endPt.x(), endPt.y() ) ); //add last point of c

  //transform back to layer CRS in case map CRS and layer CRS are different
  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  const QgsCoordinateTransform* ct =  mCanvas->mapSettings().layerTransform( vlayer );
  if ( ct )
  {
    c->transform( *ct, QgsCoordinateTransform::ReverseTransform );
  }
  mCaptureCurve.addCurve( c );

  return 0;
}


void QgsMapToolCapture::undo()
{
  if ( mRubberBand )
  {
    int rubberBandSize = mRubberBand->numberOfVertices();
    int tempRubberBandSize = mTempRubberBand->numberOfVertices();
    int captureListSize = size();

    if ( rubberBandSize < 1 || captureListSize < 1 )
    {
      return;
    }

    mRubberBand->removePoint( -1 );

    if ( rubberBandSize > 1 )
    {
      if ( tempRubberBandSize > 1 )
      {
        const QgsPoint *point = mRubberBand->getPoint( 0, rubberBandSize - 2 );
        mTempRubberBand->movePoint( tempRubberBandSize - 2, *point );
      }
    }
    else
    {
      mTempRubberBand->reset( mCaptureMode == CapturePolygon ? true : false );
    }

    QgsVertexId vertexToRemove;
    vertexToRemove.part = 0; vertexToRemove.ring = 0; vertexToRemove.vertex = size() - 1;
    mCaptureCurve.deleteVertex( vertexToRemove );

    validateGeometry();
  }
}

void QgsMapToolCapture::keyPressEvent( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
  {
    undo();

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
  else if ( e->key() == Qt::Key_Escape )
  {
    stopCapturing();

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
}

void QgsMapToolCapture::startCapturing()
{
  mCapturing = true;
}

bool QgsMapToolCapture::isCapturing() const
{
  return mCapturing;
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
  Q_FOREACH ( QWidget *w, qApp->topLevelWidgets() )
  {
    if ( w->objectName() == "QgisApp" )
    {
      if ( mSkipNextContextMenuEvent++ == 0 )
        w->installEventFilter( this );
      break;
    }
  }
#endif

  mCapturing = false;
  mCaptureCurve.clear();
  if ( currentVectorLayer() )
    currentVectorLayer()->triggerRepaint();
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
  mCaptureCurve.close();
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

  mValidationWarnings.clear();
  mGeomErrors.clear();
  while ( !mGeomErrorMarkers.isEmpty() )
  {
    delete mGeomErrorMarkers.takeFirst();
  }

  QScopedPointer<QgsGeometry> g;

  switch ( mCaptureMode )
  {
    case CaptureNone:
    case CapturePoint:
      return;
    case CaptureLine:
      if ( size() < 2 )
        return;
      g.reset( new QgsGeometry( mCaptureCurve.curveToLine() ) );
      break;
    case CapturePolygon:
      if ( size() < 3 )
        return;
      QgsLineStringV2* exteriorRing = mCaptureCurve.curveToLine();
      exteriorRing->close();
      QgsPolygonV2* polygon = new QgsPolygonV2();
      polygon->setExteriorRing( exteriorRing );
      g.reset( new QgsGeometry( polygon ) );
      break;
  }

  if ( !g.data() )
    return;

  mValidator = new QgsGeometryValidator( g.data() );
  connect( mValidator, SIGNAL( errorFound( QgsGeometry::Error ) ), this, SLOT( addError( QgsGeometry::Error ) ) );
  connect( mValidator, SIGNAL( finished() ), this, SLOT( validationFinished() ) );
  mValidator->start();
  messageEmitted( tr( "Validation started" ) );
}

void QgsMapToolCapture::addError( QgsGeometry::Error e )
{
  mGeomErrors << e;
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
    return;

  mValidationWarnings << e.what();

  if ( e.hasWhere() )
  {
    QgsVertexMarker *vm =  new QgsVertexMarker( mCanvas );
    vm->setCenter( mCanvas->mapSettings().layerToMapCoordinates( vlayer, e.where() ) );
    vm->setIconType( QgsVertexMarker::ICON_X );
    vm->setPenWidth( 2 );
    vm->setToolTip( e.what() );
    vm->setColor( Qt::green );
    vm->setZValue( vm->zValue() + 1 );
    mGeomErrorMarkers << vm;
  }

  emit messageDiscarded();
  emit messageEmitted( mValidationWarnings.join( "\n" ), QgsMessageBar::WARNING );
}

int QgsMapToolCapture::size()
{
  return mCaptureCurve.numPoints();
}

QList<QgsPoint> QgsMapToolCapture::points()
{
  QList<QgsPointV2> pts;
  QList<QgsPoint> points;
  mCaptureCurve.points( pts );
  QgsGeometry::convertPointList( pts, points );
  return points;
}

void QgsMapToolCapture::setPoints( const QList<QgsPoint>& pointList )
{
  QList<QgsPointV2> pts;
  QgsGeometry::convertPointList( pointList, pts );

  QgsLineStringV2* line = new QgsLineStringV2();
  line->setPoints( pts );

  mCaptureCurve.clear();
  mCaptureCurve.addCurve( line );
}

#ifdef Q_OS_WIN
bool QgsMapToolCapture::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() != QEvent::ContextMenu )
    return false;

  if ( --mSkipNextContextMenuEvent == 0 )
    obj->removeEventFilter( this );

  return mSkipNextContextMenuEvent >= 0;
}
#endif
