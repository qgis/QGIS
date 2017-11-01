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

#include "qgis_3d.h"

class QgsPolygon;

#include <QVector>


/**
 * \ingroup 3d
 * Class that takes care of tessellation of polygons into triangles.
 *
 * It is expected that client code will create the tessellator object, then repeatedly call
 * addPolygon() method that will generate triangles, and finally call data() to get final vertex data.
 *
 * Optionally provides extrusion by adding triangles that serve as walls when extrusion height is non-zero.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsTessellator
{
  public:
    //! Creates tessellator with a specified origin point of the world (in map coordinates)
    QgsTessellator( double originX, double originY, bool addNormals );

    //! Tessellates a triangle and adds its vertex entries to the output data array
    void addPolygon( const QgsPolygon &polygon, float extrusionHeight );

    //! Returns array of triangle vertex data
    QVector<float> data() const { return mData; }
    //! Returns size of one vertex entry in bytes
    int stride() const { return mStride; }

  private:
    double mOriginX, mOriginY;
    bool mAddNormals;
    QVector<float> mData;
    int mStride;
};

#endif // QGSTESSELLATOR_H
