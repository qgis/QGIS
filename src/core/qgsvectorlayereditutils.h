/***************************************************************************
    qgsvectorlayereditutils.h
    ---------------------
    begin                : Dezember 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSVECTORLAYEREDITUTILS_H
#define QGSVECTORLAYEREDITUTILS_H


#include "qgis_core.h"
#include "qgis.h"
#include "qgsgeometry.h"
#include "qgsfeatureid.h"
#include "qgsvectorlayer.h"

class QgsCurve;

/**
 * \ingroup core
 * \class QgsVectorLayerEditUtils
 */
class CORE_EXPORT QgsVectorLayerEditUtils
{
  public:
    QgsVectorLayerEditUtils( QgsVectorLayer *layer );

    /**
     * Insert a new vertex before the given vertex number,
     * in the given ring, item (first number is index 0), and feature
     * Not meaningful for Point geometries
     */
    bool insertVertex( double x, double y, QgsFeatureId atFeatureId, int beforeVertex );

    /**
     * Inserts a new vertex before the given vertex number,
     * in the given ring, item (first number is index 0), and feature
     * Not meaningful for Point geometries
     */
    bool insertVertex( const QgsPoint &point, QgsFeatureId atFeatureId, int beforeVertex );

    /**
     * Moves the vertex at the given position number,
     * ring and item (first number is index 0), and feature
     * to the given coordinates
     */
    bool moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex );

    /**
     * Moves the vertex at the given position number,
     * ring and item (first number is index 0), and feature
     * to the given coordinates
     * \note available in Python bindings as moveVertexV2
     */
    bool moveVertex( const QgsPoint &p, QgsFeatureId atFeatureId, int atVertex ) SIP_PYNAME( moveVertexV2 );

    /**
     * Deletes a vertex from a feature.
     * \param featureId ID of feature to remove vertex from
     * \param vertex index of vertex to delete
     * \since QGIS 2.14
     */
    QgsVectorLayer::EditResult deleteVertex( QgsFeatureId featureId, int vertex );

    /**
     * Adds a ring to polygon/multipolygon features
     * \param ring ring to add
     * \param targetFeatureIds if specified, only these features will be the candidates for adding a ring. Otherwise
     * all intersecting features are tested and the ring is added to the first valid feature.
     * \param modifiedFeatureId if specified, feature ID for feature that ring was added to will be stored in this parameter
     * \return OperationResult result code: success or reason of failure
     */
    QgsGeometry::OperationResult addRing( const QVector<QgsPointXY> &ring, const QgsFeatureIds &targetFeatureIds = QgsFeatureIds(), QgsFeatureId *modifiedFeatureId = nullptr );

    /**
     * Adds a ring to polygon/multipolygon features
     * \param ring ring to add
     * \param targetFeatureIds if specified, only these features will be the candidates for adding a ring. Otherwise
     * all intersecting features are tested and the ring is added to the first valid feature.
     * \param modifiedFeatureId if specified, feature ID for feature that ring was added to will be stored in this parameter
     * \return OperationResult result code: success or reason of failure
     * \note available in python bindings as addCurvedRing
     */
    QgsGeometry::OperationResult addRing( QgsCurve *ring, const QgsFeatureIds &targetFeatureIds = QgsFeatureIds(), QgsFeatureId *modifiedFeatureId = nullptr ) SIP_PYNAME( addCurvedRing );

    /**
     * Adds a new part polygon to a multipart feature
     * \returns - QgsGeometry::Success
     * - QgsGeometry::AddPartSelectedGeometryNotFound
     * - QgsGeometry::AddPartNotMultiGeometry
     * - QgsGeometry::InvalidBaseGeometry
     * - QgsGeometry::InvalidInput
     */
    QgsGeometry::OperationResult addPart( const QList<QgsPointXY> &ring, QgsFeatureId featureId );

    /**
     * Adds a new part polygon to a multipart feature
     *
     * \returns - QgsGeometry::Success
     * - QgsGeometry::AddPartSelectedGeometryNotFound
     * - QgsGeometry::AddPartNotMultiGeometry
     * - QgsGeometry::InvalidBaseGeometry
     * - QgsGeometry::InvalidInput
     * \note available in python bindings as addPartV2
     */
    QgsGeometry::OperationResult addPart( const QgsPointSequence &ring, QgsFeatureId featureId );

    /**
     * Adds a new part polygon to a multipart feature
     *
     * \returns - QgsGeometry::Success
     * - QgsGeometry::AddPartSelectedGeometryNotFound
     * - QgsGeometry::AddPartNotMultiGeometry
     * - QgsGeometry::InvalidBaseGeometry
     * - QgsGeometry::InvalidInput
     *
     * \note available in python bindings as addCurvedPart
     */
    QgsGeometry::OperationResult addPart( QgsCurve *ring, QgsFeatureId featureId ) SIP_PYNAME( addCurvedPart );

    /**
     * Translates feature by dx, dy
     * \param featureId id of the feature to translate
     * \param dx translation of x-coordinate
     * \param dy translation of y-coordinate
     * \return 0 in case of success
     */
    int translateFeature( QgsFeatureId featureId, double dx, double dy );

    /**
     * Splits parts cut by the given line
     * \param splitLine line that splits the layer feature parts
     * \param topologicalEditing true if topological editing is enabled
     * \returns  - QgsGeometry::InvalidBaseGeometry
     *  - QgsGeometry::Success
     *  - QgsGeometry::InvalidInput
     *  - QgsGeometry::NothingHappened if a selection is present but no feature has been split
     *  - QgsGeometry::InvalidBaseGeometry
     *  - QgsGeometry::GeometryEngineError
     *  - QgsGeometry::SplitCannotSplitPoint
     */
    QgsGeometry::OperationResult splitParts( const QVector<QgsPointXY> &splitLine, bool topologicalEditing = false );

    /**
     * Splits features cut by the given line
     * \param splitLine line that splits the layer features
     * \param topologicalEditing true if topological editing is enabled
     * \returns 0 in case of success,
     *  4 if there is a selection but no feature split
     */
    QgsGeometry::OperationResult splitFeatures( const QVector<QgsPointXY> &splitLine, bool topologicalEditing = false );

    /**
     * Adds topological points for every vertex of the geometry.
     * \param geom the geometry where each vertex is added to segments of other features
     * \return 0 in case of success
     * \note geom is not going to be modified by the function
     */
    int addTopologicalPoints( const QgsGeometry &geom );

    /**
     * Adds a vertex to segments which intersect point p but don't
     * already have a vertex there. If a feature already has a vertex at position p,
     * no additional vertex is inserted. This method is useful for topological
     * editing.
     * \param p position of the vertex
     * \return 0 in case of success
     */
    int addTopologicalPoints( const QgsPointXY &p );

  private:

    /**
     * Little helper function that gives bounding box from a list of points.
     * \returns True in case of success
     */
    bool boundingBoxFromPointList( const QVector<QgsPointXY> &list, double &xmin, double &ymin, double &xmax, double &ymax ) const;

    QgsVectorLayer *mLayer = nullptr;
};

#endif // QGSVECTORLAYEREDITUTILS_H
