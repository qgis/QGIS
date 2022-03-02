/***************************************************************************
  qgsvectortilestructure.h
  --------------------------------------
  Date                 : March 2022
  Copyright            : (C) 2022 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILESTRUCTURE_H
#define QGSVECTORTILESTRUCTURE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscoordinatereferencesystem.h"

class QgsTileMatrix;

/**
 * Encapsulates properties of a vector tile structure, including tile origins and scaling information.
 * \ingroup core
 * \since QGIS 3.22.6
 */
class CORE_EXPORT QgsVectorTileStructure
{
  public:

    /**
     * Returns a vector tile structure corresponding to the standard web mercator/GoogleCRS84Quad setup.
     */
    static QgsVectorTileStructure fromWebMercator();

    /**
     * Returns the minimum x coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see setZ0xMin()
     */
    double z0xMin() const { return mZ0xMin; }

    /**
     * Sets the minimum \a x coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see z0xMin()
     */
    void setZ0xMin( double x ) { mZ0xMin = x; }

    /**
     * Returns the maximum x coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see setZ0xMax()
     */
    double z0xMax() const { return mZ0xMax; }

    /**
     * Sets the maximum \a x coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see z0xMin()
     */
    void setZ0xMax( double x ) { mZ0xMax = x; }

    /**
     * Returns the minimum y coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see setZ0xMin()
     */
    double z0yMin() const { return mZ0yMin; }

    /**
     * Sets the minimum \a y coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see z0yMin()
     */
    void setZ0yMin( double y ) { mZ0yMin = y; }

    /**
     * Returns the maximum y coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see setZ0yMax()
     */
    double z0yMax() const { return mZ0yMax; }

    /**
     * Sets the maximum \a y coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see z0yMin()
     */
    void setZ0yMax( double y ) { mZ0yMax = y; }

    /**
     * Returns the dimension (width or height, in map units) of the root tiles in zoom level 0.
     *
     * \see setZ0Dimension()
     */
    double z0Dimension() const { return mZ0Dimension; }

    /**
     * Sets the \a dimension (width or height, in map units) of the root tiles in zoom level 0.
     *
     * \see z0Dimension()
     */
    void setZ0Dimension( double dimension ) { mZ0Dimension = dimension;}

    /**
     * Returns the scale denominator corresponding to root tiles in zoom level 0.
     *
     * \see setZ0Dimension()
     */
    double z0Scale() const { return mZ0Scale; }

    /**
     * Sets the \a scale denominator of the root tiles in zoom level 0.
     *
     * \see z0Scale()
     */
    void setZ0Scale( double scale ) { mZ0Scale = scale;}

    /**
     * Returns the coordinate reference system associated with the tiles.
     *
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const { return mCrs; }

    /**
     * Sets the coordinate reference system associated with the tiles.
     *
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs ) { mCrs = crs; }

    /**
     * Returns the minimum zoom level for tiles (where negative = unconstrained).
     *
     * \see setMinimumZoom()
     */
    int minimumZoom() const { return mMinZoom; }

    /**
     * Sets the minimum \a zoom level for tiles (where negative = unconstrained).
     *
     * \see minimumZoom()
     */
    void setMinimumZoom( int zoom ) { mMinZoom = zoom; }

    /**
     * Returns the maximum zoom level for tiles (where negative = unconstrained).
     *
     * \see setMaximumZoom()
     */
    int maximumZoom() const { return mMaxZoom; }

    /**
     * Sets the maximum \a zoom level for tiles (where negative = unconstrained).
     *
     * \see maximumZoom()
     */
    void setMaximumZoom( int zoom ) { mMaxZoom = zoom; }

    /**
     * Returns the tile matrix corresponding to the specified \a zoom.
     */
    QgsTileMatrix tileMatrix( int zoom ) const;

    /**
     * Finds zoom level given a map \a scale denominator.
     */
    double scaleToZoom( double scale ) const;

    /**
     * Finds the best fitting (integer) zoom level given a map \a scale denominator.
     *
     * Values are constrained to the zoom levels between minimumZoom() and maximumZoom().
     */
    int scaleToZoomLevel( double scale ) const;

  private:

    double mZ0xMin = 0;
    double mZ0xMax = 0;
    double mZ0yMin = 0;
    double mZ0yMax = 0;

    double mZ0Dimension = 0;
    double mZ0Scale = 0;

    int mMinZoom = -1;
    int mMaxZoom = -1;

    QgsCoordinateReferenceSystem mCrs;

};

#endif // QGSVECTORTILESTRUCTURE_H
