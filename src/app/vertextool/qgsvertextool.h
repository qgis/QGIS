/***************************************************************************
  qgsvertextool.h
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

#ifndef QGSVERTEXTOOL_H
#define QGSVERTEXTOOL_H

#include <memory>

#include <QPointer>

#include "qgis_app.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgsgeometry.h"
#include "qgspointlocator.h"
#include "qobjectuniqueptr.h"

class QRubberBand;

class QgsGeometryValidator;
class QgsVertexEditor;
class QgsLockedFeature;
class QgsSnapIndicator;
class QgsVertexMarker;

//! helper structure for a vertex being dragged
struct Vertex
{
  Vertex( QgsVectorLayer *layer, QgsFeatureId fid, int vertexId )
    : layer( layer )
    , fid( fid )
    , vertexId( vertexId ) {}

  // TODO c++20 - replace with = default
  bool operator==( const Vertex &other ) const
  {
    return layer == other.layer && fid == other.fid && vertexId == other.vertexId;
  }
  bool operator!=( const Vertex &other ) const
  {
    return !operator==( other );
  }

  QgsVectorLayer *layer = nullptr;
  QgsFeatureId fid;
  int vertexId;
};

//! qHash implementation - we use Vertex in QSet
uint qHash( const Vertex &v );



class APP_EXPORT QgsVertexTool : public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT
  public:

    enum VertexToolMode
    {
      ActiveLayer,
      AllLayers
    };
    Q_ENUM( VertexToolMode )

    QgsVertexTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock, VertexToolMode mode = QgsVertexTool::AllLayers );

    //! Cleanup canvas items we have created
    ~QgsVertexTool() override;

    void cadCanvasPressEvent( QgsMapMouseEvent *e ) override;

    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;

    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;

    //! Start addition of a new vertex on double-click
    void canvasDoubleClickEvent( QgsMapMouseEvent *e ) override;

    void activate() override;

    void deactivate() override;

    //! Clean drawings on map canvas
    void clean() override;

    void keyPressEvent( QKeyEvent *e ) override;

    QgsGeometry cachedGeometry( const QgsVectorLayer *layer, QgsFeatureId fid );

    //! Toggle the vertex editor
    void showVertexEditor();  //#spellok

    //! Update vertex editor to show feature from the given match
    void updateVertexEditor( QgsVectorLayer *layer, QgsFeatureId fid );

  private slots:
    //! update geometry of our feature
    void onCachedGeometryChanged( QgsFeatureId fid, const QgsGeometry &geom );

    void onCachedGeometryDeleted( QgsFeatureId fid );

    void clearGeometryCache();

    void deleteVertexEditorSelection();

    void validationErrorFound( const QgsGeometry::Error &e );

    void validationFinished();

    void startRangeVertexSelection();

    void cleanEditor( QgsFeatureId id );

    void lockedFeatureSelectionChanged();

    void currentLayerChanged( QgsMapLayer *layer );

  private:

    void buildDragBandsForVertices( const QSet<Vertex> &movingVertices, const QgsPointXY &dragVertexMapPoint );

    void addDragBand( const QgsPointXY &v1, const QgsPointXY &v2 );

    void addDragStraightBand( QgsVectorLayer *layer, QgsPointXY v0, QgsPointXY v1, bool moving0, bool moving1, const QgsPointXY &mapPoint );

    void addDragCircularBand( QgsVectorLayer *layer, QgsPointXY v0, QgsPointXY v1, QgsPointXY v2, bool moving0, bool moving1, bool moving2, const QgsPointXY &mapPoint );

    void moveDragBands( const QgsPointXY &mapPoint );

    void clearDragBands();

    void mouseMoveDraggingVertex( QgsMapMouseEvent *e );

    void mouseMoveDraggingEdge( QgsMapMouseEvent *e );

    void removeTemporaryRubberBands();

    void cleanupVertexEditor();

    void cleanupLockedFeature();

    /**
     * Temporarily override snapping config and snap to vertices and edges
    * of any editable vector layer, to allow selection of vertex for editing
    * (if snapped to edge, it would offer creation of a new vertex there).
    */
    QgsPointLocator::Match snapToEditableLayer( QgsMapMouseEvent *e );

    /**
     * Tries to find a match in polygon interiors. This is useful for mouse move
     * events to keep features highlighted to see their area.
     */
    QgsPointLocator::Match snapToPolygonInterior( QgsMapMouseEvent *e );

    /**
     * Returns a list of all matches at the given map point. That is a concatenation
     * of all vertex, edge and area matches (vertex/edge matches using standard search tolerance).
     * Layer is only searched if it is editable.
     */
    QList<QgsPointLocator::Match> findEditableLayerMatches( const QgsPointXY &mapPoint, QgsVectorLayer *layer );

    /**
     * Returns a set of all matches at the given map point from all editable layers (respecting the mode).
     * The set does not contain only the closest match from each layer, but all matches in the standard
     * vertex search tolerance. It also includes area matches.
     */
    QSet<QPair<QgsVectorLayer *, QgsFeatureId> > findAllEditableFeatures( const QgsPointXY &mapPoint );

    /**
     * Implements behavior for mouse right-click to select a feature for editing (and in case of multiple
     * features in one place, repeated right-clicks will cycle through the features).
     */
    void tryToSelectFeature( QgsMapMouseEvent *e );

    //! check whether we are still close to the mEndpointMarker
    bool isNearEndpointMarker( const QgsPointXY &mapPoint );

    bool isMatchAtEndpoint( const QgsPointLocator::Match &match );

    QgsPointXY positionForEndpointMarker( const QgsPointLocator::Match &match );

    void mouseMoveNotDragging( QgsMapMouseEvent *e );

    QgsGeometry cachedGeometryForVertex( const Vertex &vertex );

    void startDragging( QgsMapMouseEvent *e );

    void startDraggingMoveVertex( const QgsPointLocator::Match &m );

    //! Gets list of matches of all vertices of a layer exactly snapped to a map point
    QList<QgsPointLocator::Match> layerVerticesSnappedToPoint( QgsVectorLayer *layer, const QgsPointXY &mapPoint );

    //! Gets list of matches of all segments of a layer coincident with the given segment
    QList<QgsPointLocator::Match> layerSegmentsSnappedToSegment( QgsVectorLayer *layer, const QgsPointXY &mapPoint1, const QgsPointXY &mapPoint2 );

    void startDraggingAddVertex( const QgsPointLocator::Match &m );

    void startDraggingAddVertexAtEndpoint( const QgsPointXY &mapPoint );

    void startDraggingEdge( const QgsPointLocator::Match &m, const QgsPointXY &mapPoint );

    void stopDragging();

    QgsPoint matchToLayerPoint( const QgsVectorLayer *destLayer, const QgsPointXY &mapPoint, const QgsPointLocator::Match *match );

    //! Finish moving of an edge
    void moveEdge( const QgsPointXY &mapPoint );

    void moveVertex( const QgsPointXY &mapPoint, const QgsPointLocator::Match *mapPointMatch );

    void deleteVertex();

    void toggleVertexCurve();

    typedef QHash<QgsVectorLayer *, QHash<QgsFeatureId, QgsGeometry> > VertexEdits;

    void addExtraVerticesToEdits( VertexEdits &edits, const QgsPointXY &mapPoint, QgsVectorLayer *dragLayer = nullptr, const QgsPoint &layerPoint = QgsPoint() );

    void addExtraSegmentsToEdits( QgsVertexTool::VertexEdits &edits, const QgsPointXY &mapPoint, QgsVectorLayer *dragLayer, const QgsPoint &layerPoint );

    void applyEditsToLayers( VertexEdits &edits );

    /**
     * For the given set of vertices (possibly from multiple layers) find any another vertices that are coincident with
     * them and not yet included in the set. This is used for topological editing to find all vertices that should be moved.
     */
    QSet<Vertex> findCoincidentVertices( const QSet<Vertex> &vertices );

    /**
     * For the given set of vertices, prepare mDraggingExtraVertices and mDraggingExtraVerticesOffset arrays.
     * The parameters anchorPoint and anchorLayer are used to calculate offsets.
     */
    void buildExtraVertices( const QSet<Vertex> &vertices, const QgsPointXY &anchorPoint, QgsVectorLayer *anchorLayer );

    //! Returns a list of canvas layers filtered to just editable spatial vector layers
    QList<QgsVectorLayer *> editableVectorLayers();

    enum HighlightMode
    {
      ModeReset, //!< Reset any current selection
      ModeAdd, //!< Add to current selection
      ModeSubtract, //!< Remove from current selection
    };

    void setHighlightedVertices( const QList<Vertex> &listVertices, HighlightMode mode = ModeReset );

    void setHighlightedVerticesVisible( bool visible );

    //! Allow moving back and forth selected vertex within a feature
    void highlightAdjacentVertex( double offset );

    //! Initialize the rubberband for vertex selection using a rectangle
    void initSelectionRubberBand();

    //! Update the rubberband for vertex selection using a rectangle
    void updateSelectionRubberBand( QgsMapMouseEvent *e );

    void stopSelectionRubberBand();

    /**
     * Using a given edge match and original map point, find out
     * center of the edge and whether we are close enough to the center
     */
    bool matchEdgeCenterTest( const QgsPointLocator::Match &m, const QgsPointXY &mapPoint, QgsPointXY *edgeCenterPtr = nullptr );

    //! Run validation on a geometry (in a background thread)
    void validateGeometry( QgsVectorLayer *layer, QgsFeatureId featureId );

    //! Makes sure that the vertex is visible in map canvas
    void zoomToVertex( const Vertex &vertex );

    //! Returns a list of vertices between the two given vertex indices (including those)
    QList<Vertex> verticesInRange( QgsVectorLayer *layer, QgsFeatureId fid, int vertexId0, int vertexId1, bool longWay );

    void updateFeatureBand( const QgsPointLocator::Match &m );

    //! Updates vertex band based on the current match
    void updateVertexBand( const QgsPointLocator::Match &m );

    //! Handles mouse press event when in range selection method
    void rangeMethodPressEvent( QgsMapMouseEvent *e );
    //! Handles mouse release event when in range selection method
    void rangeMethodReleaseEvent( QgsMapMouseEvent *e );
    //! Handles mouse move event when in range selection method
    void rangeMethodMoveEvent( QgsMapMouseEvent *e );

    void stopRangeVertexSelection();

    //! update the highlight of vertices from the locked feature
    void updateLockedFeatureVertices();

  private:

    QgsVertexEditor *vertexEditor();

    // members used for temporary highlight of stuff

    //! marker of a snap match (if any) when dragging a vertex
    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    /**
     * marker in the middle of an edge while pointer is close to a vertex and not dragging anything
     * (highlighting point that can be clicked to add a new vertex)
     */
    QgsVertexMarker *mEdgeCenterMarker = nullptr;
    //! rubber band for highlight of a whole feature on mouse over and not dragging anything
    QgsRubberBand *mFeatureBand = nullptr;
    //! rubber band for highlight of all vertices of a feature on mouse over and not dragging anything, also used for locked feature vertices
    QgsRubberBand *mFeatureBandMarkers = nullptr;
    //! source layer for mFeatureBand (null if mFeatureBand is null)
    const QgsVectorLayer *mFeatureBandLayer = nullptr;
    //! source feature id for mFeatureBand (zero if mFeatureBand is null)
    QgsFeatureId mFeatureBandFid = 0;
    //! highlight of a vertex while mouse pointer is close to a vertex and not dragging anything
    QgsRubberBand *mVertexBand = nullptr;
    //! highlight of an edge while mouse pointer is close to an edge and not dragging anything
    QgsRubberBand *mEdgeBand = nullptr;
    //! highlight of locked feature vertices (but not selected)
    QList<QgsVertexMarker *> mLockedFeatureVerticesMarkers;

    // members for dragging operation

    enum DraggingVertexType
    {
      NotDragging,
      MovingVertex,
      AddingVertex,
      AddingEndpoint,
    };

    /**
     * markers for points used only for moving standalone point geoetry
     * (there are no adjacent vertices so it is not used in mDragBands)
     */
    QList<QgsVertexMarker *> mDragPointMarkers;

    /**
     * companion array to mDragPointMarkers: for each marker it keeps offset
     * (in map units) from the position of the main vertex
     */
    QList<QgsVector> mDragPointMarkersOffset;

    //! structure to keep information about a rubber band used for dragging of a straight line segment
    struct StraightBand
    {
      QgsRubberBand *band = nullptr;       //!< Pointer to the actual rubber band
      QgsPointXY p0, p1;                     //!< What are the original positions of points (in map units)
      bool moving0, moving1;               //!< Which points of the band are moving with mouse cursor
      QgsVector offset0, offset1;          //!< If the point is moving, what is the offset from the mouse cursor
    };

    //! structure to keep information about a rubber band used for dragging of a circular segment
    struct CircularBand
    {
      QgsRubberBand *band = nullptr;        //!< Pointer to the actual rubber band
      QgsPointXY p0, p1, p2;                  //!< What are the original positions of points (in map units)
      bool moving0, moving1, moving2;       //!< Which points of the band are moving with mouse cursor
      QgsVector offset0, offset1, offset2;  //!< If the point is moving, what is the offset from the mouse cursor

      //! update geometry of the rubber band band on the current mouse cursor position (in map units)
      void updateRubberBand( const QgsPointXY &mapPoint );
    };

    //! list of active straight line rubber bands
    QList<StraightBand> mDragStraightBands;
    //! list of active rubber bands for circular segments
    QList<CircularBand> mDragCircularBands;
    //! instance of Vertex that is being currently moved or nothing
    std::unique_ptr<Vertex> mDraggingVertex;
    //! whether moving a vertex or adding one
    DraggingVertexType mDraggingVertexType = NotDragging;
    //! whether we are currently dragging an edge
    bool mDraggingEdge = false;

    /**
     * list of Vertex instances of further vertices that are dragged together with
     * the main vertex (mDraggingVertex) - either topologically connected points
     * (if topo editing is allowed) or the ones coming from the highlight
     */
    QList<Vertex> mDraggingExtraVertices;

    /**
     * companion array to mDraggingExtraVertices: for each vertex in mDraggingExtraVertices
     * this is offset (in units of the layer) of the vertex from the position of the main
     * vertex (mDraggingVertex)
     */
    QList<QgsVector> mDraggingExtraVerticesOffset;

    /**
     * list of Vertex instances identifying segments (by their first vertex index) that should
     * also get a new vertex: this is used for topo editing when adding a vertex to existing segment
     */
    QList<Vertex> mDraggingExtraSegments;

    // members for selection handling

    //! list of Vertex instances of vertices that are selected
    QList<Vertex> mSelectedVertices;
    //! list of vertex markers
    QList<QgsVertexMarker *> mSelectedVerticesMarkers;

    // members for rectangle for selection

    //! the rubberband for rectangle/polygon selection visualization
    std::unique_ptr<QgsRubberBand> mSelectionRubberBand;
    //! Initial point (in screen coordinates) when using rectangle/polygon selection
    std::unique_ptr<QPoint> mSelectionRubberBandStartPos;

    // members for addition of vertices at the end of a curve

    //! Vertex instance or None
    std::unique_ptr<Vertex> mMouseAtEndpoint;
    //! QgsPointXY or None (can't get center from QgsVertexMarker)
    std::unique_ptr<QgsPointXY> mEndpointMarkerCenter;

    /**
     * marker shown near the end of a curve to suggest that the user
     * may add a new vertex at the end of the curve
     */
    QgsVertexMarker *mEndpointMarker = nullptr;

    /**
     * keeps information about previously used snap match
     * to stick with the same highlighted feature next time if there are more options
     */
    std::unique_ptr<QgsPointLocator::Match> mLastSnap;

    /**
     * When double-clicking to add a new vertex, this member keeps the snap
     * match from "press" event used to be used in following "release" event
     */
    std::unique_ptr<QgsPointLocator::Match> mNewVertexFromDoubleClick;

    //! Geometry cache for fast access to geometries (coordinates are in their layer's CRS)
    QHash<const QgsVectorLayer *, QHash<QgsFeatureId, QgsGeometry> > mCache;

    // support for vertex editor

    //! Locked feature for the vertex editor
    QObjectUniquePtr<QgsLockedFeature> mLockedFeature;

    /**
     * Data structure that stores alternative features to the currently selected (locked) feature.
     * This is used when user clicks with right mouse button multiple times in one location
     * to easily switch to the desired feature.
     */
    struct LockedFeatureAlternatives
    {
      QPoint screenPoint;
      QList< QPair<QgsVectorLayer *, QgsFeatureId> > alternatives;
      int index = -1;
    };

    //! Keeps information about other possible features to select with right click. Null if no info is currently held.
    std::unique_ptr<LockedFeatureAlternatives> mLockedFeatureAlternatives;

    // support for validation of geometries

    //! data structure for validation of one geometry of a vector layer
    struct GeometryValidation
    {
      QgsVertexTool *tool = nullptr;               //!< Pointer to the parent vertex tool (for connections / canvas)
      QgsVectorLayer *layer = nullptr;            //!< Pointer to the layer of the validated geometry (for reporojection)
      QgsGeometryValidator *validator = nullptr;  //!< Object that does validation. Non-null if active
      QList<QgsVertexMarker *> errorMarkers;      //!< Markers created by validation
      QString errors;                             //!< Full error text from validation

      void start( QgsGeometry &geom, QgsVertexTool *tool, QgsVectorLayer *l );  //!< Start validation
      void addError( QgsGeometry::Error e );  //!< Add another error to the validation
      void cleanup(); //!< Delete everything
    };

    //! data structure to keep validation details
    QHash< QPair<QgsVectorLayer *, QgsFeatureId>, GeometryValidation> mValidations;

    //! Enumeration of methods for selection of vertices
    enum VertexSelectionMethod
    {
      SelectionNormal,   //!< Default selection: clicking vertex starts move, ctrl+click selects vertex, dragging rectangle select multiple vertices
      SelectionRange,    //!< Range selection: clicking selects start vertex, next click select final vertex, vertices in the range get selected
      SelectionPolygon,  //!< Polygon selection: alt+click starts digitizing a polygon, subsequent clicks add vertices, right click selects vertices within the polygon
    };

    //! Current vertex selection method
    VertexSelectionMethod mSelectionMethod = SelectionNormal;

    //! Starting vertex when using range selection (null if not yet selected)
    std::unique_ptr<Vertex> mRangeSelectionFirstVertex;

    VertexToolMode mMode = AllLayers;
};


#endif // QGSVERTEXTOOL_H
