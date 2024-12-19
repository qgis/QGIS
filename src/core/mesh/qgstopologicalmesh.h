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

#include <QSet>

#include "qgsmeshdataprovider.h"

#if defined(_MSC_VER)
template CORE_EXPORT QVector<int> SIP_SKIP;
template CORE_EXPORT QList<int> SIP_SKIP;
template CORE_EXPORT QVector<QVector<int>> SIP_SKIP;
#endif

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
     * This class supports unique shared vertex between faces
     *
     * \since QGIS 3.22
     */
    class CORE_EXPORT TopologicalFaces
    {
      public:

        //! Returns faces
        QVector<QgsMeshFace>  meshFaces() const {return mFaces;}

        //! Clears all data contained in the instance.
        void clear();

        //! Returns the face neighborhood of the faces, indexing is local
        QVector<FaceNeighbors> facesNeighborhood() const;

        //! Returns a face linked to the vertices with index \a vertexIndex
        int vertexToFace( int vertexIndex ) const;

      private:
        QVector<QgsMeshFace> mFaces; // the faces containing the vertices indexes in the mesh
        QVector<FaceNeighbors> mFacesNeighborhood; // neighborhood of the faces, face indexes are local
        QMultiHash<int, int> mVerticesToFace; // map of vertices to incident face, face indexes are local
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
    class CORE_EXPORT Changes
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

        //! Returns the indexes of vertices to remove
        QList<int> verticesToRemoveIndexes() const;

        //! Returns the indexes of vertices that have changed coordinates
        QList<int> changedCoordinatesVerticesIndexes() const;

        //! Returns the new Z values of vertices that have changed their coordinates
        QList<double> newVerticesZValues() const;

        //! Returns the new (X,Y) values of vertices that have changed their coordinates
        QList<QgsPointXY> newVerticesXYValues() const;

        //! Returns the old (X,Y) values of vertices that have changed their coordinates
        QList<QgsPointXY> oldVerticesXYValues() const;

        //! Returns a list of the native face indexes that have a geometry changed
        QList<int> nativeFacesIndexesGeometryChanged() const;

        //! Returns whether changes are empty, that there is nothing to change
        bool isEmpty() const;

      protected:
        int mAddedFacesFirstIndex = 0;
        QList<int> mFaceIndexesToRemove; // the removed faces indexes in the mesh
        QVector<QgsMeshFace> mFacesToAdd;
        QVector<FaceNeighbors> mFacesNeighborhoodToAdd;
        QVector<QgsMeshFace> mFacesToRemove;
        QVector<FaceNeighbors> mFacesNeighborhoodToRemove;
        QList<std::array<int, 4>> mNeighborhoodChanges; // {index of concerned face, neighbor position, previous value, changed value}

        QVector<QgsMeshVertex> mVerticesToAdd;
        QVector<int> mVertexToFaceToAdd;
        QList<int> mVerticesToRemoveIndexes;
        QList<QgsMeshVertex> mRemovedVertices;
        QList<int> mVerticesToFaceRemoved;
        QList<std::array<int, 3>> mVerticesToFaceChanges; // {index of concerned vertex, previous value, changed value}

        QList<int> mChangeCoordinateVerticesIndexes;
        QList<double> mNewZValues;
        QList<double> mOldZValues;
        QList<QgsPointXY> mNewXYValues;
        QList<QgsPointXY> mOldXYValues;
        QList<int> mNativeFacesIndexesGeometryChanged;

        //! Clears all changes
        void clearChanges();

      private:
        int addedFaceIndexInMesh( int internalIndex ) const;
        int removedFaceIndexInMesh( int internalIndex ) const;

        friend class QgsTopologicalMesh;
    };

    /**
     * Creates a topologicaly consistent mesh with \a mesh, this static method modifies \a mesh to be topological consistent
     * and return a QgsTopologicalMesh instance that contains and handles this mesh (does not take ownership).
     */
    static QgsTopologicalMesh createTopologicalMesh( QgsMesh *mesh, int maxVerticesPerFace, QgsMeshEditingError &error );

    //! Creates new topological faces that are not yet included in the mesh
    static TopologicalFaces  createNewTopologicalFaces( const QVector<QgsMeshFace> &faces, bool uniqueSharedVertexAllowed, QgsMeshEditingError &error );

    //----------- access element methods

    //! Returns the indexes of neighbor faces of the face with index \a faceIndex
    QVector<int> neighborsOfFace( int faceIndex ) const;

    //! Returns the indexes of faces that are around the vertex with index \a vertexIndex
    QList<int> facesAroundVertex( int vertexIndex ) const;

    //! Returns a pointer to the wrapped mesh
    QgsMesh *mesh() const;

    //! Returns the index of the first face linked, returns -1 if it is a free vertex or out of range index
    int firstFaceLinked( int vertexIndex ) const;

    //! Returns whether the vertex is on a boundary
    bool isVertexOnBoundary( int vertexIndex ) const;

    //! Returns whether the vertex is a free vertex
    bool isVertexFree( int vertexIndex ) const;

    //! Returns a list of vertices are not linked to any faces
    QList<int> freeVerticesIndexes() const;

    //! Returns a vertex circulator linked to this mesh around the vertex with index \a vertexIndex
    QgsMeshVertexCirculator vertexCirculator( int vertexIndex ) const;

    //----------- editing methods

    //! Returns whether the faces can be added to the mesh
    QgsMeshEditingError facesCanBeAdded( const TopologicalFaces &topologicalFaces ) const;

    /**
     * Adds faces \a topologicFaces to the topologic mesh.
     * The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes addFaces( const TopologicalFaces &topologicFaces );

    /**
     * Returns whether faces with index in \a faceIndexes can be removed/
     * The method an error object with type QgsMeshEditingError::NoError if the faces can be removed, otherwise returns the corresponding error
     */
    QgsMeshEditingError facesCanBeRemoved( const QList<int> &facesIndexes );

    /**
     * Removes faces with index in \a faceIndexes.
     * The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes removeFaces( const QList<int> &facesIndexes );

    /**
     * Returns TRUE if the edge can be flipped (only available for edge shared by two faces with 3 vertices)
     */
    bool edgeCanBeFlipped( int vertexIndex1, int vertexIndex2 ) const;

    /**
     * Flips edge (\a vertexIndex1, \a vertexIndex2)
     * The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes flipEdge( int vertexIndex1, int vertexIndex2 );

    /**
     * Returns TRUE if faces separated by vertices with indexes \a vertexIndex1 and \a vertexIndex2 can be merged
     */
    bool canBeMerged( int vertexIndex1, int vertexIndex2 ) const;

    /**
     * Merges faces separated by vertices with indexes \a vertexIndex1 and \a vertexIndex2
     * The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes merge( int vertexIndex1, int vertexIndex2 );

    /**
     * Returns TRUE if face with index \a faceIndex can be split
     */
    bool canBeSplit( int faceIndex ) const;

    /**
     * Splits face with index \a faceIndex
     * The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes splitFace( int faceIndex );

    /**
     * Adds a \a vertex in the face with index \a faceIndex. The including face is removed and new faces surrounding the added vertex are added.
     * The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes addVertexInFace( int faceIndex, const QgsMeshVertex &vertex );

    /**
     * Inserts a \a vertex in the edge of face with index \a faceIndex at \a position . The faces that are on each side of the edge are removed and replaced
     * by new faces constructed by a triangulation.
     */
    Changes insertVertexInFacesEdge( int faceIndex, int position, const QgsMeshVertex &vertex );

    /**
     * Adds a free \a vertex in the face, that is a vertex that is not included or linked with any faces.
     * The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes addFreeVertex( const QgsMeshVertex &vertex );

    /**
     * Removes the vertex with index \a vertexIndex.
     * If the vertex in linked with faces, the operation leads also to remove the faces. In this case, the hole is filled by a triangulation.
     * The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes removeVertexFillHole( int vertexIndex );

    /**
     * Removes all the vertices with index in the list \a vertices
     * If vertices in linked with faces, the operation leads also to remove the faces without filling holes.
     * The method returns a instance of the class QgsTopologicalMesh::Change that can be used to reverse or reapply the operation.
     */
    Changes removeVertices( const QList<int> &vertices );

    /**
     * Changes the Z values of the vertices with indexes in \a vertices indexes with the values in \a newValues
     */
    Changes changeZValue( const QList<int> &verticesIndexes, const QList<double> &newValues );

    /**
     * Changes the (X,Y) values of the vertices with indexes in \a vertices indexes with the values in \a newValues
     */
    Changes changeXYValue( const QList<int> &verticesIndexes, const QList<QgsPointXY> &newValues );


    //! Applies the changes
    void applyChanges( const Changes &changes );

    //! Reverses the changes
    void reverseChanges( const Changes &changes );

    //! Checks the topology of the face and sets it counter clockwise if necessary
    static QgsMeshEditingError counterClockwiseFaces( QgsMeshFace &face, QgsMesh *mesh );

    /**
     * Checks the topology of the \a vertices as they are contained in a face and returns indication on direction.
     * If the face is clockwise, \a clockwise is TRUE
     *
     * \since QGIS 3.30
     */
    static QgsMeshEditingError checkTopologyOfVerticesAsFace( const QVector<QgsMeshVertex> &vertices, bool &clockwise );

    /**
     * Reindexes faces and vertices, after this operation, the topological
     * mesh can't be edited anymore and only the method mesh can be used to access to the raw mesh.
     */
    void reindex();

    /**
     * Renumbers the  indexes of vertices and faces using the Reverse CutHill McKee Algorithm
     */
    bool renumber();

    //! Checks the consistency of the topological mesh and return FALSE if there is a consistency issue
    QgsMeshEditingError checkConsistency() const;

    //! Checks the topology of the mesh \a mesh, if error occurs, this mesh can't be edited
    static QgsMeshEditingError checkTopology( const QgsMesh &mesh, int maxVerticesPerFace );

  private:

    //! Creates topological faces from mesh faces
    static TopologicalFaces  createTopologicalFaces(
      const QVector<QgsMeshFace> &faces,
      QVector<int> *globalVertexToFace,
      QgsMeshEditingError &error,
      bool allowUniqueSharedVertex );

    //! Returns all faces indexes that are concerned by the face with index in \a faceIndex, that is sharing a least one vertex or one edge
    QSet<int> concernedFacesBy( const QList<int> &faceIndexes ) const;

    //! References the vertex as a free vertex to be able to access to all free vertices
    void referenceAsFreeVertex( int vertexIndex );
    //! References the vertex as a free vertex
    void dereferenceAsFreeVertex( int vertexIndex );

    /**
     * Returns faces that are either side of th edge (\a vertexIndex1, \a vertexIndex2)
     * and neighbor vertices of entry vertex in each faces
     */
    bool eitherSideFacesAndVertices( int vertexIndex1,
                                     int vertexIndex2,
                                     int &face1,
                                     int &face2,
                                     int &neighborVertex1InFace1,
                                     int &neighborVertex1InFace2,
                                     int &neighborVertex2inFace1,
                                     int &neighborVertex2inFace2 ) const;

    bool renumberVertices( QVector<int> &oldToNewIndex ) const;
    bool renumberFaces( QVector<int> &oldToNewIndex ) const;

    //Attributes
    QgsMesh *mMesh = nullptr;
    QVector<int> mVertexToFace;
    QVector<FaceNeighbors> mFacesNeighborhood;

    QSet<int> mFreeVertices;

    int mMaximumVerticesPerFace = 0;

    friend class QgsMeshVertexCirculator;

};

/**
 * \ingroup core
 *
 * \brief  Convenient class that turn around a vertex and provide information about faces and vertices
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsMeshVertexCirculator
{
  public:

    //! Constructor with \a topologicalMesh and \a vertexIndex
    QgsMeshVertexCirculator( const QgsTopologicalMesh &topologicalMesh, int vertexIndex );

    /**
     * Constructor with \a topologicFaces, \a vertexIndex and the index \a faceIndex of the face containing the vertex
     * \note This circulator only concerns faces that are in the same bloc of the face \a faceIndex. Other faces that could be share only
     * the vertex \a vertexIndex can't be accessible with this circulator
     */
    QgsMeshVertexCirculator( const QgsTopologicalMesh::TopologicalFaces &topologicalFaces, int faceIndex, int vertexIndex );

    /**
     * Constructor with \a topologicFaces, \a vertexIndex
     * \note This circulator only concerns faces that are in the same bloc of the first face linked to the vertex \a vertexIndex.
     * Other faces that could be share only the vertex \a vertexIndex can't be accessible with this circulator
     */
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
    int oppositeVertexClockwise() const;

    //! Returns the opposite vertex of the current face and on the edge on the side turning counter clockwise
    int oppositeVertexCounterClockwise() const;

    //! Returns  whether the vertex circulator is valid
    bool isValid() const;

    //! Returns all the faces indexes around the vertex
    QList<int> facesAround() const;

    //! Returns the degree of the vertex, that is the count of other vertices linked
    int degree() const;

  private:
    const QVector<QgsMeshFace> mFaces;
    const QVector<QgsTopologicalMesh::FaceNeighbors> mFacesNeighborhood;
    const int mVertexIndex = -1;
    mutable int mCurrentFace = -1;
    mutable int mLastValidFace = -1;
    bool mIsValid = false;
    mutable int mDegree = -1;

    int positionInCurrentFace() const;
};

#endif // QGSTOPOLOGICALMESH_H
