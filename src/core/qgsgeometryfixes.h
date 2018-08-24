/***************************************************************************
                          qgsgeometryfixes.h
                             -------------------
    begin                : Aug 23, 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYFIXES_H
#define QGSGEOMETRYFIXES_H

#include "qgsgeometry.h"

/**
 * The QgsGeometryFixes class contains options to automatically adjust geometries to
 * constraints on a layer.
 *
 * \ingroup core
 */
class CORE_EXPORT QgsGeometryFixes
{
  public:

    /**
     * Create a new QgsGeometryFixes object.
     */
    QgsGeometryFixes() = default;

    /**
     * Automatically remove duplicate nodes on all geometries which are edited on this layer.
     *
     * \since QGIS 3.4
     */
    bool removeDuplicateNodes() const;

    /**
     * Automatically remove duplicate nodes on all geometries which are edited on this layer.
     *
     * \since QGIS 3.4
     */
    void setRemoveDuplicateNodes( bool value );

    /**
     * The precision in which geometries on this layer should be saved.
     * Geometries which are edited on this layer will be rounded to multiples of this value (snap to grid).
     * Set to 0.0 to disable.
     *
     * \since QGIS 3.4
     */
    double geometryPrecision() const;

    /**
     * The precision in which geometries on this layer should be saved.
     * Geometries which are edited on this layer will be rounded to multiples of this value (snap to grid).
     * Set to 0.0 to disable.
     *
     * \since QGIS 3.4
     */
    void setGeometryPrecision( double value );

    /**
     * Determines if at least one fix is enabled.
     *
     * \since QGIS 3.4
     */
    bool isActive() const;

    /**
     * Apply any fixes configured on this class to \a geometry.
     *
     * \since QGIS 3.4
     */
    void apply( QgsGeometry &geometry ) const;

  private:

    /**
     * Automatically remove duplicate nodes on all geometries which are edited on this layer.
     *
     * \since QGIS 3.4
     */
    bool mRemoveDuplicateNodes = false;

    /**
     * The precision in which geometries on this layer should be saved.
     * Geometries which are edited on this layer will be rounded to multiples of this value (snap to grid).
     * Set to 0.0 to disable.
     *
     * \since QGIS 3.4
     */
    double mGeometryPrecision = 0.0;
};

#endif // QGSGEOMETRYFIXES_H
