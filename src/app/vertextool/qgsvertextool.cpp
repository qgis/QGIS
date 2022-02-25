/***************************************************************************
  qgsvertextool.cpp
  --------------------------------------
  Date                 : February 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmessagelog.h"
#include "qgsvertextool.h"

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgscurve.h"
#include "qgslinestring.h"
#include "qgscircularstring.h"
#include "qgscurvepolygon.h"
#include "qgsgeometryutils.h"
#include "qgsgeometryvalidator.h"
#include "qgsguiutils.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmulticurve.h"
#include "qgsmultipoint.h"
#include "qgspointlocator.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgssettingsregistrycore.h"
#include "qgssnapindicator.h"
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"
#include "qgsvertexmarker.h"
#include "qgsstatusbar.h"
#include "qgisapp.h"
#include "qgslockedfeature.h"
#include "qgsvertexeditor.h"
#include "qgsmapmouseevent.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmessagebar.h"

#include <QMenu>
#include <QRubberBand>
#include <QTimer>

uint qHash( const Vertex &v )
{
  return qHash( v.layer ) ^ qHash( v.fid ) ^ qHash( v.vertexId );
}

//
// geomutils - may get moved elsewhere
//


//! Find out whether vertex at the given index is an endpoint (assuming linear geometry)
static bool isEndpointAtVertexIndex( const QgsGeometry &geom, int vertexIndex )
{
  const QgsAbstractGeometry *g = geom.constGet();
  if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve *>( g ) )
  {
    return vertexIndex == 0 || vertexIndex == curve->numPoints() - 1;
  }
  else if ( const QgsMultiCurve *multiCurve = qgsgeometry_cast<const QgsMultiCurve *>( g ) )
  {
    for ( int i = 0; i < multiCurve->numGeometries(); ++i )
    {
      const QgsCurve *part = multiCurve->curveN( i );
      Q_ASSERT( part );
      if ( vertexIndex < part->numPoints() )
        return vertexIndex == 0 || vertexIndex == part->numPoints() - 1;
      vertexIndex -= part->numPoints();
    }
    Q_ASSERT( false );  // should not get here
    return false;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "is_endpoint_at_vertex_index: unexpected geometry type!" ) );
    return false;
  }
}


//! Returns the index of vertex adjacent to the given endpoint. Assuming linear geometries.
int adjacentVertexIndexToEndpoint( const QgsGeometry &geom, int vertexIndex )
{
  const QgsAbstractGeometry *g = geom.constGet();
  if ( const QgsCurve *curve = qgsgeometry_cast<const QgsCurve *>( g ) )
  {
    return vertexIndex == 0 ? 1 : curve->numPoints() - 2;
  }
  else if ( const QgsMultiCurve *multiCurve = qgsgeometry_cast<const QgsMultiCurve *>( g ) )
  {
    int offset = 0;
    for ( int i = 0; i < multiCurve->numGeometries(); ++i )
    {
      const QgsCurve *part = multiCurve->curveN( i );
      Q_ASSERT( part );
      if ( vertexIndex < part->numPoints() )
        return vertexIndex == 0 ? offset + 1 : offset + part->numPoints() - 2;
      vertexIndex -= part->numPoints();
      offset += part->numPoints();
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "adjacent_vertex_index_to_endpoint: unexpected geometry type!" ) );
  }
  return -1;
}


/**
 * Determine whether a vertex is in the middle of a circular edge or not
 * (wrapper for slightly awkward API)
 */
static bool isCircularVertex( const QgsGeometry &geom, int vertexIndex )
{
  QgsVertexId vid;
  return geom.vertexIdFromVertexNr( vertexIndex, vid ) && vid.type == Qgis::VertexType::Curve;
}


//! Create a multi-point geometry that can be used to highlight vertices of a feature
static QgsGeometry geometryToMultiPoint( const QgsGeometry &geom )
{
  QgsMultiPoint *multiPoint = new QgsMultiPoint();
  QgsGeometry outputGeom( multiPoint );
  for ( auto pointIt = geom.vertices_begin(); pointIt != geom.vertices_end(); ++pointIt )
    multiPoint->addGeometry( ( *pointIt ).clone() );
  return outputGeom;
}


//
// snapping match filters
//


//! a filter to allow just one particular feature
class OneFeatureFilter : public QgsPointLocator::MatchFilter
{
  public:
    OneFeatureFilter( const QgsVectorLayer *layer, QgsFeatureId fid )
      : layer( layer )
      , fid( fid )
    {}

    bool acceptMatch( const QgsPointLocator::Match &match ) override
    {
      return match.layer() == layer && match.featureId() == fid;
    }

  private:
    const QgsVectorLayer *layer = nullptr;
    QgsFeatureId fid;
};


//! a filter just to gather all matches at the same place
class MatchCollectingFilter : public QgsPointLocator::MatchFilter
{
  public:
    QList<QgsPointLocator::Match> matches;
    QgsVertexTool *vertextool = nullptr;

    MatchCollectingFilter( QgsVertexTool *vertextool )
      : vertextool( vertextool ) {}

    bool acceptMatch( const QgsPointLocator::Match &match ) override
    {
      if ( match.distance() > 0 )
        return false;

      // there may be multiple points at the same location, but we get only one
      // result... the locator API needs a new method verticesInRect()
      QgsGeometry matchGeom = vertextool->cachedGeometry( match.layer(), match.featureId() );
      bool isPolygon = matchGeom.type() == QgsWkbTypes::PolygonGeometry;
      QgsVertexId polygonRingVid;
      QgsVertexId vid;
      QgsPoint pt;
      while ( matchGeom.constGet()->nextVertex( vid, pt ) )
      {
        int vindex = matchGeom.vertexNrFromVertexId( vid );
        if ( pt.x() == match.point().x() && pt.y() == match.point().y() )
        {
          if ( isPolygon )
          {
            // for polygons we need to handle the case where the first vertex is matching because the
            // last point will have the same coordinates and we would have a duplicate match which
            // would make subsequent code behave incorrectly (topology editing mode would add a single
            // vertex twice)
            if ( vid.vertex == 0 )
            {
              polygonRingVid = vid;
            }
            else if ( vid.ringEqual( polygonRingVid ) && vid.vertex == matchGeom.constGet()->vertexCount( vid.part, vid.ring ) - 1 )
            {
              continue;
            }
          }

          QgsPointLocator::Match extra_match( match.type(), match.layer(), match.featureId(),
                                              0, match.point(), vindex );
          matches.append( extra_match );
        }
      }
      return true;
    }
};


/**
 * Keeps the best match from a selected feature so that we can possibly use it with higher priority.
 * If we do not encounter any selected feature within tolerance, we use the best match as usual.
 *
 * There are two kinds of "selected" features... if we have a "locked" feature in vertex tool,
 * we get a non-null QgsLockedFeature which has feature's layer and feature id. In such case we
 * only allow matches from that feature. The other kind is if some features within layer are
 * in the layer's list of selected features - in that case we accept any match from those features
 * (but only if we do not have "locked" feature)
 */
class SelectedMatchFilter : public QgsPointLocator::MatchFilter
{
  public:
    explicit SelectedMatchFilter( double tol, QgsLockedFeature *selectedFeature )
      : mTolerance( tol )
      , mLockedFeature( selectedFeature ) {}

    bool acceptMatch( const QgsPointLocator::Match &match ) override
    {
      if ( match.distance() <= mTolerance && match.layer() )
      {
        // option 1: we have "locked" feature - we consider just a match from that feature
        // option 2: we do not have "locked" feature - we consider matches from any selected feature
        if ( ( mLockedFeature && mLockedFeature->layer() == match.layer() && mLockedFeature->featureId() == match.featureId() )
             || ( !mLockedFeature && match.layer()->selectedFeatureIds().contains( match.featureId() ) ) )
        {
          if ( !mBestSelectedMatch.isValid() || match.distance() < mBestSelectedMatch.distance() )
            mBestSelectedMatch = match;
        }
      }
      return true;
    }

    bool hasSelectedMatch() const { return mBestSelectedMatch.isValid(); }
    QgsPointLocator::Match bestSelectedMatch() const { return mBestSelectedMatch; }

  private:
    double mTolerance;
    QgsLockedFeature *mLockedFeature;   // not null in case of selected (locked) feature
    QgsPointLocator::Match mBestSelectedMatch;
};


//
//
//


QgsVertexTool::QgsVertexTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock, VertexToolMode mode )
  : QgsMapToolAdvancedDigitizing( canvas, cadDock )
  , mMode( mode )
{
  setAdvancedDigitizingAllowed( false );

  mSnapIndicator.reset( new QgsSnapIndicator( canvas ) );

  mEdgeCenterMarker = new QgsVertexMarker( canvas );
  mEdgeCenterMarker->setIconType( QgsVertexMarker::ICON_CROSS );
  mEdgeCenterMarker->setColor( Qt::red );
  mEdgeCenterMarker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
  mEdgeCenterMarker->setPenWidth( QgsGuiUtils::scaleIconSize( 3 ) );
  mEdgeCenterMarker->setVisible( false );

  mFeatureBand = createRubberBand( QgsWkbTypes::LineGeometry );
  mFeatureBand->setVisible( false );

  QColor color = digitizingStrokeColor();
  mFeatureBandMarkers = new QgsRubberBand( canvas );
  mFeatureBandMarkers->setIcon( QgsRubberBand::ICON_CIRCLE );
  mFeatureBandMarkers->setColor( color );
  mFeatureBandMarkers->setWidth( QgsGuiUtils::scaleIconSize( 2 ) );
  mFeatureBandMarkers->setBrushStyle( Qt::NoBrush );
  mFeatureBandMarkers->setIconSize( QgsGuiUtils::scaleIconSize( 6 ) );
  mFeatureBandMarkers->setVisible( false );

  mVertexBand = new QgsRubberBand( canvas );
  mVertexBand->setIcon( QgsRubberBand::ICON_CIRCLE );
  mVertexBand->setColor( color );
  mVertexBand->setWidth( QgsGuiUtils::scaleIconSize( 2 ) );
  mVertexBand->setBrushStyle( Qt::NoBrush );
  mVertexBand->setIconSize( QgsGuiUtils::scaleIconSize( 15 ) );
  mVertexBand->setVisible( false );

  QColor color2( color );
  color2.setAlpha( color2.alpha() / 3 );
  mEdgeBand = new QgsRubberBand( canvas );
  mEdgeBand->setColor( color2 );
  mEdgeBand->setWidth( QgsGuiUtils::scaleIconSize( 10 ) );
  mEdgeBand->setVisible( false );

  mEndpointMarker = new QgsVertexMarker( canvas );
  mEndpointMarker->setIconType( QgsVertexMarker::ICON_CROSS );
  mEndpointMarker->setColor( Qt::red );
  mEndpointMarker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
  mEndpointMarker->setPenWidth( QgsGuiUtils::scaleIconSize( 3 ) );
  mEndpointMarker->setVisible( false );
}

QgsVertexTool::~QgsVertexTool()
{
  delete mEdgeCenterMarker;
  delete mFeatureBand;
  delete mFeatureBandMarkers;
  delete mVertexBand;
  delete mEdgeBand;
  delete mEndpointMarker;
  delete mVertexEditor;
}

void QgsVertexTool::activate()
{
  if ( QgisApp::instance() )
  {
    showVertexEditor();  //#spellok
  }
  connect( mCanvas, &QgsMapCanvas::currentLayerChanged, this, &QgsVertexTool::currentLayerChanged );

  QgsMapToolAdvancedDigitizing::activate();
}

void QgsVertexTool::deactivate()
{
  setHighlightedVertices( QList<Vertex>() );
  removeTemporaryRubberBands();
  cleanupVertexEditor();

  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  QHash< QPair<QgsVectorLayer *, QgsFeatureId>, GeometryValidation>::iterator it = mValidations.begin();
  for ( ; it != mValidations.end(); ++it )
    it->cleanup();
  mValidations.clear();

  disconnect( mCanvas, &QgsMapCanvas::currentLayerChanged, this, &QgsVertexTool::currentLayerChanged );

  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsVertexTool::currentLayerChanged( QgsMapLayer *layer )
{
  if ( mMode == QgsVertexTool::ActiveLayer )
  {
    if ( mLockedFeature && mLockedFeature->layer() != layer )
    {
      cleanupLockedFeature();
    }
  }
}

void QgsVertexTool::addDragBand( const QgsPointXY &v1, const QgsPointXY &v2 )
{
  addDragStraightBand( nullptr, v1, v2, false, true, v2 );
}

void QgsVertexTool::addDragStraightBand( QgsVectorLayer *layer, QgsPointXY v0, QgsPointXY v1, bool moving0, bool moving1, const QgsPointXY &mapPoint )
{
  // if layer is not null, the input coordinates are coming in the layer's CRS rather than map CRS
  if ( layer )
  {
    v0 = toMapCoordinates( layer, v0 );
    v1 = toMapCoordinates( layer, v1 );
  }

  StraightBand b;
  b.band = createRubberBand( QgsWkbTypes::LineGeometry, true );
  b.p0 = v0;
  b.p1 = v1;
  b.moving0 = moving0;
  b.moving1 = moving1;
  b.offset0 = v0 - mapPoint;
  b.offset1 = v1 - mapPoint;

  b.band->addPoint( v0 );
  b.band->addPoint( v1 );

  mDragStraightBands << b;
}

void QgsVertexTool::addDragCircularBand( QgsVectorLayer *layer, QgsPointXY v0, QgsPointXY v1, QgsPointXY v2, bool moving0, bool moving1, bool moving2, const QgsPointXY &mapPoint )
{
  // if layer is not null, the input coordinates are coming in the layer's CRS rather than map CRS
  if ( layer )
  {
    v0 = toMapCoordinates( layer, v0 );
    v1 = toMapCoordinates( layer, v1 );
    v2 = toMapCoordinates( layer, v2 );
  }

  CircularBand b;
  b.band = createRubberBand( QgsWkbTypes::LineGeometry, true );
  b.p0 = v0;
  b.p1 = v1;
  b.p2 = v2;
  b.moving0 = moving0;
  b.moving1 = moving1;
  b.moving2 = moving2;
  b.offset0 = v0 - mapPoint;
  b.offset1 = v1 - mapPoint;
  b.offset2 = v2 - mapPoint;
  b.updateRubberBand( mapPoint );

  mDragCircularBands << b;
}

void QgsVertexTool::clearDragBands()
{
  qDeleteAll( mDragPointMarkers );
  mDragPointMarkers.clear();
  mDragPointMarkersOffset.clear();

  for ( const StraightBand &b : std::as_const( mDragStraightBands ) )
    delete b.band;
  mDragStraightBands.clear();

  for ( const CircularBand &b : std::as_const( mDragCircularBands ) )
    delete b.band;
  mDragCircularBands.clear();
}

void QgsVertexTool::cadCanvasPressEvent( QgsMapMouseEvent *e )
{
  if ( mSelectionMethod == SelectionRange )
  {
    rangeMethodPressEvent( e );
    return;
  }

  if ( !mDraggingVertex && !mSelectedVertices.isEmpty() && !( e->modifiers() & Qt::ShiftModifier ) && !( e->modifiers() & Qt::ControlModifier ) )
  {
    // only remove highlight if not clicked on one of highlighted vertices
    bool clickedOnHighlightedVertex = false;
    QgsPointLocator::Match m = snapToEditableLayer( e );
    if ( m.hasVertex() )
    {
      for ( const Vertex &selectedVertex : std::as_const( mSelectedVertices ) )
      {
        if ( selectedVertex.layer == m.layer() && selectedVertex.fid == m.featureId() && selectedVertex.vertexId == m.vertexIndex() )
        {
          clickedOnHighlightedVertex = true;
          break;
        }
      }
    }

    if ( !clickedOnHighlightedVertex && e->button() == Qt::LeftButton )
    {
      setHighlightedVertices( QList<Vertex>() ); // reset selection
      updateLockedFeatureVertices();
    }
  }

  if ( e->button() == Qt::LeftButton )
  {
    if ( e->modifiers() & Qt::ControlModifier || e->modifiers() & Qt::ShiftModifier )
    {
      // shift or ctrl-click vertices to highlight without entering edit mode
      QgsPointLocator::Match m = snapToEditableLayer( e );
      if ( m.hasVertex() )
      {
        Vertex vertex( m.layer(), m.featureId(), m.vertexIndex() );

        HighlightMode mode = ModeReset;
        if ( e->modifiers() & Qt::ShiftModifier )
        {
          // Shift+Click to add vertex to highlight
          mode = ModeAdd;
        }
        else if ( e->modifiers() & Qt::ControlModifier )
        {
          // Ctrl+Click to remove vertex
          mode = ModeSubtract;
        }

        setHighlightedVertices( QList<Vertex>() << vertex, mode );
        return;
      }
    }

    // the user may have started dragging a rect to select vertices
    if ( !mDraggingVertex && !mDraggingEdge )
      mSelectionRubberBandStartPos.reset( new QPoint( e->pos() ) );
  }
}

void QgsVertexTool::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( mSelectionMethod == SelectionRange )
  {
    rangeMethodReleaseEvent( e );
    return;
  }

  if ( mNewVertexFromDoubleClick )
  {
    QgsPointLocator::Match m( *mNewVertexFromDoubleClick );
    if ( mLockedFeature && ( mLockedFeature->featureId() != m.featureId() || mLockedFeature->layer() != m.layer() ) )
      return;  // when a feature is bound to the vector editor, only process actions on that feature

    mNewVertexFromDoubleClick.reset();

    // dragging of edges and double-clicking on edges to add vertex are slightly overlapping
    // so we need to cancel edge moving before we start dragging new vertex
    stopDragging();
    startDraggingAddVertex( m );

    if ( e->modifiers() & Qt::ShiftModifier )
    {
      // if this was shift + double-click, immediately place the vertex
      moveVertex( m.point(), &m );
      // force update of rubber bands
      mouseMoveNotDragging( e );
    }
  }
  else if ( mSelectionRubberBand &&
            ( mSelectionMethod == SelectionNormal  ||
              ( mSelectionMethod == SelectionPolygon && e->button() == Qt::RightButton ) ) )
  {
    // only handling of selection rect being dragged
    QList<Vertex> vertices;
    QList<Vertex> selectedVertices;
    bool showInvisibleFeatureWarning = false;

    QgsGeometry rubberBandGeometry = mSelectionRubberBand->asGeometry();

    // the logic for selecting vertices using rectangle:
    // - if we have a bound (locked) feature, we only allow selection of its vertices
    // - if we don't have a bound feature, we can select vertices from any feature,
    //   but if there are some vertices coming from layer(s) with selected feature,
    //   we give them precedence.

    // for each editable layer, select vertices
    const auto editableLayers = editableVectorLayers();
    for ( QgsVectorLayer *vlayer : editableLayers )
    {
      if ( mMode == ActiveLayer && vlayer != currentVectorLayer() )
        continue;

      if ( mLockedFeature && mLockedFeature->layer() != vlayer )
        continue;  // with locked feature we only allow selection of its vertices

      QgsGeometry layerRubberBandGeometry = rubberBandGeometry;
      try
      {
        QgsCoordinateTransform ct = mCanvas->mapSettings().layerTransform( vlayer );
        if ( ct.isValid() )
          layerRubberBandGeometry.transform( ct, Qgis::TransformDirection::Reverse );
      }
      catch ( QgsCsException & )
      {
        continue;
      }

      QgsRenderContext context = QgsRenderContext::fromMapSettings( mCanvas->mapSettings() );
      context.setExpressionContext( mCanvas->createExpressionContext() );
      context.expressionContext() << QgsExpressionContextUtils::layerScope( vlayer );
      std::unique_ptr< QgsFeatureRenderer > r;
      if ( vlayer->renderer() )
      {
        r.reset( vlayer->renderer()->clone() );
        r->startRender( context, vlayer->fields() );
      }

      QgsRectangle layerRect = layerRubberBandGeometry.boundingBox();
      std::unique_ptr< QgsGeometryEngine > layerRubberBandEngine( QgsGeometry::createGeometryEngine( layerRubberBandGeometry.constGet() ) );
      layerRubberBandEngine->prepareGeometry();

      QgsFeatureRequest request;
      request.setFilterRect( layerRect );
      request.setFlags( QgsFeatureRequest::ExactIntersect );
      if ( r )
        request.setSubsetOfAttributes( r->usedAttributes( context ), vlayer->fields() );
      else
        request.setNoAttributes();
      request.setExpressionContext( context.expressionContext() );

      QgsFeature f;
      QgsFeatureIterator fi = vlayer->getFeatures( request );
      while ( fi.nextFeature( f ) )
      {
        if ( mLockedFeature && mLockedFeature->featureId() != f.id() )
          continue;  // with locked feature we only allow selection of its vertices

        context.expressionContext().setFeature( f );
        bool isFeatureInvisible = ( r && !r->willRenderFeature( f, context ) );
        // If we've already determined that we have to show users a warning about selecting invisible vertices,
        // then we don't need to check through the vertices for other invisible features.
        // We'll be showing the warning anyway regardless of the outcome!
        if ( isFeatureInvisible && showInvisibleFeatureWarning )
          continue;

        bool isFeatureSelected = vlayer->selectedFeatureIds().contains( f.id() );
        QgsGeometry g = f.geometry();
        int i = 0;
        for ( auto it = g.constGet()->vertices_begin(); it != g.constGet()->vertices_end(); ++it )
        {
          QgsPoint pt = *it;
          if ( layerRubberBandEngine->contains( &pt ) )
          {
            // we don't want to select invisible features,
            // but if the user tried to selected one we want to warn him
            if ( isFeatureInvisible )
            {
              showInvisibleFeatureWarning = true;
              break;
            }
            vertices << Vertex( vlayer, f.id(), i );

            if ( isFeatureSelected )
              selectedVertices << Vertex( vlayer, f.id(), i );
          }
          i++;
        }
      }
      if ( r )
        r->stopRender( context );
    }
    if ( showInvisibleFeatureWarning )
    {
      QgisApp::instance()->messageBar()->pushMessage(
        tr( "Invisible vertices were not selected" ),
        tr( "Vertices belonging to features that are not displayed on the map canvas were not selected." ),
        Qgis::MessageLevel::Warning );
    }

    // here's where we give precedence to vertices of selected features in case there's no bound (locked) feature
    if ( !mLockedFeature && !selectedVertices.isEmpty() )
    {
      vertices = selectedVertices;
    }

    HighlightMode mode = ModeReset;
    if ( e->modifiers() & Qt::ShiftModifier )
      mode = ModeAdd;
    else if ( e->modifiers() & Qt::ControlModifier )
      mode = ModeSubtract;

    setHighlightedVertices( vertices, mode );

    stopSelectionRubberBand();
  }
  else if ( e->button() == Qt::LeftButton && e->modifiers() & Qt::AltModifier )
  {
    mSelectionMethod = SelectionPolygon;
    initSelectionRubberBand();
    mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    return;
  }
  else if ( mSelectionRubberBand && mSelectionMethod == SelectionPolygon )
  {
    mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    return;
  }
  else  // selection rect is not being dragged
  {
    if ( e->button() == Qt::LeftButton && !( e->modifiers() & Qt::ShiftModifier ) && !( e->modifiers() & Qt::ControlModifier ) )
    {
      // accepting action
      if ( mDraggingVertex )
      {
        QgsPointLocator::Match match = e->mapPointMatch();
        moveVertex( e->mapPoint(), &match );
      }
      else if ( mDraggingEdge )
      {
        // do not use e.mapPoint() as it may be snapped
        moveEdge( toMapCoordinates( e->pos() ) );
      }
      else
      {
        startDragging( e );
      }
    }
    else if ( e->button() == Qt::RightButton )
    {
      if ( mDraggingVertex || mDraggingEdge )
      {
        // cancel action
        stopDragging();
      }
      else if ( !mSelectionRubberBand )
      {
        // Right-click to select/delect a feature for editing (also gets selected in vertex editor).
        // If there are multiple features at one location, cycle through them with subsequent right clicks.
        tryToSelectFeature( e );
      }
    }
  }

  mSelectionRubberBandStartPos.reset();
}

void QgsVertexTool::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mLockedFeatureAlternatives && ( e->pos() - mLockedFeatureAlternatives->screenPoint ).manhattanLength() >= QApplication::startDragDistance() )
  {
    // as soon as the mouse moves more than just a tiny bit, previously stored alternatives info
    // is probably not valid anymore and will need to be re-calculated
    mLockedFeatureAlternatives.reset();
  }

  if ( mSelectionMethod == SelectionRange )
  {
    rangeMethodMoveEvent( e );
    return;
  }

  if ( mDraggingVertex )
  {
    mouseMoveDraggingVertex( e );
  }
  else if ( mDraggingEdge )
  {
    mouseMoveDraggingEdge( e );
  }
  else if ( mSelectionRubberBandStartPos )
  {
    // the user may be dragging a rect to select vertices
    if ( !mSelectionRubberBand && ( e->pos() - *mSelectionRubberBandStartPos ).manhattanLength() >= 10 )
    {
      mSelectionMethod = SelectionNormal;
      initSelectionRubberBand();
    }
    if ( mSelectionRubberBand )
    {
      updateSelectionRubberBand( e );
    }
  }
  else
  {
    mouseMoveNotDragging( e );
  }
}

void QgsVertexTool::mouseMoveDraggingVertex( QgsMapMouseEvent *e )
{
  mSnapIndicator->setMatch( e->mapPointMatch() );
  mEdgeCenterMarker->setVisible( false );

  moveDragBands( e->mapPoint() );
}

void QgsVertexTool::moveDragBands( const QgsPointXY &mapPoint )
{
  for ( int i = 0; i < mDragStraightBands.count(); ++i )
  {
    StraightBand &b = mDragStraightBands[i];
    if ( b.moving0 )
      b.band->movePoint( 0, mapPoint + b.offset0 );
    if ( b.moving1 )
      b.band->movePoint( 1, mapPoint + b.offset1 );
  }

  for ( int i = 0; i < mDragCircularBands.count(); ++i )
  {
    CircularBand &b = mDragCircularBands[i];
    b.updateRubberBand( mapPoint );
  }

  // in case of moving of standalone point geometry
  for ( int i = 0; i < mDragPointMarkers.count(); ++i )
  {
    QgsVertexMarker *marker = mDragPointMarkers[i];
    QgsVector offset = mDragPointMarkersOffset[i];
    marker->setCenter( mapPoint + offset );
  }

  // make sure the temporary feature rubber band is not visible
  removeTemporaryRubberBands();
}

void QgsVertexTool::mouseMoveDraggingEdge( QgsMapMouseEvent *e )
{
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  mEdgeCenterMarker->setVisible( false );

  QgsPointXY mapPoint = toMapCoordinates( e->pos() );  // do not use e.mapPoint() as it may be snapped

  moveDragBands( mapPoint );
}

void QgsVertexTool::canvasDoubleClickEvent( QgsMapMouseEvent *e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  QgsPointLocator::Match m = snapToEditableLayer( e );
  if ( !m.hasEdge() )
    return;

  mNewVertexFromDoubleClick.reset( new QgsPointLocator::Match( m ) );
}

void QgsVertexTool::removeTemporaryRubberBands()
{
  mFeatureBand->setVisible( false );
  mFeatureBandMarkers->setVisible( false );
  mFeatureBandLayer = nullptr;
  mFeatureBandFid = QgsFeatureId();
  mVertexBand->setVisible( false );
  mEdgeBand->setVisible( false );
  mEndpointMarkerCenter.reset();
  mEndpointMarker->setVisible( false );
}

QgsPointLocator::Match QgsVertexTool::snapToEditableLayer( QgsMapMouseEvent *e )
{
  QgsSnappingUtils *snapUtils = canvas()->snappingUtils();
  QgsSnappingConfig oldConfig = snapUtils->config();
  QgsPointLocator::Match m;

  QgsPointXY mapPoint = toMapCoordinates( e->pos() );
  double tol = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );

  QgsSnappingConfig config = oldConfig;
  config.setEnabled( true );
  config.setMode( Qgis::SnappingMode::AdvancedConfiguration );
  config.setIntersectionSnapping( false );  // only snap to layers
  config.clearIndividualLayerSettings();

  typedef QHash<QgsVectorLayer *, QgsSnappingConfig::IndividualLayerSettings> SettingsHashMap;
  SettingsHashMap oldLayerSettings = oldConfig.individualLayerSettings();

  // if there is a current layer, it should have priority over other layers
  // because sometimes there may be match from multiple layers at one location
  // and selecting current layer is an easy way for the user to prioritize a layer
  if ( QgsVectorLayer *currentVlayer = currentVectorLayer() )
  {
    if ( currentVlayer->isEditable() )
    {
      const auto layers = canvas()->layers( true );
      for ( QgsMapLayer *layer : layers )
      {
        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
        if ( !vlayer )
          continue;

        QgsSnappingConfig::IndividualLayerSettings layerSettings;
        SettingsHashMap::const_iterator existingSettings = oldLayerSettings.find( vlayer );
        if ( existingSettings != oldLayerSettings.constEnd() )
        {
          layerSettings = existingSettings.value();
          layerSettings.setEnabled( vlayer == currentVlayer );
          layerSettings.setTolerance( tol );
          layerSettings.setTypeFlag( static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment ) );
          layerSettings.setUnits( QgsTolerance::ProjectUnits );
        }
        else
        {
          layerSettings = QgsSnappingConfig::IndividualLayerSettings(
                            vlayer == currentVlayer, static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment ), tol, QgsTolerance::ProjectUnits, 0.0, 0.0 );
        }

        config.setIndividualLayerSettings( vlayer, layerSettings );
      }

      snapUtils->setConfig( config );
      SelectedMatchFilter filter( tol, mLockedFeature.get() );
      m = snapUtils->snapToMap( mapPoint, &filter, true );

      // we give priority to snap matches that are from selected features
      if ( filter.hasSelectedMatch() )
      {
        m = filter.bestSelectedMatch();
        mLastSnap.reset();
      }
    }
  }

  // if there is no match from the current layer, try to use any editable vector layer
  if ( !m.isValid() && mMode == AllLayers )
  {
    const auto layers = canvas()->layers( true );
    for ( QgsMapLayer *layer : layers )
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      if ( !vlayer )
        continue;

      QgsSnappingConfig::IndividualLayerSettings layerSettings;
      SettingsHashMap::const_iterator existingSettings = oldLayerSettings.find( vlayer );
      if ( existingSettings != oldLayerSettings.constEnd() )
      {
        layerSettings = existingSettings.value();
        layerSettings.setEnabled( vlayer->isEditable() );
        layerSettings.setTolerance( tol );
        layerSettings.setTypeFlag( static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment ) );
        layerSettings.setUnits( QgsTolerance::ProjectUnits );
      }
      else
      {
        layerSettings = QgsSnappingConfig::IndividualLayerSettings( vlayer->isEditable(), static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment ), tol, QgsTolerance::ProjectUnits, 0.0, 0.0 );
      }
      config.setIndividualLayerSettings( vlayer, layerSettings );
    }

    snapUtils->setConfig( config );
    SelectedMatchFilter filter( tol, mLockedFeature.get() );
    m = snapUtils->snapToMap( mapPoint, &filter, true );

    // we give priority to snap matches that are from selected features
    if ( filter.hasSelectedMatch() )
    {
      m = filter.bestSelectedMatch();
      mLastSnap.reset();
    }
  }

  // try to stay snapped to previously used feature
  // so the highlight does not jump around at vertices where features are joined
  if ( mLastSnap )
  {
    OneFeatureFilter filterLast( mLastSnap->layer(), mLastSnap->featureId() );
    QgsPointLocator::Match lastMatch = snapUtils->snapToMap( mapPoint, &filterLast, true );
    // but skip the the previously used feature if it would only snap to segment, while now we have snap to vertex
    // so that if there is a point on a line, it gets priority (as is usual with combined vertex+segment snapping)
    bool matchHasVertexLastHasEdge = m.hasVertex() && lastMatch.hasEdge();
    if ( lastMatch.isValid() && lastMatch.distance() <= m.distance() && !matchHasVertexLastHasEdge )
    {
      m = lastMatch;
    }
  }

  snapUtils->setConfig( oldConfig );

  mLastSnap.reset( new QgsPointLocator::Match( m ) );

  return m;
}


QgsPointLocator::Match QgsVertexTool::snapToPolygonInterior( QgsMapMouseEvent *e )
{
  QgsSnappingUtils *snapUtils = canvas()->snappingUtils();
  QgsPointLocator::Match m;

  QgsPointXY mapPoint = toMapCoordinates( e->pos() );

  // if there is a current layer, it should have priority over other layers
  // because sometimes there may be match from multiple layers at one location
  // and selecting current layer is an easy way for the user to prioritize a layer
  if ( QgsVectorLayer *currentVlayer = currentVectorLayer() )
  {
    if ( currentVlayer->isEditable() && currentVlayer->geometryType() == QgsWkbTypes::PolygonGeometry )
    {
      QgsPointLocator::MatchList matchList = snapUtils->locatorForLayer( currentVlayer )->pointInPolygon( mapPoint, true );
      if ( !matchList.isEmpty() )
      {
        m = matchList.first();
      }
    }
  }

  // if there is no match from the current layer, try to use any editable vector layer
  if ( !m.isValid() && mMode == AllLayers )
  {
    const auto layers = canvas()->layers( true );
    for ( QgsMapLayer *layer : layers )
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      if ( !vlayer )
        continue;

      if ( vlayer->isEditable() && vlayer->geometryType() == QgsWkbTypes::PolygonGeometry )
      {
        QgsPointLocator::MatchList matchList = snapUtils->locatorForLayer( vlayer )->pointInPolygon( mapPoint, true );
        if ( !matchList.isEmpty() )
        {
          m = matchList.first();
          break;
        }
      }
    }
  }

  // if we don't have anything in the last snap, keep the area match
  if ( !mLastSnap && m.isValid() )
  {
    mLastSnap.reset( new QgsPointLocator::Match( m ) );
  }

  return m;
}


QList<QgsPointLocator::Match> QgsVertexTool::findEditableLayerMatches( const QgsPointXY &mapPoint, QgsVectorLayer *layer )
{
  QgsPointLocator::MatchList matchList;

  if ( !layer->isEditable() )
    return matchList;

  QgsSnappingUtils *snapUtils = canvas()->snappingUtils();
  QgsPointLocator *locator = snapUtils->locatorForLayer( layer );

  if ( layer->geometryType() == QgsWkbTypes::PolygonGeometry )
  {
    matchList << locator->pointInPolygon( mapPoint, true );
  }

  double tolerance = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );
  matchList << locator->edgesInRect( mapPoint, tolerance );
  matchList << locator->verticesInRect( mapPoint, tolerance );

  return matchList;
}


QSet<QPair<QgsVectorLayer *, QgsFeatureId> > QgsVertexTool::findAllEditableFeatures( const QgsPointXY &mapPoint )
{
  QSet< QPair<QgsVectorLayer *, QgsFeatureId> > alternatives;

  // if there is a current layer, it should have priority over other layers
  // because sometimes there may be match from multiple layers at one location
  // and selecting current layer is an easy way for the user to prioritize a layer
  if ( QgsVectorLayer *currentVlayer = currentVectorLayer() )
  {
    for ( const QgsPointLocator::Match &m : findEditableLayerMatches( mapPoint, currentVlayer ) )
    {
      alternatives.insert( qMakePair( m.layer(), m.featureId() ) );
    }
  }

  if ( mMode == AllLayers )
  {
    const auto layers = canvas()->layers( true );
    for ( QgsMapLayer *layer : layers )
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      if ( !vlayer )
        continue;

      for ( const QgsPointLocator::Match &m : findEditableLayerMatches( mapPoint, vlayer ) )
      {
        alternatives.insert( qMakePair( m.layer(), m.featureId() ) );
      }
    }
  }

  return alternatives;
}


void QgsVertexTool::tryToSelectFeature( QgsMapMouseEvent *e )
{
  if ( !mLockedFeatureAlternatives )
  {
    // this is the first right-click on this location so we currently do not have information
    // about editable features at this mouse location - let's build the alternatives info
    QSet< QPair<QgsVectorLayer *, QgsFeatureId> > alternatives = findAllEditableFeatures( toMapCoordinates( e->pos() ) );
    if ( !alternatives.isEmpty() )
    {
      QgsPointLocator::Match m = snapToEditableLayer( e );
      if ( !m.isValid() )
      {
        // as the last resort check if we are on top of a feature if there is no vertex or edge snap
        m = snapToPolygonInterior( e );
      }

      mLockedFeatureAlternatives.reset( new LockedFeatureAlternatives );
      mLockedFeatureAlternatives->screenPoint = e->pos();
      mLockedFeatureAlternatives->index = -1;
      if ( m.isValid() )
      {
        // ideally the feature that would get normally highlighted should be also the first choice
        // because as user moves mouse, different features are highlighted, so the highlighted feature
        // should be first to get selected
        QPair<QgsVectorLayer *, QgsFeatureId> firstChoice( m.layer(), m.featureId() );
        mLockedFeatureAlternatives->alternatives.append( firstChoice );
        alternatives.remove( firstChoice );
      }
      mLockedFeatureAlternatives->alternatives.append( qgis::setToList( alternatives ) );

      if ( mLockedFeature )
      {
        // in case there is already a feature locked, continue in the loop from it
        QPair<QgsVectorLayer *, QgsFeatureId> currentSelection( mLockedFeature->layer(), mLockedFeature->featureId() );
        int currentIndex = mLockedFeatureAlternatives->alternatives.indexOf( currentSelection );
        if ( currentIndex != -1 )
          mLockedFeatureAlternatives->index = currentIndex;
      }
    }
  }

  if ( mLockedFeatureAlternatives )
  {
    // move to the next alternative
    if ( mLockedFeatureAlternatives->index < mLockedFeatureAlternatives->alternatives.count() - 1 )
      ++mLockedFeatureAlternatives->index;
    else
      mLockedFeatureAlternatives->index = -1;
  }

  if ( mLockedFeatureAlternatives && mLockedFeatureAlternatives->index != -1 )
  {
    // we have a feature to select
    QPair<QgsVectorLayer *, QgsFeatureId> alternative = mLockedFeatureAlternatives->alternatives.at( mLockedFeatureAlternatives->index );
    // keep only corrsesponding vertices
    // todo: it might be nice to keep other vertices in memory, so we could select them when switching lokcked feature
    QList<Vertex> vertices;
    for ( const Vertex &v : std::as_const( mSelectedVertices ) )
      if ( v.layer == alternative.first && v.fid == alternative.second )
        vertices << v;
    setHighlightedVertices( vertices, ModeReset );

    updateVertexEditor( alternative.first, alternative.second );
  }
  else
  {
    // there's really nothing under the cursor or while cycling through the list of available features
    // we got to the end of the list - let's deselect any feature we may have had selected
    setHighlightedVertices( QList<Vertex>(), ModeReset );
    cleanupLockedFeature();
  }

  // we have either locked ourselves to a feature or unlocked again
  // in any case, we don't want feature highlight anymore (vertex editor has its own highlight)
  updateFeatureBand( QgsPointLocator::Match() );
}


bool QgsVertexTool::isNearEndpointMarker( const QgsPointXY &mapPoint )
{
  if ( !mEndpointMarkerCenter )
    return false;

  double distMarker = std::sqrt( mEndpointMarkerCenter->sqrDist( mapPoint ) );
  double tol = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );

  QgsGeometry geom = cachedGeometryForVertex( *mMouseAtEndpoint );
  QgsPointXY vertexPointV2 = geom.vertexAt( mMouseAtEndpoint->vertexId );
  QgsPointXY vertexPoint = QgsPointXY( vertexPointV2.x(), vertexPointV2.y() );
  double distVertex = std::sqrt( vertexPoint.sqrDist( mapPoint ) );

  return distMarker < tol && distMarker < distVertex;
}

bool QgsVertexTool::isMatchAtEndpoint( const QgsPointLocator::Match &match )
{
  QgsGeometry geom = cachedGeometry( match.layer(), match.featureId() );

  if ( geom.type() != QgsWkbTypes::LineGeometry )
    return false;

  return isEndpointAtVertexIndex( geom, match.vertexIndex() );
}

QgsPointXY QgsVertexTool::positionForEndpointMarker( const QgsPointLocator::Match &match )
{
  QgsGeometry geom = cachedGeometry( match.layer(), match.featureId() );

  QgsPointXY pt0 = geom.vertexAt( adjacentVertexIndexToEndpoint( geom, match.vertexIndex() ) );
  QgsPointXY pt1 = geom.vertexAt( match.vertexIndex() );

  pt0 = toMapCoordinates( match.layer(), pt0 );
  pt1 = toMapCoordinates( match.layer(), pt1 );

  double dx = pt1.x() - pt0.x();
  double dy = pt1.y() - pt0.y();
  double dist = 15 * canvas()->mapSettings().mapUnitsPerPixel();
  double angle = std::atan2( dy, dx );  // to the top: angle=0, to the right: angle=90, to the left: angle=-90
  double x = pt1.x() + std::cos( angle ) * dist;
  double y = pt1.y() + std::sin( angle ) * dist;
  return QgsPointXY( x, y );
}

void QgsVertexTool::mouseMoveNotDragging( QgsMapMouseEvent *e )
{
  if ( mMouseAtEndpoint )
  {
    // check if we are still at the endpoint, i.e. whether to keep showing
    // the endpoint indicator - or go back to snapping to editable layers
    QgsPointXY mapPoint = toMapCoordinates( e->pos() );
    if ( isNearEndpointMarker( mapPoint ) )
    {
      mEndpointMarker->setColor( Qt::red );
      mEndpointMarker->update();
      // make it clear this would add endpoint, not move the vertex
      mVertexBand->setVisible( false );
      return;
    }
  }

  // do not use snap from mouse event, use our own with any editable layer
  QgsPointLocator::Match m = snapToEditableLayer( e );
  bool targetIsAllowed = ( !mLockedFeature || ( mLockedFeature->featureId() == m.featureId() && mLockedFeature->layer() == m.layer() ) );

  // possibility to move a vertex
  if ( m.type() == QgsPointLocator::Vertex && targetIsAllowed )
  {
    updateVertexBand( m );

    // if we are at an endpoint, let's show also the endpoint indicator
    // so user can possibly add a new vertex at the end
    if ( isMatchAtEndpoint( m ) )
    {
      mMouseAtEndpoint.reset( new Vertex( m.layer(), m.featureId(), m.vertexIndex() ) );
      mEndpointMarkerCenter.reset( new QgsPointXY( positionForEndpointMarker( m ) ) );
      mEndpointMarker->setCenter( *mEndpointMarkerCenter );
      mEndpointMarker->setColor( Qt::gray );
      mEndpointMarker->setVisible( true );
      mEndpointMarker->update();
    }
    else
    {
      mMouseAtEndpoint.reset();
      mEndpointMarkerCenter.reset();
      mEndpointMarker->setVisible( false );
    }
  }
  else
  {
    mVertexBand->setVisible( false );
    mMouseAtEndpoint.reset();
    mEndpointMarkerCenter.reset();
    mEndpointMarker->setVisible( false );
  }

  // possibility to create new vertex here - or to move the edge
  if ( m.type() == QgsPointLocator::Edge && targetIsAllowed )
  {
    QgsPointXY mapPoint = toMapCoordinates( e->pos() );
    bool isCircularEdge = false;

    QgsPointXY p0, p1;
    m.edgePoints( p0, p1 );

    QgsGeometry geom = cachedGeometry( m.layer(), m.featureId() );
    if ( isCircularVertex( geom, m.vertexIndex() ) )
    {
      // circular edge at the first vertex
      isCircularEdge = true;
      QgsPointXY pX = toMapCoordinates( m.layer(), geom.vertexAt( m.vertexIndex() - 1 ) );
      QgsPointSequence points;
      QgsGeometryUtils::segmentizeArc( QgsPoint( pX ), QgsPoint( p0 ), QgsPoint( p1 ), points );
      mEdgeBand->reset();
      for ( const QgsPoint &pt : std::as_const( points ) )
        mEdgeBand->addPoint( pt );
    }
    else if ( isCircularVertex( geom, m.vertexIndex() + 1 ) )
    {
      // circular edge at the second vertex
      isCircularEdge = true;
      QgsPointXY pX = toMapCoordinates( m.layer(), geom.vertexAt( m.vertexIndex() + 2 ) );
      QgsPointSequence points;
      QgsGeometryUtils::segmentizeArc( QgsPoint( p0 ), QgsPoint( p1 ), QgsPoint( pX ), points );
      mEdgeBand->reset();
      for ( const QgsPoint &pt : std::as_const( points ) )
        mEdgeBand->addPoint( pt );
    }
    else
    {
      // straight edge
      QgsPolylineXY points;
      points << p0 << p1;
      mEdgeBand->setToGeometry( QgsGeometry::fromPolylineXY( points ), nullptr );
    }

    QgsPointXY edgeCenter;
    bool isNearCenter = matchEdgeCenterTest( m, mapPoint, &edgeCenter );
    mEdgeCenterMarker->setCenter( edgeCenter );
    mEdgeCenterMarker->setColor( isNearCenter ? Qt::red : Qt::gray );
    mEdgeCenterMarker->setVisible( !isCircularEdge );  // currently not supported for circular edges
    mEdgeCenterMarker->update();

    mEdgeBand->setVisible( !isNearCenter );
  }
  else
  {
    mEdgeCenterMarker->setVisible( false );
    mEdgeBand->setVisible( false );
  }

  if ( !m.isValid() )
  {
    // as the last resort check if we are on top of a feature if there is no vertex or edge snap
    m = snapToPolygonInterior( e );
  }

  // when we are "locked" to a feature, we don't want to highlight any other features
  // so the user does not get distracted
  updateFeatureBand( mLockedFeature ? QgsPointLocator::Match() : m );
}

void QgsVertexTool::updateVertexBand( const QgsPointLocator::Match &m )
{
  if ( m.hasVertex() && m.layer() )
  {
    mVertexBand->setToGeometry( QgsGeometry::fromPointXY( m.point() ), nullptr );
    mVertexBand->setVisible( true );
    bool isCircular = false;
    if ( m.layer() )
    {
      isCircular = isCircularVertex( cachedGeometry( m.layer(), m.featureId() ), m.vertexIndex() );
    }

    mVertexBand->setIcon( isCircular ? QgsRubberBand::ICON_FULL_DIAMOND : QgsRubberBand::ICON_CIRCLE );
  }
  else
  {
    mVertexBand->setVisible( false );
  }
}

void QgsVertexTool::updateFeatureBand( const QgsPointLocator::Match &m )
{
  // highlight feature
  if ( m.isValid() && m.layer() )
  {
    if ( mFeatureBandLayer == m.layer() && mFeatureBandFid == m.featureId() )
      return;  // skip regeneration of rubber band if not needed

    QgsGeometry geom = cachedGeometry( m.layer(), m.featureId() );
    mFeatureBandMarkers->setToGeometry( geometryToMultiPoint( geom ), m.layer() );
    mFeatureBandMarkers->setVisible( true );
    if ( QgsWkbTypes::isCurvedType( geom.wkbType() ) )
      geom = QgsGeometry( geom.constGet()->segmentize() );
    mFeatureBand->setToGeometry( geom, m.layer() );
    mFeatureBand->setVisible( true );
    mFeatureBandLayer = m.layer();
    mFeatureBandFid = m.featureId();
  }
  else
  {
    mFeatureBand->setVisible( false );
    mFeatureBandMarkers->setVisible( false );
    mFeatureBandLayer = nullptr;
    mFeatureBandFid = QgsFeatureId();
  }
}

void QgsVertexTool::keyPressEvent( QKeyEvent *e )
{
  switch ( e->key() )
  {
    case Qt::Key_Escape:
    {
      if ( mSelectionMethod == SelectionRange )
        stopRangeVertexSelection();
      if ( mSelectionMethod == SelectionPolygon )
        stopSelectionRubberBand();
      if ( mDraggingVertex || mDraggingEdge )
        stopDragging();
      break;
    }
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
    {
      if ( mSelectionMethod == SelectionPolygon )
      {
        e->ignore(); // Override default shortcut management
        mSelectionRubberBand->removeLastPoint();
        if ( mSelectionRubberBand->numberOfVertices() < 2 )
          stopSelectionRubberBand();
      }
      if ( mDraggingVertex || ( !mDraggingEdge && !mSelectedVertices.isEmpty() ) )
      {
        e->ignore();  // Override default shortcut management
        deleteVertex();
      }
      break;
    }
    case Qt::Key_O:
    {
      if ( mDraggingVertex || ( !mDraggingEdge && !mSelectedVertices.isEmpty() ) )
      {
        e->ignore();  // Override default shortcut management
        toggleVertexCurve();
      }
      break;
    }
    case Qt::Key_R:
    {
      if ( e->modifiers() & Qt::ShiftModifier && !mDraggingVertex && !mDraggingEdge )
        startRangeVertexSelection();
      break;
    }
    case Qt::Key_Less:
    case Qt::Key_Comma:
    {
      if ( !mDraggingVertex && !mDraggingEdge )
        highlightAdjacentVertex( -1 );
      break;
    }
    case Qt::Key_Greater:
    case Qt::Key_Period:
    {
      if ( !mDraggingVertex && !mDraggingEdge )
        highlightAdjacentVertex( + 1 );
      break;
    }
    default:
    {
      return;
    }
  }
  return;
}

QgsGeometry QgsVertexTool::cachedGeometry( const QgsVectorLayer *layer, QgsFeatureId fid )
{
  const bool layerWasNotInCache = !mCache.contains( layer );
  // insert if it was not in cache
  QHash<QgsFeatureId, QgsGeometry> &layerCache = mCache[layer];
  if ( layerWasNotInCache )
  {
    connect( layer, &QgsVectorLayer::geometryChanged, this, &QgsVertexTool::onCachedGeometryChanged );
    connect( layer, &QgsVectorLayer::featureDeleted, this, &QgsVertexTool::onCachedGeometryDeleted );
    connect( layer, &QgsVectorLayer::willBeDeleted, this, &QgsVertexTool::clearGeometryCache );
    connect( layer, &QgsVectorLayer::dataChanged, this, &QgsVertexTool::clearGeometryCache );
  }

  if ( !layerCache.contains( fid ) )
  {
    QgsFeature f;
    layer->getFeatures( QgsFeatureRequest( fid ).setNoAttributes() ).nextFeature( f );
    layerCache[fid] = f.geometry();
  }

  return layerCache[fid];
}

QgsGeometry QgsVertexTool::cachedGeometryForVertex( const Vertex &vertex )
{
  return cachedGeometry( vertex.layer, vertex.fid );
}

void QgsVertexTool::clearGeometryCache()
{
  const QgsVectorLayer *layer = qobject_cast<const QgsVectorLayer *>( sender() );
  mCache.remove( layer );
  disconnect( layer, &QgsVectorLayer::geometryChanged, this, &QgsVertexTool::onCachedGeometryChanged );
  disconnect( layer, &QgsVectorLayer::featureDeleted, this, &QgsVertexTool::onCachedGeometryDeleted );
  disconnect( layer, &QgsVectorLayer::willBeDeleted, this, &QgsVertexTool::clearGeometryCache );
  disconnect( layer, &QgsVectorLayer::dataChanged, this, &QgsVertexTool::clearGeometryCache );
}

void QgsVertexTool::onCachedGeometryChanged( QgsFeatureId fid, const QgsGeometry &geom )
{
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( sender() );
  Q_ASSERT( mCache.contains( layer ) );
  QHash<QgsFeatureId, QgsGeometry> &layerCache = mCache[layer];
  if ( layerCache.contains( fid ) )
    layerCache[fid] = geom;

  // refresh highlighted vertices - their position may have changed
  setHighlightedVertices( mSelectedVertices );

  // re-run validation for the feature
  validateGeometry( layer, fid );

  if ( mLockedFeature && mLockedFeature->featureId() == fid && mLockedFeature->layer() == layer )
  {
    mLockedFeature->geometryChanged( fid, geom );
    if ( mVertexEditor )
      mVertexEditor->updateEditor( mLockedFeature.get() );
    updateLockedFeatureVertices();
  }
}

void QgsVertexTool::onCachedGeometryDeleted( QgsFeatureId fid )
{
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( sender() );
  Q_ASSERT( mCache.contains( layer ) );
  QHash<QgsFeatureId, QgsGeometry> &layerCache = mCache[layer];
  if ( layerCache.contains( fid ) )
    layerCache.remove( fid );

  // refresh highlighted vertices - some may have been deleted
  setHighlightedVertices( mSelectedVertices );

  if ( mLockedFeature && mLockedFeature->featureId() == fid && mLockedFeature->layer() == layer )
  {
    mLockedFeature->featureDeleted( fid );
    updateLockedFeatureVertices();
  }
}

void QgsVertexTool::updateVertexEditor( QgsVectorLayer *layer, QgsFeatureId fid )
{
  if ( layer )
  {
    if ( mLockedFeature && mLockedFeature->featureId() == fid && mLockedFeature->layer() == layer )
    {
      // if show feature is called on a feature that's already binded to the vertex editor, toggle it off
      cleanupLockedFeature();
      return;
    }

    mLockedFeature.reset( new QgsLockedFeature( fid, layer, mCanvas ) );
    connect( mLockedFeature->layer(), &QgsVectorLayer::featureDeleted, this, &QgsVertexTool::cleanEditor );
    for ( int i = 0; i < mSelectedVertices.length(); ++i )
    {
      if ( mSelectedVertices.at( i ).layer == layer && mSelectedVertices.at( i ).fid == fid )
      {
        mLockedFeature->selectVertex( mSelectedVertices.at( i ).vertexId );
      }
    }
    connect( mLockedFeature.get(), &QgsLockedFeature::selectionChanged, this, &QgsVertexTool::lockedFeatureSelectionChanged );
  }

  updateLockedFeatureVertices();

  // make sure the vertex editor is alive and visible
  showVertexEditor();  //#spellok

  mVertexEditor->updateEditor( mLockedFeature.get() );
}

void QgsVertexTool::updateLockedFeatureVertices()
{
  qDeleteAll( mLockedFeatureVerticesMarkers );
  mLockedFeatureVerticesMarkers.clear();
  if ( mVertexEditor && mLockedFeature )
  {
    const QList<QgsVertexEntry *> &vertexMap = mLockedFeature->vertexMap();
    for ( const QgsVertexEntry *vertex : vertexMap )
    {
      if ( !vertex->isSelected() )
      {
        QgsVertexMarker *marker = new QgsVertexMarker( canvas() );
        marker->setIconType( QgsVertexMarker::ICON_CIRCLE );
        marker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
        marker->setPenWidth( QgsGuiUtils::scaleIconSize( 2 ) );
        marker->setColor( Qt::red );
        marker->setCenter( toMapCoordinates( mLockedFeature->layer(), vertex->point() ) );
        mLockedFeatureVerticesMarkers.append( marker );
      }
    }
  }
}


void QgsVertexTool::showVertexEditor()  //#spellok
{
  if ( !mVertexEditor )
  {
    mVertexEditor = new QgsVertexEditor( mCanvas );
    if ( !QgisApp::instance()->restoreDockWidget( mVertexEditor ) )
      QgisApp::instance()->addDockWidget( Qt::LeftDockWidgetArea, mVertexEditor );

    connect( mVertexEditor, &QgsVertexEditor::deleteSelectedRequested, this, &QgsVertexTool::deleteVertexEditorSelection );
    connect( mVertexEditor, &QgsVertexEditor::editorClosed, this, &QgsVertexTool::cleanupVertexEditor );

    // timer required as showing/raising the vertex editor in the same function following restoreDockWidget fails
    QTimer::singleShot( 200, this, [ = ] { if ( mVertexEditor ) { mVertexEditor->show(); mVertexEditor->raise(); } } );
  }
  else
  {
    mVertexEditor->show();
    mVertexEditor->raise();
  }
}

void QgsVertexTool::cleanupVertexEditor()
{
  if ( mVertexEditor )
  {
    cleanupLockedFeature();
    // do not delete immediately because vertex editor
    // can be still used in the qt event loop
    mVertexEditor->deleteLater();
  }
}

void QgsVertexTool::cleanupLockedFeature()
{
  mLockedFeature.reset();
  if ( mVertexEditor )
  {
    mVertexEditor->updateEditor( nullptr );
  }
  updateLockedFeatureVertices();
}

void QgsVertexTool::lockedFeatureSelectionChanged()
{
  Q_ASSERT( mLockedFeature );
  QList<QgsVertexEntry *> &vertexMap = mLockedFeature->vertexMap();
  QList<Vertex> vertices;
  for ( int i = 0, n = vertexMap.size(); i < n; ++i )
  {
    if ( vertexMap[i]->isSelected() )
    {
      vertices << Vertex( mLockedFeature->layer(), mLockedFeature->featureId(), i );
    }
  }

  setHighlightedVertices( vertices, ModeReset );

  updateLockedFeatureVertices();
}

static int _firstSelectedVertex( QgsLockedFeature &selectedFeature )
{
  QList<QgsVertexEntry *> &vertexMap = selectedFeature.vertexMap();
  for ( int i = 0, n = vertexMap.size(); i < n; ++i )
  {
    if ( vertexMap[i]->isSelected() )
    {
      return i;
    }
  }
  return -1;
}

static void _safeSelectVertex( QgsLockedFeature &selectedFeature, int vertexNr )
{
  int n = selectedFeature.vertexMap().size();
  selectedFeature.selectVertex( ( vertexNr + n ) % n );
}

void QgsVertexTool::deleteVertexEditorSelection()
{
  if ( !mLockedFeature )
    return;

  int firstSelectedIndex = _firstSelectedVertex( *mLockedFeature );
  if ( firstSelectedIndex == -1 )
    return;

  // make a list of selected vertices
  QList<Vertex> vertices;
  QList<QgsVertexEntry *> &selFeatureVertices = mLockedFeature->vertexMap();
  QgsVectorLayer *layer = mLockedFeature->layer();
  QgsFeatureId fid = mLockedFeature->featureId();
  QgsGeometry geometry = cachedGeometry( layer, fid );
  for ( QgsVertexEntry *vertex : std::as_const( selFeatureVertices ) )
  {
    if ( vertex->isSelected() )
    {
      int vertexIndex = geometry.vertexNrFromVertexId( vertex->vertexId() );
      if ( vertexIndex != -1 )
        vertices.append( Vertex( layer, fid, vertexIndex ) );
    }
  }

  // now select the vertices and delete them...
  setHighlightedVertices( vertices );
  deleteVertex();

  if ( !mLockedFeature->geometry()->isNull() )
  {
    int nextVertexToSelect = firstSelectedIndex;
    if ( mLockedFeature->geometry()->type() == QgsWkbTypes::LineGeometry )
    {
      // for lines we don't wrap around vertex selection when deleting vertices from end of line
      nextVertexToSelect = std::min( nextVertexToSelect, mLockedFeature->geometry()->constGet()->nCoordinates() - 1 );
    }

    _safeSelectVertex( *mLockedFeature, nextVertexToSelect );
  }
  mLockedFeature->layer()->triggerRepaint();
}


void QgsVertexTool::startDragging( QgsMapMouseEvent *e )
{
  QgsPointXY mapPoint = toMapCoordinates( e->pos() );
  if ( isNearEndpointMarker( mapPoint ) )
  {
    startDraggingAddVertexAtEndpoint( mapPoint );
    return;
  }

  QgsPointLocator::Match m = snapToEditableLayer( e );
  if ( !m.isValid() )
    return;
  if ( mLockedFeature && ( mLockedFeature->featureId() != m.featureId() || mLockedFeature->layer() != m.layer() ) )
    return; // when a feature is bound to the vertex editor, only process actions for that feature

  // activate advanced digitizing dock
  setAdvancedDigitizingAllowed( true );

  // adding a new vertex instead of moving a vertex
  if ( m.hasEdge() )
  {
    // only start dragging if we are near edge center
    mapPoint = toMapCoordinates( e->pos() );
    bool isNearCenter = matchEdgeCenterTest( m, mapPoint );
    if ( isNearCenter )
      startDraggingAddVertex( m );
    else
      startDraggingEdge( m, mapPoint );
  }
  else   // vertex
  {
    startDraggingMoveVertex( m );
  }
}

QList<QgsVectorLayer *> QgsVertexTool::editableVectorLayers()
{
  QList<QgsVectorLayer *> editableLayers;
  const auto layers = canvas()->layers( true );
  for ( QgsMapLayer *layer : layers )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( vlayer && vlayer->isEditable() && vlayer->isSpatial() )
      editableLayers << vlayer;
  }
  return editableLayers;
}

QSet<Vertex> QgsVertexTool::findCoincidentVertices( const QSet<Vertex> &vertices )
{
  QSet<Vertex> topoVertices;
  const auto editableLayers = editableVectorLayers();

  for ( const Vertex &v : std::as_const( vertices ) )
  {
    QgsPointXY origPointV = cachedGeometryForVertex( v ).vertexAt( v.vertexId );
    QgsPointXY mapPointV = toMapCoordinates( v.layer, origPointV );
    for ( QgsVectorLayer *vlayer : editableLayers )
    {
      const auto snappedVertices = layerVerticesSnappedToPoint( vlayer, mapPointV );
      for ( const QgsPointLocator::Match &otherMatch : snappedVertices )
      {
        Vertex otherVertex( otherMatch.layer(), otherMatch.featureId(), otherMatch.vertexIndex() );
        if ( !vertices.contains( otherVertex ) )
          topoVertices << otherVertex;
      }
    }
  }
  return topoVertices;
}

void QgsVertexTool::buildExtraVertices( const QSet<Vertex> &vertices, const QgsPointXY &anchorPoint, QgsVectorLayer *anchorLayer )
{
  for ( const Vertex &v : std::as_const( vertices ) )
  {
    if ( mDraggingVertex && v == *mDraggingVertex )
      continue;

    QgsPointXY pointV = cachedGeometryForVertex( v ).vertexAt( v.vertexId );

    // Calculate position of the anchor point in the coordinates of our vertex v
    // Try to avoid reprojection so we do not loose precision when map CRS does not match layer CRS
    QgsPointXY anchorPointLayer;
    if ( v.layer->crs() == anchorLayer->crs() )
      anchorPointLayer = anchorPoint;
    else  // reprojection is necessary: ANCHOR -> MAP -> V
      anchorPointLayer = toLayerCoordinates( v.layer, toMapCoordinates( anchorLayer, anchorPoint ) );
    QgsVector offset = pointV - anchorPointLayer;

    mDraggingExtraVertices << v;
    mDraggingExtraVerticesOffset << offset;
  }
}

void QgsVertexTool::startDraggingMoveVertex( const QgsPointLocator::Match &m )
{
  Q_ASSERT( m.hasVertex() );

  QgsGeometry geom = cachedGeometry( m.layer(), m.featureId() );

  // start dragging of snapped point of current layer
  mDraggingVertex.reset( new Vertex( m.layer(), m.featureId(), m.vertexIndex() ) );
  mDraggingVertexType = MovingVertex;
  mDraggingExtraVertices.clear();
  mDraggingExtraVerticesOffset.clear();

  setHighlightedVerticesVisible( false );  // hide any extra highlight of vertices until we are done with moving

  QSet<Vertex> movingVertices;
  movingVertices << *mDraggingVertex;
  for ( const Vertex &v : std::as_const( mSelectedVertices ) )
    movingVertices << v;

  if ( QgsProject::instance()->topologicalEditing() )
  {
    movingVertices.unite( findCoincidentVertices( movingVertices ) );
  }

  buildExtraVertices( movingVertices, geom.vertexAt( m.vertexIndex() ), m.layer() );

  cadDockWidget()->setPoints( QList<QgsPointXY>() << m.point() << m.point() );

  // now build drag rubber bands for extra vertices

  QgsPointXY dragVertexMapPoint = m.point();

  buildDragBandsForVertices( movingVertices, dragVertexMapPoint );
}

void QgsVertexTool::buildDragBandsForVertices( const QSet<Vertex> &movingVertices, const QgsPointXY &dragVertexMapPoint )
{
  QSet<Vertex> verticesInStraightBands;  // always the vertex with lower index

  // set of middle vertices that are already in a circular rubber band
  // i.e. every circular band is defined by its middle circular vertex
  QSet<Vertex> verticesInCircularBands;

  for ( const Vertex &v : std::as_const( movingVertices ) )
  {
    int v0idx, v1idx;
    QgsGeometry geom = cachedGeometry( v.layer, v.fid );
    QgsPointXY pt = geom.vertexAt( v.vertexId );

    geom.adjacentVertices( v.vertexId, v0idx, v1idx );

    if ( v0idx != -1 && v1idx != -1 && isCircularVertex( geom, v.vertexId ) )
    {
      // the vertex is in the middle of a curved segment
      if ( !verticesInCircularBands.contains( v ) )
      {
        addDragCircularBand( v.layer,
                             geom.vertexAt( v0idx ),
                             pt,
                             geom.vertexAt( v1idx ),
                             movingVertices.contains( Vertex( v.layer, v.fid, v0idx ) ),
                             true,
                             movingVertices.contains( Vertex( v.layer, v.fid, v1idx ) ),
                             dragVertexMapPoint );
        verticesInCircularBands << v;
      }

      // skip the rest - no need for further straight of circular bands for this vertex
      // because our circular rubber band spans both towards left and right
      continue;
    }

    if ( v0idx != -1 )
    {
      // there is another vertex to the left - let's build a rubber band for it
      Vertex v0( v.layer, v.fid, v0idx );
      if ( isCircularVertex( geom, v0idx ) )
      {
        // circular segment to the left
        if ( !verticesInCircularBands.contains( v0 ) )
        {
          addDragCircularBand( v.layer,
                               geom.vertexAt( v0idx - 1 ),
                               geom.vertexAt( v0idx ),
                               pt,
                               movingVertices.contains( Vertex( v.layer, v.fid, v0idx - 1 ) ),
                               movingVertices.contains( Vertex( v.layer, v.fid, v0idx ) ),
                               true,
                               dragVertexMapPoint );
          verticesInCircularBands << v0;
        }
      }
      else
      {
        // straight segment to the left
        if ( !verticesInStraightBands.contains( v0 ) )
        {
          addDragStraightBand( v.layer,
                               geom.vertexAt( v0idx ),
                               pt,
                               movingVertices.contains( v0 ),
                               true,
                               dragVertexMapPoint );
          verticesInStraightBands << v0;
        }
      }
    }

    if ( v1idx != -1 )
    {
      // there is another vertex to the right - let's build a rubber band for it
      Vertex v1( v.layer, v.fid, v1idx );
      if ( isCircularVertex( geom, v1idx ) )
      {
        // circular segment to the right
        if ( !verticesInCircularBands.contains( v1 ) )
        {
          addDragCircularBand( v.layer,
                               pt,
                               geom.vertexAt( v1idx ),
                               geom.vertexAt( v1idx + 1 ),
                               true,
                               movingVertices.contains( v1 ),
                               movingVertices.contains( Vertex( v.layer, v.fid, v1idx + 1 ) ),
                               dragVertexMapPoint );
          verticesInCircularBands << v1;
        }
      }
      else
      {
        // straight segment to the right
        if ( !verticesInStraightBands.contains( v ) )
        {
          addDragStraightBand( v.layer,
                               pt,
                               geom.vertexAt( v1idx ),
                               true,
                               movingVertices.contains( v1 ),
                               dragVertexMapPoint );
          verticesInStraightBands << v;
        }
      }
    }

    if ( v0idx == -1 && v1idx == -1 )
    {
      // this is a standalone point - we need to use a marker for it
      // to give some feedback to the user

      QgsPointXY ptMapPoint = toMapCoordinates( v.layer, pt );
      QgsVertexMarker *marker = new QgsVertexMarker( mCanvas );
      marker->setIconType( QgsVertexMarker::ICON_X );
      marker->setColor( Qt::red );
      marker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
      marker->setPenWidth( QgsGuiUtils::scaleIconSize( 3 ) );
      marker->setVisible( true );
      marker->setCenter( ptMapPoint );
      mDragPointMarkers << marker;
      mDragPointMarkersOffset << ( ptMapPoint - dragVertexMapPoint );
    }
  }
}

QList<QgsPointLocator::Match> QgsVertexTool::layerVerticesSnappedToPoint( QgsVectorLayer *layer, const QgsPointXY &mapPoint )
{
  MatchCollectingFilter myfilter( this );
  QgsPointLocator *loc = canvas()->snappingUtils()->locatorForLayer( layer );
  loc->nearestVertex( mapPoint, 0, &myfilter, true );
  return myfilter.matches;
}

QList<QgsPointLocator::Match> QgsVertexTool::layerSegmentsSnappedToSegment( QgsVectorLayer *layer, const QgsPointXY &mapPoint1, const QgsPointXY &mapPoint2 )
{
  QList<QgsPointLocator::Match> finalMatches;
  // we want segment matches that have exactly the same vertices as the given segment (mapPoint1, mapPoint2)
  // so rather than doing nearest edge search which could return any segment within a tolerance,
  // we first find matches for one endpoint and then see if there is a matching other endpoint.
  const QList<QgsPointLocator::Match> matches1 = layerVerticesSnappedToPoint( layer, mapPoint1 );
  for ( const QgsPointLocator::Match &m : matches1 )
  {
    QgsGeometry g = cachedGeometry( layer, m.featureId() );
    int v0, v1;
    g.adjacentVertices( m.vertexIndex(), v0, v1 );
    if ( v0 != -1 && QgsPointXY( g.vertexAt( v0 ) ) == mapPoint2 )
      finalMatches << QgsPointLocator::Match( QgsPointLocator::Edge, layer, m.featureId(), 0, m.point(), v0 );
    else if ( v1 != -1 && QgsPointXY( g.vertexAt( v1 ) ) == mapPoint2 )
      finalMatches << QgsPointLocator::Match( QgsPointLocator::Edge, layer, m.featureId(), 0, m.point(), m.vertexIndex() );
  }
  return finalMatches;
}

void QgsVertexTool::startDraggingAddVertex( const QgsPointLocator::Match &m )
{
  Q_ASSERT( m.hasEdge() );

  // activate advanced digitizing dock
  setAdvancedDigitizingAllowed( true );

  mDraggingVertex.reset( new Vertex( m.layer(), m.featureId(), m.vertexIndex() + 1 ) );
  mDraggingVertexType = AddingVertex;
  mDraggingExtraVertices.clear();
  mDraggingExtraVerticesOffset.clear();
  mDraggingExtraSegments.clear();

  QgsGeometry geom = cachedGeometry( m.layer(), m.featureId() );

  // TODO: handles rings correctly?
  QgsPointXY v0 = geom.vertexAt( m.vertexIndex() );
  QgsPointXY v1 = geom.vertexAt( m.vertexIndex() + 1 );

  QgsPointXY map_v0 = toMapCoordinates( m.layer(), v0 );
  QgsPointXY map_v1 = toMapCoordinates( m.layer(), v1 );

  if ( v0.x() != 0 || v0.y() != 0 )
    addDragBand( map_v0, m.point() );
  if ( v1.x() != 0 || v1.y() != 0 )
    addDragBand( map_v1, m.point() );

  if ( QgsProject::instance()->topologicalEditing() )
  {
    // find other segments coincident with the one user just picked and store them in a list
    // so we can add a new vertex also in those to keep topology correct
    const auto editableLayers = editableVectorLayers();
    for ( QgsVectorLayer *vlayer : editableLayers )
    {
      if ( vlayer->geometryType() != QgsWkbTypes::LineGeometry && vlayer->geometryType() != QgsWkbTypes::PolygonGeometry )
        continue;

      QgsPointXY pt1, pt2;
      m.edgePoints( pt1, pt2 );
      const auto snappedSegments = layerSegmentsSnappedToSegment( vlayer, pt1, pt2 );
      for ( const QgsPointLocator::Match &otherMatch : snappedSegments )
      {
        if ( otherMatch.layer() == m.layer() &&
             otherMatch.featureId() == m.featureId() &&
             otherMatch.vertexIndex() == m.vertexIndex() )
          continue;

        // start dragging of snapped point of current layer
        mDraggingExtraSegments << Vertex( otherMatch.layer(), otherMatch.featureId(), otherMatch.vertexIndex() );
      }
    }
  }

  cadDockWidget()->setPoints( QList<QgsPointXY>() << m.point() << m.point() );
}

void QgsVertexTool::startDraggingAddVertexAtEndpoint( const QgsPointXY &mapPoint )
{
  Q_ASSERT( mMouseAtEndpoint );

  // activate advanced digitizing dock
  setAdvancedDigitizingAllowed( true );

  mDraggingVertex.reset( new Vertex( mMouseAtEndpoint->layer, mMouseAtEndpoint->fid, mMouseAtEndpoint->vertexId ) );
  mDraggingVertexType = AddingEndpoint;
  mDraggingExtraVertices.clear();
  mDraggingExtraVerticesOffset.clear();

  QgsGeometry geom = cachedGeometry( mMouseAtEndpoint->layer, mMouseAtEndpoint->fid );
  QgsPointXY v0 = geom.vertexAt( mMouseAtEndpoint->vertexId );
  QgsPointXY map_v0 = toMapCoordinates( mMouseAtEndpoint->layer, v0 );

  addDragBand( map_v0, mapPoint );

  // setup CAD dock previous points to endpoint and the previous point
  QgsPointXY pt0 = geom.vertexAt( adjacentVertexIndexToEndpoint( geom, mMouseAtEndpoint->vertexId ) );
  QgsPointXY pt1 = geom.vertexAt( mMouseAtEndpoint->vertexId );

  cadDockWidget()->setPoints( QList<QgsPointXY>() << pt0 << pt1 << pt1 );
}

void QgsVertexTool::startDraggingEdge( const QgsPointLocator::Match &m, const QgsPointXY &mapPoint )
{
  Q_ASSERT( m.hasEdge() );

  // activate advanced digitizing
  setAdvancedDigitizingAllowed( true );

  mDraggingEdge = true;
  mDraggingExtraVertices.clear();
  mDraggingExtraVerticesOffset.clear();

  QgsGeometry geom = cachedGeometry( m.layer(), m.featureId() );

  QSet<Vertex> movingVertices;
  movingVertices << Vertex( m.layer(), m.featureId(), m.vertexIndex() );
  movingVertices << Vertex( m.layer(), m.featureId(), m.vertexIndex() + 1 );

  // add an extra vertex if it is circular edge - so that we move the whole edge and not just one part of it
  if ( isCircularVertex( geom, m.vertexIndex() ) )
  {
    movingVertices << Vertex( m.layer(), m.featureId(), m.vertexIndex() - 1 );
  }
  else if ( isCircularVertex( geom, m.vertexIndex() + 1 ) )
  {
    movingVertices << Vertex( m.layer(), m.featureId(), m.vertexIndex() + 2 );
  }

  buildDragBandsForVertices( movingVertices, mapPoint );

  if ( QgsProject::instance()->topologicalEditing() )
  {
    movingVertices.unite( findCoincidentVertices( movingVertices ) );
  }

  QgsPointXY layerPoint = toLayerCoordinates( m.layer(), mapPoint );
  buildExtraVertices( movingVertices, layerPoint, m.layer() );

  cadDockWidget()->setPoints( QList<QgsPointXY>() << m.point() << m.point() );
}

void QgsVertexTool::stopDragging()
{
  // deactivate advanced digitizing
  setAdvancedDigitizingAllowed( false );
  cadDockWidget()->clear();  // clear cad points and release locks

  mDraggingVertex.reset();
  mDraggingVertexType = NotDragging;
  mDraggingEdge = false;
  clearDragBands();

  setHighlightedVerticesVisible( true );  // highlight can be shown again

  mSnapIndicator->setMatch( QgsPointLocator::Match() );
}

QgsPoint QgsVertexTool::matchToLayerPoint( const QgsVectorLayer *destLayer, const QgsPointXY &mapPoint, const QgsPointLocator::Match *match )
{
  if ( match->layer() )
  {
    switch ( match->type() )
    {
      case QgsPointLocator::Vertex:
      case QgsPointLocator::LineEndpoint:
      case QgsPointLocator::All:
      {
        //  use point coordinates of the layer
        QgsFeature f;
        QgsFeatureIterator fi = match->layer()->getFeatures( QgsFeatureRequest( match->featureId() ).setNoAttributes() );
        if ( fi.nextFeature( f ) )
        {
          QgsPoint layerPoint = f.geometry().vertexAt( match->vertexIndex() );
          if ( match->layer()->crs() == destLayer->crs() )
          {
            return layerPoint;
          }
          else
          {
            QgsCoordinateTransform transform( match->layer()->crs(), destLayer->crs(), mCanvas->mapSettings().transformContext() );
            if ( transform.isValid() )
            {
              try
              {
                layerPoint.transform( transform );
                return layerPoint;
              }
              catch ( QgsCsException & )
              {
                QgsDebugMsg( QStringLiteral( "transformation to layer coordinate failed" ) );
              }
            }
            return layerPoint;
          }
        }
      }
      break;
      case QgsPointLocator::Edge:
      case QgsPointLocator::MiddleOfSegment:
        return toLayerCoordinates( destLayer, match->interpolatedPoint( mCanvas->mapSettings().destinationCrs() ) );
        break;
      case QgsPointLocator::Invalid:
      case QgsPointLocator::Area:
      case QgsPointLocator::Centroid:
        break;
    }
  }

  // fall back to reprojection of the map point to layer point
  return QgsPoint( toLayerCoordinates( destLayer, mapPoint ) );
}

void QgsVertexTool::moveEdge( const QgsPointXY &mapPoint )
{
  stopDragging();

  VertexEdits edits;
  addExtraVerticesToEdits( edits, mapPoint );

  applyEditsToLayers( edits );
}

void QgsVertexTool::moveVertex( const QgsPointXY &mapPoint, const QgsPointLocator::Match *mapPointMatch )
{
  // deactivate advanced digitizing
  setAdvancedDigitizingAllowed( false );

  QgsVectorLayer *dragLayer = mDraggingVertex->layer;
  QgsFeatureId dragFid = mDraggingVertex->fid;

  int dragVertexId = mDraggingVertex->vertexId;
  bool addingVertex = mDraggingVertexType == AddingVertex || mDraggingVertexType == AddingEndpoint;
  bool addingAtEndpoint = mDraggingVertexType == AddingEndpoint;
  QgsGeometry geom = cachedGeometryForVertex( *mDraggingVertex );
  stopDragging();

  QgsPoint layerPoint = matchToLayerPoint( dragLayer, mapPoint, mapPointMatch );

  QgsVertexId vid;
  if ( !geom.vertexIdFromVertexNr( dragVertexId, vid ) )
  {
    QgsDebugMsg( QStringLiteral( "invalid vertex index" ) );
    return;
  }

  QgsAbstractGeometry *geomTmp = geom.constGet()->clone();

  // add/move vertex
  if ( addingVertex )
  {
    if ( addingAtEndpoint && vid.vertex != 0 )  // appending?
      vid.vertex++;

    QgsPoint pt( layerPoint );
    if ( QgsWkbTypes::hasZ( dragLayer->wkbType() ) && !pt.is3D() )
      pt.addZValue( defaultZValue() );

    if ( !geomTmp->insertVertex( vid, pt ) )
    {
      QgsDebugMsg( QStringLiteral( "append vertex failed!" ) );
      return;
    }
  }
  else
  {
    if ( !geomTmp->moveVertex( vid, layerPoint ) )
    {
      QgsDebugMsg( QStringLiteral( "move vertex failed!" ) );
      return;
    }
  }

  geom.set( geomTmp );

  VertexEdits edits; // dict { layer : { fid : geom } }
  edits[dragLayer][dragFid] = geom;

  addExtraVerticesToEdits( edits, mapPoint, dragLayer, layerPoint );

  if ( addingVertex && !addingAtEndpoint && QgsProject::instance()->topologicalEditing() )
  {
    // topo editing: when adding a vertex to an existing segment, there may be other coincident segments
    // that also need adding the same vertex
    addExtraSegmentsToEdits( edits, mapPoint, dragLayer, layerPoint );
  }

  applyEditsToLayers( edits );

  if ( QgsProject::instance()->topologicalEditing() )
  {
    // topo editing: add vertex to existing segments when moving/adding a vertex to such segment.
    // this requires that the snapping match is to a segment and the segment layer's CRS
    // is the same (otherwise we would need to reproject the point and it will not be coincident)
    const auto editKeys = edits.keys();
    for ( QgsVectorLayer *layer : editKeys )
    {
      const auto editGeom = edits[layer].values();
      for ( QgsGeometry g : editGeom )
      {
        QgsGeometry p = QgsGeometry::fromPointXY( QgsPointXY( layerPoint.x(), layerPoint.y() ) );
        if ( ( ( mapPointMatch->hasEdge() || mapPointMatch->hasMiddleSegment() ) && mapPointMatch->layer() && layer->crs() == mapPointMatch->layer()->crs() )
             || ( mapPointMatch->hasVertex() && !mapPointMatch->layer() && layer->crs() == mCanvas->mapSettings().destinationCrs() ) ) // also add topological points when snapped on intersection
        {
          if ( g.convertToType( QgsWkbTypes::PointGeometry, true ).contains( p ) )
          {
            if ( !layerPoint.is3D() )
              layerPoint.addZValue( defaultZValue() );
            layer->addTopologicalPoints( layerPoint );
            if ( mapPointMatch->layer() )
              mapPointMatch->layer()->addTopologicalPoints( layerPoint );
          }
        }
        if ( QgsProject::instance()->avoidIntersectionsMode() != QgsProject::AvoidIntersectionsMode::AllowIntersections )
        {
          for ( QgsAbstractGeometry::vertex_iterator it = g.vertices_begin() ; it != g.vertices_end() ; it++ )
          {
            layer->addTopologicalPoints( *it );
          }
        }
      }
    }
  }

  updateLockedFeatureVertices();
  if ( mVertexEditor )
    mVertexEditor->updateEditor( mLockedFeature.get() );

  setHighlightedVertices( mSelectedVertices );  // update positions of existing highlighted vertices
  setHighlightedVerticesVisible( true );  // time to show highlighted vertices again

  // restart startDraggingAddVertexAtEndpoint right after it finishes
  if ( addingAtEndpoint )
  {
    if ( mMouseAtEndpoint->vertexId != 0 )
    {
      // If we were adding at the end of the feature, we need to update the index
      mMouseAtEndpoint.reset( new Vertex( mMouseAtEndpoint->layer, mMouseAtEndpoint->fid, mMouseAtEndpoint->vertexId + 1 ) );
    }
    // And then we just restart the drag
    startDraggingAddVertexAtEndpoint( mapPoint );
  }
}


void QgsVertexTool::addExtraVerticesToEdits( QgsVertexTool::VertexEdits &edits, const QgsPointXY &mapPoint, QgsVectorLayer *dragLayer, const QgsPoint &layerPoint )
{
  Q_ASSERT( mDraggingExtraVertices.count() == mDraggingExtraVerticesOffset.count() );
  // add moved vertices from other layers
  for ( int i = 0; i < mDraggingExtraVertices.count(); ++i )
  {
    const Vertex &topo = mDraggingExtraVertices[i];
    const QgsVector &offset = mDraggingExtraVerticesOffset[i];

    QHash<QgsFeatureId, QgsGeometry> &layerEdits = edits[topo.layer];
    QgsGeometry topoGeom;
    if ( layerEdits.contains( topo.fid ) )
      topoGeom = QgsGeometry( edits[topo.layer][topo.fid] );
    else
      topoGeom = QgsGeometry( cachedGeometryForVertex( topo ) );

    QgsPoint point;
    if ( dragLayer && topo.layer->crs() == dragLayer->crs() )
      point = layerPoint;  // this point may come from exact match so it may be more precise
    else
      point = QgsPoint( toLayerCoordinates( topo.layer, mapPoint ) );

    if ( offset.x() || offset.y() )
    {
      point += offset;
    }

    if ( !topoGeom.moveVertex( point.x(), point.y(), topo.vertexId ) )
    {
      QgsDebugMsg( QStringLiteral( "[topo] move vertex failed!" ) );
      continue;
    }
    edits[topo.layer][topo.fid] = topoGeom;
  }
}


void QgsVertexTool::addExtraSegmentsToEdits( QgsVertexTool::VertexEdits &edits, const QgsPointXY &mapPoint, QgsVectorLayer *dragLayer, const QgsPoint &layerPoint )
{
  // insert new vertex also to other geometries/layers
  for ( int i = 0; i < mDraggingExtraSegments.count(); ++i )
  {
    const Vertex &topo = mDraggingExtraSegments[i];

    QHash<QgsFeatureId, QgsGeometry> &layerEdits = edits[topo.layer];
    QgsGeometry topoGeom;
    if ( layerEdits.contains( topo.fid ) )
      topoGeom = QgsGeometry( edits[topo.layer][topo.fid] );
    else
      topoGeom = QgsGeometry( cachedGeometryForVertex( topo ) );

    QgsPointXY point;
    if ( dragLayer && topo.layer->crs() == dragLayer->crs() )
      point = layerPoint;  // this point may come from exact match so it may be more precise
    else
      point = QgsPoint( toLayerCoordinates( topo.layer, mapPoint ) );

    QgsPoint pt( point );
    if ( QgsWkbTypes::hasZ( topo.layer->wkbType() ) )
      pt.addZValue( defaultZValue() );

    if ( !topoGeom.insertVertex( pt, topo.vertexId + 1 ) )
    {
      QgsDebugMsg( QStringLiteral( "[topo] segment insert vertex failed!" ) );
      continue;
    }
    edits[topo.layer][topo.fid] = topoGeom;
  }
}


void QgsVertexTool::applyEditsToLayers( QgsVertexTool::VertexEdits &edits )
{
  QHash<QgsVectorLayer *, QHash<QgsFeatureId, QgsGeometry> >::iterator it = edits.begin();
  for ( ; it != edits.end(); ++it )
  {
    QgsVectorLayer *layer = it.key();
    QHash<QgsFeatureId, QgsGeometry> &layerEdits = it.value();
    layer->beginEditCommand( tr( "Moved vertex" ) );
    QHash<QgsFeatureId, QgsGeometry>::iterator it2 = layerEdits.begin();
    for ( ; it2 != layerEdits.end(); ++it2 )
    {
      QgsGeometry featGeom = it2.value();
      layer->changeGeometry( it2.key(), featGeom );
      edits[layer][it2.key()] = featGeom;
    }

    if ( mVertexEditor )
      mVertexEditor->updateEditor( mLockedFeature.get() );
  }



  for ( it = edits.begin() ; it != edits.end(); ++it )
  {
    QgsVectorLayer *layer = it.key();
    QHash<QgsFeatureId, QgsGeometry> &layerEdits = it.value();
    QHash<QgsFeatureId, QgsGeometry>::iterator it2 = layerEdits.begin();
    for ( ; it2 != layerEdits.end(); ++it2 )
    {
      QList<QgsVectorLayer *>  avoidIntersectionsLayers;
      switch ( QgsProject::instance()->avoidIntersectionsMode() )
      {
        case QgsProject::AvoidIntersectionsMode::AvoidIntersectionsCurrentLayer:
          avoidIntersectionsLayers.append( layer );
          break;
        case QgsProject::AvoidIntersectionsMode::AvoidIntersectionsLayers:
          avoidIntersectionsLayers = QgsProject::instance()->avoidIntersectionsLayers();
          break;
        case QgsProject::AvoidIntersectionsMode::AllowIntersections:
          break;
      }
      QgsGeometry featGeom = it2.value();
      layer->changeGeometry( it2.key(), featGeom );
      if ( avoidIntersectionsLayers.size() > 0 )
      {
        QHash<QgsVectorLayer *, QSet<QgsFeatureId> > ignoreFeatures;
        QSet<QgsFeatureId> id;
        id.insert( it2.key() );
        ignoreFeatures.insert( layer, id );
        int avoidIntersectionsReturn = featGeom.avoidIntersections( avoidIntersectionsLayers, ignoreFeatures );
        switch ( avoidIntersectionsReturn )
        {
          case 2:
            emit messageEmitted( tr( "The operation would change the geometry type." ), Qgis::MessageLevel::Warning );
            break;

          case 3:
            emit messageEmitted( tr( "At least one geometry intersected is invalid. These geometries must be manually repaired." ), Qgis::MessageLevel::Warning );
            break;
          default:
            break;
        }
      }
      layer->changeGeometry( it2.key(), featGeom );
      edits[layer][it2.key()] = featGeom;
    }
    layer->endEditCommand();
    layer->triggerRepaint();

    if ( mVertexEditor )
      mVertexEditor->updateEditor( mLockedFeature.get() );
  }

}


void QgsVertexTool::deleteVertex()
{
  QSet<Vertex> toDelete;
  if ( !mSelectedVertices.isEmpty() )
  {
    toDelete = qgis::listToSet( mSelectedVertices );
  }
  else
  {
    bool addingVertex = mDraggingVertexType == AddingVertex || mDraggingVertexType == AddingEndpoint;
    toDelete << *mDraggingVertex;
    toDelete += qgis::listToSet( mDraggingExtraVertices );

    if ( addingVertex )
    {
      stopDragging();
      return;   // just cancel the vertex
    }
  }

  stopDragging();
  setHighlightedVertices( QList<Vertex>() ); // reset selection

  if ( QgsProject::instance()->topologicalEditing() )
  {
    // if topo editing is enabled, delete all the vertices that are on the same location
    QSet<Vertex> topoVerticesToDelete;
    for ( const Vertex &vertexToDelete : std::as_const( toDelete ) )
    {
      QgsPointXY layerPt = cachedGeometryForVertex( vertexToDelete ).vertexAt( vertexToDelete.vertexId );
      QgsPointXY mapPt = toMapCoordinates( vertexToDelete.layer, layerPt );
      const auto snappedVertices = layerVerticesSnappedToPoint( vertexToDelete.layer, mapPt );
      for ( const QgsPointLocator::Match &otherMatch : snappedVertices )
      {
        Vertex otherVertex( otherMatch.layer(), otherMatch.featureId(), otherMatch.vertexIndex() );
        if ( toDelete.contains( otherVertex ) || topoVerticesToDelete.contains( otherVertex ) )
          continue;

        topoVerticesToDelete.insert( otherVertex );
      }
    }

    toDelete.unite( topoVerticesToDelete );
  }

  // switch from a plain list to dictionary { layer: { fid: [vertexNr1, vertexNr2, ...] } }
  QHash<QgsVectorLayer *, QHash<QgsFeatureId, QList<int> > > toDeleteGrouped;
  for ( const Vertex &vertex : std::as_const( toDelete ) )
  {
    toDeleteGrouped[vertex.layer][vertex.fid].append( vertex.vertexId );
  }

  // de-duplicate vertices in linear rings - if there is the first vertex selected,
  // then also the last vertex will be selected - but we want just one out of the pair
  // also deselect vertices of parts or rings that will be automatically removed
  QHash<QgsVectorLayer *, QHash<QgsFeatureId, QList<int> > >::iterator lIt = toDeleteGrouped.begin();
  for ( ; lIt != toDeleteGrouped.end(); ++lIt )
  {
    QgsVectorLayer *layer = lIt.key();
    QHash<QgsFeatureId, QList<int> > &featuresDict = lIt.value();

    QHash<QgsFeatureId, QList<int> >::iterator fIt = featuresDict.begin();
    for ( ; fIt != featuresDict.end(); ++fIt )
    {
      QgsFeatureId fid = fIt.key();
      QList<int> &vertexIds = fIt.value();
      if ( vertexIds.count() >= 2 && ( layer->geometryType() == QgsWkbTypes::PolygonGeometry ||
                                       layer->geometryType() == QgsWkbTypes::LineGeometry ) )
      {
        std::sort( vertexIds.begin(), vertexIds.end(), std::greater<int>() );
        const QgsGeometry geom = cachedGeometry( layer, fid );
        const QgsAbstractGeometry *ag = geom.constGet();
        QVector<QVector<int>> numberOfVertices;
        for ( int p = 0 ; p < ag->partCount() ; ++p )
        {
          numberOfVertices.append( QVector<int>() );
          for ( int r = 0 ; r < ag->ringCount( p ) ; ++r )
          {
            numberOfVertices[p].append( ag->vertexCount( p, r ) );
          }
        }
        // polygonal rings with less than 4 vertices get deleted automatically
        // linear parts with less than 2 vertices get deleted automatically
        // let's keep that number and don't remove vertices beyond that point
        const int minAllowedVertices = geom.type() == QgsWkbTypes::PolygonGeometry ? 4 : 2;
        for ( int i = vertexIds.count() - 1; i >= 0 ; --i )
        {
          QgsVertexId vid;
          if ( geom.vertexIdFromVertexNr( vertexIds[i], vid ) )
          {
            // also don't try to delete the first vertex of a ring since we have already deleted the last
            if ( numberOfVertices.at( vid.part ).at( vid.ring ) < minAllowedVertices ||
                 ( 0 == vid.vertex && geom.type() == QgsWkbTypes::PolygonGeometry ) )
              vertexIds.removeOne( vertexIds.at( i ) );
            else
              --numberOfVertices[vid.part][vid.ring];
          }
        }
      }
    }
  }

  // main for cycle to delete all selected vertices
  QHash<QgsVectorLayer *, QHash<QgsFeatureId, QList<int> > >::iterator it = toDeleteGrouped.begin();
  for ( ; it != toDeleteGrouped.end(); ++it )
  {
    QgsVectorLayer *layer = it.key();
    QHash<QgsFeatureId, QList<int> > &featuresDict = it.value();

    layer->beginEditCommand( tr( "Deleted vertex" ) );
    bool success = true;

    QHash<QgsFeatureId, QList<int> >::iterator it2 = featuresDict.begin();
    for ( ; it2 != featuresDict.end(); ++it2 )
    {
      QgsFeatureId fid = it2.key();
      QList<int> &vertexIds = it2.value();

      Qgis::VectorEditResult res = Qgis::VectorEditResult::Success;
      std::sort( vertexIds.begin(), vertexIds.end(), std::greater<int>() );
      for ( int vertexId : vertexIds )
      {
        if ( res != Qgis::VectorEditResult::EmptyGeometry )
          res = layer->deleteVertex( fid, vertexId );
        if ( res != Qgis::VectorEditResult::EmptyGeometry && res != Qgis::VectorEditResult::Success )
        {
          QgsDebugMsg( QStringLiteral( "failed to delete vertex %1 %2 %3!" ).arg( layer->name() ).arg( fid ).arg( vertexId ) );
          success = false;
        }
      }

      if ( res == Qgis::VectorEditResult::EmptyGeometry )
      {
        emit messageEmitted( tr( "Geometry has been cleared. Use the add part tool to set geometry for this feature." ) );
      }
    }

    if ( success )
    {
      layer->endEditCommand();
      layer->triggerRepaint();
    }
    else
      layer->destroyEditCommand();
  }

  // make sure the temporary feature rubber band is not visible
  removeTemporaryRubberBands();

  // pre-select next vertex for deletion if we are deleting just one vertex
  if ( toDelete.count() == 1 )
  {
    const Vertex &vertex = *toDelete.constBegin();
    QgsGeometry geom( cachedGeometryForVertex( vertex ) );
    int vertexId = vertex.vertexId;

    // if next vertex is not available, use the previous one
    if ( geom.vertexAt( vertexId ) == QgsPoint() )
      vertexId -= 1;

    if ( geom.vertexAt( vertexId ) != QgsPoint() )
    {
      QList<Vertex> vertices_new;
      vertices_new << Vertex( vertex.layer, vertex.fid, vertexId );
      setHighlightedVertices( vertices_new );
    }
  }

  if ( mVertexEditor && mLockedFeature )
    mVertexEditor->updateEditor( mLockedFeature.get() );
}


void QgsVertexTool::toggleVertexCurve()
{

  Vertex toConvert = Vertex( nullptr, -1, -1 );
  if ( mSelectedVertices.size() == 1 )
  {
    toConvert = mSelectedVertices.first();
  }
  else if ( mDraggingVertexType == AddingVertex || mDraggingVertexType == MovingVertex )
  {
    toConvert = *mDraggingVertex;
  }
  else
  {
    // TODO support more than just 1 vertex
    QgisApp::instance()->messageBar()->pushMessage(
      tr( "Could not convert vertex" ),
      tr( "Conversion can only be done on exactly one vertex." ),
      Qgis::Info );
    return;
  }

  if ( mDraggingVertex )
  {
    if ( mDraggingVertexType == AddingVertex || mDraggingVertexType == AddingEndpoint )
    {
      QgisApp::instance()->messageBar()->pushMessage(
        tr( "Could not convert vertex" ),
        tr( "Cannot convert vertex before it is added." ),
        Qgis::Warning );
      return;
    }
    stopDragging();
  }

  QgsVectorLayer *layer = toConvert.layer;

  if ( ! QgsWkbTypes::isCurvedType( layer->wkbType() ) )
  {
    QgisApp::instance()->messageBar()->pushMessage(
      tr( "Could not convert vertex" ),
      tr( "Layer of type %1 does not support curved geometries." ).arg( QgsWkbTypes::displayString( layer->wkbType() ) ),
      Qgis::Warning );
    return;
  }

  layer->beginEditCommand( tr( "Toggled vertex to/from curve" ) );

  QgsGeometry geom = layer->getFeature( toConvert.fid ).geometry();

  bool success = geom.toggleCircularAtVertex( toConvert.vertexId );

  if ( success )
  {
    layer->changeGeometry( toConvert.fid, geom );
    layer->endEditCommand();
    layer->triggerRepaint();
  }
  else
  {
    layer->destroyEditCommand();
    QgisApp::instance()->messageBar()->pushMessage(
      tr( "Could not convert vertex" ),
      tr( "Start/end of vertices of features and arcs can not be converted." ),
      Qgis::Warning );
  }

  if ( mVertexEditor && mLockedFeature )
    mVertexEditor->updateEditor( mLockedFeature.get() );
}

void QgsVertexTool::setHighlightedVertices( const QList<Vertex> &listVertices, HighlightMode mode )
{
  // we need to make a local copy of vertices - often this method gets called
  // just to refresh positions and so mSelectedVertices is passed. But then in reset mode
  // we clear that array, which could clear also listVertices.
  QList<Vertex> listVerticesLocal( listVertices );

  if ( mode == ModeReset )
  {
    qDeleteAll( mSelectedVerticesMarkers );
    mSelectedVerticesMarkers.clear();
    mSelectedVertices.clear();
  }
  else if ( mode == ModeSubtract )
  {
    // need to clear vertex markers, and rebuild later. We have no way to link
    // a marker to a vertex in order to remove one-by-one
    qDeleteAll( mSelectedVerticesMarkers );
    mSelectedVerticesMarkers.clear();
  }

  auto createMarkerForVertex = [ = ]( const Vertex & vertex )->bool
  {
    QgsGeometry geom = cachedGeometryForVertex( vertex );
    QgsVertexId vid;
    if ( !geom.vertexIdFromVertexNr( vertex.vertexId, vid ) )
      return false;  // vertex may not exist anymore

    QgsVertexMarker *marker = new QgsVertexMarker( canvas() );
    marker->setIconType( QgsVertexMarker::ICON_CIRCLE );
    marker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
    marker->setPenWidth( QgsGuiUtils::scaleIconSize( 2 ) );
    marker->setColor( Qt::blue );
    marker->setCenter( toMapCoordinates( vertex.layer, geom.vertexAt( vertex.vertexId ) ) );
    mSelectedVerticesMarkers.append( marker );
    return true;
  };

  for ( const Vertex &vertex : listVerticesLocal )
  {
    if ( mode == ModeAdd && mSelectedVertices.contains( vertex ) )
    {
      continue;
    }
    else if ( mode == ModeSubtract )
    {
      mSelectedVertices.removeAll( vertex );
      continue;
    }

    if ( !createMarkerForVertex( vertex ) )
      continue;  // vertex may not exist anymore

    mSelectedVertices.append( vertex );
  }

  if ( mode == ModeSubtract )
  {
    // rebuild markers for remaining selection
    for ( const Vertex &vertex : std::as_const( mSelectedVertices ) )
    {
      createMarkerForVertex( vertex );
    }
  }

  if ( mLockedFeature )
  {
    disconnect( mLockedFeature.get(), &QgsLockedFeature::selectionChanged, this, &QgsVertexTool::lockedFeatureSelectionChanged );

    mLockedFeature->deselectAllVertices();
    for ( const Vertex &vertex : std::as_const( mSelectedVertices ) )
    {
      // we should never be able to select vertices that are not from the locked feature
      // cppcheck-suppress assertWithSideEffect
      Q_ASSERT( mLockedFeature->featureId() == vertex.fid && mLockedFeature->layer() == vertex.layer );
      mLockedFeature->selectVertex( vertex.vertexId );
    }

    connect( mLockedFeature.get(), &QgsLockedFeature::selectionChanged, this, &QgsVertexTool::lockedFeatureSelectionChanged );
  }
}

void QgsVertexTool::setHighlightedVerticesVisible( bool visible )
{
  for ( QgsVertexMarker *marker : std::as_const( mSelectedVerticesMarkers ) )
    marker->setVisible( visible );
}

void QgsVertexTool::highlightAdjacentVertex( double offset )
{
  if ( mSelectedVertices.isEmpty() )
    return;

  Vertex vertex = mSelectedVertices[0];  // simply use the first one

  QgsGeometry geom = cachedGeometryForVertex( vertex );

  // try to wrap around polygon rings
  int newVertexId, v0idx, v1idx;
  geom.adjacentVertices( vertex.vertexId, v0idx, v1idx );
  if ( offset == -1 && v0idx != -1 )
    newVertexId = v0idx;
  else if ( offset == 1 && v1idx != -1 )
    newVertexId = v1idx;
  else
    newVertexId = vertex.vertexId + offset;

  QgsPointXY pt = geom.vertexAt( newVertexId );
  if ( pt != QgsPointXY() )
    vertex = Vertex( vertex.layer, vertex.fid, newVertexId );
  setHighlightedVertices( QList<Vertex>() << vertex );
  zoomToVertex( vertex );  // make sure the vertex is visible
}

void QgsVertexTool::initSelectionRubberBand()
{
  if ( !mSelectionRubberBand )
  {
    mSelectionRubberBand = std::make_unique<QgsRubberBand>( mCanvas, QgsWkbTypes::PolygonGeometry );
    QColor fillColor = QColor( 0, 120, 215, 63 );
    QColor strokeColor = QColor( 0, 102, 204, 100 );
    mSelectionRubberBand->setFillColor( fillColor );
    mSelectionRubberBand->setStrokeColor( strokeColor );
  }
}

void QgsVertexTool::updateSelectionRubberBand( QgsMapMouseEvent *e )
{
  switch ( mSelectionMethod )
  {
    case SelectionNormal:
    {
      QRect rect = QRect( e->pos(), *mSelectionRubberBandStartPos );
      if ( mSelectionRubberBand )
        mSelectionRubberBand->setToCanvasRectangle( rect );
      break;
    }
    case SelectionPolygon:
    {
      mSelectionRubberBand->movePoint( toMapCoordinates( e->pos() ) );
      break;
    }
    default:
      break;
  }
}

void QgsVertexTool::stopSelectionRubberBand()
{
  mSelectionRubberBand.reset();
  mSelectionRubberBandStartPos.reset();
  mSelectionMethod = SelectionNormal;
}

bool QgsVertexTool::matchEdgeCenterTest( const QgsPointLocator::Match &m, const QgsPointXY &mapPoint, QgsPointXY *edgeCenterPtr )
{
  QgsPointXY p0, p1;
  m.edgePoints( p0, p1 );

  QgsGeometry geom = cachedGeometry( m.layer(), m.featureId() );
  if ( isCircularVertex( geom, m.vertexIndex() ) || isCircularVertex( geom, m.vertexIndex() + 1 ) )
    return false;  // currently not supported for circular edges

  QgsRectangle visible_extent = canvas()->mapSettings().visibleExtent();
  // Check if one point is inside and the other outside the visible extent in order to transpose the mid marker
  // If both are inside or outside there is no such need
  if ( visible_extent.contains( p0 ) != visible_extent.contains( p1 ) )
  {
    // clip line segment to the extent so the mid-point marker is always visible
    QgsGeometry extentGeom = QgsGeometry::fromRect( visible_extent );
    QgsGeometry lineGeom = QgsGeometry::fromPolylineXY( QgsPolylineXY() << p0 << p1 );
    lineGeom = extentGeom.intersection( lineGeom );
    QgsPolylineXY polyline = lineGeom.asPolyline();
    Q_ASSERT_X( polyline.count() == 2, "QgsVertexTool::matchEdgeCenterTest", QgsLineString( polyline ).asWkt().toUtf8().constData() );
    p0 = polyline[0];
    p1 = polyline[1];
  }

  QgsPointXY edgeCenter( ( p0.x() + p1.x() ) / 2, ( p0.y() + p1.y() ) / 2 );
  if ( edgeCenterPtr )
    *edgeCenterPtr = edgeCenter;

  double distFromEdgeCenter = std::sqrt( mapPoint.sqrDist( edgeCenter ) );
  double tol = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );
  bool isNearCenter = distFromEdgeCenter < tol;
  return isNearCenter;
}

void QgsVertexTool::CircularBand::updateRubberBand( const QgsPointXY &mapPoint )
{
  QgsPointSequence points;
  QgsPointXY v0 = moving0 ? mapPoint + offset0 : p0;
  QgsPointXY v1 = moving1 ? mapPoint + offset1 : p1;
  QgsPointXY v2 = moving2 ? mapPoint + offset2 : p2;
  QgsGeometryUtils::segmentizeArc( QgsPoint( v0 ), QgsPoint( v1 ), QgsPoint( v2 ), points );
  // it would be useful to have QgsRubberBand::setPoints() call
  band->reset();
  for ( const QgsPoint &p : std::as_const( points ) )
    band->addPoint( p );
}


void QgsVertexTool::validationErrorFound( const QgsGeometry::Error &e )
{
  QgsGeometryValidator *validator = qobject_cast<QgsGeometryValidator *>( sender() );
  if ( !validator )
    return;

  QHash< QPair<QgsVectorLayer *, QgsFeatureId>, GeometryValidation>::iterator it = mValidations.begin();
  for ( ; it != mValidations.end(); ++it )
  {
    GeometryValidation &validation = *it;
    if ( validation.validator == validator )
    {
      validation.addError( e );
      break;
    }
  }
}

void QgsVertexTool::validationFinished()
{
  QgsGeometryValidator *validator = qobject_cast<QgsGeometryValidator *>( sender() );
  if ( !validator )
    return;

  QHash< QPair<QgsVectorLayer *, QgsFeatureId>, GeometryValidation>::iterator it = mValidations.begin();
  for ( ; it != mValidations.end(); ++it )
  {
    GeometryValidation &validation = *it;
    if ( validation.validator == validator )
    {
      QgsStatusBar *sb = QgisApp::instance()->statusBarIface();
      sb->showMessage( tr( "Validation finished (%n error(s) found).", "number of geometry errors", validation.errorMarkers.size() ) );
      if ( validation.errorMarkers.isEmpty() )
      {
        // not needed anymore (no markers to keep displayed)
        validation.cleanup();
        mValidations.remove( it.key() );
      }
      break;
    }
  }
}

void QgsVertexTool::GeometryValidation::start( QgsGeometry &geom, QgsVertexTool *t, QgsVectorLayer *l )
{
  tool = t;
  layer = l;
  Qgis::GeometryValidationEngine method = Qgis::GeometryValidationEngine::QgisInternal;
  if ( QgsSettingsRegistryCore::settingsDigitizingValidateGeometries.value() == 2 )
    method = Qgis::GeometryValidationEngine::Geos;

  validator = new QgsGeometryValidator( geom, nullptr, method );
  connect( validator, &QgsGeometryValidator::errorFound, tool, &QgsVertexTool::validationErrorFound );
  connect( validator, &QThread::finished, tool, &QgsVertexTool::validationFinished );
  validator->start();
}

void QgsVertexTool::GeometryValidation::addError( QgsGeometry::Error e )
{
  if ( !errors.isEmpty() )
    errors += '\n';
  errors += e.what();

  if ( e.hasWhere() )
  {
    QgsVertexMarker *marker = new QgsVertexMarker( tool->canvas() );
    marker->setCenter( tool->canvas()->mapSettings().layerToMapCoordinates( layer, e.where() ) );
    marker->setIconType( QgsVertexMarker::ICON_X );
    marker->setColor( Qt::green );
    marker->setZValue( marker->zValue() + 1 );
    marker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
    marker->setPenWidth( QgsGuiUtils::scaleIconSize( 2 ) );
    marker->setToolTip( e.what() );
    errorMarkers << marker;
  }

  QgsStatusBar *sb = QgisApp::instance()->statusBarIface();
  sb->showMessage( e.what() );
  sb->setToolTip( errors );
}

void QgsVertexTool::GeometryValidation::cleanup()
{
  if ( validator )
  {
    validator->stop();
    validator->wait();
    validator->deleteLater();
    validator = nullptr;
  }

  qDeleteAll( errorMarkers );
  errorMarkers.clear();
}

void QgsVertexTool::validateGeometry( QgsVectorLayer *layer, QgsFeatureId featureId )
{
  if ( QgsSettingsRegistryCore::settingsDigitizingValidateGeometries.value() == 0 )
    return;

  QPair<QgsVectorLayer *, QgsFeatureId> id( layer, featureId );
  if ( mValidations.contains( id ) )
  {
    mValidations[id].cleanup();
    mValidations.remove( id );
  }

  GeometryValidation validation;
  QgsGeometry geom = cachedGeometry( layer, featureId );
  validation.start( geom, this, layer );
  mValidations.insert( id, validation );
}

void QgsVertexTool::zoomToVertex( const Vertex &vertex )
{
  QgsPointXY newCenter = cachedGeometryForVertex( vertex ).vertexAt( vertex.vertexId );
  QgsPointXY mapPoint = mCanvas->mapSettings().layerToMapCoordinates( vertex.layer, newCenter );
  QPolygonF ext = mCanvas->mapSettings().visiblePolygon();
  if ( !ext.containsPoint( mapPoint.toQPointF(), Qt::OddEvenFill ) )
  {
    mCanvas->setCenter( mapPoint );
    mCanvas->refresh();
  }
}

QList<Vertex> QgsVertexTool::verticesInRange( QgsVectorLayer *layer, QgsFeatureId fid, int vertexId0, int vertexId1, bool longWay )
{
  QgsGeometry geom = cachedGeometry( layer, fid );

  if ( vertexId0 > vertexId1 )
    std::swap( vertexId0, vertexId1 );

  // check it is the same part and ring
  QgsVertexId vid0, vid1;
  geom.vertexIdFromVertexNr( vertexId0, vid0 );
  geom.vertexIdFromVertexNr( vertexId1, vid1 );
  if ( vid0.part != vid1.part || vid0.ring != vid1.ring )
    return QList<Vertex>();

  // check whether we are in a linear ring
  int vertexIdTmp = vertexId0 - 1;
  QgsVertexId vidTmp;
  while ( geom.vertexIdFromVertexNr( vertexIdTmp, vidTmp ) &&
          vidTmp.part == vid0.part && vidTmp.ring == vid0.ring )
    --vertexIdTmp;
  int startVertexIndex = vertexIdTmp + 1;

  vertexIdTmp = vertexId1 + 1;
  while ( geom.vertexIdFromVertexNr( vertexIdTmp, vidTmp ) &&
          vidTmp.part == vid0.part && vidTmp.ring == vid0.ring )
    ++vertexIdTmp;
  int endVertexIndex = vertexIdTmp - 1;

  QList<Vertex> lst;

  if ( geom.vertexAt( startVertexIndex ) == geom.vertexAt( endVertexIndex ) )
  {
    // closed curve - we need to find out which way around the curve is shorter
    double lengthTotal = 0, length0to1 = 0;
    QgsPoint ptOld = geom.vertexAt( startVertexIndex );
    for ( int i = startVertexIndex + 1; i <= endVertexIndex; ++i )
    {
      QgsPoint pt( geom.vertexAt( i ) );
      double len = ptOld.distance( pt );
      lengthTotal += len;
      if ( i > vertexId0 && i <= vertexId1 )
        length0to1 += len;
      ptOld = pt;
    }

    bool use0to1 = length0to1 < lengthTotal / 2;
    if ( longWay )
      use0to1 = !use0to1;
    for ( int i = startVertexIndex; i <= endVertexIndex; ++i )
    {
      bool isPickedVertex = i == vertexId0 || i == vertexId1;
      bool is0to1 = i > vertexId0 && i < vertexId1;
      if ( isPickedVertex || is0to1 == use0to1 )
        lst.append( Vertex( layer, fid, i ) );
    }
  }
  else
  {
    // curve that is not closed
    for ( int i = vertexId0; i <= vertexId1; ++i )
    {
      lst.append( Vertex( layer, fid, i ) );
    }
  }
  return lst;
}

void QgsVertexTool::rangeMethodPressEvent( QgsMapMouseEvent *e )
{
  // nothing to do here for now...
  Q_UNUSED( e )
}

void QgsVertexTool::rangeMethodReleaseEvent( QgsMapMouseEvent *e )
{
  if ( e->button() == Qt::RightButton )
  {
    stopRangeVertexSelection();
    return;
  }
  else if ( e->button() == Qt::LeftButton )
  {
    if ( mRangeSelectionFirstVertex )
    {
      // pick final vertex, make selection and switch back to normal selection
      QgsPointLocator::Match m = snapToEditableLayer( e );
      if ( m.hasVertex() )
      {
        if ( m.layer() == mRangeSelectionFirstVertex->layer && m.featureId() == mRangeSelectionFirstVertex->fid )
        {
          QList<Vertex> lst = verticesInRange( m.layer(), m.featureId(), mRangeSelectionFirstVertex->vertexId, m.vertexIndex(), e->modifiers() & Qt::ControlModifier );
          setHighlightedVertices( lst );

          mSelectionMethod = SelectionNormal;
        }
      }
    }
    else
    {
      // pick first vertex
      QgsPointLocator::Match m = snapToEditableLayer( e );
      if ( m.hasVertex() )
      {
        mRangeSelectionFirstVertex.reset( new Vertex( m.layer(), m.featureId(), m.vertexIndex() ) );
        setHighlightedVertices( QList<Vertex>() << *mRangeSelectionFirstVertex );
      }
    }
  }
}

void QgsVertexTool::rangeMethodMoveEvent( QgsMapMouseEvent *e )
{
  if ( e->buttons() )
    return;  // only with no buttons pressed

  QgsPointLocator::Match m = snapToEditableLayer( e );

  updateFeatureBand( m );
  updateVertexBand( m );

  if ( !m.hasVertex() )
  {
    QList<Vertex> lst;
    if ( mRangeSelectionFirstVertex )
      lst << *mRangeSelectionFirstVertex;
    setHighlightedVertices( lst );
    return;
  }

  if ( mRangeSelectionFirstVertex )
  {
    // pick temporary final vertex and highlight vertices
    if ( m.layer() == mRangeSelectionFirstVertex->layer && m.featureId() == mRangeSelectionFirstVertex->fid )
    {
      QList<Vertex> lst = verticesInRange( m.layer(), m.featureId(), mRangeSelectionFirstVertex->vertexId, m.vertexIndex(), e->modifiers() & Qt::ControlModifier );
      setHighlightedVertices( lst );
    }
  }
}


void QgsVertexTool::startRangeVertexSelection()
{
  mSelectionMethod = SelectionRange;
  setHighlightedVertices( QList<Vertex>() );
  mRangeSelectionFirstVertex.reset();
}

void QgsVertexTool::stopRangeVertexSelection()
{
  mSelectionMethod = SelectionNormal;
  setHighlightedVertices( QList<Vertex>() );
}

void QgsVertexTool::cleanEditor( QgsFeatureId id )
{
  if ( mLockedFeature && mLockedFeature->featureId() == id )
  {
    cleanupVertexEditor();
  };
}

void QgsVertexTool::clean()
{
  if ( mDraggingVertex || mDraggingEdge )
  {
    stopDragging();
  }
  if ( mSelectionRubberBand )
  {
    stopSelectionRubberBand();
    mSelectionRubberBandStartPos.reset();
  }
}
