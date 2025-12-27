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

#include <algorithm>
#include <memory>

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsapplication.h"
#include "qgsbezierdata.h"
#include "qgsbeziermarker.h"
#include "qgscompoundcurve.h"
#include "qgsexception.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometryvalidator.h"
#include "qgslinestring.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvastracer.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptoolcapturerubberband.h"
#include "qgsmaptoolshapeabstract.h"
#include "qgsmaptoolshaperegistry.h"
#include "qgsnurbscurve.h"
#include "qgspolygon.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsregistrycore.h"
#include "qgssnapindicator.h"
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"
#include "qgsvertexmarker.h"

#include <QAction>
#include <QCursor>
#include <QPixmap>
#include <QStatusBar>
#include <QWheelEvent>

#include "moc_qgsmaptoolcapture.cpp"

QgsMapToolCapture::QgsMapToolCapture( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode )
  : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
  , mCaptureMode( mode )
  , mCaptureModeFromLayer( mode == CaptureNone )
{
  mTempRubberBand.setParentOwner( canvas );

  mSnapIndicator = std::make_unique<QgsSnapIndicator>( canvas );

  setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::CapturePoint ) );

  connect( canvas, &QgsMapCanvas::currentLayerChanged, this, &QgsMapToolCapture::currentLayerChanged );

  QgsVectorLayer::LayerOptions layerOptions;
  layerOptions.skipCrsValidation = true;
  layerOptions.loadDefaultStyle = false;
  mExtraSnapLayer = new QgsVectorLayer( QStringLiteral( "LineString?crs=" ), QStringLiteral( "extra snap" ), QStringLiteral( "memory" ), layerOptions );
  mExtraSnapLayer->startEditing();
  QgsFeature f;
  mExtraSnapLayer->addFeature( f );
  mExtraSnapFeatureId = f.id();

  connect( QgsProject::instance(), &QgsProject::snappingConfigChanged, this, &QgsMapToolCapture::updateExtraSnapLayer );

  currentLayerChanged( canvas->currentLayer() );
}

QgsMapToolCapture::~QgsMapToolCapture()
{
  // during tear down we have to clean up mExtraSnapLayer first, before
  // we call stop capturing. Otherwise stopCapturing tries to access members
  // from the mapcanvas, which is likely already being destroyed and triggering
  // the deletion of this object...
  if ( mCanvas )
  {
    mCanvas->snappingUtils()->removeExtraSnapLayer( mExtraSnapLayer );
  }
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
    case Qgis::CaptureTechnique::NurbsCurve:
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

  if ( vlayer->isSpatial() )
  {
    setCursor( QgsApplication::getThemeCursor( QgsApplication::Cursor::CapturePoint ) );
  }
  else
  {
    setCursor( QCursor( Qt::ArrowCursor ) );
  }

  switch ( vlayer->geometryType() )
  {
    case Qgis::GeometryType::Point:
      mCaptureMode = CapturePoint;
      break;
    case Qgis::GeometryType::Line:
      mCaptureMode = CaptureLine;
      break;
    case Qgis::GeometryType::Polygon:
      mCaptureMode = CapturePolygon;
      break;
    default:
      mCaptureMode = CaptureNone;
      break;
  }

  if ( mTempRubberBand )
    mTempRubberBand->setRubberBandGeometryType( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line );

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
    return false; // this should not happen!

  QgsTracer::PathError err;
  QVector<QgsPointXY> points = tracer->findShortestPath( pt0, e->mapPoint(), &err );
  if ( points.isEmpty() )
  {
    tracer->reportError( err, false );
    return false;
  }

  mTempRubberBand->reset( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line, Qgis::WkbType::LineString, mCaptureFirstPoint );
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
    return false; // this should not happen!

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
  const QVector<QgsPointXY> tracedPointsInMapCrs = tracer->findShortestPath( pt0, point, &err );
  if ( tracedPointsInMapCrs.isEmpty() )
    return false; // ignore the vertex - can't find path to the end point!

  // transform points
  QgsPointSequence layerPoints;
  layerPoints.reserve( tracedPointsInMapCrs.size() );
  QgsPointSequence mapPoints;
  mapPoints.reserve( tracedPointsInMapCrs.size() );
  for ( const QgsPointXY &tracedPointMapCrs : tracedPointsInMapCrs )
  {
    QgsPoint mapPoint( tracedPointMapCrs );

    QgsPoint lp; // in layer coords
    if ( nextPoint( mapPoint, lp ) != 0 )
      return false;

    // copy z and m from layer point back to mapPoint, as nextPoint() call will populate these based
    // on the context of the trace
    if ( lp.is3D() )
      mapPoint.addZValue( lp.z() );
    if ( lp.isMeasure() )
      mapPoint.addMValue( lp.m() );

    mapPoints << mapPoint;
    layerPoints << lp;
  }

  // Move the last point of the captured curve to the first point on the trace string (necessary if there is offset)
  const QgsVertexId lastVertexId( 0, 0, mCaptureCurve.numPoints() - 1 );
  mCaptureCurve.moveVertex( lastVertexId, layerPoints.first() );
  mSnappingMatches.removeLast();
  mSnappingMatches.append( QgsPointLocator::Match() );

  addCurve( new QgsLineString( mapPoints ) );

  resetRubberBand();

  // Curves de-approximation
  if ( QgsSettingsRegistryCore::settingsDigitizingConvertToCurve->value() )
  {
    // If the tool and the layer support curves
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer() );
    if ( vlayer && capabilities().testFlag( QgsMapToolCapture::Capability::SupportsCurves ) && vlayer->dataProvider()->capabilities().testFlag( Qgis::VectorProviderCapability::CircularGeometries ) )
    {
      const QgsGeometry linear = QgsGeometry( mCaptureCurve.segmentize() );
      const QgsGeometry curved = linear.convertToCurves(
        QgsSettingsRegistryCore::settingsDigitizingConvertToCurveDistanceTolerance->value(),
        QgsSettingsRegistryCore::settingsDigitizingConvertToCurveAngleTolerance->value()
      );
      if ( QgsWkbTypes::flatType( curved.wkbType() ) != Qgis::WkbType::CompoundCurve )
      {
        mCaptureCurve.clear();
        mCaptureCurve.addCurve( qgsgeometry_cast<const QgsCurve *>( curved.constGet() )->clone() );
      }
      else
      {
        mCaptureCurve = *qgsgeometry_cast<const QgsCompoundCurve *>( curved.constGet() );
      }
    }

    mSnappingMatches.resize( mCaptureCurve.numPoints() );
  }

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

  const double alphaScale = QgsSettingsRegistryCore::settingsDigitizingLineColorAlphaScale->value();
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

  mRubberBand->reset( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line );
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
      mLineDigitizingType = Qgis::WkbType::LineString;
      break;
    case Qgis::CaptureTechnique::CircularString:
      mLineDigitizingType = Qgis::WkbType::CircularString;
      break;
    case Qgis::CaptureTechnique::Streaming:
      mLineDigitizingType = Qgis::WkbType::LineString;
      mStreamingToleranceInPixels = QgsSettingsRegistryCore::settingsDigitizingStreamTolerance->value();
      break;
    case Qgis::CaptureTechnique::Shape:
      mLineDigitizingType = Qgis::WkbType::LineString;
      break;
    case Qgis::CaptureTechnique::NurbsCurve:
      mLineDigitizingType = Qgis::WkbType::NurbsCurve;
      break;
  }

  if ( mTempRubberBand )
    mTempRubberBand->setStringType( mLineDigitizingType );

  mCurrentCaptureTechnique = technique;

  // Emit help message for Poly-Bézier mode
  if ( technique == Qgis::CaptureTechnique::NurbsCurve
       && QgsSettingsRegistryCore::settingsDigitizingNurbsMode->value() == Qgis::NurbsMode::PolyBezier )
  {
    emit messageEmitted( tr( "Bézier editing: click and drag to add anchor with symmetric handles, click on handle/anchor to edit, Alt+click on anchor to extend handles, right-click to finish" ), Qgis::MessageLevel::Info );
  }

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

  mCurrentShapeMapTool.reset( shapeMapToolMetadata ? shapeMapToolMetadata->factory( this ) : nullptr );

  if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape && isActive() )
  {
    clean();
    if ( mCurrentShapeMapTool )
      mCurrentShapeMapTool->activate( mCaptureMode, mCaptureLastPoint );
  }
}

void QgsMapToolCapture::cadCanvasPressEvent( QgsMapMouseEvent *e )
{
  // Poly-Bézier mode: handle press to add anchor and start drag
  if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::NurbsCurve
       && QgsSettingsRegistryCore::settingsDigitizingNurbsMode->value() == Qgis::NurbsMode::PolyBezier
       && ( mode() == CaptureLine || mode() == CapturePolygon ) )
  {
    if ( e->button() == Qt::LeftButton )
    {
      const QgsPoint mapPoint = QgsPoint( e->mapPoint() );

      // Initialize Bézier structures if needed
      if ( !mBezierData )
        mBezierData = std::make_unique<QgsBezierData>();
      if ( !mBezierMarker )
        mBezierMarker = std::make_unique<QgsBezierMarker>( mCanvas );

      // Calculate tolerance in map units (10 pixels)
      const double tolerance = mCanvas->mapUnitsPerPixel() * 10;

      // Reset drag indices
      mBezierDragAnchorIndex = -1;
      mBezierDragHandleIndex = -1;
      mBezierMoveAnchorIndex = -1;

      // First, check if clicking on an existing handle
      const int handleIdx = mBezierData->findClosestHandle( mapPoint, tolerance );
      if ( handleIdx >= 0 )
      {
        // Start dragging this handle independently
        mBezierDragHandleIndex = handleIdx;
        mBezierDragging = true;
        mBezierMarker->setHighlightedHandle( handleIdx );
        emit messageEmitted( tr( "Bézier editing: drag to move handle, release to confirm" ), Qgis::MessageLevel::Info );
        return;
      }

      // Second, check if clicking on an existing anchor
      const int anchorIdx = mBezierData->findClosestAnchor( mapPoint, tolerance );
      if ( anchorIdx >= 0 )
      {
        if ( e->modifiers() & Qt::AltModifier )
        {
          // Alt+click on anchor: extend handles symmetrically (like creating new anchor)
          mBezierDragAnchorIndex = anchorIdx;
          mBezierDragging = true;
          mBezierMarker->setHighlightedAnchor( anchorIdx );
          emit messageEmitted( tr( "Bézier editing: drag to extend handles symmetrically, release to confirm" ), Qgis::MessageLevel::Info );
        }
        else
        {
          // Normal click: start moving this anchor
          mBezierMoveAnchorIndex = anchorIdx;
          mBezierDragging = true;
          mBezierMarker->setHighlightedAnchor( anchorIdx );
          emit messageEmitted( tr( "Bézier editing: drag to move anchor, release to confirm" ), Qgis::MessageLevel::Info );
        }
        return;
      }

      // Otherwise, add new anchor and start symmetric handle drag
      mBezierData->addAnchor( mapPoint );
      mBezierDragAnchorIndex = mBezierData->anchorCount() - 1;
      mBezierDragging = true;
      emit messageEmitted( tr( "Bézier editing: drag to define handles, release to confirm anchor" ), Qgis::MessageLevel::Info );

      // Update visualization
      mBezierMarker->updateFromData( *mBezierData );

      startCapturing();
      return;
    }
  }

  // Default handling for other modes
  QgsMapToolAdvancedDigitizing::cadCanvasPressEvent( e );
}

void QgsMapToolCapture::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  // If we are adding a record to a non-spatial layer, just return
  if ( mCaptureModeFromLayer && ( !canvas()->currentLayer() || !canvas()->currentLayer()->isSpatial() ) )
    return;

  QgsMapToolAdvancedDigitizing::cadCanvasMoveEvent( e );

  const QgsPointXY point = e->mapPoint();

  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape )
  {
    if ( !mCurrentShapeMapTool )
    {
      emit messageEmitted( tr( "Select an option from the Shape Digitizing Toolbar in order to capture shapes" ), Qgis::MessageLevel::Warning );
    }
    else
    {
      if ( !mTempRubberBand )
      {
        mTempRubberBand.reset( createCurveRubberBand() );
        mTempRubberBand->setStringType( mLineDigitizingType );
        mTempRubberBand->setRubberBandGeometryType( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line );
      }

      mCurrentShapeMapTool->cadCanvasMoveEvent( e, mCaptureMode );
      return;
    }
  }
  else if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::NurbsCurve
            && QgsSettingsRegistryCore::settingsDigitizingNurbsMode->value() == Qgis::NurbsMode::PolyBezier )
  {
    // Poly-Bézier mode handling
    const QgsPoint mapPoint = QgsPoint( point );

    if ( mBezierDragging && mBezierData )
    {
      if ( mBezierDragHandleIndex >= 0 )
      {
        // Dragging an existing handle independently
        mBezierData->moveHandle( mBezierDragHandleIndex, mapPoint );
      }
      else if ( mBezierMoveAnchorIndex >= 0 )
      {
        // Moving an existing anchor (handles move relatively)
        mBezierData->moveAnchor( mBezierMoveAnchorIndex, mapPoint );
      }
      else if ( mBezierDragAnchorIndex >= 0 )
      {
        // Creating new anchor: update both handles symmetrically (like BezierEditing's move_handle2)
        const QgsPoint &anchor = mBezierData->anchor( mBezierDragAnchorIndex );
        const int leftHandleIdx = mBezierDragAnchorIndex * 2;
        const int rightHandleIdx = mBezierDragAnchorIndex * 2 + 1;

        // Right handle follows mouse
        mBezierData->moveHandle( rightHandleIdx, mapPoint );

        // Left handle goes opposite direction: anchor - (mouse - anchor) = 2*anchor - mouse
        const QgsPoint leftHandle( anchor.x() * 2 - mapPoint.x(), anchor.y() * 2 - mapPoint.y() );
        mBezierData->moveHandle( leftHandleIdx, leftHandle );
      }

      // Update visualization
      if ( mBezierMarker )
        mBezierMarker->updateFromData( *mBezierData );
    }
    return;
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
        if ( mLineDigitizingType == Qgis::WkbType::CircularString && mTempRubberBand->stringType() == Qgis::WkbType::CircularString && mTempRubberBand->curveIsComplete() )
          mCircularItermediatePoint = mTempRubberBand->pointFromEnd( 1 );
        else if ( mLineDigitizingType == Qgis::WkbType::LineString || !mTempRubberBand->curveIsComplete() )
          mCircularItermediatePoint = QgsPoint();

        hasTrace = tracingMouseMove( e );

        if ( !hasTrace )
        {
          // Restore the temp rubber band
          mTempRubberBand->reset( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line, mLineDigitizingType, mCaptureFirstPoint );
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
      QgsPointXY mapP( mapPoint.x(), mapPoint.y() ); //#spellok
      const bool is3D = layerPoint.is3D();
      const bool isMeasure = layerPoint.isMeasure();
      mapP = toLayerCoordinates( vlayer, mapP );                                                         //transform snapped point back to layer crs  //#spellok
      layerPoint = QgsPoint( layerPoint.wkbType(), mapP.x(), mapP.y(), layerPoint.z(), layerPoint.m() ); //#spellok
      if ( QgsWkbTypes::hasZ( vlayer->wkbType() ) && !is3D )
        layerPoint.addZValue( mCadDockWidget && mCadDockWidget->cadEnabled() ? mCadDockWidget->currentPointV2().z() : defaultZValue() );
      if ( QgsWkbTypes::hasM( vlayer->wkbType() ) && !isMeasure )
        layerPoint.addMValue( mCadDockWidget && mCadDockWidget->cadEnabled() ? mCadDockWidget->currentPointV2().m() : defaultMValue() );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "transformation to layer coordinate failed" ) );
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

  if ( match.isValid() && sourceLayer )
  {
    if ( ( match.hasVertex() || match.hasLineEndpoint() ) )
    {
      if ( sourceLayer->crs() != vlayer->crs() )
      {
        layerPoint = match.interpolatedPoint();
        return 1;
      }
      QgsFeature f;
      QgsFeatureRequest request;
      request.setFilterFid( match.featureId() );
      const bool fetched = match.layer()->getFeatures( request ).nextFeature( f );
      if ( fetched )
      {
        QgsVertexId vId;
        if ( !f.geometry().vertexIdFromVertexNr( match.vertexIndex(), vId ) )
        {
          return 2;
        }
        layerPoint = f.geometry().constGet()->vertexAt( vId );
        if ( QgsWkbTypes::hasZ( vlayer->wkbType() ) && !layerPoint.is3D() )
          layerPoint.addZValue( defaultZValue() );
        if ( QgsWkbTypes::hasM( vlayer->wkbType() ) && !layerPoint.isMeasure() )
          layerPoint.addMValue( defaultMValue() );

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
      return 2;
    }
    else if ( QgsProject::instance()->topologicalEditing() && ( match.hasEdge() || match.hasMiddleSegment() ) )
    {
      layerPoint = toLayerCoordinates( vlayer, match.interpolatedPoint( mCanvas->mapSettings().destinationCrs() ) );
      return 0;
    }
  }
  return 2;
}

int QgsMapToolCapture::addVertex( const QgsPointXY &point )
{
  return addVertex( point, QgsPointLocator::Match() );
}

int QgsMapToolCapture::addVertex( const QgsPointXY &point, const QgsPointLocator::Match &match )
{
  if ( mode() == CaptureNone )
  {
    QgsDebugError( QStringLiteral( "invalid capture mode" ) );
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
  else
  {
    layerPoint = QgsPoint( point );
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
      mRubberBand.reset( createRubberBand( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line ) );

    if ( !mTempRubberBand )
    {
      mTempRubberBand.reset( createCurveRubberBand() );
      mTempRubberBand->setStringType( mLineDigitizingType );
      mTempRubberBand->reset( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line, mLineDigitizingType, mapPoint );
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
      mTempRubberBand->movePoint( mapPoint );   //move the last point of the temp rubberband before operating with it
      if ( mTempRubberBand->curveIsComplete() ) //2 points for line and 3 points for circular
      {
        if ( QgsCurve *curve = mTempRubberBand->curve() )
        {
          addCurve( curve );
          // add curve append only invalid match to mSnappingMatches,
          // so we need to remove them and add the one from here if it is valid
          if ( match.isValid() && mSnappingMatches.count() > 0 && !mSnappingMatches.last().isValid() )
          {
            mSnappingMatches.removeLast();
            if ( mTempRubberBand->stringType() == Qgis::WkbType::CircularString )
            {
              // for circular string two points are added and match for intermediate point is stored
              mSnappingMatches.removeLast();
              mSnappingMatches.append( mCircularIntermediateMatch );
            }
            mSnappingMatches.append( match );
          }
        }
        mCaptureLastPoint = mapPoint;
        mTempRubberBand->reset( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line, mLineDigitizingType, mCaptureFirstPoint );
      }
      else if ( mTempRubberBand->pointsCount() == 0 )
      {
        mCaptureLastPoint = mapPoint;
        mCaptureCurve.addVertex( layerPoint );
        mSnappingMatches.append( match );
      }
      else
      {
        if ( mTempRubberBand->stringType() == Qgis::WkbType::CircularString )
        {
          mCircularIntermediateMatch = match;
        }
      }

      mTempRubberBand->addPoint( mapPoint );
    }
    else
    {
      mTempRubberBand->reset( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line, mLineDigitizingType, mCaptureFirstPoint );
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
    mRubberBand.reset( createRubberBand( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line ) );
  }

  if ( mTempRubberBand )
  {
    mTempRubberBand->reset( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line, mLineDigitizingType, mCaptureFirstPoint );
    const QgsPoint endPt = c->endPoint();
    mTempRubberBand->addPoint( endPt ); //add last point of c
  }

  const int countBefore = mCaptureCurve.vertexCount();
  //if there is only one point, this the first digitized point that are in the this first curve added --> remove the point
  if ( mCaptureCurve.numPoints() == 1 )
    mCaptureCurve.removeCurve( 0 );

  // Transform back to layer CRS in case map CRS and layer CRS are different
  const QgsCoordinateTransform ct = mCanvas->mapSettings().layerTransform( layer() );
  if ( ct.isValid() && !ct.isShortCircuited() )
  {
    QgsLineString *segmented = c->curveToLine();
    segmented->transform( ct, Qgis::TransformDirection::Reverse );
    // Curve geometries will be converted to segments, so we explicitly set extentPrevious to false
    // to be able to remove the whole curve in undo
    mCaptureCurve.addCurve( segmented, false );
    delete c;
  }
  else
  {
    // we set the extendPrevious option to true to avoid creating compound curves with many 2 vertex linestrings -- instead we prefer
    // to extend linestring curves so that they continue the previous linestring wherever possible...
    mCaptureCurve.addCurve( c, !mStartNewCurve );
  }

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

  // Handle Poly-Bézier mode: delete the last anchor with its handles
  // This must be checked before the standard size() check since Poly-Bézier
  // doesn't use mCaptureCurve during capture
  if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::NurbsCurve
       && QgsSettingsRegistryCore::settingsDigitizingNurbsMode->value() == Qgis::NurbsMode::PolyBezier
       && mBezierData && mBezierData->anchorCount() > 0 )
  {
    mBezierData->deleteAnchor( mBezierData->anchorCount() - 1 );
    if ( mBezierMarker )
      mBezierMarker->updateFromData( *mBezierData );
    // Reset drag state
    mBezierDragging = false;
    mBezierDragAnchorIndex = -1;
    mBezierDragHandleIndex = -1;
    mBezierMoveAnchorIndex = -1;
    mCadDockWidget->removePreviousPoint();
    return;
  }

  if ( mTempRubberBand )
  {
    // Handle NURBS ControlPoints mode: remove last control point
    // This must be checked before the standard size() check since NURBS ControlPoints
    // doesn't use mCaptureCurve during capture
    if ( mTempRubberBand->stringType() == Qgis::WkbType::NurbsCurve && mTempRubberBand->pointsCount() > 1 )
    {
      const QgsPoint lastPoint = mTempRubberBand->lastPoint();
      mTempRubberBand->removeLastPoint();
      mTempRubberBand->movePoint( lastPoint );
      mCadDockWidget->removePreviousPoint();
      return;
    }

    if ( size() <= 1 && mTempRubberBand->pointsCount() != 0 )
      return;

    if ( isAutoRepeat && mIgnoreSubsequentAutoRepeatUndo )
      return;
    mIgnoreSubsequentAutoRepeatUndo = false;

    const QgsPoint lastPoint = mTempRubberBand->lastPoint();

    if ( mTempRubberBand->stringType() == Qgis::WkbType::CircularString && mTempRubberBand->pointsCount() > 2 )
    {
      mTempRubberBand->removeLastPoint();
      mTempRubberBand->movePoint( lastPoint );
      return;
    }

    QgsVertexId vertexToRemove;
    vertexToRemove.part = 0;
    vertexToRemove.ring = 0;
    vertexToRemove.vertex = size() - 1;

    // If the geometry was reprojected, remove the entire last curve.
    const QgsCoordinateTransform ct = mCanvas->mapSettings().layerTransform( layer() );
    if ( ct.isValid() && !ct.isShortCircuited() )
    {
      mCaptureCurve.removeCurve( mCaptureCurve.nCurves() - 1 );
    }
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
      const bool lastCurveIsLineString = qgsgeometry_cast<const QgsLineString *>( mCaptureCurve.curveAt( curvesBefore - 1 ) );

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

    mTempRubberBand->reset( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line, mLineDigitizingType, mCaptureFirstPoint );

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
  else if ( e->key() == Qt::Key_W && !e->isAutoRepeat() )
  {
    // Enable NURBS weight editing mode when W is pressed
    if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::NurbsCurve && mTempRubberBand && mTempRubberBand->pointsCount() >= 2 )
    {
      mWeightEditMode = true;
      // Edit the last control point by default (the one being digitized)
      mWeightEditControlPointIndex = mTempRubberBand->pointsCount() - 2; // -2 because last point is the cursor position
      emit messageEmitted( tr( "NURBS weight editing: Use mouse wheel to adjust weight (Ctrl=fine, Shift=coarse). Current weight: %1" ).arg( mTempRubberBand->weight( mWeightEditControlPointIndex ), 0, 'f', 2 ), Qgis::MessageLevel::Info );
      e->ignore();
    }
  }
}

void QgsMapToolCapture::keyReleaseEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_W && !e->isAutoRepeat() )
  {
    if ( mWeightEditMode )
    {
      mWeightEditMode = false;
      mWeightEditControlPointIndex = -1;
      emit messageEmitted( QString() ); // Clear the message
      e->accept();
      return;
    }
  }

  QgsMapToolAdvancedDigitizing::keyReleaseEvent( e );
}

void QgsMapToolCapture::wheelEvent( QWheelEvent *e )
{
  if ( mWeightEditMode && mWeightEditControlPointIndex >= 0 && mTempRubberBand )
  {
    // Calculate step based on modifiers
    double step = 0.1;
    if ( e->modifiers() & Qt::ControlModifier )
      step = 0.01; // Fine adjustment
    else if ( e->modifiers() & Qt::ShiftModifier )
      step = 1.0; // Coarse adjustment

    // Direction based on wheel delta
    const int delta = e->angleDelta().y();
    const double adjustment = ( delta > 0 ) ? step : -step;

    // Get current weight and apply adjustment
    const double currentWeight = mTempRubberBand->weight( mWeightEditControlPointIndex );
    const double newWeight = std::max( 0.01, currentWeight + adjustment );

    // Apply the new weight
    if ( mTempRubberBand->setWeight( mWeightEditControlPointIndex, newWeight ) )
    {
      emit messageEmitted( tr( "NURBS weight: %1" ).arg( newWeight, 0, 'f', 2 ), Qgis::MessageLevel::Info );
    }

    e->accept();
    return;
  }

  QgsMapToolAdvancedDigitizing::wheelEvent( e );
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

  // Clean up Bézier digitizing data
  if ( mBezierMarker )
    mBezierMarker->clear();
  mBezierData.reset();
  mBezierMarker.reset();
  mBezierDragging = false;
  mBezierDragAnchorIndex = -1;

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
  if ( QgsSettingsRegistryCore::settingsDigitizingValidateGeometries->value() == 0
       || !( capabilities() & ValidateGeometries ) )
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
  if ( QgsSettingsRegistryCore::settingsDigitizingValidateGeometries->value() == 2 )
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
  QgsPoint newPoint( Qgis::WkbType::Point, point.x(), point.y() );

  // get current layer
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer() );
  if ( !vlayer )
  {
    return newPoint;
  }

  // convert to the corresponding type for a full ZM support
  const Qgis::WkbType type = vlayer->wkbType();
  if ( QgsWkbTypes::hasZ( type ) && !QgsWkbTypes::hasM( type ) )
  {
    newPoint.convertTo( Qgis::WkbType::PointZ );
  }
  else if ( !QgsWkbTypes::hasZ( type ) && QgsWkbTypes::hasM( type ) )
  {
    newPoint.convertTo( Qgis::WkbType::PointM );
  }
  else if ( QgsWkbTypes::hasZ( type ) && QgsWkbTypes::hasM( type ) )
  {
    newPoint.convertTo( Qgis::WkbType::PointZM );
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

  if ( canvas()->snappingUtils()->config().selfSnapping() && layer() )
  {
    // the current layer may have changed
    mExtraSnapLayer->setCrs( layer()->crs() );

    QgsGeometry geom;

    // For NURBS curves, include both the evaluated curve and control points for snapping
    if ( mLineDigitizingType == Qgis::WkbType::NurbsCurve && mTempRubberBand && mTempRubberBand->pointsCount() >= 2 )
    {
      // Create a LineString that includes both the interpolated curve points AND control points
      auto lineForSnap = std::make_unique<QgsLineString>();

      // First, add all control points from the rubber band (for snapping to control points)
      const int pointCount = mTempRubberBand->pointsCount();
      // Exclude the last point (cursor position)
      for ( int i = 0; i < pointCount - 1; ++i )
      {
        lineForSnap->addVertex( mTempRubberBand->pointFromEnd( pointCount - 1 - i ) );
      }

      // Then, try to get the evaluated curve and add its vertices (for snapping to the curve)
      std::unique_ptr<QgsCurve> nurbsCurve( mTempRubberBand->curve() );
      if ( nurbsCurve )
      {
        std::unique_ptr<QgsLineString> curvePoints( nurbsCurve->curveToLine() );
        if ( curvePoints )
        {
          for ( int i = 0; i < curvePoints->numPoints(); ++i )
          {
            lineForSnap->addVertex( curvePoints->pointN( i ) );
          }
        }
      }

      // For polygon mode, close the curve to allow snapping to first point
      if ( mCaptureMode == CapturePolygon && lineForSnap->numPoints() >= 3 )
      {
        lineForSnap->close();
      }

      geom = QgsGeometry( lineForSnap.release() );
    }
    else if ( mBezierData && mBezierData->anchorCount() >= 2 )
    {
      // Poly-Bézier mode: include anchors, handles, and interpolated curve for snapping
      auto lineForSnap = std::make_unique<QgsLineString>();

      // Add all anchors for snapping
      const QVector<QgsPoint> anchors = mBezierData->anchors();
      for ( const QgsPoint &pt : anchors )
      {
        lineForSnap->addVertex( pt );
      }

      // Add all handles for snapping
      const QVector<QgsPoint> handles = mBezierData->handles();
      for ( const QgsPoint &pt : handles )
      {
        lineForSnap->addVertex( pt );
      }

      // Add interpolated curve points for snapping to the curve itself
      const QgsPointSequence interpolated = mBezierData->interpolate();
      for ( const QgsPoint &pt : interpolated )
      {
        lineForSnap->addVertex( pt );
      }

      // For polygon mode, close the curve to allow snapping to first point
      if ( mCaptureMode == CapturePolygon && lineForSnap->numPoints() >= 3 )
      {
        lineForSnap->close();
      }

      geom = QgsGeometry( lineForSnap.release() );
    }
    else if ( mCaptureCurve.numPoints() >= 2 )
    {
      // Standard capture curve
      geom = QgsGeometry( mCaptureCurve.clone() );
      // we close the curve to allow snapping on last segment
      if ( mCaptureMode == CapturePolygon && mCaptureCurve.numPoints() >= 3 )
      {
        qgsgeometry_cast<QgsCompoundCurve *>( geom.get() )->close();
      }
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
        Qgis::WkbType geomType = Qgis::WkbType::Point;
        if ( isMatchPointM && isMatchPointZ )
        {
          geomType = Qgis::WkbType::PointZM;
        }
        else if ( isMatchPointM )
        {
          geomType = Qgis::WkbType::PointM;
        }
        else if ( isMatchPointZ )
        {
          geomType = Qgis::WkbType::PointZ;
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
    QgsPointSequence nurbsControlPoints;
    QVector<double> nurbsWeights;

    // Poly-Bézier mode handling
    if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::NurbsCurve
         && QgsSettingsRegistryCore::settingsDigitizingNurbsMode->value() == Qgis::NurbsMode::PolyBezier )
    {
      if ( e->button() == Qt::LeftButton )
      {
        // End dragging on mouse release
        mBezierDragging = false;
        mBezierDragAnchorIndex = -1;
        mBezierDragHandleIndex = -1;
        mBezierMoveAnchorIndex = -1;

        // Clear highlights
        if ( mBezierMarker )
        {
          mBezierMarker->setHighlightedAnchor( -1 );
          mBezierMarker->setHighlightedHandle( -1 );
          mBezierMarker->updateFromData( *mBezierData );
        }

        // Emit help message reminder
        emit messageEmitted( tr( "Bézier editing: click and drag to add anchor, click on handle/anchor to edit, Alt+click to extend handles, right-click to finish" ), Qgis::MessageLevel::Info );
        return;
      }
      else if ( e->button() == Qt::RightButton )
      {
        // End dragging
        mBezierDragging = false;
        mBezierDragAnchorIndex = -1;
        mBezierDragHandleIndex = -1;
        mBezierMoveAnchorIndex = -1;

        if ( mBezierData && mBezierData->anchorCount() >= 2 )
        {
          // Convert Poly-Bézier to NurbsCurve
          std::unique_ptr<QgsNurbsCurve> nurbsCurve = mBezierData->asNurbsCurve();
          if ( nurbsCurve )
          {
            // Transform to layer coordinates
            QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer() );
            if ( vlayer )
            {
              const QgsCoordinateTransform ct = mCanvas->mapSettings().layerTransform( vlayer );
              if ( ct.isValid() && !ct.isShortCircuited() )
              {
                try
                {
                  nurbsCurve->transform( ct, Qgis::TransformDirection::Reverse );
                }
                catch ( QgsCsException & )
                {
                  emit messageEmitted( tr( "Cannot transform the geometry to layer coordinates" ), Qgis::MessageLevel::Warning );
                  stopCapturing();
                  return;
                }
              }

              std::unique_ptr<QgsCurve> curveToAdd;

              // Close for polygon if needed
              if ( mode() == CapturePolygon && !nurbsCurve->isClosed() )
              {
                // For polygon, wrap in compound curve and add closing segment
                auto compound = std::make_unique<QgsCompoundCurve>();
                compound->addCurve( nurbsCurve.release() );
                // Add closing line segment from end to start
                auto closingSegment = std::make_unique<QgsLineString>();
                closingSegment->addVertex( compound->endPoint() );
                closingSegment->addVertex( compound->startPoint() );
                compound->addCurve( closingSegment.release() );
                curveToAdd = std::move( compound );
              }
              else
              {
                curveToAdd.reset( nurbsCurve.release() );
              }
              QgsGeometry g;

              if ( mode() == CaptureLine )
              {
                g = QgsGeometry( curveToAdd->clone() );
                geometryCaptured( g );
                lineCaptured( curveToAdd.release() );
              }
              else // CapturePolygon
              {
                auto poly = std::make_unique<QgsCurvePolygon>();
                poly->setExteriorRing( curveToAdd.release() );
                g = QgsGeometry( poly->clone() );
                geometryCaptured( g );
                polygonCaptured( poly.get() );
              }

              digitizingFinished = true;
            }
          }
        }

        // Clean up Bézier data
        if ( mBezierMarker )
          mBezierMarker->clear();
        mBezierData.reset();
        mBezierMarker.reset();
        stopCapturing();
        return;
      }
      return;
    }
    else if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::Shape )
    {
      if ( !mCurrentShapeMapTool )
      {
        emit messageEmitted( tr( "Select an option from the Shape Digitizing Toolbar in order to capture shapes" ), Qgis::MessageLevel::Warning );
        return;
      }
      else
      {
        if ( !mTempRubberBand )
        {
          mTempRubberBand.reset( createCurveRubberBand() );
          mTempRubberBand->setStringType( mLineDigitizingType );
          mTempRubberBand->setRubberBandGeometryType( mCaptureMode == CapturePolygon ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Line );
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

        // Extract NURBS control points and weights from the rubberband
        if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::NurbsCurve && mTempRubberBand )
        {
          const int rbPointCount = mTempRubberBand->pointsCount();
          if ( rbPointCount > 1 )
          {
            // Exclude the last point (cursor position)
            for ( int i = 0; i < rbPointCount - 1; ++i )
            {
              nurbsControlPoints.append( mTempRubberBand->pointFromEnd( rbPointCount - 1 - i ) );
            }
            // Also extract weights (in correct order)
            const QVector<double> &rbWeights = mTempRubberBand->weights();
            for ( int i = 0; i < rbPointCount - 1; ++i )
            {
              if ( i < rbWeights.size() )
                nurbsWeights.append( rbWeights[i] );
              else
                nurbsWeights.append( 1.0 );
            }
          }
        }

        deleteTempRubberBand();

        if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::NurbsCurve )
        {
          // Minimum 4 control points required for degree 3 NURBS
          if ( mode() == CaptureLine && nurbsControlPoints.count() < 4 )
          {
            stopCapturing();
            return;
          }
          if ( mode() == CapturePolygon && nurbsControlPoints.count() < 4 )
          {
            stopCapturing();
            return;
          }
        }
        else
        {
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
        }

        if ( mode() == CapturePolygon || e->modifiers() == Qt::ShiftModifier )
        {
          // Close NURBS curve by adding first control point at the end
          if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::NurbsCurve && !nurbsControlPoints.isEmpty() )
          {
            // Add the first control point as the last to close the curve
            nurbsControlPoints.append( nurbsControlPoints.first() );
            // Also duplicate the first weight for closure
            if ( !nurbsWeights.isEmpty() )
              nurbsWeights.append( nurbsWeights.first() );
          }
          else
          {
            closePolygon();
          }
        }

        digitizingFinished = true;
      }
    }

    if ( digitizingFinished )
    {
      QgsGeometry g;
      std::unique_ptr<QgsCurve> curveToAdd;

      // Create a single NurbsCurve from all control points
      if ( mCurrentCaptureTechnique == Qgis::CaptureTechnique::NurbsCurve )
      {
        const int degree = 3;
        const int n = nurbsControlPoints.count();

        if ( n >= degree + 1 )
        {
          // Generate uniform clamped knot vector
          const int knotCount = n + degree + 1;
          QVector<double> knots( knotCount );

          for ( int i = 0; i <= degree; ++i )
            knots[i] = 0.0;
          for ( int i = knotCount - degree - 1; i < knotCount; ++i )
            knots[i] = 1.0;
          const int numMiddleKnots = n - degree - 1;
          for ( int i = 0; i < numMiddleKnots; ++i )
            knots[degree + 1 + i] = static_cast<double>( i + 1 ) / ( numMiddleKnots + 1 );

          // Use weights from rubber band, or default to 1.0
          QVector<double> weights;
          if ( nurbsWeights.size() >= n )
          {
            weights = nurbsWeights.mid( 0, n );
          }
          else
          {
            weights = nurbsWeights;
            while ( weights.size() < n )
              weights.append( 1.0 );
          }
          curveToAdd.reset( new QgsNurbsCurve( nurbsControlPoints, degree, knots, weights ) );
        }
        else
        {
          // Not enough points for NURBS, fallback to linestring
          curveToAdd.reset( new QgsLineString( nurbsControlPoints ) );
        }
      }
      else
      {
        curveToAdd.reset( captureCurve()->clone() );
      }

      if ( mode() == CaptureLine )
      {
        g = QgsGeometry( curveToAdd->clone() );
        geometryCaptured( g );
        lineCaptured( curveToAdd.release() );
      }
      else
      {
        // For NURBS curves, keep the already-created curve
        // For other curves, check provider support for curved segments
        if ( mCurrentCaptureTechnique != Qgis::CaptureTechnique::NurbsCurve )
        {
          //does compoundcurve contain circular strings?
          //does provider support circular strings?
          if ( QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer() ) )
          {
            const bool hasCurvedSegments = captureCurve()->hasCurvedSegments();
            const bool providerSupportsCurvedSegments = vlayer->dataProvider()->capabilities() & Qgis::VectorProviderCapability::CircularGeometries;

            if ( hasCurvedSegments && providerSupportsCurvedSegments )
            {
              curveToAdd.reset( captureCurve()->clone() );
            }
            else
            {
              curveToAdd.reset( captureCurve()->curveToLine() );
            }
          }
          else
          {
            curveToAdd.reset( captureCurve()->clone() );
          }
        }
        // curveToAdd is already set for NURBS curves, so we just use it
        auto poly = std::make_unique<QgsCurvePolygon>();
        poly->setExteriorRing( curveToAdd.release() );
        g = QgsGeometry( poly->clone() );
        geometryCaptured( g );
        polygonCaptured( poly.get() );
      }

      stopCapturing();
    }
  }
}
