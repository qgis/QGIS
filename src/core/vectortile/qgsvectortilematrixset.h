/***************************************************************************
  qgsvectortilematrixset.h
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

#ifndef QGSVECTORTILEMATRIXSET_H
#define QGSVECTORTILEMATRIXSET_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgscoordinatereferencesystem.h"
#include "qgstiles.h"

class QgsTileMatrix;

/**
 * Encapsulates properties of a vector tile matrix set, including tile origins and scaling information.
 * \ingroup core
 * \since QGIS 3.22.6
 */
class CORE_EXPORT QgsVectorTileMatrixSet : public QgsTileMatrixSet
{
  public:

    /**
     * Returns a vector tile structure corresponding to the standard web mercator/GoogleCRS84Quad setup.
     */
    static QgsVectorTileMatrixSet fromWebMercator();

    bool readXml( const QDomElement &element, QgsReadWriteContext &context ) override;
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const override;

    /**
     * Returns the minimum x coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see setZ0xMinimum()
     */
    double z0xMinimum() const { return mZ0xMin; }

    /**
     * Sets the minimum \a x coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see z0xMinimum()
     */
    void setZ0xMinimum( double x ) { mZ0xMin = x; }

    /**
     * Returns the maximum x coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see setZ0xMaximum()
     */
    double z0xMaximum() const { return mZ0xMax; }

    /**
     * Sets the maximum \a x coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see z0xMaximum()
     */
    void setZ0xMaximum( double x ) { mZ0xMax = x; }

    /**
     * Returns the minimum y coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see setZ0yMinimum()
     */
    double z0yMinimum() const { return mZ0yMin; }

    /**
     * Sets the minimum \a y coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see z0yMinimum()
     */
    void setZ0yMinimum( double y ) { mZ0yMin = y; }

    /**
     * Returns the maximum y coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see setZ0yMaximum()
     */
    double z0yMaximum() const { return mZ0yMax; }

    /**
     * Sets the maximum \a y coordinate for zoom level 0.
     *
     * This property is used when scaling raw vector tile coordinates.
     *
     * \see z0yMaximum()
     */
    void setZ0yMaximum( double y ) { mZ0yMax = y; }

    /**
     * Initializes the tile structure settings from an ESRI REST VectorTileService \a json map.
     *
     * \note This same structure is utilized in ESRI vtpk archives in the root.json file.
     */
    bool fromEsriJson( const QVariantMap &json );

  private:

    double mZ0xMin = 0;
    double mZ0xMax = 0;
    double mZ0yMin = 0;
    double mZ0yMax = 0;
};

#endif // QGSVECTORTILEMATRIXSET_H
