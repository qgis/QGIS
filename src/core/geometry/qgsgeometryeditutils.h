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

#include "qgsfeature.h"
#include "qgsgeometry.h"
#include <QMap>

/** \ingroup core
 * \class QgsGeometryEditUtils
 * \brief Convenience functions for geometry editing
 * \note added in QGIS 2.10
 * \note this API is not considered stable and may change for 2.12
 * \note not available in Python bindings
 */
class QgsGeometryEditUtils
{
  public:
    /**
     * @param geom The geometry to add the ring to
     * @param ring The ring. This will take ownership.
     * @return The result of the operation: success or reason of failure.
     */
    static QgsGeometry::OperationResult addRing( QgsAbstractGeometry* geom, QgsCurve* ring );

    /**
     * @param geom The geometry to add the ring to
     * @param ring The ring. This will take ownership.
     * @return The result of the operation: success or reason of failure.
     */
    static QgsGeometry::OperationResult addPart( QgsAbstractGeometry* geom, QgsAbstractGeometry* part );

    /** Deletes a ring from a geometry.
     * @returns true if delete was successful
     */
    static bool deleteRing( QgsAbstractGeometry* geom, int ringNum, int partNum = 0 );

    /** Deletes a part from a geometry.
     * @returns true if delete was successful
     */
    static bool deletePart( QgsAbstractGeometry* geom, int partNum );

    /** Alters a geometry so that it avoids intersections with features from all open vector layers.
     * @param geom geometry to alter
     * @param ignoreFeatures map of layer to feature id of features to ignore
     */
    static QgsAbstractGeometry* avoidIntersections( const QgsAbstractGeometry& geom, QMap<QgsVectorLayer*, QSet<QgsFeatureId> > ignoreFeatures = ( QMap<QgsVectorLayer*, QSet<QgsFeatureId> >() ) );
};

#endif // QGSGEOMETRYEDITUTILS_H
