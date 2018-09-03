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

class QgsMeshDataProvider;
class QgsMeshDatasetIndex;

#include <QVector>
#include <QSize>

///@cond PRIVATE

/**
 * \ingroup core
 * Misc utility functions used for mesh layer support
 *
 * \note not available in Python bindings
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsMeshLayerUtils
{
  public:

    /**
     * Calculates min/max values from the given vector of values.
     * Ignores any NaN values in the input. Returns NaN for min/max on error.
     */
    static void calculateMinimumMaximum( double &min, double &max, const QVector<double> &arr );

    /**
     * Calculates min/max values for the whole dataset group (considering all datasets within it).
     * Ignores any NaN values in the input. Returns NaN for min/max on error.
     */
    static void calculateMinMaxForDatasetGroup( double &min, double &max, QgsMeshDataProvider *provider, int groupIndex );

    /**
     * Calculates min/max values for one dataset.
     * Ignores any NaN values in the input. Returns NaN for min/max on error.
     */
    static void calculateMinMaxForDataset( double &min, double &max, QgsMeshDataProvider *provider, QgsMeshDatasetIndex index );

    /**
     * Transformes the bounding box to rectangle in screen coordinates (in pixels)
     * \param mtp actual renderer map to pixel
     * \param outputSize actual renderer output size
     * \param bbox bounding box in map coordinates
     * \param leftLim minimum x coordinate in pixel
     * \param rightLim maximum x coordinate in pixel
     * \param topLim minimum y coordinate in pixel
     * \param bottomLim maximum y coordinate in pixel
     */
    static void boundingBoxToScreenRectangle(
      const QgsMapToPixel &mtp,
      const QSize &outputSize,
      const QgsRectangle &bbox,
      int &leftLim, int &rightLim, int &topLim, int &bottomLim );

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
     * Calculates the bounding box of the triangle
     * \param p1 first vertex of the triangle
     * \param p2 second vertex of the triangle
     * \param p3 third vertex of the triangle
     * \returns bounding box of the triangle
     */
    static QgsRectangle triangleBoundingBox( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3 );
};

///@endcond

#endif // QGSMESHLAYERUTILS_H
