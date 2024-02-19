/***************************************************************************
  qgsaabb.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAABB_H
#define QGSAABB_H

#include "qgis_3d.h"

#include <cmath>
#include <QList>
#include <QVector3D>

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Axis-aligned bounding box - in world coords.
 * \note Not available in Python bindings
 */
class _3D_EXPORT QgsAABB
{
  public:
    //! Constructs bounding box with null coordinates
    QgsAABB() = default;

    //! Constructs bounding box
    QgsAABB( float xMin, float yMin, float zMin, float xMax, float yMax, float zMax );

    //! Returns box width in X axis
    float xExtent() const { return xMax - xMin; }
    //! Returns box width in Y axis
    float yExtent() const { return yMax - yMin; }
    //! Returns box width in Z axis
    float zExtent() const { return zMax - zMin; }

    //! Returns center in X axis
    float xCenter() const { return ( xMax + xMin ) / 2; }
    //! Returns center in Y axis
    float yCenter() const { return ( yMax + yMin ) / 2; }
    //! Returns center in Z axis
    float zCenter() const { return ( zMax + zMin ) / 2; }

    //! Returns coordinates of the center of the box
    QVector3D center() const { return QVector3D( xCenter(), yCenter(), zCenter() ); }
    //! Returns corner of the box with minimal coordinates
    QVector3D minimum() const { return QVector3D( xMin, yMin, zMin ); }
    //! Returns corner of the box with maximal coordinates
    QVector3D maximum() const { return QVector3D( xMax, yMax, zMax ); }

    //! Determines whether the box intersects some other axis aligned box
    bool intersects( const QgsAABB &other ) const;

    //! Determines whether given coordinate is inside the box
    bool intersects( float x, float y, float z ) const;

    //! Returns shortest distance from the box to a point
    float distanceFromPoint( float x, float y, float z ) const;

    //! Returns shortest distance from the box to a point
    float distanceFromPoint( QVector3D v ) const;

    //! Returns a list of pairs of vertices (useful for display of bounding boxes)
    QList<QVector3D> verticesForLines() const;

    //! Returns text representation of the bounding box
    QString toString() const;

    //! Returns true if xExtent(), yExtent() and zExtent() are all zero, false otherwise
    bool isEmpty() const
    {
      return xMin == xMax && yMin == yMax && zMin == zMax;
    }

    float xMin = 0.0f;
    float yMin = 0.0f;
    float zMin = 0.0f;
    float xMax = 0.0f;
    float yMax = 0.0f;
    float zMax = 0.0f;
};

#endif // QGSAABB_H
