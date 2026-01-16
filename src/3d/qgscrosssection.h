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
 * \ingroup qgis_3d
 * \brief Encapsulates the definition of a cross section in 3D map coordinates.
 *
 * Defined by a line (p1, p2) and a half-width (extent from the line).
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsCrossSection
{
  public:
    //! Constructs an invalid cross section
    QgsCrossSection() = default;

    //! Constructs a cross section defined by two points and a half-width
    QgsCrossSection( const QgsPoint &p1, const QgsPoint &p2, double halfWidth );

    /**
     * Returns cross section validity.
     * A valid cross section has distinct start and end points and a positive half-width.
     */
    bool isValid() const;

    //! Returns the start point of the cross section
    QgsPoint startPoint() const { return mStartPoint; }

    //! Returns the end point of the cross section
    QgsPoint endPoint() const { return mEndPoint; }

    //! Returns the half-width of the cross section
    double halfWidth() const { return mHalfWidth; }

    //! Sets the half-width of the cross section
    void setHalfWidth( const double halfWidth ) { mHalfWidth = halfWidth; }

    /**
     * Returns the cross section extent as a geometry (Polygon or LineString).
     * If a coordinate transform is provided, the geometry is transformed.
     * The transform should be from the cross section CRS (3D map CRS) to the destination CRS (2D map canvas CRS).
     */
    QgsGeometry asGeometry( const QgsCoordinateTransform *ct = nullptr ) const;

    //! Nudges the cross section to the left by the specified distance
    void nudgeLeft( double distance );

    //! Nudges the cross section to the right by the specified distance
    void nudgeRight( double distance );

  private:
    QgsPoint mStartPoint;
    QgsPoint mEndPoint;
    double mHalfWidth = 0.0;

    void nudge( double distance );
};

#endif // QGSCROSSSECTION_H
