/***************************************************************************
  qgstessellatedpolygongeometry.h
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

#ifndef QGSTESSELLATEDPOLYGONGEOMETRY_H
#define QGSTESSELLATEDPOLYGONGEOMETRY_H

#include "qgspolygon.h"

#include <Qt3DRender/QGeometry>

namespace Qt3DRender
{
  class QBuffer;
}

/** \ingroup 3d
 * Class derived from Qt3DRender::QGeometry that represents polygons tessellated into 3D geometry.
 *
 * Takes a list of polygons as input, internally it does tessellation and writes output to the internal
 * vertex buffer. Optionally it can add "walls" if the extrusion height is non-zero.
 *
 * \since QGIS 3.0
 */
class QgsTessellatedPolygonGeometry : public Qt3DRender::QGeometry
{
  public:
    //! Constructor
    QgsTessellatedPolygonGeometry( QNode *parent = nullptr );
    ~QgsTessellatedPolygonGeometry();

    //! Initializes vertex buffer from given polygons. Takes ownership of passed polygon geometries
    void setPolygons( const QList<QgsPolygonV2 *> &polygons, const QgsPointXY &origin, float extrusionHeight );

  private:
    QList<QgsPolygonV2 *> mPolygons;

    Qt3DRender::QAttribute *mPositionAttribute;
    Qt3DRender::QAttribute *mNormalAttribute;
    Qt3DRender::QBuffer *mVertexBuffer;

    bool mWithNormals;
};

#endif // QGSTESSELLATEDPOLYGONGEOMETRY_H
