/***************************************************************************
  qgsnodetool2.h
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

#ifndef QGSNODETOOL2_H
#define QGSNODETOOL2_H

#include <memory>

#include "qgis_app.h"
#include "qgsmaptooladvanceddigitizing.h"

class QRubberBand;

class QgsVertexMarker;


//! helper structure for a vertex being dragged
struct Vertex
{
  Vertex( QgsVectorLayer *layer, QgsFeatureId fid, int vertexId )
    : layer( layer )
    , fid( fid )
    , vertexId( vertexId ) {}

  bool operator==( const Vertex &other ) const
  {
    return layer == other.layer && fid == other.fid && vertexId == other.vertexId;
  }
  bool operator!=( const Vertex &other ) const
  {
    return !operator==( other );
  }

  QgsVectorLayer *layer;
  QgsFeatureId fid;
  int vertexId;
};

//! qHash implementation - we use Vertex in QSet
uint qHash( const Vertex &v );

//! helper structure for an edge being dragged
struct Edge
{
  Edge( QgsVectorLayer *layer, QgsFeatureId fid, int vertexId, const QgsPoint &startMapPoint )
    : layer( layer )
    , fid( fid )
    , edgeVertex0( vertexId )
    , startMapPoint( startMapPoint ) {}

  QgsVectorLayer *layer = nullptr;
  QgsFeatureId fid;
  int edgeVertex0;    //!< First vertex (with lower index)
  QgsPoint startMapPoint;  //!< Map point where edge drag started

  //! rubber band between the edge's endpoints (owned by mDragBands, not this instance)
  QgsRubberBand *band0to1 = nullptr;
  //! rubber bands from other edges to our edge's first endpoint (owned by mDragBands, not this instance)
  QList<QgsRubberBand *> bandsTo0;
  //! rubber bands from other edges to our edge's second endpoint (owned by mDragBands, not this instance)
  QList<QgsRubberBand *> bandsTo1;
};



class APP_EXPORT QgsNodeTool2 : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT
  public:
    QgsNodeTool2( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock );

    //! Cleanup canvas items we have created
    ~QgsNodeTool2();

    virtual void cadCanvasPressEvent( QgsMapMouseEvent *e ) override;

    virtual void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;

    virtual void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;

    //! Start addition of a new vertex on double-click
    virtual void canvasDoubleClickEvent( QgsMapMouseEvent *e ) override;

    virtual void deactivate() override;

    void keyPressEvent( QKeyEvent *e ) override;

    QgsGeometry cachedGeometry( const QgsVectorLayer *layer, QgsFeatureId fid );

  private slots:
    //! update geometry of our feature
    void onCachedGeometryChanged( QgsFeatureId fid, const QgsGeometry &geom );

    void onCachedGeometryDeleted( QgsFeatureId fid );


  private:

    void addDragBand( const QgsPoint &v1, const QgsPoint &v2, const QgsVector &offset = QgsVector() );

    void addDragMiddleBand( const QgsPoint &v1, const QgsPoint &v2, const QgsVector &offset1, const QgsVector &offset2 );

    void clearDragBands();

    void mouseMoveDraggingVertex( QgsMapMouseEvent *e );

    void mouseMoveDraggingEdge( QgsMapMouseEvent *e );

    void removeTemporaryRubberBands();

    /** Temporarily override snapping config and snap to vertices and edges
     of any editable vector layer, to allow selection of node for editing
     (if snapped to edge, it would offer creation of a new vertex there).
    */
    QgsPointLocator::Match snapToEditableLayer( QgsMapMouseEvent *e );

    //! check whether we are still close to the mEndpointMarker
    bool isNearEndpointMarker( const QgsPoint &mapPoint );

    bool isMatchAtEndpoint( const QgsPointLocator::Match &match );

    QgsPoint positionForEndpointMarker( const QgsPointLocator::Match &match );

    void mouseMoveNotDragging( QgsMapMouseEvent *e );

    QgsGeometry cachedGeometryForVertex( const Vertex &vertex );

    void startDragging( QgsMapMouseEvent *e );

    void startDraggingMoveVertex( const QgsPoint &mapPoint, const QgsPointLocator::Match &m );

    //! Get list of matches of all vertices of a layer exactly snapped to a map point
    QList<QgsPointLocator::Match> layerVerticesSnappedToPoint( QgsVectorLayer *layer, const QgsPoint &mapPoint );

    void startDraggingAddVertex( const QgsPointLocator::Match &m );

    void startDraggingAddVertexAtEndpoint( const QgsPoint &mapPoint );

    void startDraggingEdge( const QgsPointLocator::Match &m, const QgsPoint &mapPoint );

    void stopDragging();

    QgsPoint matchToLayerPoint( const QgsVectorLayer *destLayer, const QgsPoint &mapPoint, const QgsPointLocator::Match *match );

    //! Finish moving of an edge
    void moveEdge( const QgsPoint &mapPoint );

    void moveVertex( const QgsPoint &mapPoint, const QgsPointLocator::Match *mapPointMatch );

    void deleteVertex();

    void setHighlightedNodes( const QList<Vertex> &listNodes );

    void setHighlightedNodesVisible( bool visible );

    //! Allow moving back and forth selected vertex within a feature
    void highlightAdjacentVertex( double offset );

    //! Initialize rectangle that is being dragged to select nodes.
    //! Argument point0 is in screen coordinates.
    void startSelectionRect( const QPoint &point0 );

    //! Update bottom-right corner of the existing selection rectangle.
    //! Argument point1 is in screen coordinates.
    void updateSelectionRect( const QPoint &point1 );

    void stopSelectionRect();

    //! Using a given edge match and original map point, find out
    //! center of the edge and whether we are close enough to the center
    bool matchEdgeCenterTest( const QgsPointLocator::Match &m, const QgsPoint &mapPoint, QgsPoint *edgeCenterPtr = nullptr );


  private:

    // members used for temporary highlight of stuff

    //! marker of a snap match (if any) when dragging a vertex
    QgsVertexMarker *mSnapMarker = nullptr;
    //! marker in the middle of an edge while pointer is close to a vertex and not dragging anything
    //! (highlighting point that can be clicked to add a new vertex)
    QgsVertexMarker *mEdgeCenterMarker = nullptr;
    //! rubber band for highlight of a whole feature on mouse over and not dragging anything
    QgsRubberBand *mFeatureBand = nullptr;
    //! source layer for mFeatureBand (null if mFeatureBand is null)
    const QgsVectorLayer *mFeatureBandLayer = nullptr;
    //! source feature id for mFeatureBand (zero if mFeatureBand is null)
    QgsFeatureId mFeatureBandFid;
    //! highlight of a vertex while mouse pointer is close to a vertex and not dragging anything
    QgsRubberBand *mVertexBand = nullptr;
    //! highlight of an edge while mouse pointer is close to an edge and not dragging anything
    QgsRubberBand *mEdgeBand = nullptr;

    // members for dragging operation

    enum DraggingVertexType
    {
      NotDragging,
      MovingVertex,
      AddingVertex,
      AddingEndpoint,
    };

    //! markers for points used only for moving standalone point geoetry
    //! (there are no adjacent vertices so it is not used in mDragBands)
    QList<QgsVertexMarker *> mDragPointMarkers;
    //! companion array to mDragPointMarkers: for each marker it keeps offset
    //! (in map units) from the position of the main vertex
    QList<QgsVector> mDragPointMarkersOffset;
    //! list of QgsRubberBand instances used when dragging. All rubber bands
    //! have two points: first point is fixed, the other one is moved as mouse moves
    QList<QgsRubberBand *> mDragBands;
    //! companion array to mDragBands: for each rubber band it keeps offset of the second
    //! point (in map units) from the position of the main vertex (mDraggingVertex)
    QList<QgsVector> mDragBandsOffset;
    //! list of QgsRubberBand instances used when dragging multiple vertices - these rubber bands
    //! compared to mDragBands have both points moving together with mouse cursor
    QList<QgsRubberBand *> mDragMiddleBands;
    //! companion array to mDragMiddleBands: for each rubber band it keeps offset of both
    //! first and second point (in map units) from the position of the main vertex (mDraggingVertex)
    QList< QPair<QgsVector, QgsVector> > mDragMiddleBandsOffset;
    //! instance of Vertex that is being currently moved or nothing
    std::unique_ptr<Vertex> mDraggingVertex;
    //! whether moving a vertex or adding one
    DraggingVertexType mDraggingVertexType = NotDragging;
    //! instance of Edge that is being currently moved or nothing
    std::unique_ptr<Edge> mDraggingEdge;
    //! list of Vertex instances of further vertices that are dragged together with
    //! the main vertex (mDraggingVertex) - either topologically connected points
    //! (if topo editing is allowed) or the ones coming from the highlight
    QList<Vertex> mDraggingExtraVertices;
    //! companion array to mDraggingExtraVertices: for each vertex in mDraggingExtraVertices
    //! this is offset (in units of the layer) of the vertex from the position of the main
    //! vertex (mDraggingVertex)
    QList<QgsVector> mDraggingExtraVerticesOffset;

    // members for selection handling

    //! list of Vertex instances of nodes that are selected
    QList<Vertex> mSelectedNodes;
    //! list of vertex markers
    QList<QgsVertexMarker *> mSelectedNodesMarkers;

    // members for rectangle for selection

    //! QPoint if user is dragging a selection rect
    std::unique_ptr<QPoint> mSelectionRectStartPos;
    //! QRect in screen coordinates or null
    std::unique_ptr<QRect> mSelectionRect;
    //! QRubberBand to show mSelectionRect
    QRubberBand *mSelectionRectItem = nullptr;

    // members for addition of vertices at the end of a curve

    //! Vertex instance or None
    std::unique_ptr<Vertex> mMouseAtEndpoint;
    //! QgsPoint or None (can't get center from QgsVertexMarker)
    std::unique_ptr<QgsPoint> mEndpointMarkerCenter;
    //! marker shown near the end of a curve to suggest that the user
    //! may add a new vertex at the end of the curve
    QgsVertexMarker *mEndpointMarker = nullptr;

    //! keeps information about previously used snap match
    //! to stick with the same highlighted feature next time if there are more options
    std::unique_ptr<QgsPointLocator::Match> mLastSnap;

    //! List of two points that will be forced into CAD dock with fake mouse events
    //! to allow correct functioning of node tool with CAD dock.
    //! (CAD dock does various assumptions that work with simple capture tools, but not with node tool)
    QList<QgsPoint> mOverrideCadPoints;

    //! When double-clicking to add a new vertex, this member keeps the snap
    //! match from "press" event used to be used in following "release" event
    std::unique_ptr<QgsPointLocator::Match> mNewVertexFromDoubleClick;

    //! Geometry cache for fast access to geometries
    QHash<const QgsVectorLayer *, QHash<QgsFeatureId, QgsGeometry> > mCache;
};


#endif // QGSNODETOOL2_H
