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


class QgsTriangularMeshTile
{
    struct LocalVertex
    {
      int globalIndex;
      QVector3D normalVector;
    };

  public:
    QgsTriangularMeshTile( QgsTriangularMesh *triangularMesh, const QgsRectangle &extent );
    int faceCount() const;

    int verticesCount() const;

    //! return the triangle with local index vertex;
    QgsMeshFace triangle( int localTriangleIndex ) const;

    //! return the vertex with local index
    QgsMeshVertex vertex( int localIndex ) const;

    //! return teh normal vector at vertex with local index
    QVector3D vertexNormalVector( int localIndex ) const;

    float zMinimum() const;
    float zMaximum() const;

    bool operator==( const QgsTriangularMeshTile &other ) const;

    QgsRectangle tileExtent() const {return mExtent;}
    QgsRectangle realTileExtent() const {return mRealExtent;}

  private:
    void init();

    QgsTriangularMesh *mTriangularMesh;
    QgsRectangle mExtent;
    QgsRectangle mRealExtent;

    QVector<int> mFaces;

    //! used to store the global index of the vertrices, the sum of the normales of associated triangles (weighted with area)
    //! and the sum of the area of asociated triangles.
    QVector<LocalVertex> mVertices;

    //! used to retrive the local index from the glogal index
    QHash<int, int> mLocalIndexFromGlobalIndex;

    float mZmax;
    float mZmin;
};


class QgsTinDemTerrainTileGeometry_p: public  Qt3DRender::QGeometry
{
  public:
    explicit QgsTinDemTerrainTileGeometry_p( QgsTriangularMeshTile meshTile,
        float verticaleScale, QNode *parent );
    ~QgsTinDemTerrainTileGeometry_p()
    {
    }


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
