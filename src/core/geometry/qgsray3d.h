/***************************************************************************
  qgsray3d.h
  --------------------------------------
  Date                 : January 2021
  Copyright            : (C) 2021 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRAY3D_H
#define QGSRAY3D_H

#include "qgsbox3d.h"

#include <QVector3D>

/**
 * \ingroup core
 * \brief A representation of a ray in 3D.
 *
 * A ray is composed of an origin point (the start of the ray) and a direction vector.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsRay3D
{
  public:

    /**
     * Constructor
     * \note : the direction is automatically normalized
     */
    QgsRay3D( const QVector3D &origin, const QVector3D &direction );

    /**
     * Returns the origin of the ray
     * \see setOrigin()
     */
    QVector3D origin() const { return mOrigin; }

    /**
     * Returns the direction of the ray
     * see setDirection()
     */
    QVector3D direction() const { return mDirection; }

    /**
     * Returns a vector with the direction components inversed ( 1/x, 1/y, 1/z)
     * This can be used as an optimization when used in intersection logic.
     * see direction()
     */
    QVector3D directionInversed() const { return mDirectionInversed; }

    /**
     * Sets the origin of the ray
     * \see origin()
     */
    void setOrigin( const QVector3D &origin );

    /**
     * Sets the direction of the ray
     * \note : the direction is automatically normalized
     * \see direction()
     */
    void setDirection( const QVector3D direction );

    /**
     * Returns the projection of the point on the ray
     * (which is the closest point of the ray to \a point)
     */
    QVector3D projectedPoint( const QVector3D &point ) const;

    /**
     * Returns the distance of the projection of a point to the ray
     */
    float projectedDistance( const QVector3D &point ) const;

    //! Checks whether the point is in front of the ray
    bool isInFront( const QVector3D &point ) const;
    //! Returns the angle between the ray and the vector from the ray's origin and the point \a point
    double angleToPoint( const QVector3D &point ) const;

    //! Returns the point along the ray with the specified \a distance from the ray's origin
    QVector3D point( float distance ) const;

  private:
    QVector3D mOrigin;
    QVector3D mDirection;
    QVector3D mDirectionInversed;
};

#endif // QGSRAY3D_H
