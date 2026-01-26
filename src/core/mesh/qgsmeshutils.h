/***************************************************************************
                         qgsmeshutils.h
                         --------------------------
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

#ifndef QGSMESHUTILS_H
#define QGSMESHUTILS_H

#include "qgis.h"
#include "qgis_sip.h"
#include "qgsmeshdataset.h"
#include "qgspoint.h"

class QgsRasterBlock;
class QgsMeshLayer;
class QgsMeshDatasetIndex;
class QgsCoordinateReferenceSystem;
class QgsCoordinateTransform;
class QgsCoordinateTransformContext;
class QgsRectangle;
class QgsRasterBlockFeedback;
class QgsTriangularMesh;
class QgsMeshDataBlock;
class QgsPointXY;
class QgsGeometry;
class QgsPolygon;

//! xyz coords of vertex
typedef QgsPoint QgsMeshVertex;

//! List of vertex indexes
typedef QVector<int> QgsMeshFace;

/**
 * Edge is a straight line seqment between 2 points.
 * Stores the pair of vertex indexes
 * \since QGIS 3.14
 */
typedef QPair<int, int> QgsMeshEdge;


/**
 * \ingroup core
 * \brief Provides utility functions for working with mesh data.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsMeshUtils
{
  public:

    /**
     * Exports mesh layer's dataset values as raster block
     *
     * The function always fetches native mesh and dataset data
     * from data provider and calculates triangular mesh
     *
     * \param layer mesh layer
     * \param datasetIndex index from layer defining group and dataset (time) to export
     * \param destinationCrs destination/map CRS. Used to create triangular mesh from native mesh
     * \param transformContext Transform context to transform layer CRS to destination CRS
     * \param mapUnitsPerPixel map units per pixel for block
     * \param extent extent of block in destination CRS
     * \param feedback optional raster feedback object for cancellation/preview
     * \returns raster block with Float::64 values. NULLPTR on error
     *
     * \since QGIS 3.6
     */
    static QgsRasterBlock *exportRasterBlock(
      const QgsMeshLayer &layer,
      const QgsMeshDatasetIndex &datasetIndex,
      const QgsCoordinateReferenceSystem &destinationCrs,
      const QgsCoordinateTransformContext &transformContext,
      double mapUnitsPerPixel,
      const QgsRectangle &extent,
      QgsRasterBlockFeedback *feedback = nullptr
    ) SIP_FACTORY;


    /**
     * Exports mesh layer's dataset values as raster block
     *
     * \param triangularMesh the triangular mesh of the mesh layer
     * \param datasetValues dataset values used to build the raster block
     * \param activeFlags active flag values
     * \param dataType the data type iof the dataset values
     * \param transform the coordinate transform used to export the raster block
     * \param mapUnitsPerPixel map units per pixel for block
     * \param extent extent of block in destination CRS
     * \param feedback optional raster feedback object for cancellation/preview
     * \returns raster block with Float::64 values. NULLPTR on error
     *
     * \since QGIS 3.18
     */
    static QgsRasterBlock *exportRasterBlock(
      const QgsTriangularMesh &triangularMesh,
      const QgsMeshDataBlock &datasetValues,
      const QgsMeshDataBlock &activeFlags,
      const QgsMeshDatasetGroupMetadata::DataType dataType,
      const QgsCoordinateTransform &transform,
      double mapUnitsPerPixel,
      const QgsRectangle &extent,
      QgsRasterBlockFeedback *feedback = nullptr
    ) SIP_SKIP;

    /**
     * Returns face as polygon geometry.
     * \note Not available in Python bindings
     */
    SIP_SKIP static QgsGeometry toGeometry( const QgsMeshFace &face, const QVector<QgsMeshVertex> &vertices );

    /**
     * Returns the centroid of the \a face.
     * \note Not available in Python bindings
     */
    SIP_SKIP static QgsMeshVertex centroid( const QgsMeshFace &face, const QVector<QgsMeshVertex> &vertices );

    /**
     * Returns face as polygon geometry, caller is responsible for delete
     * \note Not available in Python bindings
     */
    SIP_SKIP static std::unique_ptr< QgsPolygon > toPolygon( const QgsMeshFace &face, const QVector<QgsMeshVertex> &vertices );

    /**
     * Returns unique native faces indexes from list of triangle indexes
     * \note Not available in Python bindings
     * \since QGIS 3.4
     */
    SIP_SKIP static QSet<int> nativeFacesFromTriangles( const QList<int> &triangleIndexes, const QVector<int> &trianglesToNativeFaces );

    /**
     * Returns unique native faces indexes from list of triangle indexes
     * \note Not available in Python bindings
     * \since QGIS 3.14
     */
    SIP_SKIP static QSet<int> nativeEdgesFromEdges( const QList<int> &edgesIndexes, const QVector<int> &edgesToNativeEdges );

    /**
     * Returns unique native vertex indexes from list of vertices of triangles
     * \note Not available in Python bindings
     * \since QGIS 3.14
     */
    SIP_SKIP static QSet<int> nativeVerticesFromTriangles( const QList<int> &triangleIndexes, const QVector<QgsMeshFace> &triangles );

    /**
     * Returns unique native faces indexes from list of vertices of triangles
     * \note Not available in Python bindings
     * \since QGIS 3.14
     */
    SIP_SKIP static QSet<int> nativeVerticesFromEdges( const QList<int> &edgesIndexes, const QVector<QgsMeshEdge> &edges );

    /**
     * Tests if point p is on the face defined with vertices
     * \note Not available in Python bindings
     * \since QGIS 3.12
    */
    SIP_SKIP static bool isInTriangleFace( const QgsPointXY point, const QgsMeshFace &face,  const QVector<QgsMeshVertex> &vertices );

    /**
     * Checks if the triangle is counter clockwise, if not sets it counter clockwise
     * \note Not available in Python bindings
     * \since QGIS 3.22
    */
    SIP_SKIP static void setCounterClockwise( QgsMeshFace &triangle, const QgsMeshVertex &v0, const QgsMeshVertex &v1, const QgsMeshVertex &v2 );

};

#endif // QGSMESHUTILS_H
