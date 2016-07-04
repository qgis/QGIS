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

#include "qgsgeometry.h"

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
    QgsGeometry extrude( double x, double y );

  private:
    const QgsAbstractGeometryV2* mGeometry;
};

#endif // QGSINTERNALGEOMETRYENGINE_H
