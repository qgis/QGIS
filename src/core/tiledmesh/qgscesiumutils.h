/***************************************************************************
                         qgscesiumutils.h
                         --------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCESIUMUTILS_H
#define QGSCESIUMUTILS_H

#include "qgis_core.h"
#include "qgsbox3d.h"
#include "qgis_sip.h"
#include "nlohmann/json_fwd.hpp"

#ifndef SIP_RUN
using namespace nlohmann;
#endif

class QgsSphere;

/**
 * \brief Represents a oriented (rotated) bounding box in 3 dimensions.
 *
 * \ingroup core
 *
 * \warning Non-stable API, exposed to Python for unit testing only.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsOrientedBoundingBox
{
  public:
#ifndef SIP_RUN

    /**
     * Creates an oriented bounding box from a Cesium json object.
     */
    static QgsOrientedBoundingBox fromJson( const json &json );
#endif

    /**
     * Constructor for a null bounding box.
     */
    QgsOrientedBoundingBox();

    /**
     * Constructor for a oriented bounding box, with a specified center and half axes matrix.
     */
    QgsOrientedBoundingBox( const QList<double> &center, QList< double > &halfAxes );

    /**
     * Returns TRUE if the box is a null bounding box.
     */
    bool isNull() const;

    /**
     * Returns the center x-coordinate.
     *
     * \see centerY()
     * \see centerZ()
     */
    double centerX() const { return mCenter[0]; }

    /**
     * Returns the center y-coordinate.

     * \see centerX()
     * \see centerZ()
     */
    double centerY() const { return mCenter[1]; }

    /**
     * Returns the center z-coordinate.
     *
     * \see centerX()
     * \see centerY()
     */
    double centerZ() const { return mCenter[2]; }

    /**
     * Returns the half axes matrix;
     */
    const double *halfAxes() const SIP_SKIP { return mHalfAxes; }

    /**
     * Returns the half axes matrix;
     */
    QList< double > halfAxesList() const SIP_PYNAME( halfAxes );

    /**
     * Returns the overall bounding box of the object.
     */
    QgsBox3d extent() const;

  private:

    double mCenter[ 3 ] { std::numeric_limits< double >::quiet_NaN(), std::numeric_limits< double >::quiet_NaN(), std::numeric_limits< double >::quiet_NaN() };
    double mHalfAxes[9] { 1, 0, 0, 0, 1, 0, 0, 0, 1 };

};

/**
 * \brief Contains utilities for working with Cesium data.
 *
 * \ingroup core
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsCesiumUtils
{
  public:

#ifndef SIP_RUN

    /**
    * Parses a \a region object from a Cesium JSON object to a 3D box.
    *
    * \note Not available in Python bindings.
    */
    static QgsBox3d parseRegion( const json &region );
#endif

    /**
     * Parses a \a region object from a Cesium JSON document to a 3D box.
     */
    static QgsBox3d parseRegion( const QVariantList &region );

#ifndef SIP_RUN

    /**
    * Parses a \a box object from a Cesium JSON document to an oriented bounding box.
    *
    * \note Not available in Python bindings.
    */
    static QgsOrientedBoundingBox parseBox( const json &box );
#endif

    /**
    * Parses a \a box object from a Cesium JSON document to an oriented bounding box.
    */
    static QgsOrientedBoundingBox parseBox( const QVariantList &box );

#ifndef SIP_RUN

    /**
    * Parses a \a sphere object from a Cesium JSON document.
    *
    * \note Not available in Python bindings.
    */
    static QgsSphere parseSphere( const json &sphere );
#endif

    /**
    * Parses a \a sphere object from a Cesium JSON document.
    */
    static QgsSphere parseSphere( const QVariantList &sphere );
};

#endif // QGSCESIUMUTILS_H
