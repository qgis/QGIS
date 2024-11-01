/***************************************************************************
  qgsraycastingutils.h
  --------------------------------------
  Date                 : April 2024
  Copyright            : (C) 2024 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRAYCASTINGUTILS_H
#define QGSRAYCASTINGUTILS_H

#include "qgsfeatureid.h"
#include <QVariantMap>
#include <QVector3D>
#include <QSize>

#define SIP_NO_FILE

namespace QgsRayCastingUtils
{

  /**
   * Helper struct to store ray casting results.
   */
  struct RayHit
  {
      //! Creates a new hit
      RayHit( const float distance, const QVector3D pos, const QgsFeatureId fid = FID_NULL, const QVariantMap attributes = QVariantMap() )
        : distance( distance )
        , pos( pos )
        , fid( fid )
        , attributes( attributes )
      {
      }
      float distance;         //!< Distance from ray's origin
      QVector3D pos;          //!< Hit position in world coordinates
      QgsFeatureId fid;       //!< Fid of feature hit closest to ray origin, FID_NULL if no features hit
      QVariantMap attributes; //!< Point cloud point attributes, empty map if no point cloud points hit
  };

  /**
   * Helper struct to store ray casting parameters.
   */
  struct RayCastContext
  {
      RayCastContext( bool singleResult = true, QSize screenSize = QSize(), float maxDistance = 0.f )
        : singleResult( singleResult )
        , screenSize( screenSize )
        , maxDistance( maxDistance )
      {}
      bool singleResult; //!< If set to TRUE, only the closest point cloud hit will be returned (other entities always return only closest hit)
      QSize screenSize;  //!< QSize of the 3d engine window

      /**
     * The maximum distance from ray origin to look for hits when casting a ray.
     * Should be normally set to far plane, to ignore data that will not get displayed in the 3D view
     */
      float maxDistance;
  };
} // namespace QgsRayCastingUtils

#endif // QGSRAYCASTINGUTILS_H
