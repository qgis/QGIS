/***************************************************************************
    qgsselectedfeature.h  - selected feature of nodetool
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

#ifndef QGSSELECTEDFEATURE_H
#define QGSSELECTEDFEATURE_H

#include "qgsfeature.h"
#include "qgsgeometry.h"

#include <QObject>

class QgsMapCanvas;
class QgsVectorLayer;
class QgsMapLayer;
class QgsRubberBand;
class QgsGeometryValidator;
class QgsVertexMarker;

class QgsVertexEntry;

/**
 * Constant representing zero value for distance. It's 0 because of error in double counting.
 */
const static double ZERO_TOLERANCE = 0.000000001;

/**
 * Class that keeps the selected feature
 */
class QgsSelectedFeature: public QObject
{
    Q_OBJECT

  public:
    QgsSelectedFeature( QgsFeatureId id, QgsVectorLayer *layer, QgsMapCanvas *canvas );
    ~QgsSelectedFeature();

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
     * @param v translation vector
     */
    void moveSelectedVertexes( const QgsVector &v );

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
    QList<QgsVertexEntry*> &vertexMap();

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
    QList<QgsVertexEntry*> mVertexMap;
    QgsMapCanvas* mCanvas;

    QgsGeometryValidator *mValidator;
    QString mTip;
    QList< QgsGeometry::Error > mGeomErrors;
    QList< QgsVertexMarker * > mGeomErrorMarkers;
};

#endif
