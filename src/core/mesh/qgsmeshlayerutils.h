/***************************************************************************
                         qgsmeshlayerutils.h
                         --------------------------
    begin                : August 2018
    copyright            : (C) 2018 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHLAYERUTILS_H
#define QGSMESHLAYERUTILS_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include "qgsrectangle.h"
#include "qgsmaptopixel.h"
#include "qgsmeshdataprovider.h"
#include "qgsmeshrenderersettings.h"

#include <QVector>
#include <QSize>

///@cond PRIVATE

class QgsMeshTimeSettings;
class QgsTriangularMesh;
class QgsMeshDataBlock;
class QgsMesh3dAveragingMethod;
class QgsMeshDatasetValue;
class QgsMeshLayer;

/**
 * \ingroup core
 * \brief Misc utility functions used for mesh layer/data provider support
 *
 * \note not available in Python bindings
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsMeshLayerUtils
{
  public:

    /**
     * Returns (maximum) number of values that can be extracted from the mesh by type
     *
     * It is assumed that 3D values are averaged to face values
     * \see datasetValues()
     *
     * \since QGIS 3.14
     */
    static int datasetValuesCount( const QgsMesh *mesh, QgsMeshDatasetGroupMetadata::DataType dataType );

    /**
     * Returns the type of values the datasetValues() returns
     *
     * \see datasetValues()
     * \since QGIS 3.14
     */
    static QgsMeshDatasetGroupMetadata::DataType datasetValuesType( const QgsMeshDatasetGroupMetadata::DataType &type );

    /**
     * \brief Returns N vector/scalar values from the index from the dataset
     *
     * See QgsMeshLayerUtils::datasetValuesCount() to determine maximum number of values to be requested
     * See QgsMeshLayerUtils::datasetValuesType() to see the the type of values the function returns
     * See QgsMeshDatasetGroupMetadata::isVector() to check if the returned value is vector or scalar
     *
     * \since QGIS 3.12
     */
    static QgsMeshDataBlock datasetValues(
      const QgsMeshLayer *meshLayer,
      QgsMeshDatasetIndex index,
      int valueIndex,
      int count );

    /**
     * \brief Returns gridded vector values, if extentInMap is default, uses the triangular mesh extent
     *
     * \param meshLayer pointer to the mesh layer
     * \param index dataset index
     * \param xSpacing the spacing on X coordinate in map unit
     * \param ySpacing the spacing on Y coordinate in map unit
     * \param size contains the size (count of rows and columns) of the grid supporting the vectors
     * \param minCorner coordinates of the corner with x minimum and y miminum
     * \returns vectors on a grid, empty if dataset is no vector values
     *
     * \since QGIS 3.14
     */
    static QVector<QgsVector> griddedVectorValues(
      const QgsMeshLayer *meshLayer,
      const QgsMeshDatasetIndex index,
      double xSpacing,
      double ySpacing,
      const QSize &size,
      const QgsPointXY &minCorner );

    /**
     * Calculates magnitude values from the given QgsMeshDataBlock.
     *
     * \since QGIS 3.6
     */
    static QVector<double> calculateMagnitudes( const QgsMeshDataBlock &block );

    /**
     * Transformes the bounding box to rectangle in screen coordinates (in pixels)
     * \param mtp actual renderer map to pixel
     * \param outputSize actual renderer output size
     * \param bbox bounding box in map coordinates
     * \param leftLim minimum x coordinate in pixel, clipped by 0
     * \param rightLim maximum x coordinate in pixel, clipped by outputSize width
     * \param bottomLim minimum y coordinate in pixel, clipped by 0
     * \param topLim maximum y coordinate in pixel, clipped by outputSize height
     */
    static void boundingBoxToScreenRectangle(
      const QgsMapToPixel &mtp,
      const QSize &outputSize,
      const QgsRectangle &bbox,
      int &leftLim,
      int &rightLim,
      int &bottomLim,
      int &topLim );

    /**
     * Transformes the bounding box to rectangle in screen coordinates (in pixels)
     * \param mtp actual renderer map to pixel
     * \param bbox bounding box in map coordinates
     */
    static QgsRectangle boundingBoxToScreenRectangle(
      const QgsMapToPixel &mtp,
      const QgsRectangle &bbox
    );

    /**
    * Interpolates value based on known values on the vertices of a edge
    * \returns value on the point pt a or NaN
    *
    * \since QGIS 3.14
    */
    static double interpolateFromVerticesData(
      double fraction,
      double val1, double val2
    );

    /**
    * Interpolates value based on known values on the vertices of a edge
    * \returns value on the point pt a or NaN
    *
    * \since QGIS 3.14
    */
    static QgsMeshDatasetValue interpolateFromVerticesData( double fraction,
        const QgsMeshDatasetValue &val1, const QgsMeshDatasetValue &val2
                                                          );

    /**
    * Interpolates value based on known values on the vertices of a triangle
    * \param p1 first vertex of the triangle
    * \param p2 second vertex of the triangle
    * \param p3 third vertex of the triangle
    * \param val1 value on p1 of the triangle
    * \param val2 value on p2 of the triangle
    * \param val3 value on p3 of the triangle
    * \param pt point where to calculate value
    * \returns value on the point pt or NaN in case the point is outside the triangle
    */
    static double interpolateFromVerticesData(
      const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3,
      double val1, double val2, double val3, const QgsPointXY &pt
    );

    /**
     * Interpolates the z value for a \a mesh at the specified point (\a x, \a y).
     *
     * Returns NaN if the z value cannot be calculated for the point.
     *
     * \since QGIS 3.26
     */
    static double interpolateZForPoint( const QgsTriangularMesh &mesh, double x, double y );

    /**
    * Interpolates vector based on known vector on the vertices of a triangle
    * \param p1 first vertex of the triangle
    * \param p2 second vertex of the triangle
    * \param p3 third vertex of the triangle
    * \param vect1 value on p1 of the triangle
    * \param vect2 value on p2 of the triangle
    * \param vect3 value on p3 of the triangle
    * \param pt point where to calculate value
    * \returns vector on the point pt or NaN in case the point is outside the triangle
    *
    * \since QGIS 3.12
    */
    static QgsVector interpolateVectorFromVerticesData(
      const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3,
      QgsVector vect1, QgsVector vect2, QgsVector vect3, const QgsPointXY &pt
    );

    /**
    * Interpolate value based on known value on the face of a triangle
    * \param p1 first vertex of the triangle
    * \param p2 second vertex of the triangle
    * \param p3 third vertex of the triangle
    * \param val face value
    * \param pt point where to calculate value
    * \returns value on the point pt or NaN in case the point is outside the triangle
    */
    static double interpolateFromFacesData(
      const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3,
      double val, const QgsPointXY &pt );

    /**
    * Interpolate value based on known value on the face of a triangle
    * \param p1 first vertex of the triangle
    * \param p2 second vertex of the triangle
    * \param p3 third vertex of the triangle
    * \param vect face vector
    * \param pt point where to calculate value
    * \returns vector on the point pt or NaN in case the point is outside the triangle
    *
    * \since QGIS 3.12
    */
    static QgsVector interpolateVectorFromFacesData(
      const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3,
      QgsVector vect, const QgsPointXY &pt );

    /**
    * Interpolate values on vertices from values on faces
    *
    * \since QGIS 3.12
    */
    static QVector<double> interpolateFromFacesData(
      QVector<double> valuesOnFaces,
      const QgsMesh *nativeMesh,
      const QgsTriangularMesh *triangularMesh,
      QgsMeshDataBlock *active,
      QgsMeshRendererScalarSettings::DataResamplingMethod method
    );

    /**
    * Interpolate values on vertices from values on faces
    *
    * \since QGIS 3.18
    */
    static QVector<double> interpolateFromFacesData(
      const QVector<double> &valuesOnFaces,
      const QgsMesh &nativeMesh,
      QgsMeshDataBlock *active,
      QgsMeshRendererScalarSettings::DataResamplingMethod method
    );

    /**
    * Interpolate values on vertices from values on faces
    *
    * \since QGIS 3.20
    */
    static QVector<double> interpolateFromFacesData(
      const QVector<double> &valuesOnFaces,
      const QgsMesh &nativeMesh,
      const QgsMeshDataBlock &active,
      QgsMeshRendererScalarSettings::DataResamplingMethod method
    );

    /**
    * Resamples values on vertices to values on faces
    *
    * \since QGIS 3.14
    */
    static QVector<double> resampleFromVerticesToFaces(
      const QVector<double> valuesOnVertices,
      const QgsMesh *nativeMesh,
      const QgsTriangularMesh *triangularMesh,
      const QgsMeshDataBlock *active,
      QgsMeshRendererScalarSettings::DataResamplingMethod method
    );

    /**
     * Calculates magnitude values ont vertices from the given QgsMeshDataBlock.
     * If the values are defined on faces, the values are interpolated with the given method
     * \param meshLayer the mesh layer
     * \param index the dataset index that contains the data
     * \param activeFaceFlagValues pointer to the QVector containing active face flag values
     * \param method used to interpolate the values on vertices if needed
     * \returns magnitude values of the dataset on all the vertices
     * \since QGIS 3.14
     */
    static QVector<double> calculateMagnitudeOnVertices(
      const QgsMeshLayer *meshLayer,
      const QgsMeshDatasetIndex index,
      QgsMeshDataBlock *activeFaceFlagValues,
      const QgsMeshRendererScalarSettings::DataResamplingMethod method = QgsMeshRendererScalarSettings::NeighbourAverage );

    /**
     * Calculates magnitude values on vertices from the given QgsMeshDataBlock.
     * If the values are defined on faces, the values are interpolated with the given method
     * This method is thread safe.
     * \param nativeMesh the native mesh
     * \param groupMetadata the metadata of the group where come from the dataset values
     * \param datasetValues block containing the dataset values
     * \param activeFaceFlagValues block containing active face flag values
     * \param method used to interpolate the values on vertices if needed
     * \returns magnitude values of the dataset on all the vertices
     * \since QGIS 3.18
     */
    static QVector<double> calculateMagnitudeOnVertices(
      const QgsMesh &nativeMesh,
      const QgsMeshDatasetGroupMetadata &groupMetadata,
      const QgsMeshDataBlock &datasetValues,
      const QgsMeshDataBlock &activeFaceFlagValues,
      const QgsMeshRendererScalarSettings::DataResamplingMethod method = QgsMeshRendererScalarSettings::NeighbourAverage );



    /**
     * Calculates the bounding box of the triangle
     * \param p1 first vertex of the triangle
     * \param p2 second vertex of the triangle
     * \param p3 third vertex of the triangle
     * \returns bounding box of the triangle
     */
    static QgsRectangle triangleBoundingBox( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3 );

    /**
     * Formats hours in human readable string based on settings and reference time
     * If reference time is invalid, return relative time
     * \param hours time in hours from reference time
     * \param referenceTime the reference time
     * \param settings the time settings
     * \return the formatted time
     */
    static QString formatTime( double hours, const QDateTime &referenceTime, const QgsMeshTimeSettings &settings );

    /**
     * Calculates the normals on the vertices using vertical magnitudes instead Z value of vertices
     * \param triangularMesh the triangular mesh
     * \param verticalMagnitude the vertical magnitude values used instead Z value of vertices
     * \param isRelative TRUE if the vertical magnitude is relative to the Z value of vertices
     * \returns normales (3D vector) on all the vertices
     * \since QGIS 3.14
     */
    static QVector<QVector3D> calculateNormals(
      const QgsTriangularMesh &triangularMesh,
      const QVector<double> &verticalMagnitude,
      bool isRelative );
};

///@endcond

#endif // QGSMESHLAYERUTILS_H
