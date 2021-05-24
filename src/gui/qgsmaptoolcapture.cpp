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
#include "qgssettingsregistrycore.h"
#include "qgsapplication.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsproject.h"
#include "qgsgeometryrubberband.h"

#include <QAction>
#include <QCursor>
#include <QPixmap>
#include <QStatusBar>


///@cond PRIVATE

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
  layerOptions.loadDefaultStyle = false;
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
  // during tear down we have to clean up mExtraSnapLayer first, before
  // we call stop capturing. Otherwise stopCapturing tries to access members
  // from the mapcanvas, which is likely already being destroyed and triggering
  // the deletion of this object...
  mCanvas->snappingUtils()->removeExtraSnapLayer( mExtraSnapLayer );
  mExtraSnapLayer->deleteLater();
  mExtraSnapLayer = nullptr;

  stopCapturing();

  if ( mValidator )
  {
    mValidator->deleteLater();
    mValidator = nullptr;
  }
}

QgsMapToolCapture::Capabilities QgsMapToolCapture::capabilities() const
{
  return QgsMapToolCapture::NoCapabilities;
}

bool QgsMapToolCapture::supportsTechnique( QgsMapToolCapture::CaptureTechnique technique ) const
{
  return technique == StraightSegments;
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

  if ( mTempRubberBand )
    mTempRubberBand->setRubberBandGeometryType( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );

  resetRubberBand();
}


bool QgsMapToolCapture::tracingEnabled()
{
  QgsMapCanvasTracer *tracer = QgsMapCanvasTracer::tracerForCanvas( mCanvas );
  return tracer && ( !tracer->actionEnableTracing() || tracer->actionEnableTracing()->isChecked() )
         && ( !tracer->actionEnableSnapping() || tracer->actionEnableSnapping()->isChecked() );
}


QgsPointXY QgsMapToolCapture::tracingStartPoint()
{
  // if we have starting point from previous trace, then preferably use that one
  // (useful when tracing with offset)
  if ( mTracingStartPoint != QgsPointXY() )
    return mTracingStartPoint;

  return lastCapturedMapPoint();
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

  QgsTracer::PathError err;
  QVector<QgsPointXY> points = tracer->findShortestPath( pt0, e->mapPoint(), &err );
  if ( points.isEmpty() )
  {
    tracer->reportError( err, false );
    return false;
  }

  mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, QgsWkbTypes::LineString, firstCapturedMapPoint() );
  mTempRubberBand->addPoint( lastCapturedMapPoint() );

  // if there is offset, we need to fix the rubber bands to make sure they are aligned correctly.
  // There are two cases we need to sort out:
  // 1. the last point of mRubberBand may need to be moved off the traced curve to respect the offset
  // 2. first point of mTempRubberBand may be needed to be moved to the beginning of the offset trace
  QgsPoint lastPoint = lastCapturedMapPoint();
  QgsPointXY lastPointXY( lastPoint );
  if ( lastPointXY == pt0 && points[0] != lastPointXY )
  {
    if ( mRubberBand->numberOfVertices() != 0 )
    {
      // if rubber band had just one point, for some strange reason it contains the point twice
      // we only want to move the last point if there are multiple points already
      if ( mRubberBand->numberOfVertices() > 2 || ( mRubberBand->numberOfVertices() == 2 && *mRubberBand->getPoint( 0, 0 ) != *mRubberBand->getPoint( 0, 1 ) ) )
        mRubberBand->movePoint( points[0] );
    }

    mTempRubberBand->movePoint( 0, QgsPoint( points[0] ) );
  }

  mTempRubberBand->movePoint( QgsPoint( points[0] ) );

  //  update temporary rubberband
  for ( int i = 1; i < points.count(); ++i ) //points added in the rubber band are 2D but will not be added to the capture curve
    mTempRubberBand->addPoint( QgsPoint( points.at( i ) ), i == points.count() - 1 );


  mTempRubberBand->addPoint( QgsPoint( points[points.size() - 1] ) );

  tracer->reportError( QgsTracer::ErrNone, false ); // clear messagebar if there was any error
  return true;
}


bool QgsMapToolCapture::tracingAddVertex( const QgsPointXY &point )
{
  QgsMapCanvasTracer *tracer = QgsMapCanvasTracer::tracerForCanvas( mCanvas );
  if ( !tracer )
    return false;  // this should not happen!

  if ( mTempRubberBand->pointsCount() == 0 )
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
      mTracingStartPoint = point;
    }
    return false;
  }

  QgsPointXY pt0 = tracingStartPoint();
  if ( pt0 == QgsPointXY() )
    return false;

  QgsTracer::PathError err;
  QVector<QgsPointXY> points = tracer->findShortestPath( pt0, point, &err );
  if ( points.isEmpty() )
    return false; // ignore the vertex - can't find path to the end point!

  // transform points
  QgsPointSequence layerPoints;
  QgsPoint lp; // in layer coords
  for ( int i = 0; i < points.count(); ++i )
  {
    if ( nextPoint( QgsPoint( points[i] ), lp ) != 0 )
      return false;
    layerPoints << lp;
  }

  // Move the last point of the captured curve to the first point on the trace string (necessary if there is offset)
  QgsVertexId lastVertexId( 0, 0, mCaptureCurve.numPoints() - 1 );
  mCaptureCurve.moveVertex( lastVertexId, layerPoints.first() );
  mSnappingMatches.removeLast();
  mSnappingMatches.append( QgsPointLocator::Match() );

  int pointBefore = mCaptureCurve.numPoints();
  mCaptureCurve.addCurve( new QgsLineString( layerPoints ) );

  resetRubberBand();

  // Curves de-approximation
  if ( QgsSettingsRegistryCore::settingsDigitizingConvertToCurve.value() )
  {
    // If the tool and the layer support curves
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
    if ( capabilities().testFlag( QgsMapToolCapture::Capability::SupportsCurves ) && vlayer->dataProvider()->capabilities().testFlag( QgsVectorDataProvider::Capability::CircularGeometries ) )
    {
      QgsGeometry linear = QgsGeometry( mCaptureCurve.segmentize() );
      QgsGeometry curved = linear.convertToCurves(
                             QgsSettingsRegistryCore::settingsDigitizingConvertToCurveAngleTolerance.value(),
                             QgsSettingsRegistryCore::settingsDigitizingConvertToCurveDistanceTolerance.value()
                           );
      mCaptureCurve = *qgsgeometry_cast<QgsCompoundCurve *>( curved.constGet() );
    }
  }

  // sync the snapping matches list
  int pointAfter = mCaptureCurve.numPoints();
  for ( ; pointBefore < pointAfter; ++pointBefore )
    mSnappingMatches.append( QgsPointLocator::Match() );

  tracer->reportError( QgsTracer::ErrNone, true ); // clear messagebar if there was any error

  // adjust last captured point
  const QgsPoint lastPt = mCaptureCurve.endPoint();
  mCaptureLastPoint = toMapCoordinates( qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() ), lastPt );

  return true;
}

QgsMapToolCaptureRubberBand *QgsMapToolCapture::createCurveRubberBand() const
{
  QgsMapToolCaptureRubberBand *rb = new QgsMapToolCaptureRubberBand( mCanvas );
  rb->setStrokeWidth( digitizingStrokeWidth() );
  QColor color = digitizingStrokeColor();

  double alphaScale = QgsSettingsRegistryCore::settingsDigitizingLineColorAlphaScale.value();
  color.setAlphaF( color.alphaF() * alphaScale );
  rb->setLineStyle( Qt::DotLine );
  rb->setStrokeColor( color );

  QColor fillColor = digitizingFillColor();
  rb->setFillColor( fillColor );
  rb->show();
  return rb;
}

QgsPoint QgsMapToolCapture::firstCapturedMapPoint()
{
  return mCaptureFirstPoint;
}

QgsPoint QgsMapToolCapture::lastCapturedMapPoint()
{
  return mCaptureLastPoint;
}

void QgsMapToolCapture::resetRubberBand()
{
  if ( !mRubberBand )
    return;
  QgsLineString *lineString = mCaptureCurve.curveToLine();
  mRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  mRubberBand->addGeometry( QgsGeometry( lineString ), vlayer );
}

QgsRubberBand *QgsMapToolCapture::takeRubberBand()
{
  return mRubberBand.release();
}

void QgsMapToolCapture::setCircularDigitizingEnabled( bool enable )
{
  mDigitizingType = enable ? QgsWkbTypes::CircularString : QgsWkbTypes::LineString;
  if ( mTempRubberBand )
    mTempRubberBand->setStringType( mDigitizingType );
}

void QgsMapToolCapture::setStreamDigitizingEnabled( bool enable )
{
  mStreamingEnabled = enable;
  mStartNewCurve = true;
  if ( enable )
  {
    mStreamingToleranceInPixels = QgsSettingsRegistryCore::settingsDigitizingStreamTolerance.value();
  }
}

void QgsMapToolCapture::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsMapToolAdvancedDigitizing::cadCanvasMoveEvent( e );
  QgsPointXY point = e->mapPoint();

  mSnapIndicator->setMatch( e->mapPointMatch() );

  QgsPoint mapPoint = QgsPoint( point );

  if ( mCaptureMode != CapturePoint && mTempRubberBand && mCapturing )
  {
    bool hasTrace = false;

    if ( mStreamingEnabled )
    {
      if ( !mCaptureCurve.isEmpty() )
      {
        QgsPoint prevPoint = mCaptureCurve.curveAt( mCaptureCurve.nCurves() - 1 )->endPoint();
        if ( QgsPointXY( toCanvasCoordinates( toMapCoordinates( mCanvas->currentLayer(), prevPoint ) ) ).distance( toCanvasCoordinates( point ) ) < mStreamingToleranceInPixels )
          return;
      }

      mAllowAddingStreamingPoints = true;
      addVertex( mapPoint );
      mAllowAddingStreamingPoints = false;
    }
    else if ( tracingEnabled() && mCaptureCurve.numPoints() != 0 )
    {
      // Store the intermediate point for circular string to retrieve after tracing mouse move if
      // the digitizing type is circular and the temp rubber band is effectivly circular and if this point is existing
      // Store an empty point if the digitizing type is linear ot the point is not existing (curve not complete)
      if ( mDigitizingType == QgsWkbTypes::CircularString &&
           mTempRubberBand->stringType() == QgsWkbTypes::CircularString &&
           mTempRubberBand->curveIsComplete() )
        mCircularItermediatePoint = mTempRubberBand->pointFromEnd( 1 );
      else if ( mDigitizingType == QgsWkbTypes::LineString ||
                !mTempRubberBand->curveIsComplete() )
        mCircularItermediatePoint = QgsPoint();

      hasTrace = tracingMouseMove( e );

      if ( !hasTrace )
      {
        // Restore the temp rubber band
        mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, mDigitizingType, firstCapturedMapPoint() );
        mTempRubberBand->addPoint( lastCapturedMapPoint() );
        if ( !mCircularItermediatePoint.isEmpty() )
        {
          mTempRubberBand->movePoint( mCircularItermediatePoint );
          mTempRubberBand->addPoint( mCircularItermediatePoint );
        }
      }
    }

    if ( !mStreamingEnabled && !hasTrace )
    {
      if ( mCaptureCurve.numPoints() > 0 )
      {
        const QgsPoint mapPt = lastCapturedMapPoint();

        if ( mTempRubberBand )
        {
          mTempRubberBand->movePoint( mapPoint );
          mTempRubberBand->movePoint( 0, mapPt );
        }

        // fix existing rubber band after tracing - the last point may have been moved if using offset
        if ( mRubberBand->numberOfVertices() )
          mRubberBand->movePoint( mapPt );
      }
      else if ( mTempRubberBand )
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
      layerPoint.addMValue( defaultMValue() );
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
  if ( match.isValid() && ( match.hasVertex() || match.hasLineEndpoint() || ( QgsProject::instance()->topologicalEditing() && ( match.hasEdge() || match.hasMiddleSegment() ) ) ) && sourceLayer &&
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
      if ( QgsProject::instance()->topologicalEditing() && ( match.hasEdge() || match.hasMiddleSegment() ) )
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
          layerPoint.addMValue( defaultMValue() );
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

  if ( mCapturing && mStreamingEnabled && !mAllowAddingStreamingPoints )
    return 0;

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

  if ( mCaptureMode == CapturePoint )
  {
    mCaptureCurve.addVertex( layerPoint );
    mSnappingMatches.append( match );
  }
  else
  {
    if ( mCaptureFirstPoint.isEmpty() )
    {
      mCaptureFirstPoint = mapPoint;
    }

    if ( !mRubberBand )
      mRubberBand.reset( createRubberBand( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry ) );

    if ( !mTempRubberBand )
    {
      mTempRubberBand.reset( createCurveRubberBand() );
      mTempRubberBand->setStringType( mDigitizingType );
      mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, mDigitizingType, mapPoint );
    }

    bool traceCreated = false;
    if ( tracingEnabled() )
    {
      traceCreated = tracingAddVertex( mapPoint );
    }

    // keep new tracing start point if we created a trace. This is useful when tracing with
    // offset so that the user stays "snapped"
    mTracingStartPoint = traceCreated ? point : QgsPointXY();

    if ( !traceCreated )
    {
      // ordinary digitizing
      mTempRubberBand->movePoint( mapPoint ); //move the last point of the temp rubberband before operating with it
      if ( mTempRubberBand->curveIsComplete() ) //2 points for line and 3 points for circular
      {
        const QgsCurve *curve = mTempRubberBand->curve();
        if ( curve )
        {
          addCurve( curve->clone() );
          // add curve append only invalid match to mSnappingMatches,
          // so we need to remove them and add the one from here if it is valid
          if ( match.isValid() && mSnappingMatches.count() > 0 && !mSnappingMatches.last().isValid() )
          {
            mSnappingMatches.removeLast();
            if ( mTempRubberBand->stringType() == QgsWkbTypes::CircularString )
            {
              // for circular string two points are added and match for intermediate point is stored
              mSnappingMatches.removeLast();
              mSnappingMatches.append( mCircularIntermediateMatch );
            }
            mSnappingMatches.append( match );
          }
        }
        mCaptureLastPoint = mapPoint;
        mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, mDigitizingType, firstCapturedMapPoint() );
      }
      else if ( mTempRubberBand->pointsCount() == 0 )
      {
        mCaptureLastPoint = mapPoint;
        mCaptureCurve.addVertex( layerPoint );
        mSnappingMatches.append( match );
      }
      else
      {
        if ( mTempRubberBand->stringType() == QgsWkbTypes::CircularString )
        {
          mCircularIntermediateMatch = match;
        }
      }

      mTempRubberBand->addPoint( mapPoint );
    }
    else
    {
      mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, mDigitizingType, firstCapturedMapPoint() );
      mTempRubberBand->addPoint( lastCapturedMapPoint() );
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

  if ( mTempRubberBand )
  {
    mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, mDigitizingType, firstCapturedMapPoint() );
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
  //if there is only one point, this the first digitized point that are in the this first curve added --> remove the point
  if ( mCaptureCurve.numPoints() == 1 )
    mCaptureCurve.removeCurve( 0 );

  // we set the extendPrevious option to true to avoid creating compound curves with many 2 vertex linestrings -- instead we prefer
  // to extend linestring curves so that they continue the previous linestring wherever possible...
  mCaptureCurve.addCurve( c, !mStartNewCurve );
  mStartNewCurve = false;

  int countAfter = mCaptureCurve.vertexCount();
  int addedPoint = countAfter - countBefore;

  updateExtraSnapLayer();

  for ( int i = 0; i < addedPoint; ++i )
    mSnappingMatches.append( QgsPointLocator::Match() );

  resetRubberBand();

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

void QgsMapToolCapture::undo( bool isAutoRepeat )
{
  mTracingStartPoint = QgsPointXY();

  if ( mTempRubberBand )
  {
    if ( size() <= 1 && mTempRubberBand->pointsCount() != 0 )
      return;

    if ( isAutoRepeat && mIgnoreSubsequentAutoRepeatUndo )
      return;
    mIgnoreSubsequentAutoRepeatUndo = false;

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
    if ( mCaptureCurve.numPoints() == 2 && mCaptureCurve.nCurves() == 1 )
    {
      // store the first vertex to restore if after deleting the curve
      // because when only two vertices, removing a point remove all the curve
      QgsPoint fp = mCaptureCurve.startPoint();
      mCaptureCurve.deleteVertex( vertexToRemove );
      mCaptureCurve.addVertex( fp );
    }
    else
    {
      const int curvesBefore = mCaptureCurve.nCurves();
      const bool lastCurveIsLineString = qgsgeometry_cast< QgsLineString * >( mCaptureCurve.curveAt( curvesBefore - 1 ) );

      int pointsCountBefore = mCaptureCurve.numPoints();
      mCaptureCurve.deleteVertex( vertexToRemove );
      int pointsCountAfter = mCaptureCurve.numPoints();
      for ( ; pointsCountAfter < pointsCountBefore; pointsCountAfter++ )
        if ( !mSnappingMatches.empty() )
          mSnappingMatches.removeLast();

      // if we have removed the last point in a linestring curve, then we "stick" here and ignore subsequent
      // autorepeat undo actions until the user releases the undo key and holds it down again. This allows
      // users to selectively remove portions of the geometry captured with the streaming mode by holding down
      // the undo key, without risking accidental undo of non-streamed portions.
      if ( mCaptureCurve.nCurves() < curvesBefore && lastCurveIsLineString )
        mIgnoreSubsequentAutoRepeatUndo = true;
    }

    updateExtraSnapLayer();

    resetRubberBand();

    mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, mDigitizingType, firstCapturedMapPoint() );

    if ( mCaptureCurve.numPoints() > 0 )
    {
      const QgsPoint lastPt = mCaptureCurve.endPoint();
      mCaptureLastPoint = toMapCoordinates( qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() ), lastPt );
      mTempRubberBand->addPoint( lastCapturedMapPoint() );
      mTempRubberBand->movePoint( lastPoint );
    }

    mCadDockWidget->removePreviousPoint();
    validateGeometry();
  }
}

void QgsMapToolCapture::keyPressEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
  {
    undo( e->isAutoRepeat() );

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

  mCaptureFirstPoint = QgsPoint();
  mCaptureLastPoint = QgsPoint();

  mTracingStartPoint = QgsPointXY();

  mCapturing = false;
  mCaptureCurve.clear();
  updateExtraSnapLayer();
  mSnappingMatches.clear();
  if ( auto *lCurrentVectorLayer = currentVectorLayer() )
    lCurrentVectorLayer->triggerRepaint();
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
  if ( QgsSettingsRegistryCore::settingsDigitizingValidateGeometries.value() == 0 )
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
  if ( QgsSettingsRegistryCore::settingsDigitizingValidateGeometries.value() == 2 )
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
  resetRubberBand();
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
  resetRubberBand();
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
  // set m value if necessary
  if ( QgsWkbTypes::hasM( newPoint.wkbType() ) )
  {
    newPoint.setM( defaultMValue() );
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
  if ( !mExtraSnapLayer )
    return;

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

QgsMapToolCaptureRubberBand::QgsMapToolCaptureRubberBand( QgsMapCanvas *mapCanvas, QgsWkbTypes::GeometryType geomType ):
  QgsGeometryRubberBand( mapCanvas, geomType )
{
  setVertexDrawingEnabled( false );
}

QgsCurve *QgsMapToolCaptureRubberBand::curve()
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

bool QgsMapToolCaptureRubberBand::curveIsComplete() const
{
  return ( mStringType == QgsWkbTypes::LineString && mPoints.count() > 1 ) ||
         ( mStringType == QgsWkbTypes::CircularString && mPoints.count() > 2 );
}

void QgsMapToolCaptureRubberBand::reset( QgsWkbTypes::GeometryType geomType, QgsWkbTypes::Type stringType,  const QgsPoint &firstPolygonPoint )
{
  if ( !( geomType == QgsWkbTypes::LineGeometry || geomType == QgsWkbTypes::PolygonGeometry ) )
    return;

  mPoints.clear();
  mFirstPolygonPoint = firstPolygonPoint;
  setStringType( stringType );
  setRubberBandGeometryType( geomType );
}

void QgsMapToolCaptureRubberBand::setRubberBandGeometryType( QgsWkbTypes::GeometryType geomType )
{
  QgsGeometryRubberBand::setGeometryType( geomType );
  updateCurve();
}

void QgsMapToolCaptureRubberBand::addPoint( const QgsPoint &point, bool doUpdate )
{
  if ( mPoints.count() == 0 )
    mPoints.append( point );

  mPoints.append( point );

  if ( doUpdate )
    updateCurve();
}

void QgsMapToolCaptureRubberBand::movePoint( const QgsPoint &point )
{
  if ( mPoints.count() > 0 )
    mPoints.last() = point ;

  updateCurve();
}

void QgsMapToolCaptureRubberBand::movePoint( int index, const QgsPoint &point )
{
  if ( mPoints.count() > 0 && mPoints.size() > index )
    mPoints[index] = point;

  updateCurve();
}

int QgsMapToolCaptureRubberBand::pointsCount()
{
  return mPoints.size();
}

QgsWkbTypes::Type QgsMapToolCaptureRubberBand::stringType() const
{
  return mStringType;
}

void QgsMapToolCaptureRubberBand::setStringType( const QgsWkbTypes::Type &type )
{
  if ( ( type != QgsWkbTypes::CircularString && type != QgsWkbTypes::LineString ) || type == mStringType )
    return;

  mStringType = type;
  if ( type == QgsWkbTypes::LineString && mPoints.count() == 3 )
  {
    mPoints.removeAt( 1 );
  }

  setVertexDrawingEnabled( type == QgsWkbTypes::CircularString );
  updateCurve();
}

QgsPoint QgsMapToolCaptureRubberBand::lastPoint() const
{
  if ( mPoints.empty() )
    return QgsPoint();

  return mPoints.last();
}

QgsPoint QgsMapToolCaptureRubberBand::pointFromEnd( int posFromEnd ) const
{
  if ( posFromEnd < mPoints.size() )
    return mPoints.at( mPoints.size() - 1 - posFromEnd );
  else
    return QgsPoint();
}

void QgsMapToolCaptureRubberBand::removeLastPoint()
{
  if ( mPoints.count() > 1 )
    mPoints.removeLast();

  updateCurve();
}

void QgsMapToolCaptureRubberBand::setGeometry( QgsAbstractGeometry *geom )
{
  QgsGeometryRubberBand::setGeometry( geom );
}

void QgsMapToolCaptureRubberBand::updateCurve()
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
  }
  else
  {
    setGeometry( curve.release() );
  }
}

QgsCurve *QgsMapToolCaptureRubberBand::createLinearString()
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

QgsCurve *QgsMapToolCaptureRubberBand::createCircularString()
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

///@endcond PRIVATE
