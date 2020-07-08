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
#include "qgsapplication.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsproject.h"
#include "qgsgeometryrubberband.h"

#include <QAction>
#include <QCursor>
#include <QPixmap>
#include <QStatusBar>


QgsMapToolCapture::QgsMapToolCapture( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode )
  : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
  , mCaptureMode( mode )
  , mCaptureModeFromLayer( mode == CaptureNone )
{
  mSnapIndicator.reset( new QgsSnapIndicator( canvas ) );

  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::CapturePoint ) );

  connect( canvas, &QgsMapCanvas::currentLayerChanged,
           this, &QgsMapToolCapture::currentLayerChanged );

  QgsVectorLayer::LayerOptions layerOptions;
  layerOptions.skipCrsValidation = true;
  mExtraSnapLayer = new QgsVectorLayer( QStringLiteral( "LineString?crs=" ), QStringLiteral( "extra snap" ), QStringLiteral( "memory" ), layerOptions );
  mExtraSnapLayer->startEditing();
  QgsFeature f;
  mExtraSnapLayer->addFeature( f );
  mExtraSnapFeatureId = f.id();

  connect( QgsProject::instance(), &QgsProject::snappingConfigChanged,
           this, &QgsMapToolCapture::updateExtraSnapLayer );

  currentLayerChanged( canvas->currentLayer() );
}

QgsMapToolCapture::~QgsMapToolCapture()
{
  stopCapturing();

  if ( mValidator )
  {
    mValidator->deleteLater();
    mValidator = nullptr;
  }
  mCanvas->snappingUtils()->removeExtraSnapLayer( mExtraSnapLayer );
  mExtraSnapLayer->deleteLater();
  mExtraSnapLayer = nullptr;
}

QgsMapToolCapture::Capabilities QgsMapToolCapture::capabilities() const
{
  return QgsMapToolCapture::NoCapabilities;
}

void QgsMapToolCapture::activate()
{
  if ( mTempRubberBand )
    mTempRubberBand->show();

  mCanvas->snappingUtils()->addExtraSnapLayer( mExtraSnapLayer );
  QgsMapToolAdvancedDigitizing::activate();
}

void QgsMapToolCapture::deactivate()
{
  if ( mTempRubberBand )
    mTempRubberBand->hide();

  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  mCanvas->snappingUtils()->removeExtraSnapLayer( mExtraSnapLayer );
  QgsMapToolAdvancedDigitizing::deactivate();
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
  return tracer && ( !tracer->actionEnableTracing() || tracer->actionEnableTracing()->isChecked() )
         && ( !tracer->actionEnableSnapping() || tracer->actionEnableSnapping()->isChecked() );
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
    QgsDebugMsg( QStringLiteral( "transformation to layer coordinate failed" ) );
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
  mTempRubberBand->setStringType( QgsWkbTypes::LineString );

  QgsTracer::PathError err;
  QVector<QgsPointXY> points = tracer->findShortestPath( pt0, e->mapPoint(), &err );
  if ( points.isEmpty() )
  {
    tracer->reportError( err, false );
    return false;
  }

  if ( mCaptureMode == CapturePolygon )
  {
    QgsPoint pt = mCaptureCurve.startPoint();
    QgsPoint mapPoint = toMapCoordinates( canvas()->currentLayer(), pt );
    mTempRubberBand->addPoint( mapPoint, false );
  }


  // if there is offset, we need to fix the rubber bands to make sure they are aligned correctly.
  // There are two cases we need to sort out:
  // 1. the last point of mRubberBand may need to be moved off the traced curve to respect the offset
  // 2. extra first point of mTempRubberBand may be needed if there is gap between where mRubberBand ends and trace starts
  if ( mRubberBand->numberOfVertices() != 0 )
  {
    QgsPoint pt = mCaptureCurve.startPoint();
    QgsPoint lastPoint = toMapCoordinates( canvas()->currentLayer(), pt );
    QgsPointXY lastPointXY( lastPoint );
    if ( lastPointXY == pt0 && points[0] != lastPointXY )
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
  for ( int i = 0; i < points.count(); ++i ) //points added in the rubber band are 2D but will not be added to the capture curve
    mTempRubberBand->addPoint( QgsPoint( points.at( i ) ), i == points.count() - 1 );

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

  // Curves de-approximation
  QgsSettings settings;
  if ( settings.value( QStringLiteral( "/qgis/digitizing/convert_to_curve" ), false ).toBool() )
  {
    // If the tool and the layer support curves
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
    if ( capabilities().testFlag( QgsMapToolCapture::Capability::SupportsCurves ) && vlayer->dataProvider()->capabilities().testFlag( QgsVectorDataProvider::Capability::CircularGeometries ) )
    {
      QgsGeometry linear = QgsGeometry( mCaptureCurve.segmentize() );
      QgsGeometry curved = linear.convertToCurves(
                             settings.value( QStringLiteral( "/qgis/digitizing/convert_to_curve_angle_tolerance" ), 1e-6 ).toDouble(),
                             settings.value( QStringLiteral( "/qgis/digitizing/convert_to_curve_distance_tolerance" ), 1e-6 ).toDouble()
                           );
      mCaptureCurve = *qgsgeometry_cast<QgsCompoundCurve *>( curved.constGet() );
    }
  }

  tracer->reportError( QgsTracer::ErrNone, true ); // clear messagebar if there was any error
  return true;
}

QgsMapToolCaptureRubberband *QgsMapToolCapture::createCurveRubberBand( QgsWkbTypes::GeometryType geometryType, bool alternativeBand ) const SIP_FACTORY
{
  QgsSettings settings;
  QgsMapToolCaptureRubberband *rb = new QgsMapToolCaptureRubberband( mCanvas, geometryType );
  QColor color = digitizingStrokeColor();
  if ( alternativeBand )
  {
    double alphaScale = settings.value( QStringLiteral( "qgis/digitizing/line_color_alpha_scale" ), 0.75 ).toDouble();
    color.setAlphaF( color.alphaF() * alphaScale );
    rb->setLineStyle( Qt::DotLine );
  }
  rb->setStrokeColor( color );

  QColor fillColor = digitizingFillColor();
  rb->setFillColor( fillColor );
  rb->show();
  return rb;
}


QgsRubberBand *QgsMapToolCapture::takeRubberBand()
{
  return mRubberBand.release();
}

void QgsMapToolCapture::toggleLinearCircularDigitizing()
{
  if ( !mTempRubberBand )
    return;

  if ( mTempRubberBand->stringType() == QgsWkbTypes::LineString )
    mTempRubberBand->setStringType( QgsWkbTypes::CircularString );
  else
    mTempRubberBand->setStringType( QgsWkbTypes::LineString );
}


void QgsMapToolCapture::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsMapToolAdvancedDigitizing::cadCanvasMoveEvent( e );
  QgsPointXY point = e->mapPoint();

  const QgsPointLocator::Match &match = e->mapPointMatch();
  mSnapIndicator->setMatch( match );

  QgsPoint mapPoint = QgsPoint( point );

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
        QgsPoint pt = mCaptureCurve.endPoint();
        QgsPoint mapPt = toMapCoordinates( qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() ), pt ) ;

        if ( mTempRubberBand )
        {
          mTempRubberBand->movePoint( mapPoint );
          mTempRubberBand->movePoint( 0, mapPt );
        }

        // fix existing rubber band after tracing - the last point may have been moved if using offset
        if ( mRubberBand->numberOfVertices() )
          mRubberBand->movePoint( mapPt );
      }
      else
        mTempRubberBand->movePoint( mapPoint );
    }
  }
} // mouseMoveEvent


int QgsMapToolCapture::nextPoint( const QgsPoint &mapPoint, QgsPoint &layerPoint )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
  {
    QgsDebugMsg( QStringLiteral( "no vector layer" ) );
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
    Q_UNUSED( cse )
    QgsDebugMsg( QStringLiteral( "transformation to layer coordinate failed" ) );
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
  if ( match.isValid() && ( match.hasVertex() || ( QgsProject::instance()->topologicalEditing() && match.hasEdge() ) ) && sourceLayer &&
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

      const QgsGeometry geom( f.geometry() );
      if ( QgsProject::instance()->topologicalEditing() && match.hasEdge() )
      {
        QgsVertexId vId2;
        if ( !f.geometry().vertexIdFromVertexNr( match.vertexIndex() + 1, vId2 ) )
          return 2;
        QgsLineString line( geom.constGet()->vertexAt( vId ), geom.constGet()->vertexAt( vId2 ) );

        layerPoint = QgsGeometryUtils::closestPoint( line,  QgsPoint( match.point() ) );
      }
      else
      {
        layerPoint = geom.constGet()->vertexAt( vId );
        if ( QgsWkbTypes::hasZ( vlayer->wkbType() ) && !layerPoint.is3D() )
          layerPoint.addZValue( defaultZValue() );
        if ( QgsWkbTypes::hasM( vlayer->wkbType() ) && !layerPoint.isMeasure() )
          layerPoint.addMValue( 0.0 );
      }

      // ZM support depends on the target layer
      if ( !QgsWkbTypes::hasZ( vlayer->wkbType() ) )
      {
        layerPoint.dropZValue();
      }

      if ( !QgsWkbTypes::hasM( vlayer->wkbType() ) )
      {
        layerPoint.dropMValue();
      }

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
    QgsDebugMsg( QStringLiteral( "invalid capture mode" ) );
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
  QgsPoint mapPoint = toMapCoordinates( canvas()->currentLayer(), layerPoint );

  if ( !mRubberBand )
    mRubberBand.reset( createRubberBand( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry ) );

  if ( !mTempRubberBand )
    mTempRubberBand.reset( createCurveRubberBand( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, true ) );

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

    if ( mRubberBand->size() == 0 )
    {
      mRubberBand->addPoint( point );
      mCaptureCurve.addVertex( layerPoint );
      mSnappingMatches.append( match );
      if ( mCaptureMode == CapturePolygon )
      {
        mRubberBand->addPoint( point );
        mTempRubberBand->setFirstPolygonPoint( mapPoint );
      }
    }

    mTempRubberBand->movePoint( mapPoint );
    if ( mTempRubberBand->curveIsComplete() )
    {
      const QgsCurve *curve = mTempRubberBand->curve();
      if ( curve )
        addCurve( curve->clone() );
      mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
      if ( mCaptureMode == CapturePolygon )
      {
        QgsPoint firstPt = mCaptureCurve.startPoint();
        QgsPoint mapFirstPt = toMapCoordinates( qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() ), firstPt ) ;
        mTempRubberBand->setFirstPolygonPoint( mapFirstPt );
      }
    }

    mTempRubberBand->addPoint( mapPoint );
  }
  else
  {
    mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
    if ( mCaptureMode == CapturePolygon )
    {
      QgsPoint firstPt = mCaptureCurve.startPoint();
      QgsPoint mapFirstPt = toMapCoordinates( qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() ), firstPt ) ;
      mTempRubberBand->setFirstPolygonPoint( mapFirstPt );
    }
  }

  updateExtraSnapLayer();
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
    mRubberBand.reset( createRubberBand( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry ) );
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

  if ( mTempRubberBand )
  {
    mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
    QgsPoint endPt = c->endPoint();
    mTempRubberBand->addPoint( endPt ); //add last point of c
  }

  //transform back to layer CRS in case map CRS and layer CRS are different
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  QgsCoordinateTransform ct = mCanvas->mapSettings().layerTransform( vlayer );
  if ( ct.isValid() )
  {
    c->transform( ct, QgsCoordinateTransform::ReverseTransform );
  }
  int countBefore = mCaptureCurve.vertexCount();
  mCaptureCurve.addCurve( c );
  int countAfter = mCaptureCurve.vertexCount();
  int addedPoint = countAfter - countBefore;

  updateExtraSnapLayer();

  for ( int i = 0; i < addedPoint; ++i )
    mSnappingMatches.append( QgsPointLocator::Match() );

  return 0;
}

void QgsMapToolCapture::clearCurve()
{
  mCaptureCurve.clear();
  updateExtraSnapLayer();
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
    QgsPoint lastPoint = mTempRubberBand->lastPoint();

    if ( mTempRubberBand->stringType() == QgsWkbTypes::CircularString && mTempRubberBand->pointsCount() > 2 )
    {
      mTempRubberBand->removeLastPoint();
      mTempRubberBand->movePoint( lastPoint );
      return;
    }

    QgsVertexId vertexToRemove;
    vertexToRemove.part = 0;
    vertexToRemove.ring = 0;
    vertexToRemove.vertex = size() - 1;
    mCaptureCurve.deleteVertex( vertexToRemove );
    mSnappingMatches.removeAt( vertexToRemove.vertex );
    updateExtraSnapLayer();


    mRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
    mRubberBand->addGeometry( QgsGeometry( mCaptureCurve.clone() ), vlayer );

    mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
    if ( mCaptureMode == CapturePolygon )
    {
      QgsPoint pt = mCaptureCurve.startPoint();
      QgsPoint mapPt = toMapCoordinates( qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() ), pt );
      mTempRubberBand->setFirstPolygonPoint( mapPt );
    }

    QgsPoint pt = mCaptureCurve.endPoint();
    QgsPoint mapPt = toMapCoordinates( qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() ), pt );
    mTempRubberBand->addPoint( mapPt );
    mTempRubberBand->movePoint( lastPoint );

    mCadDockWidget->removePreviousPoint();

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
  mRubberBand.reset();

  deleteTempRubberBand();

  qDeleteAll( mGeomErrorMarkers );
  mGeomErrorMarkers.clear();
  mGeomErrors.clear();

  mTracingStartPoint = QgsPointXY();

  mCapturing = false;
  mCaptureCurve.clear();
  updateExtraSnapLayer();
  mSnappingMatches.clear();
  if ( currentVectorLayer() )
    currentVectorLayer()->triggerRepaint();
}

void QgsMapToolCapture::deleteTempRubberBand()
{
  mTempRubberBand.reset();
}

void QgsMapToolCapture::clean()
{
  stopCapturing();
  clearCurve();
}

void QgsMapToolCapture::closePolygon()
{
  mCaptureCurve.close();
  updateExtraSnapLayer();
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

  if ( geom.isNull() )
    return;

  QgsGeometry::ValidationMethod method = QgsGeometry::ValidatorQgisInternal;
  if ( settings.value( QStringLiteral( "qgis/digitizing/validate_geometries" ), 1 ).toInt() == 2 )
    method = QgsGeometry::ValidatorGeos;
  mValidator = new QgsGeometryValidator( geom, nullptr, method );
  connect( mValidator, &QgsGeometryValidator::errorFound, this, &QgsMapToolCapture::addError );
  mValidator->start();
  QgsDebugMsgLevel( QStringLiteral( "Validation started" ), 4 );
}

void QgsMapToolCapture::addError( const QgsGeometry::Error &e )
{
  mGeomErrors << e;
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
    return;

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
}

int QgsMapToolCapture::size()
{
  return mCaptureCurve.numPoints();
}

QVector<QgsPointXY> QgsMapToolCapture::points() const
{
  QVector<QgsPointXY> pointsXY;
  QgsGeometry::convertPointList( pointsZM(), pointsXY );

  return pointsXY;
}

QgsPointSequence QgsMapToolCapture::pointsZM() const
{
  QgsPointSequence pts;
  mCaptureCurve.points( pts );
  return pts;
}

void QgsMapToolCapture::setPoints( const QVector<QgsPointXY> &pointList )
{
  QgsLineString *line = new QgsLineString( pointList );
  mCaptureCurve.clear();
  mCaptureCurve.addCurve( line );
  updateExtraSnapLayer();
  mSnappingMatches.clear();
  for ( int i = 0; i < line->length(); ++i )
    mSnappingMatches.append( QgsPointLocator::Match() );
}

void QgsMapToolCapture::setPoints( const QgsPointSequence &pointList )
{
  QgsLineString *line = new QgsLineString( pointList );
  mCaptureCurve.clear();
  mCaptureCurve.addCurve( line );
  updateExtraSnapLayer();
  mSnappingMatches.clear();
  for ( int i = 0; i < line->length(); ++i )
    mSnappingMatches.append( QgsPointLocator::Match() );
}

QgsPoint QgsMapToolCapture::mapPoint( const QgsPointXY &point ) const
{
  QgsPoint newPoint( QgsWkbTypes::Point, point.x(), point.y() );

  // get current layer
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
  {
    return newPoint;
  }

  // convert to the corresponding type for a full ZM support
  const QgsWkbTypes::Type type = vlayer->wkbType();
  if ( QgsWkbTypes::hasZ( type ) && !QgsWkbTypes::hasM( type ) )
  {
    newPoint.convertTo( QgsWkbTypes::PointZ );
  }
  else if ( !QgsWkbTypes::hasZ( type ) && QgsWkbTypes::hasM( type ) )
  {
    newPoint.convertTo( QgsWkbTypes::PointM );
  }
  else if ( QgsWkbTypes::hasZ( type ) && QgsWkbTypes::hasM( type ) )
  {
    newPoint.convertTo( QgsWkbTypes::PointZM );
  }

  // set z value if necessary
  if ( QgsWkbTypes::hasZ( newPoint.wkbType() ) )
  {
    newPoint.setZ( defaultZValue() );
  }

  return newPoint;
}

QgsPoint QgsMapToolCapture::mapPoint( const QgsMapMouseEvent &e ) const
{
  QgsPoint newPoint = mapPoint( e.mapPoint() );

  // set z value from snapped point if necessary
  if ( QgsWkbTypes::hasZ( newPoint.wkbType() ) )
  {
    // if snapped, z dimension is taken from the corresponding snapped
    // point.
    if ( e.isSnapped() )
    {
      const QgsPointLocator::Match match = e.mapPointMatch();

      if ( match.layer() && QgsWkbTypes::hasZ( match.layer()->wkbType() ) )
      {
        const QgsFeature ft = match.layer()->getFeature( match.featureId() );
        newPoint.setZ( ft.geometry().vertexAt( match.vertexIndex() ).z() );
      }
    }
  }

  return newPoint;
}

void QgsMapToolCapture::updateExtraSnapLayer()
{
  if ( canvas()->snappingUtils()->config().selfSnapping() && mCanvas->currentLayer() && mCaptureCurve.numPoints() >= 2 )
  {
    // the current layer may have changed
    mExtraSnapLayer->setCrs( mCanvas->currentLayer()->crs() );
    QgsGeometry geom = QgsGeometry( mCaptureCurve.clone() );
    // we close the curve to allow snapping on last segment
    if ( mCaptureMode == CapturePolygon && mCaptureCurve.numPoints() >= 3 )
    {
      qgsgeometry_cast<QgsCompoundCurve *>( geom.get() )->close();
    }
    mExtraSnapLayer->changeGeometry( mExtraSnapFeatureId, geom );
  }
  else
  {
    QgsGeometry geom;
    mExtraSnapLayer->changeGeometry( mExtraSnapFeatureId, geom );
  }
}

QgsMapToolCaptureRubberband::QgsMapToolCaptureRubberband( QgsMapCanvas *mapCanvas, QgsWkbTypes::GeometryType geomType ):
  QgsGeometryRubberBand( mapCanvas, geomType )
{
  setIsVerticesDrawn( false );
}

QgsCurve *QgsMapToolCaptureRubberband::curve()
{
  if ( mPoints.empty() )
    return nullptr;

  switch ( mStringType )
  {
    case QgsWkbTypes::LineString:
      return new QgsLineString( mPoints ) ;
      break;
    case QgsWkbTypes::CircularString:
      if ( mPoints.count() != 3 )
        return nullptr;
      return new QgsCircularString(
               mPoints[0],
               mPoints[1],
               mPoints[2] ) ;
      break;
    default:
      return nullptr;
  }
}

bool QgsMapToolCaptureRubberband::curveIsComplete() const
{
  return ( mStringType == QgsWkbTypes::LineString && mPoints.count() > 1 ) ||
         ( mStringType == QgsWkbTypes::CircularString && mPoints.count() > 2 );
}

void QgsMapToolCaptureRubberband::reset( QgsWkbTypes::GeometryType geomType )
{
  if ( !( geomType == QgsWkbTypes::LineGeometry || geomType == QgsWkbTypes::PolygonGeometry ) )
    return;

  mPoints.clear();
  updateCurve();
}

void QgsMapToolCaptureRubberband::addPoint( const QgsPoint &point, bool doUpdate )
{
  if ( mPoints.count() == 0 )
    mPoints.append( point );

  mPoints.append( point );

  if ( doUpdate )
    updateCurve();
}

void QgsMapToolCaptureRubberband::movePoint( const QgsPoint &point )
{
  if ( mPoints.count() > 0 )
    mPoints.last() = point ;

  updateCurve();
}

void QgsMapToolCaptureRubberband::movePoint( int index, const QgsPoint &point )
{
  if ( mPoints.count() > 0 && mPoints.size() > index )
    mPoints[index] = point;

  updateCurve();
}

int QgsMapToolCaptureRubberband::pointsCount()
{
  return mPoints.size();
}

void QgsMapToolCaptureRubberband::setFirstPolygonPoint( const QgsPoint &point )
{
  mFirstPolygonPoint =  point;
}

QgsWkbTypes::Type QgsMapToolCaptureRubberband::stringType() const
{
  return mStringType;
}

void QgsMapToolCaptureRubberband::setStringType( const QgsWkbTypes::Type &type )
{
  if ( ( type != QgsWkbTypes::CircularString && type != QgsWkbTypes::LineString ) || type == mStringType )
    return;

  mStringType = type;
  if ( type == QgsWkbTypes::LineString && mPoints.count() == 3 )
  {
    mPoints.removeAt( 1 );
  }

  setIsVerticesDrawn( type == QgsWkbTypes::CircularString );
  updateCurve();
}

QgsPoint QgsMapToolCaptureRubberband::lastPoint() const
{
  if ( mPoints.empty() )
    return QgsPoint();

  return mPoints.last();
}

void QgsMapToolCaptureRubberband::removeLastPoint()
{
  if ( mPoints.count() > 1 )
    mPoints.removeLast();

  updateCurve();
}

void QgsMapToolCaptureRubberband::setGeometry( QgsAbstractGeometry *geom )
{
  QgsGeometryRubberBand::setGeometry( geom );
}

void QgsMapToolCaptureRubberband::updateCurve()
{
  std::unique_ptr<QgsCurve> curve;
  switch ( mStringType )
  {
    case  QgsWkbTypes::LineString:
      curve.reset( createLinearString() );
      break;
    case  QgsWkbTypes::CircularString:
      curve.reset( createCircularString() );
      break;
    default:
      return;
      break;
  }

  if ( geometryType() == QgsWkbTypes::PolygonGeometry )
  {
    std::unique_ptr<QgsCurvePolygon> geom( new QgsCurvePolygon );
    geom->setExteriorRing( curve.release() );
    setGeometry( geom.release() );
    return;
  }

  if ( geometryType() == QgsWkbTypes::LineGeometry )
  {
    setGeometry( curve.release() );
  }
}

QgsCurve *QgsMapToolCaptureRubberband::createLinearString()
{
  std::unique_ptr<QgsLineString> curve( new QgsLineString );
  if ( geometryType() == QgsWkbTypes::PolygonGeometry )
  {
    QgsPointSequence points = mPoints;
    points.prepend( mFirstPolygonPoint );
    curve->setPoints( points );
  }
  else
    curve->setPoints( mPoints );

  return curve.release();
}

QgsCurve *QgsMapToolCaptureRubberband::createCircularString()
{
  std::unique_ptr<QgsCircularString> curve( new QgsCircularString );
  curve->setPoints( mPoints );
  if ( geometryType() == QgsWkbTypes::PolygonGeometry )
  {
    // add a linear string to close the polygon
    std::unique_ptr<QgsCompoundCurve> polygonCurve( new QgsCompoundCurve );
    polygonCurve->addVertex( mFirstPolygonPoint );
    if ( !mPoints.empty() )
      polygonCurve->addVertex( mPoints.first() );
    polygonCurve->addCurve( curve.release() );
    return polygonCurve.release();
  }
  else
    return curve.release();
}
