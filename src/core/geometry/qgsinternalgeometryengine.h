/***************************************************************************
  qgsinternalgeometryengine.h - QgsInternalGeometryEngine

 ---------------------
 begin                : 13.1.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSINTERNALGEOMETRYENGINE_H
#define QGSINTERNALGEOMETRYENGINE_H

class QgsGeometry;
class QgsAbstractGeometry;

/**
 * \ingroup core
 * This class offers geometry processing methods.
 *
 * The methods are available via QgsGeometry::[geometryfunction]
 * and therefore this does not need to be accessed directly.
 *
 * @note not available in Python bindings
 */

class QgsInternalGeometryEngine
{
  public:

    /**
     * The caller is responsible that the geometry is available and unchanged
     * for the whole lifetime of this object.
     * @param geometry
     */
    explicit QgsInternalGeometryEngine( const QgsGeometry& geometry );

    /**
     * Will extrude a line or (segmentized) curve by a given offset and return a polygon
     * representation of it.
     *
     * @param x offset in x direction
     * @param y offset in y direction
     * @return an extruded polygon
     */
    QgsGeometry extrude( double x, double y ) const;

    /**
     * Calculates the approximate pole of inaccessibility for a surface, which is the
     * most distant internal point from the boundary of the surface. This function
     * uses the 'polylabel' algorithm (Vladimir Agafonkin, 2016), which is an iterative
     * approach guaranteed to find the true pole of inaccessibility within a specified
     * tolerance. More precise tolerances require more iterations and will take longer
     * to calculate.
     * Optionally, the distance to the polygon boundary from the pole can be stored.
     */
    QgsGeometry poleOfInaccessibility( double precision, double* distanceFromBoundary = nullptr ) const;

  private:
    const QgsAbstractGeometry* mGeometry;
};

#endif // QGSINTERNALGEOMETRYENGINE_H
