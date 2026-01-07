/***************************************************************************
    qgscrosssection.h
    ---------------------
    begin                : January 2026
    copyright            : (C) 2026 by Dominik Cindrić
    email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCROSSSECTION_H
#define QGSCROSSSECTION_H

#include "qgis_3d.h"
#include "qgsgeometry.h"
#include "qgspointxy.h"

class QgsCoordinateTransform;

/**
 * \ingroup 3d
 * \brief Encapsulates the definition of a cross section in 3D map coordinates.
 *
 * Defined by a line (p1, p2) and a half-width (extent from the line).
 */
class _3D_EXPORT QgsCrossSection
{
  public:
    QgsCrossSection() = default;
    QgsCrossSection( const QgsPoint &p1, const QgsPoint &p2, double halfWidth );

    QgsPoint startPoint() const { return mStartPoint; }
    QgsPoint endPoint() const { return mEndPoint; }

    double halfWidth() const { return mHalfWidth; }

    void setHalfWidth( const double halfWidth ) { mHalfWidth = halfWidth; }

    /**
     * Returns the cross section extent as a geometry (Polygon or LineString).
     * If a coordinate transform is provided, the geometry is transformed.
     * The transform should be from the cross section CRS (3D map CRS) to the destination CRS (2D map canvas CRS).
     */
    QgsGeometry asGeometry( const QgsCoordinateTransform *ct = nullptr ) const;

    void nudgeLeft( double distance );

    void nudgeRight( double distance );

  private:
    QgsPoint mStartPoint;
    QgsPoint mEndPoint;
    double mHalfWidth = 0.0;

    void nudge( double distance );
};

#endif // QGSCROSSSECTION_H
