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

#include <Qt3DRender/QCamera>

#include <QDomDocument>

QDomElement QgsCameraPose::writeXml( QDomDocument &doc ) const
{
  QDomElement elemCamera = doc.createElement( QStringLiteral( "camera-pose" ) );
  elemCamera.setAttribute( QStringLiteral( "x" ), mCenterPoint.x() );
  elemCamera.setAttribute( QStringLiteral( "y" ), mCenterPoint.y() );
  elemCamera.setAttribute( QStringLiteral( "z" ), mCenterPoint.z() );
  elemCamera.setAttribute( QStringLiteral( "dist" ), mDistanceFromCenterPoint );
  elemCamera.setAttribute( QStringLiteral( "pitch" ), mPitchAngle );
  elemCamera.setAttribute( QStringLiteral( "heading" ), mHeadingAngle );
  return elemCamera;
}

void QgsCameraPose::readXml( const QDomElement &elem )
{
  const double x = elem.attribute( QStringLiteral( "x" ) ).toDouble();
  const double y = elem.attribute( QStringLiteral( "y" ) ).toDouble();
  const double z = elem.attribute( QStringLiteral( "z" ) ).toDouble();
  mCenterPoint = QgsVector3D( x, y, z );

  mDistanceFromCenterPoint = elem.attribute( QStringLiteral( "dist" ) ).toFloat();
  mPitchAngle = elem.attribute( QStringLiteral( "pitch" ) ).toFloat();
  mHeadingAngle = elem.attribute( QStringLiteral( "heading" ) ).toFloat();
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
  // prevent bug in QgsCameraPose::updateCamera when updating camera rotation.
  // With a mPitchAngle < 0.2 or > 179.8, QQuaternion::fromEulerAngles( mPitchAngle, mHeadingAngle, 0 )
  // will return bad rotation angle in Qt5.
  // See https://bugreports.qt.io/browse/QTBUG-72103
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  mPitchAngle = std::clamp( pitch, 0.2f, 179.8f );
#else
  mPitchAngle = std::clamp( pitch, 0.0f, 180.0f );
#endif
}

void QgsCameraPose::updateCamera( Qt3DRender::QCamera *camera )
{
  // first rotate by pitch angle around X axis, then by heading angle around Z axis
  // (we use two separate fromEulerAngles() calls because one would not do rotations in order we need)
  QQuaternion q = QQuaternion::fromEulerAngles( 0, 0, mHeadingAngle ) *
                  QQuaternion::fromEulerAngles( mPitchAngle, 0, 0 );
  QVector3D cameraToCenter = q * QVector3D( 0, 0, -mDistanceFromCenterPoint );
  camera->setUpVector( q * QVector3D( 0, 1, 0 ) );
  camera->setPosition( mCenterPoint.toVector3D() - cameraToCenter );
  camera->setViewCenter( mCenterPoint.toVector3D() );
}
