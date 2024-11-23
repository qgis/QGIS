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

#include <QSet>
#include <QVariantMap>

#include "qgshttpheaders.h"


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
class QgsMapBoxGlStyleConversionContext;

/**
 * \ingroup core
 * \brief Random utility functions for working with vector tiles
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsVectorTileUtils
{
  public:

    /**
     * Parses the style URL to update the source URLs in the \a uri.
     * If \a forceUpdate is TRUE, any existing source will be updated.
     * \since QGIS 3.40
     */
    static void updateUriSources( QString &uri SIP_INOUT, bool forceUpdate = false );

    //! Orders tile requests according to the distance from view center (given in tile matrix coords)
    static void sortTilesByDistanceFromCenter( QVector<QgsTileXYZ> &tiles, QPointF center );

    /**
     * Returns polygon (made by four corners of the tile) in screen coordinates
     *
     * \throws QgsCsException
     */
    static QPolygon tilePolygon( QgsTileXYZ id, const QgsCoordinateTransform &ct, const QgsTileMatrix &tm, const QgsMapToPixel &mtp );

    //! Returns QgsFields instance based on the set of field names
    static QgsFields makeQgisFields( const QSet<QString> &flds );

    /**
     * Finds zoom level given map scale denominator.
     *
     * The \a z0Scale argument gives the scale denominator at zoom level 0, where the default
     * value corresponds to GoogleCRS84Quad tile matrix set
     *
     * \since QGIS 3.16
     */
    static double scaleToZoom( double mapScale, double z0Scale = 559082264.0287178 );

    /**
     * Finds the best fitting zoom level given a map scale denominator and allowed zoom level range.
     *
     * The \a z0Scale argument gives the scale denominator at zoom level 0, where the default
     * value corresponds to GoogleCRS84Quad tile matrix set.
     */
    static int scaleToZoomLevel( double mapScale, int sourceMinZoom, int sourceMaxZoom, double z0Scale = 559082264.0287178 );
    //! Returns a temporary vector layer for given sub-layer of tile in vector tile layer
    static QgsVectorLayer *makeVectorLayerForTile( QgsVectorTileLayer *mvt, QgsTileXYZ tileID, const QString &layerName );
    //! Returns formatted tile URL string replacing {x}, {y}, {z} placeholders (or {-y} instead of {y} for TMS convention)
    static QString formatXYZUrlTemplate( const QString &url, QgsTileXYZ tile, const QgsTileMatrix &tileMatrix );
    //! Checks whether the URL template string is correct (contains {x}, {y} / {-y}, {z} placeholders)
    static bool checkXYZUrlTemplate( const QString &url );

    /**
     * Downloads the sprite image and sets it to the conversion context
     * \param styleDefinition the style definition map
     * \param context the style conversion context
     * \param styleUrl optional the style url
     */
    static void loadSprites( const QVariantMap &styleDefinition, QgsMapBoxGlStyleConversionContext &context, const QString &styleUrl = QString() );

  private:
    //! Parses the style URL to get a list of named source URLs.
    static QMap<QString, QString> parseStyleSourceUrl( const QString &styleUrl, const QgsHttpHeaders &headers = QgsHttpHeaders(), const QString &authCfg = QString() );

    //! Returns the tiles URLs of a source
    static QVariantList parseStyleSourceContentUrl( const QString &sourceUrl, const QgsHttpHeaders &headers = QgsHttpHeaders(), const QString &authCfg = QString() );

    friend class TestQgsVectorTileUtils;

};

#endif // QGSVECTORTILEUTILS_H
