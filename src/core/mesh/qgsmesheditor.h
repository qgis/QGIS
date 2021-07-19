/***************************************************************************
  qgsmesheditor.h - QgsMeshEditor

 ---------------------
 begin                : 8.6.2021
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
#ifndef QGSMESHEDITOR_H
#define QGSMESHEDITOR_H

#include <QObject>
#include <QUndoCommand>
#include <QPointer>

#include "qgis.h"
#include "qgsmeshdataset.h"
#include "qgsmeshdataprovider.h"
#include "qgstriangularmesh.h"
#include "qgstopologicalmesh.h"


class QgsMeshAdvancedEditing;

/**
 * \ingroup core
 *
 * \brief Class that represents an error during mesh editing
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsMeshEditingError
{
  public:

    //! Constructor of the default error, that is NoError
    QgsMeshEditingError();

    //! Constructor with eht error \a type and the index of the element \a elementIndex
    QgsMeshEditingError( Qgis::MeshEditingErrorType type, int elementIndex );

    Qgis::MeshEditingErrorType errorType;

    int elementIndex;

    bool operator==( const QgsMeshEditingError &other ) const {return ( other.errorType == errorType && other.elementIndex == elementIndex );}
    bool operator!=( const QgsMeshEditingError &other ) const {return !operator==( other );}
};

/**
 * \ingroup core
 *
 * \brief Class that makes edit operation on a mesh
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsMeshEditor : public QObject
{
    Q_OBJECT
  public:

    //! Constructor with a specified layer \a meshLayer
    QgsMeshEditor( QgsMeshLayer *meshLayer );

    //! Constructoe with a specific mesh \a nativeMesh and an associatd triangular mesh \a triangularMesh
    QgsMeshEditor( QgsMesh *nativeMesh, QgsTriangularMesh *triangularMesh, QObject *parent = nullptr ); SIP_SKIP
    ~QgsMeshEditor();

    //! Initialize the mesh editor and return errors if the internal native mesh have topologic errors
    QgsMeshEditingError initialize();

    //! Returns TRUE if a \a face can be added to the mesh
    bool faceCanBeAdded( const QgsMeshFace &face );

    /**
     * Returns TRUE if the face does not intersect or contains any other elements (faces or vertices)
     * The topological compatibility is not checked
     */
    bool isFaceGeometricallyCompatible( const QgsMeshFace &face );

    //! Adds faces \a faces to the mesh, returns topological errors if this operation fails (operation is not realized)
    QgsMeshEditingError addFaces( const QVector<QgsMeshFace> &faces ); SIP_SKIP

    //! Adds a face \a face to the mesh with vertex indexes \a vertexIndexes, returns topological errors if this operation fails (operation is not realized)
    QgsMeshEditingError addFace( const QVector<int> &vertexIndexes );

    //! Removes faces \a faces to the mesh, returns topological errors if this operation fails (operation is not realized)
    QgsMeshEditingError removeFaces( const QList<int> &facesToRemove );

    //! Returns TRUE if the edge can be flipped (only available for edge shared by two faces with 3 vertices)
    bool edgeCanBeFlipped( int vertexIndex1, int vertexIndex2 ) const;

    //! Flips edge (\a vertexIndex1, \a vertexIndex2)
    void flipEdge( int vertexIndex1, int vertexIndex2 );

    /**
     * Returns TRUE if faces separated by vertices with indexes \a vertexIndex1 and \a vertexIndex2 can be merged
     */
    bool canBeMerged( int vertexIndex1, int vertexIndex2 ) const;

    //! Merges faces separated by vertices with indexes \a vertexIndex1 and \a vertexIndex2
    void merge( int vertexIndex1, int vertexIndex2 );

    /**
     * Returns TRUE if face with index \a faceIndex can be split
     */
    bool faceCanBeSplit( int faceIndex ) const;

    /**
     * Splits faces with index \a faceIndexes. Only faces that can be split are split.
     * Returns the count of faces effictively split
     */
    int splitFaces( const QList<int> &faceIndexes );

    /**
     *  Adds vertices in triangular mesh coordinate in the mesh. Vertex is effectivly added if the transform
     *  from triangular coordinate to layer coordinate succeeds or if any vertices are next the added vertex (under \a tolerance distance).
     *  The method returns the number of vertices effectivly added.
     *
     *  \note this operation remove including face if exists and replace it by new faces surrounding the vertex
     *  if the mesh hasn't topological error before this operation, the toological operation always succeed.
     */
    int addVertices( const QVector<QgsMeshVertex> &vertices, double tolerance ); SIP_SKIP

    /**
     *  Adds points as vertices in triangular mesh coordinate in the mesh. Vertex is effectivly added if the transform
     *  from triangular coordinate to layer coordinate succeeds or if any vertices are next the added vertex (under \a tolerance distance).
     *  The method returns the number of vertices effectivly added.
     *
     *  \note this operation remove including face if exists and replace it by new faces surrounding the vertex
     *  if the mesh hasn't topological error before this operation, the toological operation always succeed
     */
    int addPointsAsVertices( const QVector<QgsPoint> &point, double tolerance );

    /**
     *  Removes vertices with indexes in the list \a verticesToRemoveIndexes in the mesh
     *  if \a fillHoles is set to TRUE, this operation will fill holes created in the mesh, if not remove the surrounding faces
     *
     *  If removing these vertices leads to a topological errors, the method will return the corresponding error and the operatio is canceled
     */
    QgsMeshEditingError removeVertices( const QList<int> &verticesToRemoveIndexes, bool fillHoles = false );

    /**
     * Changes the Z values of the vertices with indexes in \a vertices indexes with the values in \a newValues
     */
    void changeZValues( const QList<int> &verticesIndexes, const QList<double> &newValues );

    /**
     * Changes the (X,Y) coordinates values of the vertices with indexes in \a vertices indexes with the values in \a newValues.
     * The caller has the responsibility to check if changing the vertices coordinates does not lead to topological errors
     */
    void changeXYValues( const QList<int> &verticesIndexes, const QList<QgsPointXY> &newValues );

    /**
     * Applies an advance editing on the edited mesh, see QgsMeshAdvancedEditing
     */
    void advancedEdit( QgsMeshAdvancedEditing *editing );

    //! Stops editing
    void stopEditing();

    //! Returns the extent of the edited mesh
    QgsRectangle extent() const;

    //! Returns whether the mesh has been modified
    bool isModified() const;

    //----------- access element methods

    //! Returns all the free vertices indexes
    QList<int> freeVerticesIndexes();

    //! Returns whether the vertex with index \a vertexIndex is on a boundary
    bool isVertexOnBoundary( int vertexIndex ) const;

    //! Returns whether the vertex with index \a vertexIndex is a free vertex
    bool isVertexFree( int vertexIndex ) const;

    /**
     *  Returns a vertex circulator linked to this mesh around the vertex with index \a vertexIndex.
     *  If the vertex does not exist or is a free vertex, the cirxulator is invalid.
     *  If stopEditing() is called, circulator created before and new circulator are valid and must not be used.
     *  It is recommended to destruct all circulator created before calling any edit methods or stopEditing() to save memory usage.
     *  Calling initialize() allows creation of new circulator after stopEditing() is called.
     */
    QgsMeshVertexCirculator vertexCirculator( int vertexIndex ) const; SIP_SKIP

    //! Returns a reference to the topological mesh
    QgsTopologicalMesh &topologicalMesh(); SIP_SKIP

    //! Return TRUE if the edited mesh is consistent
    bool checkConsistency() const;

  signals:
    //! Emitted when the mesh is edited
    void meshEdited();

  private:
    QgsMesh *mMesh = nullptr;
    QgsTopologicalMesh mTopologicalMesh;
    QgsTriangularMesh *mTriangularMesh = nullptr;
    int mMaximumVerticesPerFace = 0;

    QVector<QgsMeshFace> prepareFaces( const QVector<QgsMeshFace> &faces, QgsMeshEditingError &error );

    //! undo/redo stuff
    QUndoStack *mUndoStack = nullptr;

    struct Edit
    {
      QgsTopologicalMesh::Changes topologicalChanges;
      QgsTriangularMesh::Changes triangularMeshChanges;
    };
    void applyEdit( Edit &edit );
    void reverseEdit( Edit &edit );

    void applyAddVertex( Edit &edit, const QgsMeshVertex &vertex );
    void applyRemoveVertexFillHole( Edit &edit, int vertexIndex );
    void applyRemoveVerticesWithoutFillHole( QgsMeshEditor::Edit &edit, const QList<int> &verticesIndexes );
    void applyAddFaces( Edit &edit, const QgsTopologicalMesh::TopologicalFaces &faces );
    void applyRemoveFaces( Edit &edit, const QList<int> &faceToRemoveIndex );
    void applyChangeZValue( Edit &edit, const QList<int> &verticesIndexes, const QList<double> &newValues );
    void applyChangeXYValue( Edit &edit, const QList<int> &verticesIndexes, const QList<QgsPointXY> &newValues );
    void applyFlipEdge( Edit &edit, int vertexIndex1, int vertexIndex2 );
    void applyMerge( Edit &edit, int vertexIndex1, int vertexIndex2 );
    void applySplit( QgsMeshEditor::Edit &edit, int faceIndex );
    void applyAdvancedEdit( Edit &edit, QgsMeshAdvancedEditing *editing );

    void applyEditOnTriangularMesh( Edit &edit, const QgsTopologicalMesh::Changes &topologicChanges );

    friend class TestQgsMeshEditor;
    friend class QgsMeshLayerUndoCommandMeshEdit;
    friend class QgsMeshLayerUndoCommandAddVertices;
    friend class QgsMeshLayerUndoCommandRemoveVertices;
    friend class QgsMeshLayerUndoCommandAddFaces;
    friend class QgsMeshLayerUndoCommandRemoveFaces;
    friend class QgsMeshLayerUndoCommandSetZValue;
    friend class QgsMeshLayerUndoCommandChangeZValue;
    friend class QgsMeshLayerUndoCommandChangeXYValue;
    friend class QgsMeshLayerUndoCommandFlipEdge;
    friend class QgsMeshLayerUndoCommandMerge;
    friend class QgsMeshLayerUndoCommandSplitFaces;

    friend class QgsMeshLayerUndoCommandAdvancedEditing;
};

#ifndef SIP_RUN

/**
 * \ingroup core
 *
 * \brief Base class for undo/redo command for mesh editing
 *
 * \since QGIS 3.22
 */
class QgsMeshLayerUndoCommandMeshEdit : public QUndoCommand
{
  public:

    void undo() override;
    void redo() override;

  protected:

    //! Constructor for the base class
    QgsMeshLayerUndoCommandMeshEdit( QgsMeshEditor *meshEditor );
    QPointer<QgsMeshEditor> mMeshEditor;
    QList<QgsMeshEditor::Edit> mEdits;
};

/**
 * \ingroup core
 *
 * \brief  Class for undo/redo command for adding vertices in mesh
 *
 * \since QGIS 3.22
 */
class QgsMeshLayerUndoCommandAddVertices : public QgsMeshLayerUndoCommandMeshEdit
{
  public:

    //! Constructor with the associated \a meshEditor and \a vertices that will be added
    QgsMeshLayerUndoCommandAddVertices( QgsMeshEditor *meshEditor, const QVector<QgsMeshVertex> &vertices );
    void redo() override;

  private:
    QVector<QgsMeshVertex> mVertices;
};

/**
 * \ingroup core
 *
 * \brief  Class for undo/redo command for removing vertices in mesh
 *
 * \since QGIS 3.22
 */
class QgsMeshLayerUndoCommandRemoveVertices : public QgsMeshLayerUndoCommandMeshEdit
{
  public:
    //! Constructor with the associated \a meshEditor and \a vertices that will be removed and the flag \a fillHole
    QgsMeshLayerUndoCommandRemoveVertices( QgsMeshEditor *meshEditor, const QList<int> &verticesToRemoveIndexes, bool fillHole );
    void redo() override;

  private:
    QList<int> mVerticesToRemoveIndexes;
    bool mFillHole = false;
};

/**
 * \ingroup core
 *
 * \brief  Class for undo/redo command for adding faces in mesh
 *
 * \since QGIS 3.22
 */
class QgsMeshLayerUndoCommandAddFaces : public QgsMeshLayerUndoCommandMeshEdit
{
  public:

    //! Constructor with the associated \a meshEditor and \a faces that will be added
    QgsMeshLayerUndoCommandAddFaces( QgsMeshEditor *meshEditor, QgsTopologicalMesh::TopologicalFaces &faces );

    void redo() override;
  private:
    QgsTopologicalMesh::TopologicalFaces mFaces;
};

/**
 * \ingroup core
 *
 * \brief  Class for undo/redo command for removing faces in mesh
 *
 * \since QGIS 3.22
 */
class QgsMeshLayerUndoCommandRemoveFaces : public QgsMeshLayerUndoCommandMeshEdit
{
  public:

    //! Constructor with the associated \a meshEditor and indexes \a facesToRemoveIndexes of the faces that will be removed
    QgsMeshLayerUndoCommandRemoveFaces( QgsMeshEditor *meshEditor, const QList<int> &facesToRemoveIndexes );

    void redo() override;
  private:
    QList<int> mfacesToRemoveIndexes;
};

/**
 * \ingroup core
 *
 * \brief  Class for undo/redo command for changing Z value of vertices
 *
 * \since QGIS 3.22
 */
class QgsMeshLayerUndoCommandChangeZValue : public QgsMeshLayerUndoCommandMeshEdit
{
  public:

    /**
     * Constructor with the associated \a meshEditor and indexes \a verticesIndexes of the vertices that will have
     * the Z value changed with \a newValues
     */
    QgsMeshLayerUndoCommandChangeZValue( QgsMeshEditor *meshEditor, const QList<int> &verticesIndexes, const QList<double> &newValues );
    void redo() override;

  private:
    QList<int> mVerticesIndexes;
    QList<double> mNewValues;
};

/**
 * \ingroup core
 *
 * \brief  Class for undo/redo command for changing (X,Y) value of vertices
 *
 * \since QGIS 3.22
 */
class QgsMeshLayerUndoCommandChangeXYValue : public QgsMeshLayerUndoCommandMeshEdit
{
  public:

    /**
     * Constructor with the associated \a meshEditor and indexes \a verticesIndexes of the vertices that will have
     * the (X,Y) values changed with \a newValues
     */
    QgsMeshLayerUndoCommandChangeXYValue( QgsMeshEditor *meshEditor, const QList<int> &verticesIndexes, const QList<QgsPointXY> &newValues );
    void redo() override;

  private:
    QList<int> mVerticesIndexes;
    QList<QgsPointXY> mNewValues;
};

/**
 * \ingroup core
 *
 * \brief  Class for undo/redo command for flipping edge
 *
 * \since QGIS 3.22
 */
class QgsMeshLayerUndoCommandFlipEdge : public QgsMeshLayerUndoCommandMeshEdit
{
  public:

    /**
     * Constructor with the associated \a meshEditor and the vertex indexes of the edge (\a vertexIndex1, \a vertexIndex2)
     */
    QgsMeshLayerUndoCommandFlipEdge( QgsMeshEditor *meshEditor, int vertexIndex1, int vertexIndex2 );
    void redo() override;

  private:
    int mVertexIndex1 = -1;
    int mVertexIndex2 = -1;
};

/**
 * \ingroup core
 *
 * \brief  Class for undo/redo command for merging face
 *
 * \since QGIS 3.22
 */
class QgsMeshLayerUndoCommandMerge : public QgsMeshLayerUndoCommandMeshEdit
{
  public:

    /**
     * Constructor with the associated \a meshEditor and the vertex indexes of
     * the edge (\a vertexIndex1, \a vertexIndex2) that separate the face to merge
     */
    QgsMeshLayerUndoCommandMerge( QgsMeshEditor *meshEditor, int vertexIndex1, int vertexIndex2 );
    void redo() override;

  private:
    int mVertexIndex1 = -1;
    int mVertexIndex2 = -1;
};

/**
 * \ingroup core
 *
 * \brief  Class for undo/redo command for splitting faces
 *
 * \since QGIS 3.22
 */
class QgsMeshLayerUndoCommandSplitFaces : public QgsMeshLayerUndoCommandMeshEdit
{
  public:

    /**
     * Constructor with the associated \a meshEditor and indexes \a faceIndexes of the faces to split
     */
    QgsMeshLayerUndoCommandSplitFaces( QgsMeshEditor *meshEditor, const QList<int> &faceIndexes );
    void redo() override;

  private:
    QList<int> mFaceIndexes;
};


/**
 * \ingroup core
 *
 * \brief  Class for undo/redo command for applying advanced editing
 * \since QGIS 3.22
 */
class QgsMeshLayerUndoCommandAdvancedEditing : public QgsMeshLayerUndoCommandMeshEdit
{
  public:

    //! Constructor with the associated \a meshEditor
    QgsMeshLayerUndoCommandAdvancedEditing( QgsMeshEditor *meshEditor, QgsMeshAdvancedEditing *advancdEdit );
    void redo() override;

  private:
    QgsMeshAdvancedEditing *mAdvancedEditing = nullptr;
};




#endif //SIP_RUN

#endif // QGSMESHEDITOR_H
