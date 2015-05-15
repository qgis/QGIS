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

class QgsAbstractGeometryV2;
class QgsCurveV2;
class QgsGeometryEngine;
class QgsVectorLayer;

#include "qgsfeature.h"
#include <QMap>

/**Convenience functions for geometry editing*/
class QgsGeometryEditUtils
{
  public:
    /**Adds interior ring (taking ownership).
    @return 0 in case of success (ring added), 1 problem with geometry type, 2 ring not closed,
    3 ring is not valid geometry, 4 ring not disjoint with existing rings, 5 no polygon found which contained the ring*/
    static int addRing( QgsAbstractGeometryV2* geom, QgsCurveV2* ring );

    /**Adds part to multi type geometry (taking ownership)
    @return 0 in case of success, 1 if not a multigeometry, 2 if part is not a valid geometry, 3 if new polygon ring
    not disjoint with existing polygons of the feature*/
    static int addPart( QgsAbstractGeometryV2* geom, QgsAbstractGeometryV2* part );

    static bool deleteRing( QgsAbstractGeometryV2* geom, int ringNum, int partNum = 0 );

    static bool deletePart( QgsAbstractGeometryV2* geom, int partNum );

    static QgsAbstractGeometryV2* avoidIntersections( const QgsAbstractGeometryV2& geom, QMap<QgsVectorLayer*, QSet<QgsFeatureId> > ignoreFeatures = ( QMap<QgsVectorLayer*, QSet<QgsFeatureId> >() ) );

    static QgsGeometryEngine* createGeometryEngine( const QgsAbstractGeometryV2* geometry );
};

#endif // QGSGEOMETRYEDITUTILS_H
