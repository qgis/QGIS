/***************************************************************************
  qgsmeshadvancedediting.h - QgsMeshAdvancedEditing

 ---------------------
 begin                : 9.7.2021
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
#ifndef QGSMESHADVANCEDEDITING_H
#define QGSMESHADVANCEDEDITING_H


#include "qgis_core.h"
#include "qgstopologicalmesh.h"
#include "qgstriangularmesh.h"

class QgsMeshEditor;
class QgsProcessingFeedback;
class QgsExpressionContext;

/**
 * \ingroup core
 *
 * \brief Abstract class that can be derived to implement advanced editing on mesh
 *
 * To apply the advanced editing, a pointer to an instance of a derived class is passed
 * in the method QgsMeshEditor::advancedEdit().
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsMeshAdvancedEditing : protected QgsTopologicalMesh::Changes SIP_ABSTRACT
{
  public:

    //! Constructor
    QgsMeshAdvancedEditing();
    //! Destructor
    virtual ~QgsMeshAdvancedEditing();

    //! Sets the input vertices indexes that will be used for the editing
    void setInputVertices( const QList<int> verticesIndexes );

    //! Sets the input faces indexes that will be used for the editing
    void setInputFaces( const QList<int> faceIndexes );

    //! Returns a message that can be provided by the advanced editing when applying is done
    QString message() const;

    //! Removes all data provided to the editing or created by the editing
    void clear();

    /**
     *  Returns whether the advanced edit is finished,
     *  if not, this edit has to be applied again with QgsMeshEditor::advancedEdit() until is finished returns TRUE
     */
    virtual bool isFinished() const;

    //! Returns a short text string describing what this advanced edit does. Default implementation return a void string.
    virtual QString text() const;

  protected:
    QList<int> mInputVertices;
    QList<int> mInputFaces;
    QString mMessage;
    bool mIsFinished = false;

    /**
     * Apply a change to \a mesh Editor. This method is called by the QgsMeshEditor to apply the editing on the topological mesh
     *
     * The method has to be implemented in the derived class to provide the changes of the advanced editing
     */
    virtual QgsTopologicalMesh::Changes apply( QgsMeshEditor *meshEditor ) = 0; SIP_SKIP

    friend class QgsMeshEditor;
};

/**
 * \ingroup core
 *
 * \brief Class that can do a refinement of faces of a mesh.
 * This refinement is operated only on faces with 3 or 4 vertices (triangles or quads) by adding a vertex on the middle of each refined face.
 * For quad faces, a vertex is added on the centroid of the original face.
 * New vertices Z value are interpolated between original vertices.
 * Original triangle faces are replaced by four triangles, and original quad faces are replaced by four quads.
 * Neighboring faces are triangulated to take account of the new vertex in the shared edge.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsMeshEditRefineFaces : public QgsMeshAdvancedEditing
{
  public:

    //! Constructor
    QgsMeshEditRefineFaces();

    QString text() const override;

  private:
    QgsTopologicalMesh::Changes apply( QgsMeshEditor *meshEditor ) override;

    struct FaceRefinement
    {
      QList<int> newVerticesLocalIndex; // new vertices in the same order of the vertex index (ccw)
      QList<bool> refinedFaceNeighbor;
      QList<bool> borderFaceNeighbor;
      int newCenterVertexIndex;
      QList<int> newFacesChangesIndex;
    };

    struct BorderFace
    {
      QList<bool> refinedFacesNeighbor;
      QList<bool> borderFacesNeighbor;
      QList<bool> unchangeFacesNeighbor;
      QList<int> newVerticesLocalIndex;
      QList<int> edgeFace; //global index of the dirst face exposed on edge
    };

    //! Create new vertices of the refinement and populate helper containers
    void createNewVerticesAndRefinedFaces( QgsMeshEditor *meshEditor,
                                           QSet<int> &facesToRefine,
                                           QHash<int, FaceRefinement> &facesRefinement );

    bool createNewBorderFaces( QgsMeshEditor *meshEditor,
                               const QSet<int> &facesToRefine,
                               QHash<int, FaceRefinement> &facesRefinement,
                               QHash<int, BorderFace> &borderFaces );

    friend class TestQgsMeshEditor;
};


/**
 * \ingroup core
 *
 * \brief Class that can transform vertices of a mesh by expression
 *
 * Each coordinates are associated with an expression that can be defined with function
 * returning the current coordinates (see setExpressions()):
 *
 * - $vertex_x
 * - $vertex_y
 * - $vertex_z
 *
 * Example:
 * Transposing a mesh and translate following axe X with a distance of 50 and increase the level of the mesh
 * with an height of 80 when previous X coordinate is under 100 and de crease the level of 150 when X is under 100:
 *
 * expressionX: "$vertex_y + 50"
 * expressionY: "$vertex_x"
 * expressionZ: "if( $vertex_x <= 100 , $vertex_z + 80 , $vertex_z - 150)"
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsMeshTransformVerticesByExpression : public QgsMeshAdvancedEditing
{
  public:

    //! Constructor
    QgsMeshTransformVerticesByExpression() = default;

    QString text() const override;

    /**
     * Sets the expressions for the coordinates transformation.
     *
     * \note Expressions are optional for each coordinate, the coordinate will not be transformed if the string is void.
     */
    void setExpressions( const QString &expressionX, const QString &expressionY, const QString &expressionZ );

    /**
     * Calculates the transformed vertices of the mesh \a layer, returns FALSE if this leads to topological or geometrical errors.
     * The mesh layer must be in edit mode.
     *
     * \note this method not apply new vertices to the mesh layer but only store the calculated transformation
     *       that can be apply later with QgsMeshEditor::advancedEdit()
     */
    bool calculate( QgsMeshLayer *layer );

    /**
     * Returns the transformed vertex from its index \a vertexIndex for the mesh \a layer
     *
     * If \a layer is not the same than the one used to make the calculation, this will create an undefined behavior
     */
    QgsMeshVertex transformedVertex( QgsMeshLayer *layer, int vertexIndex ) const;

  private:
    QString mExpressionX;
    QString mExpressionY;
    QString mExpressionZ;
    QHash<int, int> mChangingVertexMap;

    QgsTopologicalMesh::Changes apply( QgsMeshEditor *meshEditor ) override;

    friend class TestQgsMeshEditor;
};

#endif // QGSMESHADVANCEDEDITING_H
