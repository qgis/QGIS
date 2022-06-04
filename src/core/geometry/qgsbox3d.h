/***************************************************************************
                             qgsbox3d.h
                             ----------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSBOX3D_H
#define QGSBOX3D_H

#include "qgis_core.h"
#include "qgsrectangle.h"

#include <QVector3D>

#include "qgspoint.h"

/**
 * \ingroup core
 * \brief A 3-dimensional box composed of x, y, z coordinates.
 *
 * A box composed of x/y/z minimum and maximum values. It is often used to return the 3D
 * extent of a geometry or collection of geometries.
 *
 * \see QgsRectangle
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsBox3d
{
  public:

    /**
     * Constructor for QgsBox3D which accepts the ranges of x/y/z coordinates.
     */
    QgsBox3d( double xmin = 0, double ymin = 0, double zmin = 0, double xmax = 0, double ymax = 0, double zmax = 0 ) SIP_HOLDGIL;

    /**
     * Constructs a QgsBox3D from two points representing opposite corners of the box.
     * The box is normalized after construction.
     */
    QgsBox3d( const QgsPoint &p1, const QgsPoint &p2 ) SIP_HOLDGIL;

    /**
     * Constructs a QgsBox3D from a rectangle.
     * Z Minimum and Z Maximum are set to 0.0.
     */
    QgsBox3d( const QgsRectangle &rect ) SIP_HOLDGIL;

    /**
     * Sets the minimum \a x value.
     * \see xMinimum()
     * \see setXMaximum()
     */
    void setXMinimum( double x ) SIP_HOLDGIL;

    /**
     * Sets the maximum \a x value.
     * \see xMaximum()
     * \see setXMinimum()
     */
    void setXMaximum( double x ) SIP_HOLDGIL;

    /**
     * Returns the minimum x value.
     * \see setXMinimum()
     * \see xMaximum()
     */
    double xMinimum() const SIP_HOLDGIL { return mBounds2d.xMinimum(); }

    /**
     * Returns the maximum x value.
     * \see setXMaximum()
     * \see xMinimum()
     */
    double xMaximum() const SIP_HOLDGIL { return mBounds2d.xMaximum(); }

    /**
     * Sets the minimum \a y value.
     * \see yMinimum()
     * \see setYMaximum()
     */
    void setYMinimum( double y ) SIP_HOLDGIL;

    /**
     * Sets the maximum \a y value.
     * \see yMaximum()
     * \see setYMinimum()
     */
    void setYMaximum( double y ) SIP_HOLDGIL;

    /**
     * Returns the minimum y value.
     * \see setYMinimum()
     * \see yMaximum()
     */
    double yMinimum() const SIP_HOLDGIL { return mBounds2d.yMinimum(); }

    /**
     * Returns the maximum y value.
     * \see setYMaximum()
     * \see yMinimum()
     */
    double yMaximum() const SIP_HOLDGIL { return mBounds2d.yMaximum(); }

    /**
     * Sets the minimum \a z value.
     * \see zMinimum()
     * \see setZMaximum()
     */
    void setZMinimum( double z ) SIP_HOLDGIL;

    /**
     * Sets the maximum \a z value.
     * \see zMaximum()
     * \see setZMinimum()
     */
    void setZMaximum( double z ) SIP_HOLDGIL;

    /**
     * Returns the minimum z value.
     * \see setZMinimum()
     * \see zMaximum()
     */
    double zMinimum() const SIP_HOLDGIL { return mZmin; }

    /**
     * Returns the maximum z value.
     * \see setZMaximum()
     * \see zMinimum()
     */
    double zMaximum() const SIP_HOLDGIL { return mZmax; }

    /**
     * Normalize the box so it has non-negative width/height/depth.
     */
    void normalize();

    /**
     * Returns the width of the box.
     * \see height()
     * \see depth()
     */
    double width() const SIP_HOLDGIL { return mBounds2d.width(); }

    /**
     * Returns the height of the box.
     * \see width()
     * \see depth()
     */
    double height() const SIP_HOLDGIL { return mBounds2d.height(); }

    /**
     * Returns the depth of the box.
     * \see width()
     * \see height()
     */
    double depth() const SIP_HOLDGIL { return mZmax - mZmin; }

    /**
     * Returns the volume of the box.
     */
    double volume() const SIP_HOLDGIL { return mBounds2d.area() * ( mZmax - mZmin ); }

    /**
     * Returns the intersection of this box and another 3D box.
     */
    QgsBox3d intersect( const QgsBox3d &other ) const;

    /**
     * Returns TRUE if the box can be considered a 2-dimensional box, i.e.
     * it has equal minimum and maximum z values.
     */
    bool is2d() const SIP_HOLDGIL;

    /**
     * Returns TRUE if box intersects with another box.
     */
    bool intersects( const QgsBox3d &other ) const;

    /**
     * Returns TRUE when box contains other box.
     */
    bool contains( const QgsBox3d &other ) const;

    /**
     * Returns TRUE when box contains a \a point.
     *
     * If the point is a 2D point (no z-coordinate), then the containment test
     * will be performed on the x/y extent of the box only.
     */
    bool contains( const QgsPoint &point ) const;

    /**
     * Converts the box to a 2D rectangle.
     */
    QgsRectangle toRectangle() const { return mBounds2d; }

    /**
     * Returns the smallest distance between the box and the point \a point
     * (returns 0 if the point is inside the box)
     *
     * \since QGIS 3.18
     */
    double distanceTo( const  QVector3D &point ) const;

    bool operator==( const QgsBox3d &other ) const;

    /**
     * Scale the rectangle around a \a center QgsPoint.
     *
     * If no \a center point is specified then the current center of the box will be used.
     *
     * \since QGIS 3.26
     */
    void scale( double scaleFactor, const QgsPoint &center = QgsPoint() );

    /**
     * Scale the rectangle around a center coordinates.
     *
     * \since QGIS 3.26
     */
    void scale( double scaleFactor, double centerX, double centerY, double centerZ );

  private:

    QgsRectangle mBounds2d;
    double mZmin = 0.0;
    double mZmax = 0.0;

};

#endif // QGSBOX3D_H
