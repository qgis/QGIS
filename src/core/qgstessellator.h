/***************************************************************************
  qgstessellator.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTESSELLATOR_H
#define QGSTESSELLATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsPolygon;
class QgsMultiPolygon;

#include <QVector>
#include <memory>

/**
 * \ingroup core
 * Class that takes care of tessellation of polygons into triangles.
 *
 * It is expected that client code will create the tessellator object, then repeatedly call
 * addPolygon() method that will generate triangles, and finally call data() to get final vertex data.
 *
 * Optionally provides extrusion by adding triangles that serve as walls when extrusion height is non-zero.
 *
 * \since QGIS 3.4 (since QGIS 3.0 in QGIS_3D library)
 */
class CORE_EXPORT QgsTessellator
{
  public:
    //! Creates tessellator with a specified origin point of the world (in map coordinates)
    QgsTessellator( double originX, double originY, bool addNormals, bool invertNormals = false, bool addBackFaces = false );

    //! Tessellates a triangle and adds its vertex entries to the output data array
    void addPolygon( const QgsPolygon &polygon, float extrusionHeight );

    /**
     * Returns array of triangle vertex data
     *
     * Vertice coordinates are stored as (x, z, -y)
     */
    QVector<float> data() const { return mData; }

    //! Returns the number of vertices stored in the output data array
    int dataVerticesCount() const;

    //! Returns size of one vertex entry in bytes
    int stride() const { return mStride; }

    /**
     * Returns the triangulation as a multipolygon geometry.
     */
    std::unique_ptr< QgsMultiPolygon > asMultiPolygon() const SIP_SKIP;

  private:
    double mOriginX = 0, mOriginY = 0;
    bool mAddNormals = false;
    bool mInvertNormals = false;
    bool mAddBackFaces = false;
    QVector<float> mData;
    int mStride;
};

#endif // QGSTESSELLATOR_H
