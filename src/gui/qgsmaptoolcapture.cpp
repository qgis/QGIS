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
#include "qgsexception.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometryvalidator.h"
#include "qgslayertreeview.h"
#include "qgslinestring.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvastracer.h"
#include "qgsmapmouseevent.h"
#include "qgspolygon.h"
#include "qgsrubberband.h"
#include "qgssnapindicator.h"
#include "qgsvectorlayer.h"
#include "qgsvertexmarker.h"
#include "qgssettings.h"

#include <QAction>
#include <QCursor>
#include <QPixmap>
#include <QMouseEvent>
#include <QStatusBar>


QgsMapToolCapture::QgsMapToolCapture( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode )
  : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
  , mCaptureMode( mode )
#ifdef Q_OS_WIN
  , mSkipNextContextMenuEvent( 0 )
#endif
{
  mCaptureModeFromLayer = mode == CaptureNone;
  mCapturing = false;

  mSnapIndicator.reset( new QgsSnapIndicator( canvas ) );

  QPixmap mySelectQPixmap = QPixmap( ( const char ** ) capture_point_cursor );
  setCursor( QCursor( mySelectQPixmap, 8, 8 ) );

  connect( canvas, &QgsMapCanvas::currentLayerChanged,
           this, &QgsMapToolCapture::currentLayerChanged );
}

QgsMapToolCapture::~QgsMapToolCapture()
{
  stopCapturing();

  if ( mValidator )
  {
    mValidator->deleteLater();
    mValidator = nullptr;
  }
}

void QgsMapToolCapture::activate()
{
  if ( mTempRubberBand )
    mTempRubberBand->show();

  QgsMapToolAdvancedDigitizing::activate();
}

void QgsMapToolCapture::deactivate()
{
  if ( mTempRubberBand )
    mTempRubberBand->hide();

  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolCapture::validationFinished()
{
  emit messageDiscarded();
  QString msgFinished = tr( "Validation finished" );
  if ( !mValidationWarnings.isEmpty() )
  {
    emit messageEmitted( mValidationWarnings.join( QStringLiteral( "\n" ) ).append( "\n" ).append( msgFinished ), QgsMessageBar::WARNING );
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
    case QgsWkbTypes::PointGeometry:
      mCaptureMode = CapturePoint;
      break;
    case QgsWkbTypes::LineGeometry:
      mCaptureMode = CaptureLine;
      break;
    case QgsWkbTypes::PolygonGeometry:
      mCaptureMode = CapturePolygon;
      break;
    default:
      mCaptureMode = CaptureNone;
      break;
  }
}


bool QgsMapToolCapture::tracingEnabled()
{
  QgsMapCanvasTracer *tracer = QgsMapCanvasTracer::tracerForCanvas( mCanvas );
  return tracer && tracer->actionEnableTracing() && tracer->actionEnableTracing()->isChecked();
}


QgsPointXY QgsMapToolCapture::tracingStartPoint()
{
  try
  {
    QgsMapLayer *layer = mCanvas->currentLayer();
    if ( !layer )
      return QgsPointXY();

    // if we have starting point from previous trace, then preferably use that one
    // (useful when tracing with offset)
    if ( mTracingStartPoint != QgsPointXY() )
      return mTracingStartPoint;

    QgsPoint v = mCaptureCurve.endPoint();
    return toMapCoordinates( layer, QgsPointXY( v.x(), v.y() ) );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( "transformation to layer coordinate failed" );
    return QgsPointXY();
  }
}


bool QgsMapToolCapture::tracingMouseMove( QgsMapMouseEvent *e )
{
  if ( !e->isSnapped() )
    return false;

  QgsPointXY pt0 = tracingStartPoint();
  if ( pt0 == QgsPointXY() )
    return false;

  QgsMapCanvasTracer *tracer = QgsMapCanvasTracer::tracerForCanvas( mCanvas );
  if ( !tracer )
    return false;  // this should not happen!

  mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );

  QgsTracer::PathError err;
  QVector<QgsPointXY> points = tracer->findShortestPath( pt0, e->mapPoint(), &err );
  if ( points.isEmpty() )
  {
    tracer->reportError( err, false );
    return false;
  }

  if ( mCaptureMode == CapturePolygon )
    mTempRubberBand->addPoint( *mRubberBand->getPoint( 0, 0 ), false );

  // if there is offset, we need to fix the rubber bands to make sure they are aligned correctly.
  // There are two cases we need to sort out:
  // 1. the last point of mRubberBand may need to be moved off the traced curve to respect the offset
  // 2. extra first point of mTempRubberBand may be needed if there is gap between where mRubberBand ends and trace starts
  if ( mRubberBand->numberOfVertices() != 0 )
  {
    QgsPointXY lastPoint = *mRubberBand->getPoint( 0, mRubberBand->numberOfVertices() - 1 );
    if ( lastPoint == pt0 && points[0] != lastPoint )
    {
      // if rubber band had just one point, for some strange reason it contains the point twice
      // we only want to move the last point if there are multiple points already
      if ( mRubberBand->numberOfVertices() > 2 || ( mRubberBand->numberOfVertices() == 2 && *mRubberBand->getPoint( 0, 0 ) != *mRubberBand->getPoint( 0, 1 ) ) )
        mRubberBand->movePoint( points[0] );
    }
    else
    {
      mTempRubberBand->addPoint( lastPoint, false );
    }
  }

  //  update rubberband
  for ( int i = 0; i < points.count(); ++i )
    mTempRubberBand->addPoint( points.at( i ), i == points.count() - 1 );

  tracer->reportError( QgsTracer::ErrNone, false ); // clear messagebar if there was any error
  return true;
}


bool QgsMapToolCapture::tracingAddVertex( const QgsPointXY &point )
{
  QgsMapCanvasTracer *tracer = QgsMapCanvasTracer::tracerForCanvas( mCanvas );
  if ( !tracer )
    return false;  // this should not happen!

  if ( mCaptureCurve.numPoints() == 0 )
  {
    if ( !tracer->init() )
    {
      tracer->reportError( QgsTracer::ErrTooManyFeatures, true );
      return false;
    }

    // only accept first point if it is snapped to the graph (to vertex or edge)
    bool res = tracer->isPointSnapped( point );
    if ( res )
    {
      QgsPoint layerPoint;
      nextPoint( QgsPoint( point ), layerPoint ); // assuming the transform went fine earlier

      mRubberBand->addPoint( point );
      mCaptureCurve.addVertex( layerPoint );
      mSnappingMatches.append( QgsPointLocator::Match() );
    }
    return res;
  }

  QgsPointXY pt0 = tracingStartPoint();
  if ( pt0 == QgsPointXY() )
    return false;

  QgsTracer::PathError err;
  QVector<QgsPointXY> points = tracer->findShortestPath( pt0, point, &err );
  if ( points.isEmpty() )
    return false; // ignore the vertex - can't find path to the end point!

  if ( !mCaptureCurve.isEmpty() )
  {
    QgsPoint lp; // in layer coords
    if ( nextPoint( QgsPoint( pt0 ), lp ) != 0 )
      return false;
    QgsPoint last;
    QgsVertexId::VertexType type;
    mCaptureCurve.pointAt( mCaptureCurve.numPoints() - 1, last, type );
    if ( last == lp )
    {
      // remove the last point in the curve if it is the same as our first point
      if ( mCaptureCurve.numPoints() != 2 )
        mCaptureCurve.deleteVertex( QgsVertexId( 0, 0, mCaptureCurve.numPoints() - 1 ) );
      else
      {
        // there is a strange behavior in deleteVertex() that with just two points
        // the whole curve is cleared - so we need to do this little dance to work it around
        QgsPoint first = mCaptureCurve.startPoint();
        mCaptureCurve.clear();
        mCaptureCurve.addVertex( first );
      }
      // for unknown reasons, rubber band has 2 points even if only one point has been added - handle that case
      if ( mRubberBand->numberOfVertices() == 2 && *mRubberBand->getPoint( 0, 0 ) == *mRubberBand->getPoint( 0, 1 ) )
        mRubberBand->removeLastPoint();
      mRubberBand->removeLastPoint();
      mSnappingMatches.removeLast();
    }
  }

  // transform points
  QgsPointSequence layerPoints;
  QgsPoint lp; // in layer coords
  for ( int i = 0; i < points.count(); ++i )
  {
    if ( nextPoint( QgsPoint( points[i] ), lp ) != 0 )
      return false;
    layerPoints << lp;
  }

  for ( int i = 0; i < points.count(); ++i )
  {
    if ( i == 0 && !mCaptureCurve.isEmpty() && mCaptureCurve.endPoint() == layerPoints[0] )
      continue;  // avoid duplicate of the first vertex
    if ( i > 0 && points[i] == points[i - 1] )
      continue; // avoid duplicate vertices if there are any
    mRubberBand->addPoint( points[i], i == points.count() - 1 );
    mCaptureCurve.addVertex( layerPoints[i] );
    mSnappingMatches.append( QgsPointLocator::Match() );
  }

  tracer->reportError( QgsTracer::ErrNone, true ); // clear messagebar if there was any error
  return true;
}


void QgsMapToolCapture::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsMapToolAdvancedDigitizing::cadCanvasMoveEvent( e );
  QgsPointXY point = e->mapPoint();

  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( !mTempRubberBand && mCaptureCurve.numPoints() > 0 )
  {
    mTempRubberBand = createRubberBand( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, true );
    QgsPoint pt = mCaptureCurve.endPoint();
    mTempRubberBand->addPoint( QgsPointXY( pt.x(), pt.y() ) );
    mTempRubberBand->addPoint( point );
  }


  if ( mCaptureMode != CapturePoint && mTempRubberBand && mCapturing )
  {
    bool hasTrace = false;
    if ( tracingEnabled() && mCaptureCurve.numPoints() != 0 )
    {
      hasTrace = tracingMouseMove( e );
    }

    if ( !hasTrace )
    {
      if ( mCaptureCurve.numPoints() > 0 )
      {
        // fix temporary rubber band after tracing which may have added multiple points
        mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
        if ( mCaptureMode == CapturePolygon )
          mTempRubberBand->addPoint( *mRubberBand->getPoint( 0, 0 ), false );
        QgsPoint pt = mCaptureCurve.endPoint();
        QgsPointXY mapPt = toMapCoordinates( qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() ), QgsPointXY( pt.x(), pt.y() ) );
        mTempRubberBand->addPoint( mapPt );
        mTempRubberBand->addPoint( point );

        // fix existing rubber band after tracing - the last point may have been moved if using offset
        if ( mRubberBand->numberOfVertices() )
          mRubberBand->movePoint( mapPt );
      }
      else
        mTempRubberBand->movePoint( point );
    }
  }
} // mouseMoveEvent


int QgsMapToolCapture::nextPoint( const QgsPoint &mapPoint, QgsPoint &layerPoint )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
  {
    QgsDebugMsg( "no vector layer" );
    return 1;
  }
  try
  {
    QgsPointXY mapP( mapPoint.x(), mapPoint.y() );  //#spellok
    layerPoint = QgsPoint( toLayerCoordinates( vlayer, mapP ) ); //transform snapped point back to layer crs  //#spellok
    if ( QgsWkbTypes::hasZ( vlayer->wkbType() ) )
      layerPoint.addZValue( defaultZValue() );
    if ( QgsWkbTypes::hasM( vlayer->wkbType() ) )
      layerPoint.addMValue( 0.0 );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsDebugMsg( "transformation to layer coordinate failed" );
    return 2;
  }

  return 0;
}

int QgsMapToolCapture::nextPoint( QPoint p, QgsPoint &layerPoint, QgsPoint &mapPoint )
{
  mapPoint = QgsPoint( toMapCoordinates( p ) );
  return nextPoint( mapPoint, layerPoint );
}

int QgsMapToolCapture::fetchLayerPoint( const QgsPointLocator::Match &match, QgsPoint &layerPoint )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  QgsVectorLayer *sourceLayer = match.layer();
  if ( match.isValid() && match.hasVertex() && sourceLayer &&
       ( sourceLayer->crs() == vlayer->crs() ) )
  {
    QgsFeature f;
    QgsFeatureRequest request;
    request.setFilterFid( match.featureId() );
    bool fetched = match.layer()->getFeatures( request ).nextFeature( f );
    if ( fetched )
    {
      QgsVertexId vId;
      if ( !f.geometry().vertexIdFromVertexNr( match.vertexIndex(), vId ) )
        return 2;
      layerPoint = f.geometry().constGet()->vertexAt( vId );
      return 0;
    }
    else
    {
      return 2;
    }
  }
  else
  {
    return 1;
  }
}

int QgsMapToolCapture::addVertex( const QgsPointXY &point )
{
  return addVertex( point, QgsPointLocator::Match() );
}

int QgsMapToolCapture::addVertex( const QgsPointXY &point, const QgsPointLocator::Match &match )
{
  if ( mode() == CaptureNone )
  {
    QgsDebugMsg( "invalid capture mode" );
    return 2;
  }

  int res;
  QgsPoint layerPoint;
  res = fetchLayerPoint( match, layerPoint );
  if ( res != 0 )
  {
    res = nextPoint( QgsPoint( point ), layerPoint );
    if ( res != 0 )
    {
      return res;
    }
  }

  if ( !mRubberBand )
  {
    mRubberBand = createRubberBand( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
  }

  if ( !mTempRubberBand )
  {
    mTempRubberBand = createRubberBand( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, true );
  }
  else
  {
    mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
  }

  bool traceCreated = false;
  if ( tracingEnabled() )
  {
    traceCreated = tracingAddVertex( point );
  }

  // keep new tracing start point if we created a trace. This is useful when tracing with
  // offset so that the user stays "snapped"
  mTracingStartPoint = traceCreated ? point : QgsPointXY();

  if ( !traceCreated )
  {
    // ordinary digitizing
    mRubberBand->addPoint( point );
    mCaptureCurve.addVertex( layerPoint );
    mSnappingMatches.append( match );
  }

  if ( mCaptureMode == CaptureLine )
  {
    mTempRubberBand->addPoint( point );
  }
  else if ( mCaptureMode == CapturePolygon )
  {
    const QgsPointXY *firstPoint = mRubberBand->getPoint( 0, 0 );
    mTempRubberBand->addPoint( *firstPoint );
    mTempRubberBand->movePoint( point );
    mTempRubberBand->addPoint( point );
  }

  validateGeometry();

  return 0;
}

int QgsMapToolCapture::addCurve( QgsCurve *c )
{
  if ( !c )
  {
    return 1;
  }

  if ( !mRubberBand )
  {
    mRubberBand = createRubberBand( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
  }

  QgsLineString *lineString = c->curveToLine();
  QgsPointSequence linePoints;
  lineString->points( linePoints );
  delete lineString;
  QgsPointSequence::const_iterator ptIt = linePoints.constBegin();
  for ( ; ptIt != linePoints.constEnd(); ++ptIt )
  {
    mRubberBand->addPoint( QgsPointXY( ptIt->x(), ptIt->y() ) );
  }

  if ( !mTempRubberBand )
  {
    mTempRubberBand = createRubberBand( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, true );
  }
  else
  {
    mTempRubberBand->reset();
  }
  QgsPoint endPt = c->endPoint();
  mTempRubberBand->addPoint( QgsPointXY( endPt.x(), endPt.y() ) ); //add last point of c

  //transform back to layer CRS in case map CRS and layer CRS are different
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  QgsCoordinateTransform ct = mCanvas->mapSettings().layerTransform( vlayer );
  if ( ct.isValid() )
  {
    c->transform( ct, QgsCoordinateTransform::ReverseTransform );
  }
  mCaptureCurve.addCurve( c );
  for ( int i = 0; i < c->length(); ++i )
    mSnappingMatches.append( QgsPointLocator::Match() );

  return 0;
}

void QgsMapToolCapture::clearCurve()
{
  mCaptureCurve.clear();
}

QList<QgsPointLocator::Match> QgsMapToolCapture::snappingMatches() const
{
  return mSnappingMatches;
}


void QgsMapToolCapture::undo()
{
  mTracingStartPoint = QgsPointXY();

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
        const QgsPointXY *point = mRubberBand->getPoint( 0, rubberBandSize - 2 );
        mTempRubberBand->movePoint( tempRubberBandSize - 2, *point );
      }
    }
    else
    {
      mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
    }

    QgsVertexId vertexToRemove;
    vertexToRemove.part = 0;
    vertexToRemove.ring = 0;
    vertexToRemove.vertex = size() - 1;
    mCaptureCurve.deleteVertex( vertexToRemove );
    mSnappingMatches.removeAt( vertexToRemove.vertex );

    validateGeometry();
  }
}

void QgsMapToolCapture::keyPressEvent( QKeyEvent *e )
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
    mRubberBand = nullptr;
  }

  if ( mTempRubberBand )
  {
    delete mTempRubberBand;
    mTempRubberBand = nullptr;
  }

  while ( !mGeomErrorMarkers.isEmpty() )
  {
    delete mGeomErrorMarkers.takeFirst();
  }

  mGeomErrors.clear();

  mTracingStartPoint = QgsPointXY();

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
  mSnappingMatches.clear();
  if ( currentVectorLayer() )
    currentVectorLayer()->triggerRepaint();
}

void QgsMapToolCapture::deleteTempRubberBand()
{
  if ( mTempRubberBand )
  {
    delete mTempRubberBand;
    mTempRubberBand = nullptr;
  }
}

void QgsMapToolCapture::clean()
{
  stopCapturing();
  clearCurve();
}

void QgsMapToolCapture::closePolygon()
{
  mCaptureCurve.close();
}

void QgsMapToolCapture::validateGeometry()
{
  QgsSettings settings;
  if ( settings.value( QStringLiteral( "qgis/digitizing/validate_geometries" ), 1 ).toInt() == 0 )
    return;

  if ( mValidator )
  {
    mValidator->deleteLater();
    mValidator = nullptr;
  }

  mValidationWarnings.clear();
  mGeomErrors.clear();
  while ( !mGeomErrorMarkers.isEmpty() )
  {
    delete mGeomErrorMarkers.takeFirst();
  }

  QgsGeometry geom;

  switch ( mCaptureMode )
  {
    case CaptureNone:
    case CapturePoint:
      return;
    case CaptureLine:
      if ( size() < 2 )
        return;
      geom = QgsGeometry( mCaptureCurve.curveToLine() );
      break;
    case CapturePolygon:
      if ( size() < 3 )
        return;
      QgsLineString *exteriorRing = mCaptureCurve.curveToLine();
      exteriorRing->close();
      QgsPolygon *polygon = new QgsPolygon();
      polygon->setExteriorRing( exteriorRing );
      geom = QgsGeometry( polygon );
      break;
  }

  if ( !geom )
    return;

  QgsGeometry::ValidationMethod method = QgsGeometry::ValidatorQgisInternal;
  if ( settings.value( QStringLiteral( "qgis/digitizing/validate_geometries" ), 1 ).toInt() == 2 )
    method = QgsGeometry::ValidatorGeos;
  mValidator = new QgsGeometryValidator( geom, nullptr, method );
  connect( mValidator, &QgsGeometryValidator::errorFound, this, &QgsMapToolCapture::addError );
  connect( mValidator, &QThread::finished, this, &QgsMapToolCapture::validationFinished );
  mValidator->start();
  QgsDebugMsgLevel( "Validation started", 4 );
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
    QgsVertexMarker *vm = new QgsVertexMarker( mCanvas );
    vm->setCenter( mCanvas->mapSettings().layerToMapCoordinates( vlayer, e.where() ) );
    vm->setIconType( QgsVertexMarker::ICON_X );
    vm->setPenWidth( 2 );
    vm->setToolTip( e.what() );
    vm->setColor( Qt::green );
    vm->setZValue( vm->zValue() + 1 );
    mGeomErrorMarkers << vm;
  }

  emit messageDiscarded();
  emit messageEmitted( mValidationWarnings.join( QStringLiteral( "\n" ) ), QgsMessageBar::WARNING );
}

int QgsMapToolCapture::size()
{
  return mCaptureCurve.numPoints();
}

QVector<QgsPointXY> QgsMapToolCapture::points()
{
  QgsPointSequence pts;
  QVector<QgsPointXY> points;
  mCaptureCurve.points( pts );
  QgsGeometry::convertPointList( pts, points );
  return points;
}

void QgsMapToolCapture::setPoints( const QVector<QgsPointXY> &pointList )
{
  QgsLineString *line = new QgsLineString( pointList );
  mCaptureCurve.clear();
  mCaptureCurve.addCurve( line );
  mSnappingMatches.clear();
  for ( int i = 0; i < line->length(); ++i )
    mSnappingMatches.append( QgsPointLocator::Match() );
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
