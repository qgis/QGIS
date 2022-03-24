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
  if ( pitch > 180.0f )
    mPitchAngle = 180.0f;
  else if ( pitch < 0.0f )
    mPitchAngle = 0.0f;
  else
    mPitchAngle = pitch;
}

void QgsCameraPose::updateCamera( Qt3DRender::QCamera *camera )
{
  // basic scene setup:
  // - x grows to the right
  // - z grows to the bottom
  // - y grows towards camera
  // so a point on the plane (x',y') is transformed to (x,-z) in our 3D world
  camera->setUpVector( QVector3D( 0, 0, -1 ) );
  camera->setPosition( QVector3D( mCenterPoint.x(), mDistanceFromCenterPoint + mCenterPoint.y(), mCenterPoint.z() ) );
  camera->setViewCenter( QVector3D( mCenterPoint.x(), mCenterPoint.y(), mCenterPoint.z() ) );
  camera->rotateAboutViewCenter( QQuaternion::fromEulerAngles( mPitchAngle, mHeadingAngle, 0 ) );
}
