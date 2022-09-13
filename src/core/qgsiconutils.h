/***************************************************************************
                             qgsiconutils.h
                             -------------------
    begin                : May 2021
    copyright            : (C) 2021 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSICONUTILS_H
#define QGSICONUTILS_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"
#include "qgswkbtypes.h"

class QgsMapLayer;
class QIcon;

/**
 * \ingroup core
 * \brief Contains utility functions for working with icons.
 * \since QGIS 3.20
*/
class CORE_EXPORT QgsIconUtils
{

  public:

    /**
     * Returns the icon for a vector layer whose geometry \a type is provided.
     */
    static QIcon iconForWkbType( QgsWkbTypes::Type type );

    /**
     * Returns the icon for a vector layer whose geometry \a typeGroup is provided.
     * 1since QGIS 3.28
     */
    static QIcon iconForGeometryType( QgsWkbTypes::GeometryType typeGroup );

    /**
     * Returns an icon representing point geometries.
     */
    static QIcon iconPoint();

    /**
     * Returns an icon representing line geometries.
     */
    static QIcon iconLine();

    /**
     * Returns an icon representing polygon geometries.
     */
    static QIcon iconPolygon();

    /**
     * Returns an icon representing geometry collections.
     *
     * \since QGIS 3.22
     */
    static QIcon iconGeometryCollection();

    /**
     * Returns an icon representing non-spatial layers (tables).
     */
    static QIcon iconTable();

    /**
     * Returns an icon representing raster layers.
     */
    static QIcon iconRaster();

    /**
     * Returns a default icon for layers, which aren't the standard raster/vector/... types.
     */
    static QIcon iconDefaultLayer();

    /**
     * Returns an icon representing mesh layers.
     */
    static QIcon iconMesh();

    /**
     * Returns an icon representing vector tile layers.
     */
    static QIcon iconVectorTile();

    /**
     * Returns an icon representing point cloud layers.
     */
    static QIcon iconPointCloud();

    /**
     * Returns the icon corresponding to a specified map \a layer.
     */
    static QIcon iconForLayer( const QgsMapLayer *layer );

    /**
     * Returns the default icon for the specified layer \a type.
     *
     * \since QGIS 3.22
     */
    static QIcon iconForLayerType( QgsMapLayerType type );

};

#endif // QGSICONUTILS_H


