/***************************************************************************
  qgsnodetool2.cpp
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

#include "qgsnodetool2.h"

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgscurve.h"
#include "qgscurvepolygon.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmulticurve.h"
#include "qgspointlocator.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"
#include "qgsvertexmarker.h"

#include <QRubberBand>


//
// geomutils - may get moved elsewhere
//


//! Find out whether vertex at the given index is an endpoint (assuming linear geometry)
static bool isEndpointAtVertexIndex( const QgsGeometry &geom, int vertexIndex )
{
  QgsAbstractGeometry *g = geom.geometry();
  if ( QgsCurve *curve = dynamic_cast<QgsCurve *>( g ) )
  {
    return vertexIndex == 0 or vertexIndex == curve->numPoints() - 1;
  }
  else if ( QgsMultiCurve *multiCurve = dynamic_cast<QgsMultiCurve *>( g ) )
  {
    for ( int i = 0; i < multiCurve->numGeometries(); ++i )
    {
      QgsCurve *part = dynamic_cast<QgsCurve *>( multiCurve->geometryN( i ) );
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
    QgsDebugMsg( "is_endpoint_at_vertex_index: unexpected geometry type!" );
    return false;
  }
}


//! Return index of vertex adjacent to the given endpoint. Assuming linear geometries.
int adjacentVertexIndexToEndpoint( const QgsGeometry &geom, int vertexIndex )
{
  QgsAbstractGeometry *g = geom.geometry();
  if ( QgsCurve *curve = dynamic_cast<QgsCurve *>( g ) )
  {
    return vertexIndex == 0 ? 1 : curve->numPoints() - 2;
  }
  else if ( QgsMultiCurve *multiCurve = dynamic_cast<QgsMultiCurve *>( g ) )
  {
    int offset = 0;
    for ( int i = 0; i < multiCurve->numGeometries(); ++i )
    {
      QgsCurve *part = dynamic_cast<QgsCurve *>( multiCurve->geometryN( i ) );
      Q_ASSERT( part );
      if ( vertexIndex < part->numPoints() )
        return vertexIndex == 0 ? offset + 1 : offset + part->numPoints() - 2;
      vertexIndex -= part->numPoints();
      offset += part->numPoints();
    }
  }
  else
  {
    QgsDebugMsg( "adjacent_vertex_index_to_endpoint: unexpected geometry type!" );
  }
  return -1;
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

    virtual bool acceptMatch( const QgsPointLocator::Match &match ) override
    {
      return match.layer() == layer && match.featureId() == fid;
    }

  private:
    const QgsVectorLayer *layer;
    QgsFeatureId fid;
};


//! a filter just to gather all matches at the same place
class MatchCollectingFilter : public QgsPointLocator::MatchFilter
{
  public:
    QList<QgsPointLocator::Match> matches;
    QgsNodeTool2 *nodetool;

    MatchCollectingFilter( QgsNodeTool2 *nodetool )
      : nodetool( nodetool ) {}

    virtual bool acceptMatch( const QgsPointLocator::Match &match ) override
    {
      if ( match.distance() > 0 )
        return false;
      matches.append( match );

      // there may be multiple points at the same location, but we get only one
      // result... the locator API needs a new method verticesInRect()
      QgsGeometry matchGeom = nodetool->cachedGeometry( match.layer(), match.featureId() );
      QgsVertexId vid;
      QgsPointV2 pt;
      while ( matchGeom.geometry()->nextVertex( vid, pt ) )
      {
        int vindex = matchGeom.vertexNrFromVertexId( vid );
        if ( pt.x() == match.point().x() && pt.y() == match.point().y() && vindex != match.vertexIndex() )
        {
          QgsPointLocator::Match extra_match( match.type(), match.layer(), match.featureId(),
                                              0, match.point(), vindex );
          matches.append( extra_match );
        }
      }
      return true;
    }
};

//
//
//


QgsNodeTool2::QgsNodeTool2( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock )
  : QgsMapToolAdvancedDigitizing( canvas, cadDock )
{
  mSnapMarker = new QgsVertexMarker( canvas );
  mSnapMarker->setIconType( QgsVertexMarker::ICON_CROSS );
  mSnapMarker->setColor( Qt::magenta );
  mSnapMarker->setPenWidth( 3 );
  mSnapMarker->setVisible( false );

  mEdgeCenterMarker = new QgsVertexMarker( canvas );
  mEdgeCenterMarker->setIconType( QgsVertexMarker::ICON_CROSS );
  mEdgeCenterMarker->setColor( Qt::red );
  mEdgeCenterMarker->setPenWidth( 3 );
  mEdgeCenterMarker->setVisible( false );

  mDragPointMarker = new QgsVertexMarker( canvas );
  mDragPointMarker->setIconType( QgsVertexMarker::ICON_X );
  mDragPointMarker->setColor( Qt::red );
  mDragPointMarker->setPenWidth( 3 );
  mDragPointMarker->setVisible( false );

  mFeatureBand = createRubberBand( QgsWkbTypes::LineGeometry );
  mFeatureBand->setVisible( false );

  QColor color = digitizingStrokeColor();
  mVertexBand = new QgsRubberBand( canvas );
  mVertexBand->setIcon( QgsRubberBand::ICON_CIRCLE );
  mVertexBand->setColor( color );
  mVertexBand->setIconSize( 15 );
  mVertexBand->setVisible( false );

  QColor color2( color );
  color2.setAlpha( color2.alpha() / 3 );
  mEdgeBand = new QgsRubberBand( canvas );
  mEdgeBand->setColor( color2 );
  mEdgeBand->setWidth( 10 );
  mEdgeBand->setVisible( false );

  mEndpointMarker = new QgsVertexMarker( canvas );
  mEndpointMarker->setIconType( QgsVertexMarker::ICON_CROSS );
  mEndpointMarker->setColor( Qt::red );
  mEndpointMarker->setPenWidth( 3 );
  mEndpointMarker->setVisible( false );
}

QgsNodeTool2::~QgsNodeTool2()
{
  delete mSnapMarker;
  delete mEdgeCenterMarker;
  delete mDragPointMarker;
  delete mFeatureBand;
  delete mVertexBand;
  delete mEdgeBand;
  delete mEndpointMarker;
}

void QgsNodeTool2::deactivate()
{
  setHighlightedNodes( QList<Vertex>() );
  removeTemporaryRubberBands();
  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsNodeTool2::addDragBand( const QgsPoint &v1, const QgsPoint &v2 )
{
  QgsRubberBand *dragBand = createRubberBand( QgsWkbTypes::LineGeometry, true );
  dragBand->addPoint( v1 );
  dragBand->addPoint( v2 );
  mDragBands << dragBand;
}

void QgsNodeTool2::clearDragBands()
{
  qDeleteAll( mDragBands );
  mDragBands.clear();

  // for the case when standalone point geometry is being dragged
  mDragPointMarker->setVisible( false );
}

void QgsNodeTool2::cadCanvasPressEvent( QgsMapMouseEvent *e )
{
  setHighlightedNodes( QList<Vertex>() ); // reset selection

  if ( e->button() == Qt::LeftButton )
  {
    // Ctrl+Click to highlight nodes without entering editing mode
    if ( e->modifiers() & Qt::ControlModifier )
    {
      QgsPointLocator::Match m = snapToEditableLayer( e );
      if ( m.hasVertex() )
      {
        Vertex node( m.layer(), m.featureId(), m.vertexIndex() );
        setHighlightedNodes( QList<Vertex>() << node );
      }
      return;
    }

    // the user may have started dragging a rect to select vertices
    if ( !mDraggingVertex && !mDraggingEdge )
      mSelectionRectStartPos.reset( new QPoint( e->pos() ) );
  }
}

void QgsNodeTool2::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( mNewVertexFromDoubleClick )
  {
    QgsPointLocator::Match m( *mNewVertexFromDoubleClick );
    mNewVertexFromDoubleClick.reset();

    // dragging of edges and double clicking on edges to add vertex are slightly overlapping
    // so we need to cancel edge moving before we start dragging new vertex
    stopDragging();
    startDraggingAddVertex( m );
  }
  else if ( mSelectionRect )
  {
    // only handling of selection rect being dragged
    QgsPoint pt0 = toMapCoordinates( *mSelectionRectStartPos );
    QgsPoint pt1 = toMapCoordinates( e->pos() );
    QgsRectangle map_rect( pt0, pt1 );
    QList<Vertex> nodes;

    // for each editable layer, select nodes
    Q_FOREACH ( QgsMapLayer *layer, canvas()->layers() )
    {
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      if ( !vlayer || !vlayer->isEditable() )
        continue;

      QgsRectangle layerRect = toLayerCoordinates( vlayer, map_rect );
      QgsFeature f;
      QgsFeatureIterator fi = vlayer->getFeatures( QgsFeatureRequest( layerRect ).setSubsetOfAttributes( QgsAttributeList() ) );
      while ( fi.nextFeature( f ) )
      {
        QgsGeometry g = f.geometry();
        for ( int i = 0; i < g.geometry()->nCoordinates(); ++i )
        {
          QgsPoint pt = g.vertexAt( i );
          if ( layerRect.contains( pt ) )
            nodes << Vertex( vlayer, f.id(), i );
        }
      }
    }

    setHighlightedNodes( nodes );

    stopSelectionRect();
  }
  else  // selection rect is not being dragged
  {
    if ( e->button() == Qt::LeftButton )
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
      // cancel action
      stopDragging();
    }
  }

  mSelectionRectStartPos.reset();

  // there may be a temporary list of points (up to two) that need to be injected
  // into CAD dock widget in order to make it behave as we need
  if ( !mOverrideCadPoints.isEmpty() )
  {
    Q_FOREACH ( const QgsPoint &pt, mOverrideCadPoints )
    {
      QMouseEvent mouseEvent( QEvent::MouseButtonRelease,
                              toCanvasCoordinates( pt ),
                              Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
      QgsMapMouseEvent me( canvas(), &mouseEvent );
      cadDockWidget()->canvasReleaseEvent( &me, QgsAdvancedDigitizingDockWidget::TwoPoints );  // TODO: correct second flag?
    }

    mOverrideCadPoints.clear();
  }
}

void QgsNodeTool2::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mDraggingVertex )
  {
    mouseMoveDraggingVertex( e );
  }
  else if ( mDraggingEdge )
  {
    mouseMoveDraggingEdge( e );
  }
  else if ( mSelectionRectStartPos )
  {
    // the user may be dragging a rect to select vertices
    if ( !mSelectionRect && ( e->pos() - *mSelectionRectStartPos ).manhattanLength() >= 10 )
    {
      startSelectionRect( *mSelectionRectStartPos );
    }
    if ( mSelectionRect )
    {
      updateSelectionRect( e->pos() );
    }
  }
  else
  {
    mouseMoveNotDragging( e );
  }
}

void QgsNodeTool2::mouseMoveDraggingVertex( QgsMapMouseEvent *e )
{
  if ( e->mapPointMatch().isValid() )
  {
    mSnapMarker->setCenter( e->mapPoint() );
    mSnapMarker->setVisible( true );
  }
  else
  {
    mSnapMarker->setVisible( false );
  }

  mEdgeCenterMarker->setVisible( false );

  Q_FOREACH ( QgsRubberBand *band, mDragBands )
    band->movePoint( 1, e->mapPoint() );

  // in case of moving of standalone point geometry
  if ( mDragPointMarker->isVisible() )
  {
    mDragPointMarker->setCenter( e->mapPoint() );
  }

  // make sure the temporary feature rubber band is not visible
  removeTemporaryRubberBands();
}

void QgsNodeTool2::mouseMoveDraggingEdge( QgsMapMouseEvent *e )
{
  mSnapMarker->setVisible( false );
  mEdgeCenterMarker->setVisible( false );

  const QgsVectorLayer *dragLayer = mDraggingEdge->layer;
  QgsFeatureId dragFid = mDraggingEdge->fid;
  int dragVertex0 = mDraggingEdge->edgeVertex0;
  QgsPoint dragStartPoint = mDraggingEdge->startMapPoint;
  QgsPoint mapPoint = toMapCoordinates( e->pos() );  // do not use e.mapPoint() as it may be snapped

  double diffX = mapPoint.x() - dragStartPoint.x();
  double diffY = mapPoint.y() - dragStartPoint.y();

  QgsGeometry geom( cachedGeometry( dragLayer, dragFid ) );
  QgsPoint origMapPoint0 = toMapCoordinates( dragLayer, geom.vertexAt( dragVertex0 ) );
  QgsPoint origMapPoint1 = toMapCoordinates( dragLayer, geom.vertexAt( dragVertex0 + 1 ) );
  QgsPoint newMapPoint0 = QgsPoint( origMapPoint0.x() + diffX, origMapPoint0.y() + diffY );
  QgsPoint newMapPoint1 = QgsPoint( origMapPoint1.x() + diffX, origMapPoint1.y() + diffY );

  mDraggingEdge->band0to1->movePoint( 0, newMapPoint0 );
  mDraggingEdge->band0to1->movePoint( 1, newMapPoint1 );

  Q_FOREACH ( QgsRubberBand *band, mDraggingEdge->bandsTo0 )
    band->movePoint( 1, newMapPoint0 );

  Q_FOREACH ( QgsRubberBand *band, mDraggingEdge->bandsTo1 )
    band->movePoint( 1, newMapPoint1 );

  // make sure the temporary feature rubber band is not visible
  removeTemporaryRubberBands();
}

void QgsNodeTool2::canvasDoubleClickEvent( QgsMapMouseEvent *e )
{
  QgsPointLocator::Match m = snapToEditableLayer( e );
  if ( !m.hasEdge() )
    return;

  mNewVertexFromDoubleClick.reset( new QgsPointLocator::Match( m ) );
}

void QgsNodeTool2::removeTemporaryRubberBands()
{
  mFeatureBand->setVisible( false );
  mFeatureBandLayer = nullptr;
  mFeatureBandFid = QgsFeatureId();
  mVertexBand->setVisible( false );
  mEdgeBand->setVisible( false );
  mEndpointMarkerCenter.reset();
  mEndpointMarker->setVisible( false );
}

QgsPointLocator::Match QgsNodeTool2::snapToEditableLayer( QgsMapMouseEvent *e )
{
  QgsPoint mapPoint = toMapCoordinates( e->pos() );
  double tol = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );

  QgsSnappingConfig config( QgsProject::instance() );
  config.setEnabled( true );
  config.setMode( QgsSnappingConfig::AdvancedConfiguration );
  config.setIntersectionSnapping( false );  // only snap to layers

  Q_FOREACH ( QgsMapLayer *layer, canvas()->layers() )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( !vlayer || !vlayer->isEditable() )
      continue;

    config.setIndividualLayerSettings( vlayer, QgsSnappingConfig::IndividualLayerSettings(
                                         true, QgsSnappingConfig::VertexAndSegment, tol, QgsTolerance::ProjectUnits ) );
  }

  QgsSnappingUtils *snapUtils = canvas()->snappingUtils();
  QgsSnappingConfig oldConfig = snapUtils->config();
  snapUtils->setConfig( config );

  QgsPointLocator::Match m = snapUtils->snapToMap( mapPoint );

  // try to stay snapped to previously used feature
  // so the highlight does not jump around at nodes where features are joined
  if ( mLastSnap )
  {
    OneFeatureFilter filterLast( mLastSnap->layer(), mLastSnap->featureId() );
    QgsPointLocator::Match lastMatch = snapUtils->snapToMap( mapPoint, &filterLast );
    if ( lastMatch.isValid() && lastMatch.distance() <= m.distance() )
      m = lastMatch;
  }

  snapUtils->setConfig( oldConfig );

  mLastSnap.reset( new QgsPointLocator::Match( m ) );

  return m;
}

bool QgsNodeTool2::isNearEndpointMarker( const QgsPoint &mapPoint )
{
  if ( !mEndpointMarkerCenter )
    return false;

  double distMarker = sqrt( mEndpointMarkerCenter->sqrDist( mapPoint ) );
  double tol = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );

  QgsGeometry geom = cachedGeometryForVertex( *mMouseAtEndpoint );
  QgsPoint vertexPointV2 = geom.vertexAt( mMouseAtEndpoint->vertexId );
  QgsPoint vertexPoint = QgsPoint( vertexPointV2.x(), vertexPointV2.y() );
  double distVertex = sqrt( vertexPoint.sqrDist( mapPoint ) );

  return distMarker < tol and distMarker < distVertex;
}

bool QgsNodeTool2::isMatchAtEndpoint( const QgsPointLocator::Match &match )
{
  QgsGeometry geom = cachedGeometry( match.layer(), match.featureId() );

  if ( geom.type() != QgsWkbTypes::LineGeometry )
    return false;

  return isEndpointAtVertexIndex( geom, match.vertexIndex() );
}

QgsPoint QgsNodeTool2::positionForEndpointMarker( const QgsPointLocator::Match &match )
{
  QgsGeometry geom = cachedGeometry( match.layer(), match.featureId() );

  QgsPoint pt0 = geom.vertexAt( adjacentVertexIndexToEndpoint( geom, match.vertexIndex() ) );
  QgsPoint pt1 = geom.vertexAt( match.vertexIndex() );
  double dx = pt1.x() - pt0.x();
  double dy = pt1.y() - pt0.y();
  double dist = 15 * canvas()->mapSettings().mapUnitsPerPixel();
  double angle = atan2( dy, dx );  // to the top: angle=0, to the right: angle=90, to the left: angle=-90
  double x = pt1.x() + cos( angle ) * dist;
  double y = pt1.y() + sin( angle ) * dist;
  return QgsPoint( x, y );
}

void QgsNodeTool2::mouseMoveNotDragging( QgsMapMouseEvent *e )
{
  if ( mMouseAtEndpoint )
  {
    // check if we are still at the endpoint, i.e. whether to keep showing
    // the endpoint indicator - or go back to snapping to editable layers
    QgsPoint mapPoint = toMapCoordinates( e->pos() );
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

  // possibility to move a node
  if ( m.type() == QgsPointLocator::Vertex )
  {
    mVertexBand->setToGeometry( QgsGeometry::fromPoint( m.point() ), nullptr );
    mVertexBand->setVisible( true );
    bool isCircular = false;
    if ( m.layer() )
    {
      QgsGeometry geom = cachedGeometry( m.layer(), m.featureId() );
      QgsVertexId v_id;
      if ( geom.vertexIdFromVertexNr( m.vertexIndex(), v_id ) )
        isCircular = ( v_id.type == QgsVertexId::CurveVertex );
    }

    mVertexBand->setIcon( isCircular ? QgsRubberBand::ICON_FULL_BOX : QgsRubberBand::ICON_CIRCLE );
    // if we are at an endpoint, let's show also the endpoint indicator
    // so user can possibly add a new vertex at the end
    if ( isMatchAtEndpoint( m ) )
    {
      mMouseAtEndpoint.reset( new Vertex( m.layer(), m.featureId(), m.vertexIndex() ) );
      mEndpointMarkerCenter.reset( new QgsPoint( positionForEndpointMarker( m ) ) );
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

  // possibility to create new node here - or to move the edge
  if ( m.type() == QgsPointLocator::Edge )
  {
    QgsPoint mapPoint = toMapCoordinates( e->pos() );
    QgsPoint edgeCenter;
    bool isNearCenter = matchEdgeCenterTest( m, mapPoint, &edgeCenter );
    mEdgeCenterMarker->setCenter( edgeCenter );
    mEdgeCenterMarker->setColor( isNearCenter ? Qt::red : Qt::gray );
    mEdgeCenterMarker->setVisible( true );
    mEdgeCenterMarker->update();

    QgsPoint p0, p1;
    m.edgePoints( p0, p1 );
    QgsPolyline points;
    points << p0 << p1;
    mEdgeBand->setToGeometry( QgsGeometry::fromPolyline( points ), nullptr );
    mEdgeBand->setVisible( !isNearCenter );
  }
  else
  {
    mEdgeCenterMarker->setVisible( false );
    mEdgeBand->setVisible( false );
  }

  // highlight feature
  if ( m.isValid() && m.layer() )
  {
    if ( mFeatureBandLayer == m.layer() && mFeatureBandFid == m.featureId() )
      return;  // skip regeneration of rubber band if not needed
    QgsGeometry geom = cachedGeometry( m.layer(), m.featureId() );
    if ( QgsWkbTypes::isCurvedType( geom.geometry()->wkbType() ) )
      geom = QgsGeometry( geom.geometry()->segmentize() );
    mFeatureBand->setToGeometry( geom, m.layer() );
    mFeatureBand->setVisible( true );
    mFeatureBandLayer = m.layer();
    mFeatureBandFid = m.featureId();
  }
  else
  {
    mFeatureBand->setVisible( false );
    mFeatureBandLayer = nullptr;
    mFeatureBandFid = QgsFeatureId();
  }
}

void QgsNodeTool2::keyPressEvent( QKeyEvent *e )
{
  if ( !mDraggingVertex && mSelectedNodes.count() == 0 )
    return;

  if ( e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace )
  {
    e->ignore();  // Override default shortcut management
    deleteVertex();
  }
  else if ( e->key() == Qt::Key_Escape )
  {
    if ( mDraggingVertex )
      stopDragging();
  }
  else if ( e->key() == Qt::Key_Comma )
  {
    highlightAdjacentVertex( -1 );
  }
  else if ( e->key() == Qt::Key_Period )
  {
    highlightAdjacentVertex( + 1 );
  }
}

QgsGeometry QgsNodeTool2::cachedGeometry( const QgsVectorLayer *layer, QgsFeatureId fid )
{
  if ( !mCache.contains( layer ) )
  {
    connect( layer, &QgsVectorLayer::geometryChanged, this, &QgsNodeTool2::onCachedGeometryChanged );
    connect( layer, &QgsVectorLayer::featureDeleted, this, &QgsNodeTool2::onCachedGeometryDeleted );
    // TODO: also clear cache when layer is deleted
  }

  QHash<QgsFeatureId, QgsGeometry> &layerCache = mCache[layer];
  if ( !layerCache.contains( fid ) )
  {
    QgsFeature f;
    layer->getFeatures( QgsFeatureRequest( fid ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( f );
    layerCache[fid] = f.geometry();
  }

  return layerCache[fid];
}

QgsGeometry QgsNodeTool2::cachedGeometryForVertex( const Vertex &vertex )
{
  return cachedGeometry( vertex.layer, vertex.fid );
}

void QgsNodeTool2::onCachedGeometryChanged( QgsFeatureId fid, const QgsGeometry &geom )
{
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( sender() );
  Q_ASSERT( mCache.contains( layer ) );
  QHash<QgsFeatureId, QgsGeometry> &layerCache = mCache[layer];
  if ( layerCache.contains( fid ) )
    layerCache[fid] = geom;
}

void QgsNodeTool2::onCachedGeometryDeleted( QgsFeatureId fid )
{
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( sender() );
  Q_ASSERT( mCache.contains( layer ) );
  QHash<QgsFeatureId, QgsGeometry> &layerCache = mCache[layer];
  if ( layerCache.contains( fid ) )
    layerCache.remove( fid );
}

void QgsNodeTool2::startDragging( QgsMapMouseEvent *e )
{
  QgsPoint mapPoint = toMapCoordinates( e->pos() );
  if ( isNearEndpointMarker( mapPoint ) )
  {
    startDraggingAddVertexAtEndpoint( mapPoint );
    return;
  }

  QgsPointLocator::Match m = snapToEditableLayer( e );
  if ( !m.isValid() )
    return;

  // activate advanced digitizing dock
  setMode( CaptureLine );

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
    startDraggingMoveVertex( e->mapPoint(), m );
  }
}

void QgsNodeTool2::startDraggingMoveVertex( const QgsPoint &mapPoint, const QgsPointLocator::Match &m )
{
  Q_ASSERT( m.hasVertex() );

  QgsGeometry geom = cachedGeometry( m.layer(), m.featureId() );

  // start dragging of snapped point of current layer
  mDraggingVertex.reset( new Vertex( m.layer(), m.featureId(), m.vertexIndex() ) );
  mDraggingVertexType = MovingVertex;
  mDraggingTopo.clear();

  int v0idx, v1idx;
  geom.adjacentVertices( m.vertexIndex(), v0idx, v1idx );
  if ( v0idx != -1 )
  {
    QgsPoint layerPoint0 = geom.vertexAt( v0idx );
    QgsPoint mapPoint0 = toMapCoordinates( m.layer(), layerPoint0 );
    addDragBand( mapPoint0, m.point() );
  }
  if ( v1idx != -1 )
  {
    QgsPoint layerPoint1 = geom.vertexAt( v1idx );
    QgsPoint mapPoint1 = toMapCoordinates( m.layer(), layerPoint1 );
    addDragBand( mapPoint1, m.point() );
  }

  if ( v0idx == -1 && v1idx == -1 )
  {
    // this is a standalone point - we need to use a marker for it
    // to give some feedback to the user
    mDragPointMarker->setCenter( mapPoint );
    mDragPointMarker->setVisible( true );
  }

  mOverrideCadPoints.clear();
  mOverrideCadPoints << m.point() << m.point();

  if ( !QgsProject::instance()->topologicalEditing() )
    return;   // we are done now

  // support for topo editing - find extra features
  Q_FOREACH ( QgsMapLayer *layer, canvas()->layers() )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
    if ( !vlayer || !vlayer->isEditable() )
      continue;

    Q_FOREACH ( const QgsPointLocator::Match &otherMatch, layerVerticesSnappedToPoint( vlayer, mapPoint ) )
    {
      if ( otherMatch == m )
        continue;

      QgsGeometry otherGeom = cachedGeometry( otherMatch.layer(), otherMatch.featureId() );

      // start dragging of snapped point of current layer
      mDraggingTopo << Vertex( otherMatch.layer(), otherMatch.featureId(), otherMatch.vertexIndex() );

      otherGeom.adjacentVertices( otherMatch.vertexIndex(), v0idx, v1idx );
      if ( v0idx != -1 )
      {
        QgsPoint otherPoint0 = otherGeom.vertexAt( v0idx );
        QgsPoint otherMapPoint0 = toMapCoordinates( otherMatch.layer(), otherPoint0 );
        addDragBand( otherMapPoint0, otherMatch.point() );
      }
      if ( v1idx != -1 )
      {
        QgsPoint otherPoint1 = otherGeom.vertexAt( v1idx );
        QgsPoint otherMapPoint1 = toMapCoordinates( otherMatch.layer(), otherPoint1 );
        addDragBand( otherMapPoint1, otherMatch.point() );
      }
    }
  }
}

QList<QgsPointLocator::Match> QgsNodeTool2::layerVerticesSnappedToPoint( QgsVectorLayer *layer, const QgsPoint &mapPoint )
{
  MatchCollectingFilter myfilter( this );
  QgsPointLocator *loc = canvas()->snappingUtils()->locatorForLayer( layer );
  loc->nearestVertex( mapPoint, 0, &myfilter );
  return myfilter.matches;
}

void QgsNodeTool2::startDraggingAddVertex( const QgsPointLocator::Match &m )
{
  Q_ASSERT( m.hasEdge() );

  // activate advanced digitizing dock
  setMode( CaptureLine );

  mDraggingVertex.reset( new Vertex( m.layer(), m.featureId(), m.vertexIndex() + 1 ) );
  mDraggingVertexType = AddingVertex;
  mDraggingTopo.clear();

  QgsGeometry geom = cachedGeometry( m.layer(), m.featureId() );

  // TODO: handles rings correctly?
  QgsPoint v0 = geom.vertexAt( m.vertexIndex() );
  QgsPoint v1 = geom.vertexAt( m.vertexIndex() + 1 );

  QgsPoint map_v0 = toMapCoordinates( m.layer(), v0 );
  QgsPoint map_v1 = toMapCoordinates( m.layer(), v1 );

  if ( v0.x() != 0 || v0.y() != 0 )
    addDragBand( map_v0, m.point() );
  if ( v1.x() != 0 || v1.y() != 0 )
    addDragBand( map_v1, m.point() );

  mOverrideCadPoints.clear();
  mOverrideCadPoints << m.point() << m.point();
}

void QgsNodeTool2::startDraggingAddVertexAtEndpoint( const QgsPoint &mapPoint )
{
  Q_ASSERT( mMouseAtEndpoint );

  // activate advanced digitizing dock
  setMode( CaptureLine );

  mDraggingVertex.reset( new Vertex( mMouseAtEndpoint->layer, mMouseAtEndpoint->fid, mMouseAtEndpoint->vertexId ) );
  mDraggingVertexType = AddingEndpoint;
  mDraggingTopo.clear();

  QgsGeometry geom = cachedGeometry( mMouseAtEndpoint->layer, mMouseAtEndpoint->fid );
  QgsPoint v0 = geom.vertexAt( mMouseAtEndpoint->vertexId );
  QgsPoint map_v0 = toMapCoordinates( mMouseAtEndpoint->layer, v0 );

  addDragBand( map_v0, mapPoint );

  // setup CAD dock previous points to endpoint and the previous point
  QgsPoint pt0 = geom.vertexAt( adjacentVertexIndexToEndpoint( geom, mMouseAtEndpoint->vertexId ) );
  QgsPoint pt1 = geom.vertexAt( mMouseAtEndpoint->vertexId );
  mOverrideCadPoints.clear();
  mOverrideCadPoints << pt0 << pt1;
}

void QgsNodeTool2::startDraggingEdge( const QgsPointLocator::Match &m, const QgsPoint &mapPoint )
{
  Q_ASSERT( m.hasEdge() );

  // activate advanced digitizing
  setMode( CaptureLine );

  mDraggingEdge.reset( new Edge( m.layer(), m.featureId(), m.vertexIndex(), mapPoint ) );
  mDraggingTopo.clear();

  QgsPoint edge_p0, edge_p1;
  m.edgePoints( edge_p0, edge_p1 );
  QgsGeometry geom = cachedGeometry( m.layer(), m.featureId() );

  // add drag bands
  addDragBand( edge_p0, edge_p1 );
  int v0idx, v1idx, vidxUnused;
  geom.adjacentVertices( m.vertexIndex(), v0idx, vidxUnused );
  geom.adjacentVertices( m.vertexIndex() + 1, vidxUnused, v1idx );
  if ( v0idx != -1 )
  {
    QgsPoint layerPoint0 = geom.vertexAt( v0idx );
    QgsPoint mapPoint0 = toMapCoordinates( m.layer(), layerPoint0 );
    addDragBand( mapPoint0, edge_p0 );
    mDraggingEdge->bandsTo0 << mDragBands.last();
  }
  if ( v1idx != -1 )
  {
    QgsPoint layerPoint1 = geom.vertexAt( v1idx );
    QgsPoint mapPoint1 = toMapCoordinates( m.layer(), layerPoint1 );
    addDragBand( mapPoint1, edge_p1 );
    mDraggingEdge->bandsTo1 << mDragBands.last();
  }

  mDraggingEdge->band0to1 = mDragBands.first();

  mOverrideCadPoints.clear();
  mOverrideCadPoints << m.point() << m.point();

  // TODO: add topo points (?)
}

void QgsNodeTool2::stopDragging()
{
  // deactivate advanced digitizing
  setMode( CaptureNone );

  // stop adv digitizing
  QMouseEvent mouseEvent( QEvent::MouseButtonRelease,
                          QPoint(),
                          Qt::RightButton, Qt::RightButton, Qt::NoModifier );
  QgsMapMouseEvent me( canvas(), &mouseEvent );
  cadDockWidget()->canvasReleaseEvent( &me, QgsAdvancedDigitizingDockWidget::SinglePoint ); // TODO: correct second arg?

  mDraggingVertex.reset();
  mDraggingVertexType = NotDragging;
  mDraggingEdge.reset();
  clearDragBands();
}

QgsPoint QgsNodeTool2::matchToLayerPoint( const QgsVectorLayer *destLayer, const QgsPoint &mapPoint, const QgsPointLocator::Match *match )
{
  // try to use point coordinates in the original CRS if it is the same
  if ( match && match->hasVertex() && match->layer() && match->layer()->crs() == destLayer->crs() )
  {
    QgsFeature f;
    QgsFeatureIterator fi = match->layer()->getFeatures( QgsFeatureRequest( match->featureId() ).setSubsetOfAttributes( QgsAttributeList() ) );
    if ( fi.nextFeature( f ) )
      return f.geometry().vertexAt( match->vertexIndex() );
  }

  // fall back to reprojection of the map point to layer point if they are not the same CRS
  return toLayerCoordinates( destLayer, mapPoint );
}

void QgsNodeTool2::moveEdge( const QgsPoint &mapPoint )
{
  QgsVectorLayer *dragLayer = mDraggingEdge->layer;
  QgsFeatureId dragFid = mDraggingEdge->fid;
  int dragVertex0 = mDraggingEdge->edgeVertex0;
  QgsPoint dragStartPoint = mDraggingEdge->startMapPoint;

  stopDragging();

  double diffX = mapPoint.x() - dragStartPoint.x();
  double diffY = mapPoint.y() - dragStartPoint.y();

  QgsGeometry geom = cachedGeometry( dragLayer, dragFid );

  // TODO: move topo points (?)

  dragLayer->beginEditCommand( tr( "Moved edge" ) );

  // move first endpoint
  QgsPoint origMapPoint0 = toMapCoordinates( dragLayer, geom.vertexAt( dragVertex0 ) );
  QgsPoint newMapPoint0 = QgsPoint( origMapPoint0.x() + diffX, origMapPoint0.y() + diffY );
  mDraggingVertex.reset( new Vertex( dragLayer, dragFid, dragVertex0 ) );
  mDraggingVertexType = MovingVertex;
  moveVertex( newMapPoint0, nullptr );

  // move second endpoint
  QgsPoint origMapPoint1 = toMapCoordinates( dragLayer, geom.vertexAt( dragVertex0 + 1 ) );
  QgsPoint newMapPoint1 = QgsPoint( origMapPoint1.x() + diffX, origMapPoint1.y() + diffY );
  mDraggingVertex.reset( new Vertex( dragLayer, dragFid, dragVertex0 + 1 ) );
  mDraggingVertexType = MovingVertex;
  moveVertex( newMapPoint1, nullptr );

  dragLayer->endEditCommand();
}

void QgsNodeTool2::moveVertex( const QgsPoint &mapPoint, const QgsPointLocator::Match *mapPointMatch )
{
  // deactivate advanced digitizing
  setMode( CaptureNone );

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
    QgsDebugMsg( "invalid vertex index" );
    return;
  }

  QgsAbstractGeometry *geomTmp = geom.geometry()->clone();

  // add/move vertex
  if ( addingVertex )
  {
    if ( addingAtEndpoint && vid.vertex != 0 )  // appending?
      vid.vertex++;

    if ( !geomTmp->insertVertex( vid, QgsPointV2( layerPoint ) ) )
    {
      QgsDebugMsg( "append vertex failed!" );
      return;
    }
  }
  else
  {
    if ( !geomTmp->moveVertex( vid, QgsPointV2( layerPoint ) ) )
    {
      QgsDebugMsg( "move vertex failed!" );
      return;
    }
  }

  geom.setGeometry( geomTmp );

  QHash<QgsVectorLayer *, QHash<QgsFeatureId, QgsGeometry> > edits; // dict { layer : { fid : geom } }
  edits[dragLayer][dragFid] = geom;

  // add moved vertices from other layers
  Q_FOREACH ( const Vertex &topo, mDraggingTopo )
  {
    QHash<QgsFeatureId, QgsGeometry> &layerEdits = edits[topo.layer];
    QgsGeometry topoGeom;
    if ( layerEdits.contains( topo.fid ) )
      topoGeom = QgsGeometry( edits[topo.layer][topo.fid] );
    else
      topoGeom = QgsGeometry( cachedGeometryForVertex( topo ) );

    QgsPoint point;
    if ( topo.layer->crs() == dragLayer->crs() )
      point = layerPoint;
    else
      point = toLayerCoordinates( topo.layer, mapPoint );

    if ( !topoGeom.moveVertex( point.x(), point.y(), topo.vertexId ) )
    {
      QgsDebugMsg( "[topo] move vertex failed!" );
      continue;
    }
    edits[topo.layer][topo.fid] = topoGeom;
  }

  // TODO: topo editing: add points when adding a vertex on a common edge

  // TODO: add topological points: when moving vertex - if snapped to something

  // do the changes to layers
  QHash<QgsVectorLayer *, QHash<QgsFeatureId, QgsGeometry> >::iterator it = edits.begin();
  for ( ; it != edits.end(); ++it )
  {
    QgsVectorLayer *layer = it.key();
    QHash<QgsFeatureId, QgsGeometry> &layerEdits = it.value();
    layer->beginEditCommand( tr( "Moved vertex" ) );
    QHash<QgsFeatureId, QgsGeometry>::iterator it2 = layerEdits.begin();
    for ( ; it2 != layerEdits.end(); ++it2 )
      layer->changeGeometry( it2.key(), it2.value() );
    layer->endEditCommand();
    layer->triggerRepaint();
  }
}

void QgsNodeTool2::deleteVertex()
{
  QList<Vertex> toDelete;
  if ( !mSelectedNodes.isEmpty() )
  {
    toDelete = mSelectedNodes;
  }
  else
  {
    bool addingVertex = mDraggingVertexType == AddingVertex || mDraggingVertexType == AddingEndpoint;
    toDelete << *mDraggingVertex;
    toDelete += mDraggingTopo;
    stopDragging();

    if ( addingVertex )
      return;   // just cancel the vertex
  }

  setHighlightedNodes( QList<Vertex>() ); // reset selection

  // switch from a plain list to dictionary { layer: { fid: [vertexNr1, vertexNr2, ...] } }
  QHash<QgsVectorLayer *, QHash<QgsFeatureId, QList<int> > > toDeleteGrouped;
  Q_FOREACH ( const Vertex &vertex, toDelete )
  {
    toDeleteGrouped[vertex.layer][vertex.fid].append( vertex.vertexId );
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

      bool res = QgsVectorLayer::Success;
      qSort( vertexIds.begin(), vertexIds.end(), qGreater<int>() );
      Q_FOREACH ( int vertexId, vertexIds )
      {
        if ( res != QgsVectorLayer::EmptyGeometry )
          res = layer->deleteVertex( fid, vertexId );
        if ( res != QgsVectorLayer::EmptyGeometry && res != QgsVectorLayer::Success )
        {
          QgsDebugMsg( QString( "failed to delete vertex %1 %2 %3!" ).arg( layer->name() ).arg( fid ).arg( vertexId ) );
          success = false;
        }
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

  // pre-select next node for deletion if we are deleting just one node
  if ( toDelete.count() == 1 )
  {
    const Vertex &vertex = toDelete[0];
    QgsGeometry geom( cachedGeometryForVertex( vertex ) );
    int vertexId = vertex.vertexId;

    // if next vertex is not available, use the previous one
    if ( geom.vertexAt( vertexId ) == QgsPoint() )
      vertexId -= 1;

    if ( geom.vertexAt( vertexId ) != QgsPoint() )
    {
      QList<Vertex> nodes_new;
      nodes_new << Vertex( vertex.layer, vertex.fid, vertexId );
      setHighlightedNodes( nodes_new );
    }
  }

}

void QgsNodeTool2::setHighlightedNodes( const QList<Vertex> &listNodes )
{
  qDeleteAll( mSelectedNodesMarkers );
  mSelectedNodesMarkers.clear();

  Q_FOREACH ( const Vertex &node, listNodes )
  {
    QgsGeometry geom = cachedGeometryForVertex( node );
    QgsVertexMarker *marker = new QgsVertexMarker( canvas() );
    marker->setIconType( QgsVertexMarker::ICON_CIRCLE );
    marker->setPenWidth( 3 );
    marker->setColor( Qt::blue );
    marker->setCenter( geom.vertexAt( node.vertexId ) );
    mSelectedNodesMarkers.append( marker );
  }
  mSelectedNodes = listNodes;
}

void QgsNodeTool2::highlightAdjacentVertex( double offset )
{
  if ( mSelectedNodes.isEmpty() )
    return;

  Vertex node = mSelectedNodes[0];  // simply use the first one

  QgsGeometry geom = cachedGeometryForVertex( node );
  QgsPoint pt = geom.vertexAt( node.vertexId + offset );
  if ( pt != QgsPoint() )
    node = Vertex( node.layer, node.fid, node.vertexId + offset );
  setHighlightedNodes( QList<Vertex>() << node );
}

void QgsNodeTool2::startSelectionRect( const QPoint &point0 )
{
  Q_ASSERT( !mSelectionRect );
  mSelectionRect.reset( new QRect() );
  mSelectionRect->setTopLeft( point0 );
  mSelectionRectItem = new QRubberBand( QRubberBand::Rectangle, canvas() );
}

void QgsNodeTool2::updateSelectionRect( const QPoint &point1 )
{
  Q_ASSERT( mSelectionRect );
  mSelectionRect->setBottomRight( point1 );
  mSelectionRectItem->setGeometry( mSelectionRect->normalized() );
  mSelectionRectItem->show();
}

void QgsNodeTool2::stopSelectionRect()
{
  Q_ASSERT( mSelectionRect );
  mSelectionRectItem->deleteLater();
  mSelectionRectItem = nullptr;
  mSelectionRect.reset();
}

bool QgsNodeTool2::matchEdgeCenterTest( const QgsPointLocator::Match &m, const QgsPoint &mapPoint, QgsPoint *edgeCenterPtr )
{
  QgsPoint p0, p1;
  m.edgePoints( p0, p1 );

  QgsRectangle visible_extent = canvas()->mapSettings().visibleExtent();
  if ( !visible_extent.contains( p0 ) || !visible_extent.contains( p1 ) )
  {
    // clip line segment to the extent so the mid-point marker is always visible
    QgsGeometry extentGeom = QgsGeometry::fromRect( visible_extent );
    QgsGeometry lineGeom = QgsGeometry::fromPolyline( QgsPolyline() << p0 << p1 );
    lineGeom = extentGeom.intersection( lineGeom );
    QgsPolyline polyline = lineGeom.asPolyline();
    Q_ASSERT( polyline.count() == 2 );
    p0 = polyline[0];
    p1 = polyline[1];
  }

  QgsPoint edgeCenter( ( p0.x() + p1.x() ) / 2, ( p0.y() + p1.y() ) / 2 );
  if ( edgeCenterPtr )
    *edgeCenterPtr = edgeCenter;

  double distFromEdgeCenter = sqrt( mapPoint.sqrDist( edgeCenter ) );
  double tol = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );
  bool isNearCenter = distFromEdgeCenter < tol;
  return isNearCenter;
}
