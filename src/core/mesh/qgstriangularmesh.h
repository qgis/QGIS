/***************************************************************************
                         qgstriangularmesh.h
                         -------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTRIANGULARMESH_H
#define QGSTRIANGULARMESH_H


#define SIP_NO_FILE

#include <QVector>
#include <QVector3D>
#include <QSet>
#include <QList>
#include <memory>
#include "qgis_core.h"
#include "qgsmeshdataprovider.h"
#include "qgsgeometry.h"
#include "qgsmeshspatialindex.h"
#include "qgstopologicalmesh.h"
#include "qgscoordinatetransform.h"

class QgsRenderContext;
class QgsRectangle;

/**
 * \ingroup core
 *
 * \brief Triangular/Derived Mesh is mesh with vertices in map coordinates.
 *
 * It creates spatial index for identification of a triangle that contains a particular point
 * on the map.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsTriangularMesh // TODO rename to QgsRendererMesh in QGIS 4
{
  public:
    //! Ctor
    QgsTriangularMesh();
    //! Dtor
    ~QgsTriangularMesh();

    /**
     * Constructs triangular mesh from layer's native mesh and transform to destination CRS. Populates spatial index.
     * \param nativeMesh QgsMesh to access native vertices and faces
     * \param transform Transformation from layer CRS to destination (e.g. map) CRS. With invalid transform, it keeps the native mesh CRS
     * \returns TRUE if the mesh is effectivly updated, and FALSE if not
    */
    bool update( QgsMesh *nativeMesh, const QgsCoordinateTransform &transform );

    /**
     * Constructs triangular mesh from layer's native mesh using the coordinate transform already set. Populates spatial index.
     * \param nativeMesh QgsMesh to access native vertices and faces
     *
     * \returns TRUE if the mesh is effectivly updated, and FALSE if not
     *
     * \note if the coordinate transform is not already set, it uses the native mesh CRS
     *
     * \since QGIS 3.28
    */
    bool update( QgsMesh *nativeMesh );

    /**
     * Returns vertices in map coordinate system
     *
     * The list of consist of vertices from native mesh (0-N) and
     * extra vertices needed to create triangles (N+1 - len)
     */
    const QVector<QgsMeshVertex> &vertices() const ;

    //! Returns triangles
    const QVector<QgsMeshFace> &triangles() const ;

    //! Returns edges
    const QVector<QgsMeshEdge> &edges() const ;

    /**
     * Returns centroids of the native faces in map CRS
     *
     * \deprecated since QGIS 3.14 use faceCentroids() instead
     */
    Q_DECL_DEPRECATED const QVector<QgsMeshVertex> &centroids() const ;

    /**
     * Returns centroids of the native faces in map CRS
     * \since QGIS 3.14
     */
    const QVector<QgsMeshVertex> &faceCentroids() const ;

    /**
     * Returns centroids of the native edges in map CRS
     * \since QGIS 3.14
     */
    const QVector<QgsMeshVertex> &edgeCentroids() const ;

    //! Returns mapping between triangles and original faces
    const QVector<int> &trianglesToNativeFaces() const ;

    //! Returns mapping between edges and original edges
    const QVector<int> &edgesToNativeEdges() const ;

    /**
     * Transforms a point from layer coordinates system to triangular Mesh coordinates system.
     *
     * \param point point in layer coordinate system
     * \returns point in triangular mesh coordinate
     *
     * \since QGIS 3.22
     */
    QgsPointXY transformFromLayerToTrianglesCoordinates( const QgsPointXY &point ) const;

    /**
     * Finds index of triangle at given point
     * It uses spatial indexing
     *
     * \param point point in map coordinate system
     * \returns triangle index that contains the given point, -1 if no such triangle exists
     *
     * \since QGIS 3.4
     */
    int faceIndexForPoint( const QgsPointXY &point ) const ;

    /**
     * Finds index of triangle at given point
     * It uses spatial indexing and don't use geos to be faster
     *
     * \param point point in map coordinate system
     * \returns triangle index that contains the given point, -1 if no such triangle exists
     *
     * \since QGIS 3.12
     */
    int faceIndexForPoint_v2( const QgsPointXY &point ) const;

    /**
     * Finds index of native face at given point
     * It uses spatial indexing
     *
     * \param point point in map coordinate system
     * \returns native face index that contains the given point, -1 if no such faces exists
     *
     * \since QGIS 3.22
     */
    int nativeFaceIndexForPoint( const QgsPointXY &point ) const ;

    /**
     * Finds indexes of native faces which bounding boxes intersect given bounding box
     * It uses spatial indexing
     *
     * \param rectangle bounding box in map coordinate system
     * \returns native face indexes that have bounding box that intersect the rectangle
     *
     * \since QGIS 3.22
     */
    QList<int> nativeFaceIndexForRectangle( const QgsRectangle &rectangle ) const ;

    /**
     * Finds indexes of triangles intersecting given bounding box
     * It uses spatial indexing
     *
     * \param rectangle bounding box in map coordinate system
     * \returns triangle indexes that intersect the rectangle
     *
     * \since QGIS 3.4
     */
    QList<int> faceIndexesForRectangle( const QgsRectangle &rectangle ) const ;

    /**
     * Finds indexes of edges intersecting given bounding box
     * It uses spatial indexing
     *
     * \param rectangle bounding box in map coordinate system
     * \returns edges indexes that intersect the rectangle
     */
    QList<int> edgeIndexesForRectangle( const QgsRectangle &rectangle ) const ;

    /**
     * Calculates and returns normal vector on each vertex that is part of any face
     *
     * \returns all normals at vertices
     *
     * \since QGIS 3.12
     */

    QVector<QVector3D> vertexNormals( float vertScale ) const;

    /**
     * Returns simplified meshes.
     * The first simplified mesh is simplified with a goal of a number of triangles equal to the
     * number of triangles of the base mesh divided by the reduction factor. For the following mesh the same reduction factor is used with
     * the preceding goal of number of triangles.
     * There are as many simplified meshes as necessary to have the minimum triangles count on the last simplified mesh.
     *
     * The caller has to take the ownership of returned meshes.
     *
     * Not implemented for Edge meshes and Mixed meshes
     *
     * \param reductionFactor is the factor used to reduce the number of triangles of the mesh
     * \param minimumTrianglesCount is the minimal faces count on simplified mesh
     * \since QGIS 3.14
     */
    QVector<QgsTriangularMesh *> simplifyMesh( double reductionFactor, int minimumTrianglesCount = 10 ) const;

    /**
     * Returns the average size of triangles in map unit. It is calculated using the maximum of the height/width of the
     * bounding box of each triangles.
     *
     * \since QGIS 3.14
     */
    double averageTriangleSize() const;

    /**
     * Returns the corresponding index of level of detail on which this mesh is associated
     *
     * - 0: base mesh
     * - 1: first simplified mesh
     * - 2: second simplified mesh (lower level of detail)
     * - ...
     *
     * \since QGIS 3.14
     */
    int levelOfDetail() const;

    /**
     * Returns the extent of the triangular mesh in map coordinates
     *
     * \returns bounding box of the triangular mesh
     *
     * \since QGIS 3.14
     */
    QgsRectangle extent() const;

    /**
     * Returns whether the mesh contains mesh elements of given type
     *  \since QGIS 3.14
     */
    bool contains( const QgsMesh::ElementType &type ) const;

    /**
     * \ingroup core
     *
     * \brief The Changes class is used to make changes of the triangular and to keep traces of this changes
     *        If a Changes instance is applied (see QgsTriangularMesh::applyChanges()),
     *        these changes can be reversed (see QgsTriangularMesh::reverseChanges()) as long as other changes are not applied
     *
     * \since QGIS 3.22
     */
    class Changes
    {
      public:

        //! Default constructor, no changes
        Changes() = default;

        /**
         * Constructor of the triangular changes from \a topological changes and with the native mesh \a nativeMesh
         */
        Changes( const QgsTopologicalMesh::Changes &topologicalChanges, const QgsMesh &nativeMesh );

      private:
        // triangular mesh elements that can be changed if the triangular mesh is updated, are not stored and have to be retrieve
        QVector<QgsMeshVertex> mAddedVertices;
        QList<int> mVerticesIndexesToRemove;
        QList<int> mChangedVerticesCoordinates;
        mutable QList<double> mOldZValue;
        QList<double> mNewZValue;
        QList<QgsPointXY> mOldXYValue;
        QList<QgsPointXY> mNewXYValue;

        QVector<QgsMeshFace> mNativeFacesToAdd;
        QList<int> mNativeFaceIndexesToRemove;
        QVector<QgsMeshFace> mNativeFacesToRemove;
        QList<int> mNativeFaceIndexesGeometryChanged;
        QVector<QgsMeshFace> mNativeFacesGeometryChanged;
        mutable QList<int> mTriangleIndexesGeometryChanged;

        mutable int mTrianglesAddedStartIndex = 0;  // defined each time the changes are apply
        mutable QList<int> mRemovedTriangleIndexes; // defined when changes are apply the first time

        friend class QgsTriangularMesh;
    };

    /**
     * Applies the changes on the triangular mesh (see Changes)
     *
     * \since QGIS 3.22
     */
    void applyChanges( const Changes &changes );

    /**
     * Reverses the changes on the triangular mesh (see Changes)
     *
     * \since QGIS 3.22
     */
    void reverseChanges( const Changes &changes, const QgsMesh &nativeMesh );

    /**
     * Transforms the \a vertex from native coordinates system to triangular mesh coordinates system.
     * Returns empty vertex if the transform fails.
     *
     * \since QGIS 3.22
     */
    QgsMeshVertex nativeToTriangularCoordinates( const QgsMeshVertex &vertex ) const;

    /**
     * Transforms the \a vertex from triangular mesh coordinates system to native coordinates system.
     * Returns empty vertex if the transform fails.
     *
     * \since QGIS 3.22
     */
    QgsMeshVertex triangularToNativeCoordinates( const QgsMeshVertex &vertex ) const;

    /**
     *
     * Returns the extent of the mesh in the native mesh coordinates system, returns empty extent if the transformation fails
     *
     * \since QGIS 3.22
     */
    QgsRectangle nativeExtent();

  private:

    /**
     * Triangulates native face to triangles
     *
     * Triangulation does not create any new vertices and uses
     * "Ear clipping method". Number of vertices in face is usually
     * less than 10 and the faces are usually convex and without holes
     *
     * Skips the input face if it is not possible to triangulate
     * with the given algorithm (e.g. only 2 vertices, polygon with holes)
     */
    void triangulate( const QgsMeshFace &face, int nativeIndex );

    void addVertex( const QgsMeshVertex &vertex );

    QgsMeshVertex transformVertex( const QgsMeshVertex &vertex, Qgis::TransformDirection direction ) const;

    // calculate the centroid of the native mesh, mNativeMeshCentroids container must have the emplacment for the corresponding centroid before calling this method
    QgsMeshVertex calculateCentroid( const QgsMeshFace &nativeFace );

    // check clock wise and calculate average size of triangles
    void finalizeTriangles();

    // vertices: map CRS; 0-N ... native vertices, N+1 - len ... extra vertices
    // faces are derived triangles
    QgsMesh mTriangularMesh;
    QVector<int> mTrianglesToNativeFaces; //len(mTrianglesToNativeFaces) == len(mTriangles). Mapping derived -> native
    QVector<int> mEdgesToNativeEdges; //len(mEdgesToNativeEdges) == len(mEdges). Mapping derived -> native

    // centroids of the native faces in map CRS
    QVector<QgsMeshVertex> mNativeMeshFaceCentroids;

    // centroids of the native edges in map CRS
    QVector<QgsMeshVertex> mNativeMeshEdgeCentroids;

    QgsMeshSpatialIndex mSpatialFaceIndex;
    QgsMeshSpatialIndex mSpatialEdgeIndex;
    QgsCoordinateTransform mCoordinateTransform; //coordinate transform used to convert native mesh vertices to map vertices

    mutable QgsRectangle mExtent;
    mutable bool mIsExtentValid = false;

    // average size of the triangles
    double mAverageTriangleSize = 0;
    int mLod = 0;

    const QgsTriangularMesh *mBaseTriangularMesh = nullptr;

    friend class TestQgsTriangularMesh;
};

namespace QgsMeshUtils
{
  //! Returns face as polygon geometry
  CORE_EXPORT QgsGeometry toGeometry( const QgsMeshFace &face, const QVector<QgsMeshVertex> &vertices );

  //! Returns the centroid of the \a face
  CORE_EXPORT QgsMeshVertex centroid( const QgsMeshFace &face, const QVector<QgsMeshVertex> &vertices );

  //! Returns face as polygon geometry, caller is responsible for delete
  CORE_EXPORT std::unique_ptr< QgsPolygon > toPolygon( const QgsMeshFace &face, const QVector<QgsMeshVertex> &vertices );

  /**
   * Returns unique native faces indexes from list of triangle indexes
   * \since QGIS 3.4
   */
  CORE_EXPORT QSet<int> nativeFacesFromTriangles( const QList<int> &triangleIndexes, const QVector<int> &trianglesToNativeFaces );

  /**
   * Returns unique native faces indexes from list of triangle indexes
   * \since QGIS 3.14
   */
  CORE_EXPORT QSet<int> nativeEdgesFromEdges( const QList<int> &edgesIndexes, const QVector<int> &edgesToNativeEdges );

  /**
   * Returns unique native vertex indexes from list of vertices of triangles
   * \since QGIS 3.14
   */
  CORE_EXPORT QSet<int> nativeVerticesFromTriangles( const QList<int> &triangleIndexes, const QVector<QgsMeshFace> &triangles );

  /**
   * Returns unique native faces indexes from list of vertices of triangles
   * \since QGIS 3.14
   */
  CORE_EXPORT QSet<int> nativeVerticesFromEdges( const QList<int> &edgesIndexes, const QVector<QgsMeshEdge> &edges );

  /**
   * Tests if point p is on the face defined with vertices
   * \since QGIS 3.12
  */
  bool isInTriangleFace( const QgsPointXY point, const QgsMeshFace &face,  const QVector<QgsMeshVertex> &vertices );

  /**
   * Checks if the triangle is counter clockwise, if not sets it counter clockwise
   * \since QGIS 3.22
  */
  void setCounterClockwise( QgsMeshFace &triangle, const QgsMeshVertex &v0, const QgsMeshVertex &v1, const QgsMeshVertex &v2 );

};

#endif // QGSTRIANGULARMESH_H
