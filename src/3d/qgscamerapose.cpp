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
