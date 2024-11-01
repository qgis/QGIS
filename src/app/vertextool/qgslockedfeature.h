/***************************************************************************
    qgslockedfeature.h  - selected feature of vertextool
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

#ifndef QGSLOCKEDFEATURE_H
#define QGSLOCKEDFEATURE_H

#include <QObject>

#include "qgis_app.h"
#include "qgsgeometry.h"
#include "qgsfeatureid.h"

class QgsMapCanvas;
class QgsVectorLayer;
class QgsMapLayer;
class QgsGeometryValidator;
class QgsVertexMarker;
class QgsVertexEntry;

/**
 * Class that keeps the selected feature
 */
class APP_EXPORT QgsLockedFeature : public QObject
{
    Q_OBJECT

  public:
    /**
     * Creates a locked feature
     * \param featureId id of feature which was selected
     * \param layer vector layer in which feature is selected
     * \param canvas mapCanvas on which we are working
     */
    QgsLockedFeature( QgsFeatureId id, QgsVectorLayer *layer, QgsMapCanvas *canvas );

    ~QgsLockedFeature() override;

    /**
     * Function to select vertex with number
     * \param vertexNr number of vertex which is to be selected
     */
    void selectVertex( int vertexNr );

    /**
     * Function to deselect vertex with number
     * \param vertexNr number of vertex which is to be deselected
     */
    void deselectVertex( int vertexNr );

    /**
     * Deselects all vertices of selected feature
     */
    void deselectAllVertices();

    /**
     * Inverts selection of vertex with number
     * \param vertexNr number of vertex which is to be inverted
     */
    void invertVertexSelection( int vertexNr );

    /**
     * Inverts selection of a set of vertices at once.
     * \param vertexIndices list of vertex indices to invert whether or not they are selected
     */
    void invertVertexSelection( const QVector<int> &vertexIndices );

    /**
     * Tells if vertex is selected
     * \param vertexNr number of vertex for which we are getting info
     * \returns TRUE if vertex is selected, FALSE otherwise
     */
    bool isSelected( int vertexNr );

    /**
     * Getting feature Id of feature selected
     * \returns feature id of selected feature
     */
    QgsFeatureId featureId();

    /**
     * Getting vertex map of vertices
     * \returns currently used vertex map
     */
    QList<QgsVertexEntry *> &vertexMap();

    /**
     * Updates whole selection object from the selected object
     */
    void replaceVertexMap();

    /**
     * Gets the layer of the selected feature
     * \returns used vector layer
     */
    QgsVectorLayer *layer();

    /**
     * Getter for the current geometry
     */
    QgsGeometry *geometry();

    void beginGeometryChange();
    void endGeometryChange();

  signals:
    void selectionChanged();
    void vertexMapChanged();

  public slots:
    /*
     * geometry validation found a problem
     */
    void addError( QgsGeometry::Error );

    /*
     * geometry validation finished
     */
    void validationFinished();

    /*
     * a feature was removed from the layer - might be the selected
     */
    void featureDeleted( QgsFeatureId );

    /*
     * the geometry of a feature from the layer was changed - might be the selected
     */
    void geometryChanged( QgsFeatureId, const QgsGeometry & );

    /*
     * the changes are rolling back - stop monitoring the geometry
     */
    void beforeRollBack();

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
     * Updates stored geometry to actual one loaded from layer
     * (or already available geometry)
     */
    void updateGeometry( const QgsGeometry *geom );

    /**
     * Validates the geometry
     */
    void validateGeometry( QgsGeometry *g = nullptr );

    QgsFeatureId mFeatureId;
    QgsGeometry *mGeometry = nullptr;
    bool mChangingGeometry = false;
    QgsVectorLayer *mLayer = nullptr;
    QList<QgsVertexEntry *> mVertexMap;
    QgsMapCanvas *mCanvas = nullptr;

    QgsGeometryValidator *mValidator = nullptr;
    QString mTip;
    QList<QgsGeometry::Error> mGeomErrors;
    QList<QgsVertexMarker *> mGeomErrorMarkers;
};

#endif
