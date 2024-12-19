/***************************************************************************
  qgscamerapose.h
  --------------------------------------
  Date                 : July 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCAMERAPOSE_H
#define QGSCAMERAPOSE_H

#include "qgis_3d.h"

#include "qgsvector3d.h"

#ifndef SIP_RUN
namespace Qt3DRender
{
  class QCamera;
}
#endif

class QDomDocument;
class QDomElement;

/**
 * \ingroup 3d
 * \brief Class that encapsulates camera pose in a 3D scene.
 *
 * The pose is defined with the following parameters:
 *
 * - center point - towards which point the camera is looking
 * - distance from the center point - how far is the camera from the point towards which it is looking
 * - pitch angle - vertical rotation of the camera (0 degrees = camera looking down, 90 degrees = camera looking from the side)
 * - yaw angle - horizontal rotation of the camera
 *
 * \since QGIS 3.4
 */
class _3D_EXPORT QgsCameraPose
{
  public:

    //! Returns center point (towards which point the camera is looking)
    QgsVector3D centerPoint() const { return mCenterPoint; }
    //! Sets center point (towards which point the camera is looking)
    void setCenterPoint( const QgsVector3D &point );

    //! Returns distance of the camera from the center point
    float distanceFromCenterPoint() const { return mDistanceFromCenterPoint; }
    //! Sets distance of the camera from the center point
    void setDistanceFromCenterPoint( float distance );

    //! Returns pitch angle in degrees
    float pitchAngle() const { return mPitchAngle; }
    //! Sets pitch angle in degrees
    void setPitchAngle( float pitch );

    //! Returns heading (yaw) angle in degrees
    float headingAngle() const { return mHeadingAngle; }
    //! Sets heading (yaw) angle in degrees
    void setHeadingAngle( float heading ) { mHeadingAngle = heading; }

    //! Update Qt3D camera view matrix based on the pose
    void updateCamera( Qt3DRender::QCamera *camera ) SIP_SKIP;

    //! Writes configuration to a new DOM element and returns it
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads configuration from a DOM element previously written using writeXml()
    void readXml( const QDomElement &elem );

    // TODO c++20 - replace with = default
    bool operator==( const QgsCameraPose &other ) const
    {
      return mCenterPoint == other.mCenterPoint &&
             mDistanceFromCenterPoint == other.mDistanceFromCenterPoint &&
             mPitchAngle == other.mPitchAngle &&
             mHeadingAngle == other.mHeadingAngle;
    }
    bool operator!=( const QgsCameraPose &other ) const
    {
      return !operator==( other );
    }

  private:
    //! ground point towards which the camera is looking
    QgsVector3D mCenterPoint;
    //! distance of camera from the point it is looking at
    float mDistanceFromCenterPoint = 1000;
    //! aircraft nose up/down (0 = looking straight down to the plane). angle in degrees
    // prevent bug in QgsCameraPose::updateCamera when updating camera rotation.
    // With a mPitchAngle < 0.2 or > 179.8, QQuaternion::fromEulerAngles( mPitchAngle, mHeadingAngle, 0 )
    // will return bad rotation angle.
    // See https://bugreports.qt.io/browse/QTBUG-72103
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    float mPitchAngle = 0.2f;
#else
    float mPitchAngle = 0.0f;
#endif

    //! aircraft nose left/right. angle in degrees
    float mHeadingAngle = 0;
};


#endif // QGSCAMERAPOSE_H
