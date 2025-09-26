/***************************************************************************
    qgsraycastcontext.h
    ---------------------
    begin                : September 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRAYCASTCONTEXT_H
#define QGSRAYCASTCONTEXT_H

#include "qgis_3d.h"

/**
 * \ingroup qgis_3d
 *
 * \brief Responsible for defining parameters of the ray casting operations in 3D map canvases.
 *
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsRayCastContext
{
  public:
    QgsRayCastContext() = default;

    /**
     * Sets whether to fetch only the closest hit for each layer or entity type.
     * Setting this to FALSE will return all ray hits.
     * \note Currently only point cloud layers support FALSE.
     */
    void setSingleResult( bool enable );

    /**
     * Returns whether to fetch only the closest hit for each layer or entity type.
     * If not set it defaults to TRUE.
     */
    bool singleResult() const;

    /**
     * Sets the maximum \a distance from ray origin to look for hits when casting a ray.
     * Setting to -1 will set the maximum distance to the camera's far plane.
     */
    void setMaximumDistance( float distance );

    /**
     * The maximum distance from ray origin to look for hits when casting a ray.
     * Default value is -1, meaning that the far plane will be used as a maximum distance.
     */
    float maximumDistance() const;

    /**
     * Sets an \a angle threshold in degrees for ray intersections, effectively turning a ray into a cone.
     * \note Currently only supported for point cloud layers.
     */
    void setAngleThreshold( float angle );

    /**
     * Sets an \a angle threshold in degrees for ray intersections, effectively turning a ray into a cone.
     */
    float angleThreshold() const;

  private:
    bool mSingleResult = true;
    float mMaxDistance = -1.0f;
    float mAngleThreshold = -0.0f;
};

#endif // QGSRAYCASTCONTEXT_H
