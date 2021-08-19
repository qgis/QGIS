/***************************************************************************
                        qgsgeometryeditutils.h
  -------------------------------------------------------------------
Date                 : 21 Jan 2015
Copyright            : (C) 2015 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYEDITUTILS_H
#define QGSGEOMETRYEDITUTILS_H

class QgsAbstractGeometry;
class QgsCurve;
class QgsGeometryEngine;
class QgsVectorLayer;

#define SIP_NO_FILE

#include "qgsfeatureid.h"
#include "qgis.h"
#include <QMap>
#include <memory>

/**
 * \ingroup core
 * \class QgsGeometryEditUtils
 * \brief Convenience functions for geometry editing
 * \note not available in Python bindings
 * \since QGIS 2.10
 */
class QgsGeometryEditUtils
{
  public:

    /**
     * Add an interior \a ring to a \a geometry.
     * Ownership of the \a ring is transferred.
     * \returns 0 in case of success (ring added), 1 problem with geometry type, 2 ring not closed,
     * 3 ring is not valid geometry, 4 ring not disjoint with existing rings, 5 no polygon found which contained the ring
     */
    static Qgis::GeometryOperationResult addRing( QgsAbstractGeometry *geometry, std::unique_ptr< QgsCurve > ring );

    /**
     * Add a \a part to multi type \a geometry.
     * Ownership of the \a part is transferred.
     * \returns 0 in case of success, 1 if not a multigeometry, 2 if part is not a valid geometry, 3 if new polygon ring
     * not disjoint with existing polygons of the feature
     */
    static Qgis::GeometryOperationResult addPart( QgsAbstractGeometry *geometry, std::unique_ptr< QgsAbstractGeometry > part );

    /**
     * Deletes a ring from a geometry.
     * \returns TRUE if delete was successful
     */
    static bool deleteRing( QgsAbstractGeometry *geom, int ringNum, int partNum = 0 );

    /**
     * Deletes a part from a geometry.
     * \returns TRUE if delete was successful
     */
    static bool deletePart( QgsAbstractGeometry *geom, int partNum );

    /**
     * Alters a geometry so that it avoids intersections with features from all open vector layers.
     * \param geom geometry to alter
     * \param avoidIntersectionsLayers list of layers to check for intersections
     * \param haveInvalidGeometry returns true if at least one geometry intersected is invalid. In this case, the algorithm may not work and return the same geometry as the input. You must fix your intersecting geometries.
     * \param ignoreFeatures map of layer to feature id of features to ignore
     */
    static std::unique_ptr< QgsAbstractGeometry > avoidIntersections( const QgsAbstractGeometry &geom,
        const QList<QgsVectorLayer *> &avoidIntersectionsLayers,
        bool &haveInvalidGeometry,
        const QHash<QgsVectorLayer *, QSet<QgsFeatureId> > &ignoreFeatures = ( QHash<QgsVectorLayer *, QSet<QgsFeatureId> >() ) );
};

#endif // QGSGEOMETRYEDITUTILS_H
