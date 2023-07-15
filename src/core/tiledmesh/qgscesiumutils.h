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
class QgsOrientedBoundingBox;

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
