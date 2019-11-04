/***************************************************************************
  qgistindemterraintilegeometry_p.h
  --------------------------------------
  Date                 : october 2019
  Copyright            : (C) 2019 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTINDEMTERRAINTILEGEOMETRY_P_H
#define QGSTINDEMTERRAINTILEGEOMETRY_P_H


#include <Qt3DExtras/qt3dextras_global.h>
#include <Qt3DRender/qgeometry.h>
#include <QVector3D>

#include "qgstriangularmesh.h"
#include "qgstindemterraintileloader_p.h"

namespace Qt3DRender
{
  class QAttribute;
  class QBuffer;
}

/**
 * \ingroup 3d
 * Stores attributes and vertex/index buffers for one terrain tile based on TIN DEM.
 * \since QGIS 3.12
 */
class QgsTinDemTerrainTileGeometry_p: public  Qt3DRender::QGeometry
{
  public:

    //! Constructs a terrain tile geometry from triangular mesh. Resolution is the number of vertices on one side of the tile,
    explicit QgsTinDemTerrainTileGeometry_p( QgsTriangularMeshTile meshTile,
        float verticaleScale, QNode *parent );

  private:
    void init();

    QgsTriangularMeshTile mTriangularMeshTile;
    QList<int> mFacesIndexes;
    QMap<int, int> mLocalIndexesMap;
    float mVertScale;

    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mNormalAttribute = nullptr;
    Qt3DRender::QAttribute *mTexCoordAttribute = nullptr;
    Qt3DRender::QAttribute *mIndexAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    Qt3DRender::QBuffer *mIndexBuffer = nullptr;
};

#endif // QGSTINDEMTERRAINTILEGEOMETRY_P_H
