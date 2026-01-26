/***************************************************************************
  qgscamerapose.cpp
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

#include "qgscamerapose.h"

#include "qgs3dutils.h"

#include <QDomDocument>
#include <Qt3DRender/QCamera>

QDomElement QgsCameraPose::writeXml( QDomDocument &doc ) const
{
  QDomElement elemCamera = doc.createElement( u"camera-pose"_s );
  elemCamera.setAttribute( u"x"_s, mCenterPoint.x() );
  elemCamera.setAttribute( u"y"_s, mCenterPoint.y() );
  elemCamera.setAttribute( u"z"_s, mCenterPoint.z() );
  elemCamera.setAttribute( u"dist"_s, mDistanceFromCenterPoint );
  elemCamera.setAttribute( u"pitch"_s, mPitchAngle );
  elemCamera.setAttribute( u"heading"_s, mHeadingAngle );
  return elemCamera;
}

void QgsCameraPose::readXml( const QDomElement &elem )
{
  const double x = elem.attribute( u"x"_s ).toDouble();
  const double y = elem.attribute( u"y"_s ).toDouble();
  const double z = elem.attribute( u"z"_s ).toDouble();
  mCenterPoint = QgsVector3D( x, y, z );

  mDistanceFromCenterPoint = elem.attribute( u"dist"_s ).toFloat();
  mPitchAngle = elem.attribute( u"pitch"_s ).toFloat();
  mHeadingAngle = elem.attribute( u"heading"_s ).toFloat();
}

void QgsCameraPose::setCenterPoint( const QgsVector3D &point )
{
  // something went horribly wrong. Prevent further errors
  if ( std::isnan( point.x() ) || std::isnan( point.y() ) || std::isnan( point.z() ) )
    qWarning() << "Not updating camera position: it cannot be NaN!";
  else
    mCenterPoint = point;
}

void QgsCameraPose::setDistanceFromCenterPoint( float distance )
{
  mDistanceFromCenterPoint = std::max( distance, 10.0f );
}

void QgsCameraPose::setPitchAngle( float pitch )
{
  // prevent going over the head
  mPitchAngle = std::clamp( pitch, 0.0f, 180.0f );
}

void QgsCameraPose::updateCamera( Qt3DRender::QCamera *camera )
{
  // first rotate by pitch angle around X axis, then by heading angle around Z axis
  QQuaternion q = Qgs3DUtils::rotationFromPitchHeadingAngles( mPitchAngle, mHeadingAngle );
  QVector3D cameraToCenter = q * QVector3D( 0, 0, -mDistanceFromCenterPoint );
  camera->setUpVector( q * QVector3D( 0, 1, 0 ) );
  camera->setPosition( mCenterPoint.toVector3D() - cameraToCenter );
  camera->setViewCenter( mCenterPoint.toVector3D() );
}

void QgsCameraPose::updateCameraGlobe( Qt3DRender::QCamera *camera, double lat, double lon )
{
  // how the camera setup works:
  // - we are using ECEF coordinates (https://en.wikipedia.org/wiki/Earth-centered,_Earth-fixed_coordinate_system)
  //    - point (0,0,0) is in the center of reference ellipsoid
  //    - X and Y axes define equator plane
  //    - X axis grows towards the prime meridian (zero longitude)
  //    - Y axis grows towards 90 degrees of longitude
  //    - Z axis grows towards the north pole

  QVector3D viewCenter = mCenterPoint.toVector3D();

  // rotate camera so that it is looking towards the tangent plane to ellipsoid at given
  // lat/lon coordinates
  QQuaternion qLatLon = QQuaternion::fromAxisAndAngle( QVector3D( 0, 0, 1 ), static_cast<float>( lon ) ) * QQuaternion::fromAxisAndAngle( QVector3D( 0, -1, 0 ), static_cast<float>( lat ) );

  // rotate camera using the pitch and heading angles
  QQuaternion qPitchHeading = QQuaternion::fromAxisAndAngle( QVector3D( 1, 0, 0 ), mHeadingAngle ) * QQuaternion::fromAxisAndAngle( QVector3D( 0, 1, 0 ), mPitchAngle );

  // combine the two rotations (the order is important: pitch/heading is applied first)
  QQuaternion q = qLatLon * qPitchHeading;

  QVector3D cameraToCenter = ( q * QVector3D( -1, 0, 0 ) ) * mDistanceFromCenterPoint;
  camera->setUpVector( q * QVector3D( 0, 0, 1 ) );
  camera->setPosition( viewCenter - cameraToCenter );
  camera->setViewCenter( viewCenter );
}
