/***************************************************************************
  qgistindemterraintileloader_p.h
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
#ifndef QGISTINDEMTERRAINTILELOADER_P_H
#define QGISTINDEMTERRAINTILELOADER_P_H

#include <QObject>
#include <QtConcurrent/QtConcurrentRun>
#include "qgsmeshlayer.h"
#include "qgstriangularmesh.h"
#include "qgsterraintileloader_p.h"
#include "qgstilingscheme.h"

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
    //! cosntruct a empty mesh tile
    QgsTriangularMeshTile() = default;

    //! Constructs a tile of triangles
    QgsTriangularMeshTile( QgsTriangularMesh triangularMesh,
                           const QgsRectangle &extent );

    //! Returns the number of faces in the mesh tile
    int faceCount() const;

    //! Returns the number of vertices in the mesh tile
    int verticesCount() const;

    //! Returns the triangle with local index vertex;
    QgsMeshFace triangle( int localTriangleIndex ) const;

    //! Returns the vertex with local index
    QgsMeshVertex vertex( int localIndex ) const;

    //! Returns the normal vector on vertex with local index
    QVector3D vertexUnitNormalVector( int localIndex ) const;

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

    QgsTriangularMesh mTriangularMesh ;
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
 * Generate asynchronusly ay triangular mesh tile
 * \since QGIS 3.12
 */
class QgsTriangularMeshTileGenerator: public QObject
{
    Q_OBJECT
  public:
    //! Constructor
    explicit QgsTriangularMeshTileGenerator( const QgsTriangularMesh &triangularMesh, const QgsRectangle &extent, QObject *parent = nullptr );

    //! run asynchronously the mesh tile generator
    void generate();

    //! Returns the generated meshTile
    QgsTriangularMeshTile meshTile();

  signals:
    //! emitted when the mesh tile is ready
    void meshTileIsReady();

  private:
    QFuture<QgsTriangularMeshTile> mMeshTileFuture;
    const QgsTriangularMesh mTriangularMesh;
    const QgsRectangle mExtent;
};


/**
 * \ingroup 3d
 * Chunk loader for TIN DEM terrain tiles.
 * \since QGIS 3.12
 */
class QgsTinDemTerrainTileLoader: public QgsTerrainTileLoader
{
    Q_OBJECT
  public:
    //! Constructs loader for the given chunk node
    QgsTinDemTerrainTileLoader( QgsTerrainEntity *terrain, QgsChunkNode *node, const QgsTriangularMesh &triangularMesh );
    Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private slots:
    void onMeshTileReady();

  private:
    QgsTriangularMeshTile mMeshTile;
    QgsTriangularMeshTileGenerator *meshTileGenerator = nullptr;
};



#endif // QGISTINDEMTERRAINTILELOADER_P_H
