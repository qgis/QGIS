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

class QRubberBand;

class QgsRubberBand;
class QgsVertexEntry;
class QgsSelectedFeature;

/**
 * Set representing set of vertex numbers
 */
typedef QSet<int> Vertexes;

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

  public slots:
    void selectedFeatureDestroyed();

    /*
     * the current layer changed
     */
    void currentLayerChanged( QgsMapLayer *layer );

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
    void createTopologyRubberBands( QgsVectorLayer* vlayer, const QList<QgsVertexEntry*> &vertexMap, int vertex );

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
    QgsSelectedFeature *mSelectedFeature;

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
    QPoint mPressCoordinates;

    /** closest vertex to click in map coordinates */
    QgsPoint mClosestMapVertex;

    /** backup of map coordinates to be able to count change between moves */
    QgsPoint mPosMapCoordBackup;

    /** active rubberband for selecting vertexes */
    QRubberBand *mSelectionRubberBand;

    /** rectangle defining area for selecting vertexes */
    QRect* mRect;

    /** flag to tell if edition points */
    bool mIsPoint;

    /** vertex to deselect on release */
    int mDeselectOnRelease;
};

#endif
