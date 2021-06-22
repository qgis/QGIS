/***************************************************************************
  qgstopologicalmesh.h - QgsTopologicalMesh

 ---------------------
 begin                : 18.6.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTOPOLOGICALMESH_H
#define QGSTOPOLOGICALMESH_H

#include "qgsmeshdataprovider.h"

SIP_NO_FILE

class QgsMeshEditingError;
class QgsMeshVertexCirculator;

/**
 * \ingroup core
 *
 * \brief Class that wraps a QgsMesh to ensure the consistency of the mesh during editing and help to access to elements from other elements
 *
 *  A topological face need to:
 *
 * - be convex
 * - counter clock wise
 * - not share an unique vertex with another face
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsTopologicalMesh
{
  public:

    using FaceNeighbors = QVector<int>;

    /**
     * \ingroup core
     *
     * \brief Class that contains independent faces an topological information about this faces
     *
     * \since QGIS 3.22
     */
    class TopologicalFaces
    {
      public:

        //! Returns faces
        QVector<QgsMeshFace>  meshFaces() const {return mFaces;}

        //! Clear all data contained in the instance.
        void clear();

      private:
        QVector<QgsMeshFace> mFaces; // the faces containing the vertices indexes in the mesh
        QVector<FaceNeighbors> mFacesNeighborhood; // neighborhood of the faces, face indexes are local
        QHash<int, int> mVerticesToFace; // map of vertices to incident face, face indexes are local
        QList<int> mBoundaries; // list of boundary vertices indexes in the mesh

        friend class QgsTopologicalMesh;
        friend class QgsMeshVertexCirculator;
    };


    /**
     * \ingroup core
     *
     * \brief Class that contains topological differences between two states of a topological mesh, only accessible from the QgsTopologicalMesh class
     *
     * \since QGIS 3.22
     */
    class Changes
    {
      public:

        //! Returns the face that are added with this changes
        QVector<QgsMeshFace> addedFaces() const;

        //! Returns the faces that are removed with this changes
        QVector<QgsMeshFace> removedFaces() const;

        //! Returns the indexes of the faces that are removed with this changes
        QList<int> removedFaceIndexes() const;

        //! Returns the added vertices with this changes
        QVector<QgsMeshVertex> addedVertices() const;

      private:
        int mAddedFacesFirstIndex = 0;
        QList<int> mFaceIndexesToRemove; // the removed faces indexes in the mesh
        QVector<QgsMeshFace> mFacesToAdd;
        QVector<FaceNeighbors> mFacesNeighborhoodToAdd;
        QVector<QgsMeshFace> mFacesToRemove;
        QVector<FaceNeighbors> mFacesNeighborhoodToRemove;
        QList<std::array<int, 4>> mNeighborhoodChanges; // {index of concerned face, neigbor position, previous value, changed value}

        QVector<QgsMeshVertex> mVerticesToAdd;
        QVector<int> mVertexToFaceToAdd;
        QList<int> mVerticesToRemoveIndexes;
        QList<QgsMeshVertex> mRemovedVertices;
        QList<int> mVerticesToFaceRemoved;
        QList<std::array<int, 3>> mVerticesToFaceChanges; // {index of concerned vertex, previous value, changed value}

        int addedFaceIndexInMesh( int internalIndex ) const;
        int removedFaceIndexInmesh( int internalIndex ) const;

        friend class QgsTopologicalMesh;
    };

    /**
     * Creates a topologicaly consistent mesh with \a mesh, this static method modifies \a mesh to be topological consistent
     * and return a topological mesh instance that contains and handeles this mesh (do not take ownership).
     */
    static QgsTopologicalMesh createTopologicalMesh( QgsMesh *mesh, QgsMeshEditingError &error );

    //! Creates new topological faces that are not yet included in the mesh
    TopologicalFaces  createNewTopologicalFaces( const QVector<QgsMeshFace> &faces, QgsMeshEditingError &error );

    //----------- access element methods

    //! Returns the indexes of neighbor faces of the face with index \a faceIndex
    QList<int> neighborsOfFace( int faceIndex ) const;

    //! Returns the indexes of faces that are around the vertex with index \a vertexIndex
    QList<int> facesAroundVertex( int vertexIndex ) const;

    //! Returns a pointer to the wrapped mesh
    QgsMesh *mesh() const;

    //! Returns whether the vertex is on a boundary
    bool isVertexOnBoundary( int vertexIndex ) const;

    //----------- editing methods

    //! Returns whether the faces can be added to the mesh
    QgsMeshEditingError canFacesBeAdded( const TopologicalFaces &topologicalFaces ) const;

    /**
     *  Adds faces \a topologicFaces to the topologic mesh.
     *  The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes addFaces( const TopologicalFaces &topologicFaces );

    /**
     *  Returns whether faces with index in \a faceIndexes can be removed/
     *  The method an error object with type QgsMeshEditingError::NoError if the faces can be removed, otherwise returns the corresponding error
     */
    QgsMeshEditingError canFacesBeRemoved( const QList<int> facesIndexes );

    /**
     *  Removes faces with index in \a faceIndexes.
     *  The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes removeFaces( const QList<int> facesIndexes );

    /**
     *  Add a \a vertex in the face with index \a faceIndex. The including face is removed and new faces surrounding the added vertex are added.
     *  The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes addVertexInface( int faceIndex, const QgsMeshVertex &vertex );

    /**
     *  Add a free \a vertex in the face, that is a vertex tha tis not included or linked with any faces.
     *  The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes addFreeVertex( const QgsMeshVertex &vertex );

    /**
     * Removes the vertex with index \a vertexIndex.
     * If the vertex in linked with faces, the operation leads also to remove the faces. In this case, if the flag \a fillHole is set to TRUE,
     * the hole is filled by a triangulation.
     * The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes removeVertex( int vertexIndex, bool fillHole = false );

    //! Applies the changes
    void applyChanges( const Changes &changes );

    //! Reverses the changes
    void reverseChanges( const Changes &changes );

    //! Check the topology of the face and set it counter clock wise if necessary
    static QgsMeshEditingError counterClockWiseFaces( QgsMeshFace &face, QgsMesh *mesh );

    //! Reindexes faces and vertices, after this operation, topological can't be edited anymore
    void reindex();

  private:

    //! Creates topological faces from mesh faces
    TopologicalFaces  createTopologicalFaces(
      const QVector<QgsMeshFace> &faces,
      QgsMeshEditingError &error,
      bool allowUniqueSharedVertex,
      bool writeInVertices );

    QgsMeshVertexCirculator vertexCirculator( int vertexIndex ) const;
    static bool facesCanBeJoinedWithCommonIndex( const QgsMeshFace &face1, const QgsMeshFace &face2, int commonIndex );

    QSet<int> concernedFacesBy( const QList<int> faceIndex ) const;

    //Attributes
    QgsMesh *mMesh = nullptr;
    QVector<int> mVertexToface;
    QVector<FaceNeighbors> mFacesNeighborhood;

    friend class QgsMeshVertexCirculator;

};

/**
 * \ingroup core
 *
 * \brief  Convenient class that turn around a vertex and provide information about faces and vertices
 *
 * \since QGIS 3.22
 */
class QgsMeshVertexCirculator
{
  public:

    //! Constructor with \a topologicalMesh and \a vertexIndex
    QgsMeshVertexCirculator( const QgsTopologicalMesh &topologicalMesh, int vertexIndex );

    //! Constructor with \a topologicFaces, \a vertexIndex and the index \a faceIndex of the face containing the vertex
    QgsMeshVertexCirculator( const QgsTopologicalMesh::TopologicalFaces &topologicalFaces, int faceIndex, int vertexIndex );

    //! Constructor with \a topologicFaces, \a vertexIndex and the index \a faceIndex of the face containing the vertex
    QgsMeshVertexCirculator( const QgsTopologicalMesh::TopologicalFaces &topologicalFaces, int vertexIndex );

    //! Turns counter clockwise around the vertex and returns the new current face, -1 if the circulator pass a boundary or circulator is invalid
    int turnCounterClockwise() const;

    //! Turns counter clockwise around the vertex and returns the new current face, -1 if the circulator pass a boundary or circulator is invalid
    int turnClockwise() const;

    //! Returns the current face index, -1 if the circulator has passed a boundary or circulator is invalid
    int currentFaceIndex() const;

    //! Returns the current face, empty face if  the circulator pass a boundary or circulator is invalid
    QgsMeshFace currentFace() const;

    //! Sets the circulator on the boundary face turning clockwise, return false is there isn't boundary face
    bool goBoundaryClockwise() const;

    //! Sets the circulator on the boundary face turning counter clockwise, return false is there isn't boundary face
    bool goBoundaryCounterClockwise() const;

    //! Returns the opposite vertex of the current face and on the edge on the side turning clockwise
    int oppositeVertexClockWise() const;

    //! Returns the opposite vertex of the current face and on the edge on the side turning counter clockwise
    int oppositeVertexCounterClockWise() const;

    //! Returns  whether the vertex circulator is valid
    bool isValid() const;

  private:
    const QVector<QgsMeshFace> mFaces;
    const QVector<QgsTopologicalMesh::FaceNeighbors> mFacesNeighborhood;
    const int mVertexIndex = -1;
    mutable int mCurrentFace = -1;
    mutable int mLastValidFace = -1;
    bool mIsValid = false;

    int positionInCurrentFace() const;
};

#endif // QGSTOPOLOGICALMESH_H
