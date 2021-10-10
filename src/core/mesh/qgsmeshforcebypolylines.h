/***************************************************************************
  qgsmeshforcebypolylines.h - QgsMeshForceByPolylines

 ---------------------
 begin                : 5.9.2021
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
#ifndef QGSMESHFORCEBYPOLYLINES_H
#define QGSMESHFORCEBYPOLYLINES_H


#include "qgis_core.h"
#include "qgstopologicalmesh.h"
#include "qgstriangularmesh.h"
#include "qgsabstractgeometry.h"
#include "qgsmeshadvancedediting.h"

/**
 * \ingroup core
 *
 * \brief Class derived from QgsMeshAdvancedEditing that forces mesh based on a line
 *
 * Forcing lines consist of line that the faces are forced to follow, that is edges of encountered faces have to be on theses lines.
 *
 * Caller of this class has to set the line with setInputLine() before applying the edition with QgsMeshEditor::advancedEdit()
 *
 * Other option has also to be set before calling QgsMeshEditor::advancedEdit()
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsMeshEditForceByLine : public QgsMeshAdvancedEditing
{
  public:

    //! Constructor
    QgsMeshEditForceByLine() = default;

    //! Sets the input forcing line in rendering coordinates
    void setInputLine( const QgsPoint &pt1, const QgsPoint &pt2, double tolerance, bool newVertexOnIntersection );

    //! Sets the tolerance in redering coordinate system unit
    void setTolerance( double tolerance );

    //! Sets whether vertices will be added when the lines will intersect internal edges of faces, default is FALSE
    void setAddVertexOnIntersection( bool addVertex );

    //! Sets the default value of Z coordinate to use for new vertices, this value will be used if the Z value
    void setDefaultZValue( double defaultZValue );

    /**
     * Sets whether the new created vertices will have their value interpolated from the existing mesh.
     * If not, Z value will be interpolated from the lines,
     * in case these line are not 3D, the default value will be used (\see setDefaultZValue())
     */
    void setInterpolateZValueOnMesh( bool interpolateZValueOnMesh );

  private:
    QgsPoint mPoint1;
    QgsPoint mPoint2;
    bool mNewVertexOnIntersection = false;
    double mTolerance = 1e-8;
    double mDefaultZValue = 0;
    bool mInterpolateZValueOnMesh = false;

    QgsTopologicalMesh::Changes apply( QgsMeshEditor *meshEditor ) override;

    virtual void finish();

    //members and method used for the calculation
    QgsMeshEditor *mEditor = nullptr;
    QList<int> mRemovedFaces;
    QList<int> mHoleOnLeft; // contains the border vertices of hole to fill on the right of the line (line go up)
    QList<int> mNeighborOnLeft; //contains the neighbor face on the right of the line (line go up)
    QList<int> mHoleOnRight; // contains the border vertices of hole to fill on the right of the line (line go up)
    QList<int> mNeighborOnRight; // contains the neighbor face on the right of the line (line go up)
    QList<int> mNewVerticesIndexesOnLine; //the new vertices intersecting edges except
    bool mEndOnPoint2 = false;
    int mPoint2VertexIndex = -1;
    int mCurrentSnappedVertex = -1; // Last snapped point
    QgsPoint mCurrentPointPosition;

    bool mFirstPointChecked = false;
    bool mSecondPointChecked = false;

    void interpolateZValueOnMesh( QgsPoint &point ) const;
    void interpolateZValueOnMesh( int faceIndex, QgsPoint &point ) const;
    void interpolateZValue( QgsMeshVertex &point, const QgsPoint &otherPoint1, const QgsPoint &otherPoint2 );


    bool buildForcedElements();

    bool edgeIntersection( int vertex1,
                           int vertex2,
                           int &closestSnappedVertex,
                           QgsPoint &intersectionPoint,
                           bool outAllowed );

    bool searchIntersectionEdgeFromSnappedVertex(
      int &intersectionFaceIndex,
      int &previousSnappedVertex,
      int &currentSnappedVertexIndex,
      QgsPoint &intersectionPoint,
      int &edgePosition,
      QSet<int> &treatedFaces );

    // Insert a new vertex and returns its local index (0 is first index in th
    int insertNewVertex( const QgsMeshVertex &vertex );

    bool triangulateHoles( const QList<int> &holeOnLeft,
                           const QList<int> &neighborOnLeft,
                           bool isLeftHole,
                           QList<std::array<int, 2> > &newFacesOnLine,
                           std::array<int, 2> &extremeFaces );

    bool finishForcingLine();

    friend class TestQgsMeshEditor;
    friend class QgsMeshEditForceByPolylines;
};



/**
 * \ingroup core
 *
 * \brief Class derived from QgsMeshEditForceByLine that forces mesh based on polyline.
 *
 * Forcing lines consist of line that the faces are forced to follow, that is edges of encountered faces have to be on theses lines.
 *
 * Caller of this class has to add the lines from QgsGeometry instances with addLineFromGeometry() or addLinesFromGeometries()
 * before applying the edition with QgsMeshEditor::advancedEdit()
 *
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsMeshEditForceByPolylines : public QgsMeshEditForceByLine
{
  public:

    //! Constructor
    QgsMeshEditForceByPolylines() = default;

    QString text() const override;
    bool isFinished() const override;

    /**
     * Adds a input forcing line geometry in rendering coordinates
     *
     * \note if the geometry is not 3D, the default Z value will be used for the Z value of the geometry's vertices.
     *       This default Z value has to be set before adding the geometry (\see setDefaultZValue()
     */
    void addLineFromGeometry( const QgsGeometry &geom );

    /**
     * Adds a list of input forcing lines geometry in rendering coordinates
     *
     * \note if the geometry is not 3D, the default Z value will be used for the Z value of the geometry's vertices.
     *       This default Z value has to be set before adding the geometry (\see setDefaultZValue()
     */
    void addLinesFromGeometries( const QList<QgsGeometry> geometries );

  private:
    QList<QgsPointSequence> mPolylines;
    int mCurrentPolyline = 0;
    int mCurrentSegment = 0;

    void incrementSegment();

    using QgsMeshEditForceByLine::setInputLine;
    QgsTopologicalMesh::Changes apply( QgsMeshEditor *meshEditor ) override;

};

#endif // QGSMESHFORCEBYPOLYLINES_H
