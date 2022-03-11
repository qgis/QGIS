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
#include "qgsmaptoolcapturerubberband.h"
#include "qgsmaptoolshapeabstract.h"
#include "qgsmaptoolshaperegistry.h"
#include "qgsgui.h"

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
  return QgsMapToolCapture::ValidateGeometries;
}

bool QgsMapToolCapture::supportsTechnique( Qgis::CaptureTechnique technique ) const
{
  switch ( technique )
  {
    case Qgis::CaptureTechnique::StraightSegments:
      return true;
    case Qgis::CaptureTechnique::CircularString:
    case Qgis::CaptureTechnique::Streaming:
    case Qgis::CaptureTechnique::Shape:
      return false;
  }
  BUILTIN_UNREACHABLE
}

void QgsMapToolCapture::activate()
{
  if ( mTempRubberBand )
    mTempRubberBand->show();

  mCanvas->snappingUtils()->addExtraSnapLayer( mExtraSnapLayer );
  QgsMapToolAdvancedDigitizing::activate();

  if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape && mCurrentShapeMapTool )
    mCurrentShapeMapTool->activate( mCaptureMode, mCaptureLastPoint );
}

void QgsMapToolCapture::deactivate()
{
  if ( mTempRubberBand )
    mTempRubberBand->hide();

  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  mCanvas->snappingUtils()->removeExtraSnapLayer( mExtraSnapLayer );

  if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape && mCurrentShapeMapTool )
    mCurrentShapeMapTool->deactivate();

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
  cadDockWidget()->switchZM();
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

  return mCaptureLastPoint;
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

  mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, QgsWkbTypes::LineString, mCaptureFirstPoint );
  mTempRubberBand->addPoint( mCaptureLastPoint );

  // if there is offset, we need to fix the rubber bands to make sure they are aligned correctly.
  // There are two cases we need to sort out:
  // 1. the last point of mRubberBand may need to be moved off the traced curve to respect the offset
  // 2. first point of mTempRubberBand may be needed to be moved to the beginning of the offset trace
  const QgsPoint lastPoint = mCaptureLastPoint;
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
    const bool res = tracer->isPointSnapped( point );
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
  const QgsVertexId lastVertexId( 0, 0, mCaptureCurve.numPoints() - 1 );
  mCaptureCurve.moveVertex( lastVertexId, layerPoints.first() );
  mSnappingMatches.removeLast();
  mSnappingMatches.append( QgsPointLocator::Match() );

  int pointBefore = mCaptureCurve.numPoints();
  addCurve( new QgsLineString( layerPoints ) );

  resetRubberBand();

  // Curves de-approximation
  if ( QgsSettingsRegistryCore::settingsDigitizingConvertToCurve.value() )
  {
    // If the tool and the layer support curves
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer() );
    if ( vlayer && capabilities().testFlag( QgsMapToolCapture::Capability::SupportsCurves ) && vlayer->dataProvider()->capabilities().testFlag( QgsVectorDataProvider::Capability::CircularGeometries ) )
    {
      const QgsGeometry linear = QgsGeometry( mCaptureCurve.segmentize() );
      const QgsGeometry curved = linear.convertToCurves(
                                   QgsSettingsRegistryCore::settingsDigitizingConvertToCurveAngleTolerance.value(),
                                   QgsSettingsRegistryCore::settingsDigitizingConvertToCurveDistanceTolerance.value()
                                 );
      mCaptureCurve = *qgsgeometry_cast<QgsCompoundCurve *>( curved.constGet() );
    }
  }

  // sync the snapping matches list
  const int pointAfter = mCaptureCurve.numPoints();
  for ( ; pointBefore < pointAfter; ++pointBefore )
    mSnappingMatches.append( QgsPointLocator::Match() );

  tracer->reportError( QgsTracer::ErrNone, true ); // clear messagebar if there was any error

  // adjust last captured point
  const QgsPoint lastPt = mCaptureCurve.endPoint();
  mCaptureLastPoint = toMapCoordinates( layer(), lastPt );

  return true;
}

QgsMapToolCaptureRubberBand *QgsMapToolCapture::createCurveRubberBand() const
{
  QgsMapToolCaptureRubberBand *rb = new QgsMapToolCaptureRubberBand( mCanvas );
  rb->setStrokeWidth( digitizingStrokeWidth() );
  QColor color = digitizingStrokeColor();

  const double alphaScale = QgsSettingsRegistryCore::settingsDigitizingLineColorAlphaScale.value();
  color.setAlphaF( color.alphaF() * alphaScale );
  rb->setLineStyle( Qt::DotLine );
  rb->setStrokeColor( color );

  const QColor fillColor = digitizingFillColor();
  rb->setFillColor( fillColor );
  rb->show();
  return rb;
}

void QgsMapToolCapture::resetRubberBand()
{
  if ( !mRubberBand )
    return;
  QgsLineString *lineString = mCaptureCurve.curveToLine();
  mRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
  mRubberBand->addGeometry( QgsGeometry( lineString ), layer() );
}

QgsRubberBand *QgsMapToolCapture::takeRubberBand()
{
  return mRubberBand.release();
}

void QgsMapToolCapture::setCircularDigitizingEnabled( bool enable )
{
  if ( enable )
    setCurrentCaptureTechnique( Qgis::CaptureTechnique::CircularString );
  else
    setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );
}

void QgsMapToolCapture::setStreamDigitizingEnabled( bool enable )
{
  if ( enable )
    setCurrentCaptureTechnique( Qgis::CaptureTechnique::Streaming );
  else
    setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );
}

void QgsMapToolCapture::setCurrentCaptureTechnique( Qgis::CaptureTechnique technique )
{
  if ( mCurrentCaptureTechnique == technique )
    return;

  mStartNewCurve = true;

  if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape && mCurrentShapeMapTool )
  {
    mCurrentShapeMapTool->deactivate();
    clean();
  }

  switch ( technique )
  {
    case Qgis::CaptureTechnique::StraightSegments:
      mLineDigitizingType = QgsWkbTypes::LineString;
      break;
    case Qgis::CaptureTechnique::CircularString:
      mLineDigitizingType = QgsWkbTypes::CircularString;
      break;
    case Qgis::CaptureTechnique::Streaming:
      mLineDigitizingType = QgsWkbTypes::LineString;
      mStreamingToleranceInPixels = QgsSettingsRegistryCore::settingsDigitizingStreamTolerance.value();
      break;
    case Qgis::CaptureTechnique::Shape:
      mLineDigitizingType = QgsWkbTypes::LineString;
      break;

  }

  if ( mTempRubberBand )
    mTempRubberBand->setStringType( mLineDigitizingType );

  mCurrentCaptureTechnique = technique;

  if ( technique == Qgis::CaptureTechnique::Shape && mCurrentShapeMapTool && isActive() )
  {
    clean();
    mCurrentShapeMapTool->activate( mCaptureMode, mCaptureLastPoint );
  }
}

void QgsMapToolCapture::setCurrentShapeMapTool( const QgsMapToolShapeMetadata *shapeMapToolMetadata )
{
  if ( mCurrentShapeMapTool )
  {
    if ( shapeMapToolMetadata && mCurrentShapeMapTool->id() == shapeMapToolMetadata->id() )
      return;
    if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape )
      mCurrentShapeMapTool->deactivate();
    mCurrentShapeMapTool->deleteLater();
  }

  mCurrentShapeMapTool = shapeMapToolMetadata ? shapeMapToolMetadata->factory( this ) : nullptr;

  if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape && isActive() )
  {
    clean();
    mCurrentShapeMapTool->activate( mCaptureMode, mCaptureLastPoint );
  }
}

void QgsMapToolCapture::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsMapToolAdvancedDigitizing::cadCanvasMoveEvent( e );

  const QgsPointXY point = e->mapPoint();

  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape )
  {
    if ( !mCurrentShapeMapTool )
    {
      emit messageEmitted( tr( "Cannot capture a shape without a shape tool defined" ), Qgis::MessageLevel::Warning );
    }
    else
    {
      if ( !mTempRubberBand )
      {
        mTempRubberBand.reset( createCurveRubberBand() );
        mTempRubberBand->setStringType( mLineDigitizingType );
        mTempRubberBand->setRubberBandGeometryType( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
      }

      mCurrentShapeMapTool->cadCanvasMoveEvent( e, mCaptureMode );
      return;
    }
  }
  else
  {
    const QgsPoint mapPoint = QgsPoint( point );

    if ( mCaptureMode != CapturePoint && mTempRubberBand && mCapturing )
    {
      bool hasTrace = false;

      if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Streaming )
      {
        if ( !mCaptureCurve.isEmpty() )
        {
          const QgsPoint prevPoint = mCaptureCurve.curveAt( mCaptureCurve.nCurves() - 1 )->endPoint();
          if ( QgsPointXY( toCanvasCoordinates( toMapCoordinates( layer(), prevPoint ) ) ).distance( toCanvasCoordinates( point ) ) < mStreamingToleranceInPixels )
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
        if ( mLineDigitizingType == QgsWkbTypes::CircularString &&
             mTempRubberBand->stringType() == QgsWkbTypes::CircularString &&
             mTempRubberBand->curveIsComplete() )
          mCircularItermediatePoint = mTempRubberBand->pointFromEnd( 1 );
        else if ( mLineDigitizingType == QgsWkbTypes::LineString ||
                  !mTempRubberBand->curveIsComplete() )
          mCircularItermediatePoint = QgsPoint();

        hasTrace = tracingMouseMove( e );

        if ( !hasTrace )
        {
          // Restore the temp rubber band
          mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, mLineDigitizingType, mCaptureFirstPoint );
          mTempRubberBand->addPoint( mCaptureLastPoint );
          if ( !mCircularItermediatePoint.isEmpty() )
          {
            mTempRubberBand->movePoint( mCircularItermediatePoint );
            mTempRubberBand->addPoint( mCircularItermediatePoint );
          }
        }
      }

      if ( mCurrentCaptureTechnique != Qgis::CaptureTechnique::Streaming && !hasTrace )
      {
        if ( mCaptureCurve.numPoints() > 0 )
        {
          const QgsPoint mapPt = mCaptureLastPoint;

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
  }
} // mouseMoveEvent


int QgsMapToolCapture::nextPoint( const QgsPoint &mapPoint, QgsPoint &layerPoint )
{
  if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer() ) )
  {
    try
    {
      const QgsPointXY mapP( mapPoint.x(), mapPoint.y() );  //#spellok
      layerPoint = QgsPoint( toLayerCoordinates( vlayer, mapP ) ); //transform snapped point back to layer crs  //#spellok
      if ( QgsWkbTypes::hasZ( vlayer->wkbType() ) && !layerPoint.is3D() )
        layerPoint.addZValue( mCadDockWidget && mCadDockWidget->cadEnabled() ? mCadDockWidget->currentPointV2().z() : defaultZValue() );
      if ( QgsWkbTypes::hasM( vlayer->wkbType() ) && !layerPoint.isMeasure() )
        layerPoint.addMValue( mCadDockWidget && mCadDockWidget->cadEnabled() ? mCadDockWidget->currentPointV2().m() : defaultMValue() );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "transformation to layer coordinate failed" ) );
      return 2;
    }
  }
  else
  {
    layerPoint = QgsPoint( toLayerCoordinates( layer(), mapPoint ) );
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
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer() );
  QgsVectorLayer *sourceLayer = match.layer();
  if ( mCadDockWidget && mCadDockWidget->cadEnabled() )
  {
    layerPoint = mCadDockWidget->currentPointLayerCoordinates( layer() );
    return 0;
  }
  else if ( !vlayer )
  {
    return 1;
  }
  else
  {
    if ( match.isValid() && ( match.hasVertex() || match.hasLineEndpoint() || ( QgsProject::instance()->topologicalEditing() && ( match.hasEdge() || match.hasMiddleSegment() ) ) ) && sourceLayer &&
         ( sourceLayer->crs() == vlayer->crs() ) )
    {
      QgsFeature f;
      QgsFeatureRequest request;
      request.setFilterFid( match.featureId() );
      const bool fetched = match.layer()->getFeatures( request ).nextFeature( f );
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
          const QgsLineString line( geom.constGet()->vertexAt( vId ), geom.constGet()->vertexAt( vId2 ) );

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

  if ( mCapturing && mCurrentCaptureTechnique == Qgis::CaptureTechnique::Streaming && !mAllowAddingStreamingPoints )
    return 0;

  QgsPoint layerPoint;
  if ( layer() )
  {
    int res = fetchLayerPoint( match, layerPoint );
    if ( res != 0 )
    {
      res = nextPoint( QgsPoint( point ), layerPoint );
      if ( res != 0 )
      {
        return res;
      }
    }
  }
  const QgsPoint mapPoint = toMapCoordinates( layer(), layerPoint );

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
      mTempRubberBand->setStringType( mLineDigitizingType );
      mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, mLineDigitizingType, mapPoint );
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
        mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, mLineDigitizingType, mCaptureFirstPoint );
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
      mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, mLineDigitizingType, mCaptureFirstPoint );
      mTempRubberBand->addPoint( mCaptureLastPoint );
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
    mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, mLineDigitizingType, mCaptureFirstPoint );
    const QgsPoint endPt = c->endPoint();
    mTempRubberBand->addPoint( endPt ); //add last point of c
  }

  //transform back to layer CRS in case map CRS and layer CRS are different
  const QgsCoordinateTransform ct = mCanvas->mapSettings().layerTransform( layer() );
  if ( ct.isValid() )
  {
    c->transform( ct, Qgis::TransformDirection::Reverse );
  }
  const int countBefore = mCaptureCurve.vertexCount();
  //if there is only one point, this the first digitized point that are in the this first curve added --> remove the point
  if ( mCaptureCurve.numPoints() == 1 )
    mCaptureCurve.removeCurve( 0 );

  // we set the extendPrevious option to true to avoid creating compound curves with many 2 vertex linestrings -- instead we prefer
  // to extend linestring curves so that they continue the previous linestring wherever possible...
  mCaptureCurve.addCurve( c, !mStartNewCurve );
  mStartNewCurve = false;

  const int countAfter = mCaptureCurve.vertexCount();
  const int addedPoint = countAfter - countBefore;

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

    const QgsPoint lastPoint = mTempRubberBand->lastPoint();

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
      const QgsPoint fp = mCaptureCurve.startPoint();
      mCaptureCurve.deleteVertex( vertexToRemove );
      mCaptureCurve.addVertex( fp );
    }
    else
    {
      const int curvesBefore = mCaptureCurve.nCurves();
      const bool lastCurveIsLineString = qgsgeometry_cast< QgsLineString * >( mCaptureCurve.curveAt( curvesBefore - 1 ) );

      const int pointsCountBefore = mCaptureCurve.numPoints();
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

    mTempRubberBand->reset( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, mLineDigitizingType, mCaptureFirstPoint );

    if ( mCaptureCurve.numPoints() > 0 )
    {
      const QgsPoint lastPt = mCaptureCurve.endPoint();
      mCaptureLastPoint = toMapCoordinates( layer(), lastPt );
      mTempRubberBand->addPoint( mCaptureLastPoint );
      mTempRubberBand->movePoint( lastPoint );
    }

    mCadDockWidget->removePreviousPoint();
    validateGeometry();
  }
}

void QgsMapToolCapture::keyPressEvent( QKeyEvent *e )
{
  if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape && mCurrentShapeMapTool )
  {
    mCurrentShapeMapTool->keyPressEvent( e );
    if ( e->isAccepted() )
      return;
  }

  // this is backwards, but we can't change now without breaking api because
  // forever QgsMapTools have had to explicitly mark events as ignored in order to
  // indicate that they've consumed the event and that the default behavior should not
  // be applied..!
  // see QgsMapCanvas::keyPressEvent
  e->accept();

  if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
  {
    if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape && mCurrentShapeMapTool )
    {
      if ( !e->isAutoRepeat() )
      {
        mCurrentShapeMapTool->undo();
      }
    }
    else
    {
      undo( e->isAutoRepeat() );
    }

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
  else if ( e->key() == Qt::Key_Escape )
  {
    if ( mCurrentShapeMapTool )
      mCurrentShapeMapTool->clean();

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
  if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape && mCurrentShapeMapTool )
    mCurrentShapeMapTool->clean();

  clearCurve();
}

void QgsMapToolCapture::closePolygon()
{
  mCaptureCurve.close();
  updateExtraSnapLayer();
}

void QgsMapToolCapture::validateGeometry()
{
  if ( QgsSettingsRegistryCore::settingsDigitizingValidateGeometries.value() == 0
       || !( capabilities() & ValidateGeometries )
     )
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

  Qgis::GeometryValidationEngine method = Qgis::GeometryValidationEngine::QgisInternal;
  if ( QgsSettingsRegistryCore::settingsDigitizingValidateGeometries.value() == 2 )
    method = Qgis::GeometryValidationEngine::Geos;
  mValidator = new QgsGeometryValidator( geom, nullptr, method );
  connect( mValidator, &QgsGeometryValidator::errorFound, this, &QgsMapToolCapture::addError );
  mValidator->start();
  QgsDebugMsgLevel( QStringLiteral( "Validation started" ), 4 );
}

void QgsMapToolCapture::addError( const QgsGeometry::Error &e )
{
  mGeomErrors << e;
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer() );
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
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer() );
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
    newPoint.setZ( mCadDockWidget && mCadDockWidget->cadEnabled() ? mCadDockWidget->getLineZ() : defaultZValue() );
  }
  // set m value if necessary
  if ( QgsWkbTypes::hasM( newPoint.wkbType() ) )
  {
    newPoint.setM( mCadDockWidget && mCadDockWidget->cadEnabled() ? mCadDockWidget->getLineM() : defaultMValue() );
  }
  return newPoint;
}

QgsPoint QgsMapToolCapture::mapPoint( const QgsMapMouseEvent &e ) const
{
  QgsPoint newPoint = mapPoint( e.mapPoint() );

  // set z or m value from snapped point if necessary
  if ( QgsWkbTypes::hasZ( newPoint.wkbType() ) || QgsWkbTypes::hasM( newPoint.wkbType() ) )
  {
    // if snapped, z and m dimension are taken from the corresponding snapped
    // point.
    if ( e.isSnapped() )
    {
      const QgsPointLocator::Match match = e.mapPointMatch();

      if ( match.layer() )
      {
        const QgsFeature ft = match.layer()->getFeature( match.featureId() );
        if ( QgsWkbTypes::hasZ( match.layer()->wkbType() ) )
        {
          newPoint.setZ( ft.geometry().vertexAt( match.vertexIndex() ).z() );
        }
        if ( QgsWkbTypes::hasM( match.layer()->wkbType() ) )
        {
          newPoint.setM( ft.geometry().vertexAt( match.vertexIndex() ).m() );
        }
      }
    }
  }

  return newPoint;
}

void QgsMapToolCapture::updateExtraSnapLayer()
{
  if ( !mExtraSnapLayer )
    return;

  if ( canvas()->snappingUtils()->config().selfSnapping() && layer() && mCaptureCurve.numPoints() >= 2 )
  {
    // the current layer may have changed
    mExtraSnapLayer->setCrs( layer()->crs() );
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


void QgsMapToolCapture::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  // POINT CAPTURING
  if ( mode() == CapturePoint )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    QgsPoint savePoint; //point in layer coordinates
    bool isMatchPointZ = false;
    bool isMatchPointM = false;
    try
    {
      QgsPoint fetchPoint;
      int res = fetchLayerPoint( e->mapPointMatch(), fetchPoint );
      isMatchPointZ = QgsWkbTypes::hasZ( fetchPoint.wkbType() );
      isMatchPointM = QgsWkbTypes::hasM( fetchPoint.wkbType() );

      if ( res == 0 )
      {
        QgsWkbTypes::Type geomType = QgsWkbTypes::Type::Point;
        if ( isMatchPointM && isMatchPointZ )
        {
          geomType = QgsWkbTypes::Type::PointZM;
        }
        else if ( isMatchPointM )
        {
          geomType = QgsWkbTypes::Type::PointM;
        }
        else if ( isMatchPointZ )
        {
          geomType = QgsWkbTypes::Type::PointZ;
        }
        savePoint = QgsPoint( geomType, fetchPoint.x(), fetchPoint.y(), fetchPoint.z(), fetchPoint.m() );
      }
      else
      {
        QgsPointXY point = mCanvas->mapSettings().mapToLayerCoordinates( layer(), e->mapPoint() );

        savePoint = QgsPoint( point.x(), point.y(), fetchPoint.z(), fetchPoint.m() );
      }
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse )
      emit messageEmitted( tr( "Cannot transform the point to the layer's coordinate system" ), Qgis::MessageLevel::Warning );
      return;
    }

    QgsGeometry g( std::make_unique<QgsPoint>( savePoint ) );

    // The snapping result needs to be added so it's available in the @snapping_results variable of default value etc. expression contexts
    addVertex( e->mapPoint(), e->mapPointMatch() );

    geometryCaptured( g );
    pointCaptured( savePoint );

    stopCapturing();

    // we are done with digitizing for now so instruct advanced digitizing dock to reset its CAD points
    cadDockWidget()->clearPoints();
  }

  // LINE AND POLYGON CAPTURING
  else if ( mode() == CaptureLine || mode() == CapturePolygon )
  {
    bool digitizingFinished = false;

    if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape )
    {
      if ( !mCurrentShapeMapTool )
      {
        emit messageEmitted( tr( "Cannot capture a shape without a shape tool defined" ), Qgis::MessageLevel::Warning );
        return;
      }
      else
      {
        if ( !mTempRubberBand )
        {
          mTempRubberBand.reset( createCurveRubberBand() );
          mTempRubberBand->setStringType( mLineDigitizingType );
          mTempRubberBand->setRubberBandGeometryType( mCaptureMode == CapturePolygon ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
        }

        digitizingFinished = mCurrentShapeMapTool->cadCanvasReleaseEvent( e, mCaptureMode );
        if ( digitizingFinished )
          mCurrentShapeMapTool->clean();
      }
    }
    else // i.e. not shape
    {
      //add point to list and to rubber band
      if ( e->button() == Qt::LeftButton )
      {
        const int error = addVertex( e->mapPoint(), e->mapPointMatch() );
        if ( error == 2 )
        {
          //problem with coordinate transformation
          emit messageEmitted( tr( "Cannot transform the point to the layers coordinate system" ), Qgis::MessageLevel::Warning );
          return;
        }

        startCapturing();
      }
      else if ( e->button() == Qt::RightButton )
      {
        // End of string
        deleteTempRubberBand();

        //lines: bail out if there are not at least two vertices
        if ( mode() == CaptureLine && size() < 2 )
        {
          stopCapturing();
          return;
        }

        //polygons: bail out if there are not at least two vertices
        if ( mode() == CapturePolygon && size() < 3 )
        {
          stopCapturing();
          return;
        }

        if ( mode() == CapturePolygon || e->modifiers() == Qt::ShiftModifier )
        {
          closePolygon();
        }

        digitizingFinished = true;
      }
    }

    if ( digitizingFinished )
    {
      QgsGeometry g;
      QgsCurve *curveToAdd = captureCurve()->clone();

      if ( mode() == CaptureLine )
      {
        g = QgsGeometry( curveToAdd );
        geometryCaptured( g );
        lineCaptured( curveToAdd );
      }
      else
      {
        QgsCurvePolygon *poly = new QgsCurvePolygon();
        poly->setExteriorRing( curveToAdd );
        g = QgsGeometry( poly );
        geometryCaptured( g );
        polygonCaptured( poly );
      }

      stopCapturing();
    }
  }
}
