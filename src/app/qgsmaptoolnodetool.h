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
#include <QRect>
#include <QRubberBand>

class QgsRubberBand;

/**
 * Structure to store entry about vertex of the feature
 */
struct VertexEntry 
{
   bool selected;
   QgsPoint point;
   int equals;
   QgsRubberBand *vertexMarker;;
   bool inRubberBand;
   int rubberBandNr;
   int index;
   int originalIndex;
};

/**
 * Class that supports feature which is selected/
 */
class SelectionFeature
{

  public:
    SelectionFeature();
    ~SelectionFeature();

    /**
     * Setting selected feature
     * @param featureId id of feature which was selected
     * @param vlayer vector layer in which feature is selected
     * @param rubberBand rubber band which displays feature
     * @param canvas mapCanvas on which we are working
     * @param feature feature with which we work this parameter is not mandatory if it's not filled feature will be loaded
     */
    void setSelectedFeature( int featureId,  QgsVectorLayer* vlayer,  QgsRubberBand* rubberBand, QgsMapCanvas* canvas, QgsFeature* feature = NULL);

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
     */
    void invertVertexSelection( int vertexNr, bool invert = true );

    /**
     * Updates vertex markers position accoording to changed feature geometry
     * @param canvas map canvas we are working with
     */
    void updateVertexMarkersPosition( QgsMapCanvas* canvas);

    /**
     * Tells if vertex is selected
     * @param vertexNr number of vertex for which we are getting info
     * @return true if vertex is selected, false otherwise
     */
    bool isSelected(int vertexNr);

    /**
     * Getting feature Id of feature selected
     * @return feature id of selected feature
     */
    int featureId();

    /**
     * Getting vertex map of vertexes
     * @return currently used vertex map
     */
    QList<VertexEntry> vertexMap();

    /**
     * Getting currently edited feature
     * @return selected feature
     */
    QgsFeature* feature();

    /**
     * Updates whole selection object from the selected object
     */
    void updateFromFeature();

    /**
     * Sets values for rubber band
     * @param index index of vertex for rubberbanf
     * @param inRubberBand flag if vertex is already in rubber band
     * @param rubberBandNr number of geometry (rubber band) in which this vertex should be)
     * @param indexInRubberBand
     */
    void setRubberBandValues(int index, bool inRubberBand, int rubberBandNr, int indexInRubberBand );

    /**
     * Clears data about vertexes if they are in rubber band for moving etc.
     */
    void cleanRubberBandsData();

    void setMarkerCenter(QgsRubberBand* marker, QgsPoint center);

    QgsRubberBand* createRubberBandMarker(QgsPoint center);



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
     * Updates stored feauture to actual one loaded from layer
     */
    void updateFeature();

    QgsFeature* mFeature;
    int mFeatureId;
    QgsVectorLayer* mVlayer;
    QgsRubberBand* mRubberBand;
    QList<VertexEntry> mVertexMap;
    QgsMapCanvas* mCanvas;
};

/**A maptool to move/deletes/adds vertices of line or polygon fetures*/
class QgsMapToolNodeTool: public QgsMapToolVertexEdit
{
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
    QgsPoint getClosestVertex(QgsPoint point);
    

  private:

    /** Deletes the rubber band pointers
     and clears mRubberBands*/
    void removeRubberBands();

    /** The position of the vertex to move (in map coordinates) to exclude later from snapping*/
    QList<QgsPoint> mExcludePoint;

    /** rubber bands */
    QList<QgsRubberBand*> mQgsRubberBands;

    /** object containing selected feature and it's vertexes */
    SelectionFeature* mSelectionFeature;

    /** flag if selection rectangle is active */
    bool mSelectionRectangle;

    /** flag if moving of vertexes is occuring */
    bool mMoving;

    /** flag if click action is still in queue to be processed */
    bool mClicked;

    /** flag if crtl is pressed */
    bool mCtrl;

    /** flag if selection of another frature can occur */
    bool mSelectAnother;

    /** feature id of another feature where user clicked */
    int mAnother;

    /** stored position of last press down action to count how much vertexes should be moved */
    QgsPoint* mLastCoordinates;

    /** closest vertex to click */
    QgsPoint mClosestVertex;

    /** active rubberband for selecting vertexes */
    QRubberBand* mQRubberBand;

    /** rectangle defining area for selecting vertexes */
    QRect* mRect;

};


#endif
