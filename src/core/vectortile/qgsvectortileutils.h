/***************************************************************************
  qgsvectortileutils.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILEUTILS_H
#define QGSVECTORTILEUTILS_H

#include "qgis_core.h"

#define SIP_NO_FILE

#include <QSet>

class QPointF;
class QPolygon;

class QgsCoordinateTransform;
class QgsFields;
class QgsMapToPixel;
class QgsRectangle;
class QgsVectorLayer;

class QgsTileMatrix;
class QgsTileRange;
class QgsTileXYZ;
class QgsVectorTileLayer;

/**
 * \ingroup core
 * Random utility functions for working with vector tiles
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileUtils
{
  public:

    //! Returns a list of tiles in the given tile range
    static QVector<QgsTileXYZ> tilesInRange( const QgsTileRange &range, int zoomLevel );
    //! Orders tile requests according to the distance from view center (given in tile matrix coords)
    static void sortTilesByDistanceFromCenter( QVector<QgsTileXYZ> &tiles, const QPointF &center );

    //! Returns polygon (made by four corners of the tile) in screen coordinates
    static QPolygon tilePolygon( QgsTileXYZ id, const QgsCoordinateTransform &ct, const QgsTileMatrix &tm, const QgsMapToPixel &mtp );
    //! Returns QgsFields instance based on the set of field names
    static QgsFields makeQgisFields( QSet<QString> flds );
    //! Finds best fitting zoom level (assuming GoogleCRS84Quad tile matrix set) given map scale denominator and allowed zoom level range
    static int scaleToZoomLevel( double mapScale, int sourceMinZoom, int sourceMaxZoom );
    //! Returns a temporary vector layer for given sub-layer of tile in vector tile layer
    static QgsVectorLayer *makeVectorLayerForTile( QgsVectorTileLayer *mvt, QgsTileXYZ tileID, const QString &layerName );
    //! Returns formatted tile URL string replacing {x}, {y}, {z} placeholders
    static QString formatXYZUrlTemplate( const QString &url, QgsTileXYZ tile );
};

#endif // QGSVECTORTILEUTILS_H
