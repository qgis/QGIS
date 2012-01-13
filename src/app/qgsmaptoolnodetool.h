/***************************************************************************
    qgsmaptoolnodetool.h  - add/move/delete vertex integrated in one tool
    ---------------------
    begin                : April 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf dot kostej at mail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLNODETOOL_H
#define QGSMAPTOOLNODETOOL_H

#include "qgsmaptoolvertexedit.h"
#include "qgsfeature.h"
#include "qgsvertexmarker.h"
#include "qgsgeometryvalidator.h"
#include "qgsmapcanvas.h"
#include "qgslogger.h"

#include <QRect>
#include <QRubberBand>

class QgsRubberBand;

class VertexEntry
{
    bool mSelected;
    QgsPoint mPoint;
    int mEquals;
    bool mInRubberBand;
    int mRubberBandNr;
    int mIndex;
    int mOriginalIndex;
    int mPenWidth;
    QString mToolTip;
    QgsVertexMarker::IconType mType;
    QgsVertexMarker *mMarker;
    QgsMapCanvas *mCanvas;
    QgsMapLayer *mLayer;
  public:
    VertexEntry( QgsMapCanvas *canvas, QgsMapLayer *layer, QgsPoint p, int originalIndex, QString tooltip = QString::null, QgsVertexMarker::IconType type = QgsVertexMarker::ICON_BOX, int penWidth = 2 );
    ~VertexEntry();

    QgsPoint point() const { return mPoint; }
    int equals() const { return mEquals; }
    bool isSelected() const { return mSelected; }
    bool isInRubberBand() const { return mInRubberBand; }

    void setCenter( QgsPoint p );
    void moveCenter( double x, double y );
    void setEqual( int index ) { mEquals = index; }
    void setSelected( bool selected = true );
    void setInRubberBand( bool inRubberBand = true ) { mInRubberBand = inRubberBand; }
    int rubberBandNr() const { return mRubberBandNr; }
    int index() { return mIndex; }

    void setRubberBandValues( bool inRubberBand, int rubberBandNr, int indexInRubberBand );
    void update();
};

/**
 * Set representing set of vertex numbers
 */
typedef QSet<int> Vertexes;

/**
 * Constant representing zero value for distance. It's 0 because of error in double counting.
 */
const static double ZERO_TOLERANCE = 0.000000001;

/**
 * Class that keeps the selected feature
 */
class SelectedFeature: public QObject
{
    Q_OBJECT

  public:
    SelectedFeature( QgsFeatureId id, QgsVectorLayer *layer, QgsMapCanvas *canvas );
    ~SelectedFeature();

    /**
     * Setting selected feature
     * @param featureId id of feature which was selected
     * @param vlayer vector layer in which feature is selected
     * @param rubberBand rubber band which displays feature
     * @param canvas mapCanvas on which we are working
     * @param geometry geometry of the selected feature
     */
    void setSelectedFeature( QgsFeatureId featureId, QgsVectorLayer* vlayer, QgsMapCanvas* canvas );

    /**
     * Function to select vertex with number
     * @param vertexNr number of vertex which is to be selected
     */
    void selectVertex( int vertexNr );

    /**
     * Function to deselect vertex with number
     * @param vertexNr number of vertex which is to be deselected
     */
    void deselectVertex( int vertexNr );

    /**
     * Deselects all vertexes of selected feature
     */
    void deselectAllVertexes();

    /**
     * Deletes all selected vertexes
     */
    void deleteSelectedVertexes();

    /**
     * Moves selected vertex
     * @param changeX change in X coordinate
     * @param changeY change in Y coordinate
     */
    void moveSelectedVertexes( double changeX, double changeY );

    /**
     * Inverts selection of vertex with number
     * @param vertexNr number of vertex which is to be inverted
     * @param invert flag if vertex selection should be inverted or not
     */
    void invertVertexSelection( int vertexNr, bool invert = true );

    /**
     * Tells if vertex is selected
     * @param vertexNr number of vertex for which we are getting info
     * @return true if vertex is selected, false otherwise
     */
    bool isSelected( int vertexNr );

    /**
     * Getting feature Id of feature selected
     * @return feature id of selected feature
     */
    QgsFeatureId featureId();

    /**
     * Getting vertex map of vertexes
     * @return currently used vertex map
     */
    QList<VertexEntry*> &vertexMap();

    /**
     * Updates whole selection object from the selected object
     */
    void replaceVertexMap();

    /**
     * Clears data about vertexes if they are in rubber band for moving etc.
     */
    void cleanRubberBandsData();

    /**
     * Get the layer of the selected feature
     * @return used vector layer
     */
    QgsVectorLayer *vlayer();

    /**
     * Getter for the current geometry
     */
    QgsGeometry *geometry();

    void beginGeometryChange();
    void endGeometryChange();

  public slots:
    /*
     * geometry validation found a problem
     */
    void addError( QgsGeometry::Error );

    /*
     * geometry validation finished
     */
    void validationFinished();

    /**
     * Updates vertex markers position accoording to changed feature geometry
     */
    void updateVertexMarkersPosition();

    /*
     * a feature was removed from the layer - might be the selected
     */
    void featureDeleted( QgsFeatureId );

    /*
     * the geometry of a feature from the layer was changed - might be the selected
     */
    void geometryChanged( QgsFeatureId, QgsGeometry & );

    /*
     * the current layer changed - destroy
     */
    void currentLayerChanged( QgsMapLayer *layer );

    /*
     * the current layer changed - destroy
     */
    void canvasLayersChanged();

  private:
    /**
     * Deletes whole vertex map.
     */
    void deleteVertexMap();

    /**
     * Creates vertex map when
     */
    void createVertexMap();

    /**
     *  Creates vertex map for polygon type feature
     */
    void createVertexMapPolygon();

    /**
     *  Creates vertex map for line type feature
     */
    void createVertexMapLine();

    /**
     *  Creates vertex map for ppint type feature
     */
    void createVertexMapPoint();

    /**
     * Updates stored geometry to actual one loaded from layer
     * (or already available geometry)
     */
    void updateGeometry( QgsGeometry *geom );

    /**
     * Validates the geometry
     */
    void validateGeometry( QgsGeometry *g = 0 );

    QgsFeatureId mFeatureId;
    QgsGeometry *mGeometry;
    bool mFeatureSelected;
    bool mChangingGeometry;
    QgsVectorLayer* mVlayer;
    QgsRubberBand* mRubberBand;
    QList<VertexEntry*> mVertexMap;
    QgsMapCanvas* mCanvas;

    QgsGeometryValidator *mValidator;
    QString mTip;
    QList< QgsGeometry::Error > mGeomErrors;
    QList< QgsVertexMarker * > mGeomErrorMarkers;
};

/**A maptool to move/deletes/adds vertices of line or polygon features*/
class QgsMapToolNodeTool: public QgsMapToolVertexEdit
{
    Q_OBJECT
  public:
    QgsMapToolNodeTool( QgsMapCanvas* canvas );
    virtual ~QgsMapToolNodeTool();

    void canvasMoveEvent( QMouseEvent * e );

    void canvasDoubleClickEvent( QMouseEvent * e );

    void canvasPressEvent( QMouseEvent * e );

    void canvasReleaseEvent( QMouseEvent * e );

    void keyPressEvent( QKeyEvent* e );

    void keyReleaseEvent( QKeyEvent* e );

    //! called when map tool is being deactivated
    void deactivate();

    /**
     * Returns closest vertex to given point from selected feature
     */
    QgsPoint closestVertex( QgsPoint point );

  public slots:
    void selectedFeatureDestroyed();

  private:
    /**
     * Deletes the rubber band pointers and clears mRubberBands
     */
    void removeRubberBands();

    /**
     * Creating rubber band marker for movin of point
     * @param center coordinates of point to be moved
     * @param vlayer vector layer on which we are working
     * @return rubber band marker
     */
    QgsRubberBand* createRubberBandMarker( QgsPoint center, QgsVectorLayer* vlayer );

    /**
     * Function to check if selected feature exists and is same with original one
     * stored in internal structures
     * @param vlayer vector layer for checking
     * @return if feature is same as one in internal structures
     */
    bool checkCorrectnessOfFeature( QgsVectorLayer* vlayer );

    /**
     * Creates rubberbands for moving points
     */
    void createMovingRubberBands();

    /**
     * Creates rubber bands for ther features when topology editing is enabled
     * @param vlayer vector layer for ehich rubber bands are created
     * @param vertexMap map of vertexes
     * @param vertex currently processed vertex
     */
    void createTopologyRubberBands( QgsVectorLayer* vlayer, const QList<VertexEntry*> &vertexMap, int vertex );

    /** The position of the vertex to move (in map coordinates) to exclude later from snapping*/
    QList<QgsPoint> mExcludePoint;

    /** rubber bands */
    QList<QgsRubberBand*> mRubberBands;

    /** list of topology rubber bands */
    QList<QgsRubberBand*> mTopologyRubberBand;

    /** vertexes of rubberbands which are to be moved */
    QMap<QgsFeatureId, Vertexes*> mTopologyMovingVertexes;

    /** vertexes of features with int id which were already added tu rubber bands */
    QMap<QgsFeatureId, Vertexes*> mTopologyRubberBandVertexes;

    /** object containing selected feature and it's vertexes */
    SelectedFeature *mSelectedFeature;

    /** flag if selection rectangle is active */
    bool mSelectionRectangle;

    /** flag if moving of vertexes is occuring */
    bool mMoving;

    /** flag if click action is still in queue to be processed */
    bool mClicked;

    /** flag if crtl is pressed */
    bool mCtrl;

    /** flag if selection of another feature can occur */
    bool mSelectAnother;

    /** feature id of another feature where user clicked */
    QgsFeatureId mAnother;

    /** stored position of last press down action to count how much vertexes should be moved */
    QgsPoint* mLastCoordinates;

    /** closest vertex to click */
    QgsPoint mClosestVertex;

    /** backup of map coordinates to be able to count change between moves */
    QgsPoint mPosMapCoordBackup;

    /** active rubberband for selecting vertexes */
    QRubberBand *mRubberBand;

    /** rectangle defining area for selecting vertexes */
    QRect* mRect;

    /** flag to tell if edition points */
    bool mIsPoint;
};

#endif
