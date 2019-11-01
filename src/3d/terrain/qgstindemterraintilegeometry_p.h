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

namespace Qt3DRender
{
  class QAttribute;
  class QBuffer;
}

/**
 * \ingroup 3d
 * Stores a tile of a triangular mesh (vertex, faces and normal vector) for terrain 3D visualisation
 * \since QGIS 3.12
 */
class QgsTriangularMeshTile
{
    struct LocalVertex
    {
      int globalIndex;
      QVector3D normalVector;
    };

  public:
    //! Constructs a tile of triangles
    QgsTriangularMeshTile( QgsTriangularMesh *triangularMesh, const QgsRectangle &extent );

    //! Returns the number of faces in the mesh tile
    int faceCount() const;

    //! Returns the number of vertices in the mesh tile
    int verticesCount() const;

    //! Returns the triangle with local index vertex;
    QgsMeshFace triangle( int localTriangleIndex ) const;

    //! Returns the vertex with local index
    QgsMeshVertex vertex( int localIndex ) const;

    //! Returns the normal vector on vertex with local index
    QVector3D vertexNormalVector( int localIndex ) const;

    //! Returns the minimum z value
    float zMinimum() const;

    //! Returns the maximum z value
    float zMaximum() const;

    //! Relational equal operator
    bool operator==( const QgsTriangularMeshTile &other ) const;

    //! Returns the extent which serves to define the tile (the tile extent)
    QgsRectangle tileExtent() const {return mExtent;}

    //! Returns the reeal extent of the tile with triangles that exceed the extent tile
    QgsRectangle realTileExtent() const {return mRealExtent;}

  private:
    void init();

    QgsTriangularMesh *mTriangularMesh = nullptr;
    QgsRectangle mExtent;
    QgsRectangle mRealExtent;

    QVector<int> mFaces;

    //! used to store the global index of  vertices, and normal vectors
    QVector<LocalVertex> mVertices;

    //! used to retrieve the local index from the glogal index
    QHash<int, int> mLocalIndexFromGlobalIndex;

    float mZmax = 0;
    float mZmin = 0;
};


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
