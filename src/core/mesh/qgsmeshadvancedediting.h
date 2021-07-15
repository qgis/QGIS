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

  protected:
    QList<int> mInputVertices;
    QList<int> mInputFaces;
    QString mMessage;

    /**
     * Apply a change to \a mesh Editor. This method is called by the QgsMeshEditor to apply the editing on the topological mesh
     *
     * The method has to be implemented in the derived class to provide the changes of the advancd editing
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


#endif // QGSMESHADVANCEDEDITING_H
