/***************************************************************************
  qgscameracontroller.cpp
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

#include "qgscameracontroller.h"
#include "qgsraycastingutils_p.h"
#include "qgsterrainentity_p.h"
#include "qgsvector3d.h"
#include "qgssettings.h"
#include "qgs3dutils.h"

#include "qgis.h"

#include <QDomDocument>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>
#include <Qt3DInput>

#include "qgslogger.h"

QgsCameraController::QgsCameraController( Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mCameraBeforeRotation( new Qt3DRender::QCamera )
  , mCameraBeforeDrag( new Qt3DRender::QCamera )
  , mCameraBeforeZoom( new Qt3DRender::QCamera )
  , mMouseDevice( new Qt3DInput::QMouseDevice() )
  , mKeyboardDevice( new Qt3DInput::QKeyboardDevice() )
  , mMouseHandler( new Qt3DInput::QMouseHandler )
  , mKeyboardHandler( new Qt3DInput::QKeyboardHandler )
{
  mMouseHandler->setSourceDevice( mMouseDevice );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::positionChanged,
           this, &QgsCameraController::onPositionChanged );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::wheel,
           this, &QgsCameraController::onWheel );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::pressed,
           this, &QgsCameraController::onMousePressed );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::released,
           this, &QgsCameraController::onMouseReleased );
  addComponent( mMouseHandler );

  mKeyboardHandler->setSourceDevice( mKeyboardDevice );
  connect( mKeyboardHandler, &Qt3DInput::QKeyboardHandler::pressed,
           this, &QgsCameraController::onKeyPressed );
  connect( mKeyboardHandler, &Qt3DInput::QKeyboardHandler::released,
           this, &QgsCameraController::onKeyReleased );
  addComponent( mKeyboardHandler );

  // Disable the handlers when the entity is disabled
  connect( this, &Qt3DCore::QEntity::enabledChanged,
           mMouseHandler, &Qt3DInput::QMouseHandler::setEnabled );
  connect( this, &Qt3DCore::QEntity::enabledChanged,
           mKeyboardHandler, &Qt3DInput::QMouseHandler::setEnabled );

  mFpsNavTimer = new QTimer( this );
  mFpsNavTimer->setInterval( 10 );
  connect( mFpsNavTimer, &QTimer::timeout, this, &QgsCameraController::applyFlyModeKeyMovements );
  mFpsNavTimer->start();
}

void QgsCameraController::setCameraNavigationMode( QgsCameraController::NavigationMode navigationMode )
{
  if ( navigationMode == mCameraNavigationMode )
    return;

  mCameraNavigationMode = navigationMode;
  mIgnoreNextMouseMove = true;
}

void QgsCameraController::setCameraMovementSpeed( double movementSpeed )
{
  if ( movementSpeed == mCameraMovementSpeed )
    return;

  mCameraMovementSpeed = movementSpeed;
  emit cameraMovementSpeedChanged( mCameraMovementSpeed );
}

void QgsCameraController::setVerticalAxisInversion( QgsCameraController::VerticalAxisInversion inversion )
{
  mVerticalAxisInversion = inversion;
}

void QgsCameraController::setTerrainEntity( QgsTerrainEntity *te )
{
  mTerrainEntity = te;
  // object picker for terrain for correct map panning
  if ( mTerrainEntity )
    connect( te->terrainPicker(), &Qt3DRender::QObjectPicker::pressed, this, &QgsCameraController::onPickerMousePressed );
}

void QgsCameraController::setCamera( Qt3DRender::QCamera *camera )
{
  if ( mCamera == camera )
    return;
  mCamera = camera;

  mCameraPose.updateCamera( mCamera ); // initial setup

  // TODO: set camera's parent if not set already?
  // TODO: registerDestructionHelper (?)
  emit cameraChanged();
}

void QgsCameraController::setViewport( QRect viewport )
{
  if ( mViewport == viewport )
    return;

  mViewport = viewport;
  emit viewportChanged();
}


static QVector3D unproject( QVector3D v, const QMatrix4x4 &modelView, const QMatrix4x4 &projection, QRect viewport )
{
  // Reimplementation of QVector3D::unproject() - see qtbase/src/gui/math3d/qvector3d.cpp
  // The only difference is that the original implementation uses tolerance 1e-5
  // (see qFuzzyIsNull()) as a protection against division by zero. For us it is however
  // common to get lower values (e.g. as low as 1e-8 when zoomed out to the whole Earth with web mercator).

  const QMatrix4x4 inverse = QMatrix4x4( projection * modelView ).inverted();

  QVector4D tmp( v, 1.0f );
  tmp.setX( ( tmp.x() - float( viewport.x() ) ) / float( viewport.width() ) );
  tmp.setY( ( tmp.y() - float( viewport.y() ) ) / float( viewport.height() ) );
  tmp = tmp * 2.0f - QVector4D( 1.0f, 1.0f, 1.0f, 1.0f );

  QVector4D obj = inverse * tmp;
  if ( qgsDoubleNear( obj.w(), 0, 1e-10 ) )
    obj.setW( 1.0f );
  obj /= obj.w();
  return obj.toVector3D();
}


float find_x_on_line( float x0, float y0, float x1, float y1, float y )
{
  const float d_x = x1 - x0;
  const float d_y = y1 - y0;
  const float k = ( y - y0 ) / d_y; // TODO: can we have d_y == 0 ?
  return x0 + k * d_x;
}

QPointF screen_point_to_point_on_plane( QPointF pt, QRect viewport, Qt3DRender::QCamera *camera, float y )
{
  // get two points of the ray
  const QVector3D l0 = unproject( QVector3D( pt.x(), viewport.height() - pt.y(), 0 ), camera->viewMatrix(), camera->projectionMatrix(), viewport );
  const QVector3D l1 = unproject( QVector3D( pt.x(), viewport.height() - pt.y(), 1 ), camera->viewMatrix(), camera->projectionMatrix(), viewport );

  const QVector3D p0( 0, y, 0 ); // a point on the plane
  const QVector3D n( 0, 1, 0 ); // normal of the plane
  const QVector3D l = l1 - l0; // vector in the direction of the line
  const float d = QVector3D::dotProduct( p0 - l0, n ) / QVector3D::dotProduct( l, n );
  const QVector3D p = d * l + l0;

  return QPointF( p.x(), p.z() );
}


void QgsCameraController::rotateCamera( float diffPitch, float diffYaw )
{
  const float pitch = mCameraPose.pitchAngle();
  const float yaw = mCameraPose.headingAngle();

  if ( pitch + diffPitch > 180 )
    diffPitch = 180 - pitch;  // prevent going over the head
  if ( pitch + diffPitch < 0 )
    diffPitch = 0 - pitch;   // prevent going over the head

  // Is it always going to be love/hate relationship with quaternions???
  // This quaternion combines two rotations:
  // - first it undoes the previously applied rotation so we have do not have any rotation compared to world coords
  // - then it applies new rotation
  // (We can't just apply our euler angles difference because the camera may be already rotated)
  const QQuaternion q = QQuaternion::fromEulerAngles( pitch + diffPitch, yaw + diffYaw, 0 ) *
                        QQuaternion::fromEulerAngles( pitch, yaw, 0 ).conjugated();

  // get camera's view vector, rotate it to get new view center
  const QVector3D position = mCamera->position();
  QVector3D viewCenter = mCamera->viewCenter();
  const QVector3D viewVector = viewCenter - position;
  const QVector3D cameraToCenter = q * viewVector;
  viewCenter = position + cameraToCenter;

  mCameraPose.setCenterPoint( viewCenter );
  mCameraPose.setPitchAngle( pitch + diffPitch );
  mCameraPose.setHeadingAngle( yaw + diffYaw );
}


void QgsCameraController::frameTriggered( float dt )
{
  Q_UNUSED( dt )
}

void QgsCameraController::resetView( float distance )
{
  setViewFromTop( 0, 0, distance );
}

void QgsCameraController::setViewFromTop( float worldX, float worldY, float distance, float yaw )
{
  QgsCameraPose camPose;
  if ( mTerrainEntity )
    camPose.setCenterPoint( QgsVector3D( worldX, mTerrainEntity->terrainElevationOffset(), worldY ) );
  else
    camPose.setCenterPoint( QgsVector3D( worldX, 0.0f, worldY ) );
  camPose.setDistanceFromCenterPoint( distance );
  camPose.setHeadingAngle( yaw );

  // a basic setup to make frustum depth range long enough that it does not cull everything
  mCamera->setNearPlane( distance / 2 );
  mCamera->setFarPlane( distance * 2 );

  setCameraPose( camPose );
}

QgsVector3D QgsCameraController::lookingAtPoint() const
{
  return mCameraPose.centerPoint();
}

void QgsCameraController::setLookingAtPoint( const QgsVector3D &point, float distance, float pitch, float yaw )
{
  QgsCameraPose camPose;
  camPose.setCenterPoint( point );
  camPose.setDistanceFromCenterPoint( distance );
  camPose.setPitchAngle( pitch );
  camPose.setHeadingAngle( yaw );
  setCameraPose( camPose );
}

void QgsCameraController::setCameraPose( const QgsCameraPose &camPose )
{
  if ( camPose == mCameraPose )
    return;

  mCameraPose = camPose;

  if ( mCamera )
    mCameraPose.updateCamera( mCamera );

  emit cameraChanged();
}

QDomElement QgsCameraController::writeXml( QDomDocument &doc ) const
{
  QDomElement elemCamera = doc.createElement( QStringLiteral( "camera" ) );
  elemCamera.setAttribute( QStringLiteral( "x" ), mCameraPose.centerPoint().x() );
  elemCamera.setAttribute( QStringLiteral( "y" ), mCameraPose.centerPoint().z() );
  elemCamera.setAttribute( QStringLiteral( "elev" ), mCameraPose.centerPoint().y() );
  elemCamera.setAttribute( QStringLiteral( "dist" ), mCameraPose.distanceFromCenterPoint() );
  elemCamera.setAttribute( QStringLiteral( "pitch" ), mCameraPose.pitchAngle() );
  elemCamera.setAttribute( QStringLiteral( "yaw" ), mCameraPose.headingAngle() );
  return elemCamera;
}

void QgsCameraController::readXml( const QDomElement &elem )
{
  const float x = elem.attribute( QStringLiteral( "x" ) ).toFloat();
  const float y = elem.attribute( QStringLiteral( "y" ) ).toFloat();
  const float elev = elem.attribute( QStringLiteral( "elev" ) ).toFloat();
  const float dist = elem.attribute( QStringLiteral( "dist" ) ).toFloat();
  const float pitch = elem.attribute( QStringLiteral( "pitch" ) ).toFloat();
  const float yaw = elem.attribute( QStringLiteral( "yaw" ) ).toFloat();
  setLookingAtPoint( QgsVector3D( x, elev, y ), dist, pitch, yaw );
}

double QgsCameraController::cameraCenterElevation()
{
  if ( std::isnan( mCameraPose.centerPoint().x() ) || std::isnan( mCameraPose.centerPoint().y() ) || std::isnan( mCameraPose.centerPoint().z() ) )
  {
    // something went horribly wrong but we need to at least try to fix it somehow
    qWarning() << "camera position got NaN!";
    return 0;
  }

  double res = 0.0;

  if ( mCamera && mTerrainEntity )
  {
    // figure out our distance from terrain and update the camera's view center
    // so that camera tilting and rotation is around a point on terrain, not an point at fixed elevation
    QVector3D intersectionPoint;
    QgsRayCastingUtils::Ray3D ray = QgsRayCastingUtils::rayForCameraCenter( mCamera );
    if ( mTerrainEntity->rayIntersection( ray, intersectionPoint ) )
      res = intersectionPoint.y();
    else
      res = mTerrainEntity->terrainElevationOffset();
  }

  if ( mCamera && !mTerrainEntity )
    res = 0.0;

  return res;
}

double QgsCameraController::sampleDepthBuffer( const QImage &buffer, int px, int py )
{
  double depth = 1;

  // Sample the neighbouring pixels for the closest point to the camera
  for ( int x = px - 3; x <= px + 3; ++x )
  {
    for ( int y = py - 3; y <= py + 3; ++y )
    {
      if ( buffer.valid( x, y ) )
      {
        depth = std::min( depth, Qgs3DUtils::decodeDepth( buffer.pixel( x, y ) ) );
      }
    }
  }

  if ( depth < 1 )
    return depth;

  // Returns the average of depth values that are not 1 (void area)
  depth = 0;
  int samplesCount = 0;
  for ( int x = 0; x < mDepthBufferImage.width(); ++x )
  {
    for ( int y = 0; y < mDepthBufferImage.height(); ++y )
    {
      double d = Qgs3DUtils::decodeDepth( buffer.pixel( x, y ) );
      if ( d < 1 )
      {
        depth += d;
        samplesCount += 1;
      }
    }
  }
  depth /= samplesCount;
  return depth;
}

void QgsCameraController::updateCameraFromPose()
{
  // Some changes to be inserted
  if ( std::isnan( mCameraPose.centerPoint().x() ) || std::isnan( mCameraPose.centerPoint().y() ) || std::isnan( mCameraPose.centerPoint().z() ) )
  {
    // something went horribly wrong but we need to at least try to fix it somehow
    qWarning() << "camera position got NaN!";
    mCameraPose.setCenterPoint( QgsVector3D( 0, 0, 0 ) );
  }

  if ( mCameraPose.pitchAngle() > 180 )
    mCameraPose.setPitchAngle( 180 );  // prevent going over the head
  if ( mCameraPose.pitchAngle() < 0 )
    mCameraPose.setPitchAngle( 0 );   // prevent going over the head
  if ( mCameraPose.distanceFromCenterPoint() < 10 )
    mCameraPose.setDistanceFromCenterPoint( 10 );

  if ( mCamera )
    mCameraPose.updateCamera( mCamera );
  emit cameraChanged();
}

void QgsCameraController::moveCameraPositionBy( const QVector3D &posDiff )
{
  mCameraPose.setCenterPoint( mCameraPose.centerPoint() + posDiff );

  if ( mCameraPose.pitchAngle() > 180 )
    mCameraPose.setPitchAngle( 180 );  // prevent going over the head
  if ( mCameraPose.pitchAngle() < 0 )
    mCameraPose.setPitchAngle( 0 );   // prevent going over the head
  if ( mCameraPose.distanceFromCenterPoint() < 10 )
    mCameraPose.setDistanceFromCenterPoint( 10 );

  if ( mCamera )
    mCameraPose.updateCamera( mCamera );

  emit cameraChanged();

}

void QgsCameraController::onPositionChanged( Qt3DInput::QMouseEvent *mouse )
{
  mIsInZoomInState = false;
  mCumulatedWheelY = 0;
  switch ( mCameraNavigationMode )
  {
    case TerrainBasedNavigation:
      onPositionChangedTerrainNavigation( mouse );
      break;

    case WalkNavigation:
      onPositionChangedFlyNavigation( mouse );
      break;
  }
}

void QgsCameraController::onPositionChangedTerrainNavigation( Qt3DInput::QMouseEvent *mouse )
{
  if ( mIgnoreNextMouseMove )
  {
    mIgnoreNextMouseMove = false;
    mMousePos = QPoint( mouse->x(), mouse->y() );
    return;
  }

  const int dx = mouse->x() - mMousePos.x();
  const int dy = mouse->y() - mMousePos.y();

  const bool hasShift = ( mouse->modifiers() & Qt::ShiftModifier );
  const bool hasCtrl = ( mouse->modifiers() & Qt::ControlModifier );
  const bool hasLeftButton = ( mouse->buttons() & Qt::LeftButton );
  const bool hasMiddleButton = ( mouse->buttons() & Qt::MiddleButton );
  const bool hasRightButton = ( mouse->buttons() & Qt::RightButton );

  if ( ( hasLeftButton && hasShift && !hasCtrl ) || ( hasMiddleButton && !hasShift && !hasCtrl ) )
  {
    // rotate/tilt using mouse (camera moves as it rotates around the clicked point)

    double scale = std::max( mViewport.width(), mViewport.height() );
    float pitchDiff = 180 * ( mouse->y() - mMiddleButtonClickPos.y() ) / scale;
    float yawDiff = -180 * ( mouse->x() - mMiddleButtonClickPos.x() ) / scale;

    if ( !mDepthBufferIsReady )
      return;

    if ( !mRotationCenterCalculated )
    {
      double depth = sampleDepthBuffer( mDepthBufferImage, mMiddleButtonClickPos.x(), mMiddleButtonClickPos.y() );

      mRotationCenter = Qgs3DUtils::screenPointToWorldPos( mMiddleButtonClickPos, depth, mViewport.size(), mCameraBeforeRotation.get() );

      mRotationDistanceFromCenter = ( mRotationCenter - mCameraBeforeRotation->position() ).length();

      emit cameraRotationCenterChanged( mRotationCenter );
      mRotationCenterCalculated = true;
    }

    // First transformation : Shift camera position and view center and rotate the camera
    {
      QVector3D shiftVector = mRotationCenter - mCamera->viewCenter();

      QVector3D newViewCenterWorld = camera()->viewCenter() + shiftVector;
      QVector3D newCameraPosition = camera()->position() + shiftVector;

      mCameraPose.setDistanceFromCenterPoint( ( newViewCenterWorld - newCameraPosition ).length() );
      mCameraPose.setCenterPoint( newViewCenterWorld );
      mCameraPose.setPitchAngle( mRotationPitch + pitchDiff );
      mCameraPose.setHeadingAngle( mRotationYaw + yawDiff );
      updateCameraFromPose();
    }


    // Second transformation : Shift camera position back
    {
      QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( QPoint( mMiddleButtonClickPos.x(), mMiddleButtonClickPos.y() ), mViewport.size(), mCamera );

      QVector3D clickedPositionWorld = ray.origin() + mRotationDistanceFromCenter * ray.direction();

      QVector3D shiftVector = clickedPositionWorld - mCamera->viewCenter();

      QVector3D newViewCenterWorld = camera()->viewCenter() - shiftVector;
      QVector3D newCameraPosition = camera()->position() - shiftVector;

      mCameraPose.setDistanceFromCenterPoint( ( newViewCenterWorld - newCameraPosition ).length() );
      mCameraPose.setCenterPoint( newViewCenterWorld );
      updateCameraFromPose();
    }
  }
  else if ( hasLeftButton && hasCtrl && !hasShift )
  {
    // rotate/tilt using mouse (camera stays at one position as it rotates)
    const float diffPitch = 0.2f * dy;
    const float diffYaw = - 0.2f * dx;
    rotateCamera( diffPitch, diffYaw );
    updateCameraFromPose();
  }
  else if ( hasLeftButton && !hasShift && !hasCtrl )
  {
    // translation works as if one grabbed a point on the 3D viewer and dragged it

    if ( !mDepthBufferIsReady )
      return;

    if ( !mDragPointCalculated )
    {
      double depth = sampleDepthBuffer( mDepthBufferImage, mDragButtonClickPos.x(), mDragButtonClickPos.y() );

      mDragDepth = depth;

      mDragPoint = Qgs3DUtils::screenPointToWorldPos( mDragButtonClickPos, mDragDepth, mViewport.size(), mCameraBeforeDrag.get() );
      mDragPointCalculated = true;
    }

    QVector3D cameraBeforeDragPos = mCameraBeforeDrag->position();

    QVector3D moveToPosition = Qgs3DUtils::screenPointToWorldPos( mMousePos, mDragDepth, mViewport.size(), mCameraBeforeDrag.get() );
    QVector3D cameraBeforeToMoveToPos = ( moveToPosition - mCameraBeforeDrag->position() ).normalized();
    QVector3D cameraBeforeToDragPointPos = ( mDragPoint - mCameraBeforeDrag->position() ).normalized();

    // Make sure the rays are not horizontal (add small y shift if it is)
    if ( cameraBeforeToMoveToPos.y() == 0 )
    {
      cameraBeforeToMoveToPos.setY( 0.01 );
      cameraBeforeToMoveToPos = cameraBeforeToMoveToPos.normalized();
    }

    if ( cameraBeforeToDragPointPos.y() == 0 )
    {
      cameraBeforeToDragPointPos.setY( 0.01 );
      cameraBeforeToDragPointPos = cameraBeforeToDragPointPos.normalized();
    }

    double d1 = ( mDragPoint.y() - cameraBeforeDragPos.y() ) / cameraBeforeToMoveToPos.y();
    double d2 = ( mDragPoint.y() - cameraBeforeDragPos.y() ) / cameraBeforeToDragPointPos.y();

    QVector3D from = cameraBeforeDragPos + d1 * cameraBeforeToMoveToPos;
    QVector3D to = cameraBeforeDragPos + d2 * cameraBeforeToDragPointPos;

    QVector3D shiftVector = to - from;

    mCameraPose.setCenterPoint( mCameraBeforeDrag->viewCenter() + shiftVector );
    updateCameraFromPose();
  }
  else if ( hasRightButton && !hasShift && !hasCtrl )
  {
    if ( !mDepthBufferIsReady )
      return;

    if ( !mDragPointCalculated )
    {
      double depth = sampleDepthBuffer( mDepthBufferImage, mDragButtonClickPos.x(), mDragButtonClickPos.y() );

      mDragPoint = Qgs3DUtils::screenPointToWorldPos( mDragButtonClickPos, depth, mViewport.size(), mCameraBeforeDrag.get() );
      mDragPointCalculated = true;
    }

    float dist = ( mCameraBeforeDrag->position() - mDragPoint ).length();

    // Applies smoothing
    if ( mMousePos.y() > mDragButtonClickPos.y() ) // zoom in
    {
      double f = ( double )( mMousePos.y() - mDragButtonClickPos.y() ) / ( double )( mViewport.height() - mDragButtonClickPos.y() );
      f = std::max( 0.0, std::min( 1.0, f ) );
      f = 1 - ( std::exp( -2 * f ) - 1 ) / ( std::exp( -2 ) - 1 );
      dist = dist * f;
    }
    else // zoom out
    {
      double f = 1 - ( double )( mMousePos.y() ) / ( double )( mDragButtonClickPos.y() );
      f = std::max( 0.0, std::min( 1.0, f ) );
      f = ( std::exp( 2 * f ) - 1 ) / ( std::exp( 2 ) - 1 );
      dist = dist + 2 * dist * f;
    }

    // First transformation : Shift camera position and view center and rotate the camera
    {
      QVector3D shiftVector = mDragPoint - mCamera->viewCenter();

      QVector3D newViewCenterWorld = camera()->viewCenter() + shiftVector;

      mCameraPose.setDistanceFromCenterPoint( dist );
      mCameraPose.setCenterPoint( newViewCenterWorld );
      updateCameraFromPose();
    }

    // Second transformation : Shift camera position back
    {
      QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( QPoint( mDragButtonClickPos.x(), mDragButtonClickPos.y() ), mViewport.size(), mCamera );
      QVector3D clickedPositionWorld = ray.origin() + dist * ray.direction();

      QVector3D shiftVector = clickedPositionWorld - mCamera->viewCenter();

      QVector3D newViewCenterWorld = camera()->viewCenter() - shiftVector;
      QVector3D newCameraPosition = camera()->position() - shiftVector;

      mCameraPose.setDistanceFromCenterPoint( ( newViewCenterWorld - newCameraPosition ).length() );
      mCameraPose.setCenterPoint( newViewCenterWorld );
      updateCameraFromPose();
    }
  }

  mMousePos = QPoint( mouse->x(), mouse->y() );
}

void QgsCameraController::zoom( float factor )
{
  // zoom in/out
  float dist = mCameraPose.distanceFromCenterPoint();
  dist -= dist * factor * 0.01f;
  mCameraPose.setDistanceFromCenterPoint( dist );
  updateCameraFromPose();
}

void QgsCameraController::handleTerrainNavigationWheelZoom()
{
  if ( !mDepthBufferIsReady )
    return;

  if ( !mZoomPointCalculated )
  {
    double depth = sampleDepthBuffer( mDepthBufferImage, mMousePos.x(), mMousePos.y() );

    mZoomPoint = Qgs3DUtils::screenPointToWorldPos( mMousePos, depth, mViewport.size(), mCameraBeforeZoom.get() );
    mZoomPointCalculated = true;
  }

  float f = mCumulatedWheelY / ( 120.0 * 24.0 );

  double dist = ( mZoomPoint - mCameraBeforeZoom->position() ).length();
  dist -= dist * f;

  // First transformation : Shift camera position and view center and rotate the camera
  {
    QVector3D shiftVector = mZoomPoint - mCamera->viewCenter();

    QVector3D newViewCenterWorld = camera()->viewCenter() + shiftVector;

    mCameraPose.setDistanceFromCenterPoint( dist );
    mCameraPose.setCenterPoint( newViewCenterWorld );
    updateCameraFromPose();
  }

  // Second transformation : Shift camera position back
  {
    QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( QPoint( mMousePos.x(), mMousePos.y() ), mViewport.size(), mCamera );
    QVector3D clickedPositionWorld = ray.origin() + dist * ray.direction();

    QVector3D shiftVector = clickedPositionWorld - mCamera->viewCenter();

    QVector3D newViewCenterWorld = camera()->viewCenter() - shiftVector;
    QVector3D newCameraPosition = camera()->position() - shiftVector;

    mCameraPose.setDistanceFromCenterPoint( ( newViewCenterWorld - newCameraPosition ).length() );
    mCameraPose.setCenterPoint( newViewCenterWorld );
    updateCameraFromPose();
  }
  mIsInZoomInState = false;
  mCumulatedWheelY = 0;
}

void QgsCameraController::onWheel( Qt3DInput::QWheelEvent *wheel )
{
  switch ( mCameraNavigationMode )
  {
    case QgsCameraController::WalkNavigation:
    {
      const float scaling = ( ( wheel->modifiers() & Qt::ControlModifier ) != 0 ? 0.1f : 1.0f ) / 1000.f;
      setCameraMovementSpeed( mCameraMovementSpeed + mCameraMovementSpeed * scaling * wheel->angleDelta().y() );
      break;
    }

    case TerrainBasedNavigation:
    {

      const float scaling = ( ( wheel->modifiers() & Qt::ControlModifier ) != 0 ? 0.5f : 5.f );

      // Apparently angleDelta needs to be accumulated
      // see: https://doc.qt.io/qt-5/qwheelevent.html#angleDelta
      mCumulatedWheelY += scaling * wheel->angleDelta().y();

      if ( !mIsInZoomInState )
      {
        mCameraPose.updateCamera( mCameraBeforeZoom.get() );

        mCameraBeforeZoom->setProjectionMatrix( mCamera->projectionMatrix() );
        mCameraBeforeZoom->setNearPlane( mCamera->nearPlane() );
        mCameraBeforeZoom->setFarPlane( mCamera->farPlane() );
        mCameraBeforeZoom->setAspectRatio( mCamera->aspectRatio() );
        mCameraBeforeZoom->setFieldOfView( mCamera->fieldOfView() );

        mZoomPointCalculated = false;
        mIsInZoomInState = true;
        mDepthBufferIsReady = false;
        emit requestDepthBufferCapture();
      }
      else
        handleTerrainNavigationWheelZoom();

      break;
    }
  }
}

void QgsCameraController::onMousePressed( Qt3DInput::QMouseEvent *mouse )
{
  mKeyboardHandler->setFocus( true );
  if ( mouse->button() == Qt3DInput::QMouseEvent::LeftButton || mouse->button() == Qt3DInput::QMouseEvent::RightButton )
  {
    mMousePos = QPoint( mouse->x(), mouse->y() );
    mDragButtonClickPos = QPoint( mouse->x(), mouse->y() );
    mPressedButton = mouse->button();
    mMousePressed = true;

    if ( mCaptureFpsMouseMovements )
      mIgnoreNextMouseMove = true;

    mCameraPose.updateCamera( mCameraBeforeDrag.get() );

    mCameraBeforeDrag->setProjectionMatrix( mCamera->projectionMatrix() );
    mCameraBeforeDrag->setNearPlane( mCamera->nearPlane() );
    mCameraBeforeDrag->setFarPlane( mCamera->farPlane() );
    mCameraBeforeDrag->setAspectRatio( mCamera->aspectRatio() );
    mCameraBeforeDrag->setFieldOfView( mCamera->fieldOfView() );

    mDepthBufferIsReady = false;
    mDragPointCalculated = false;

    emit requestDepthBufferCapture();
  }

  if ( mouse->button() == Qt3DInput::QMouseEvent::MiddleButton || ( ( mouse->modifiers() & Qt::ShiftModifier ) != 0 && mouse->button() == Qt3DInput::QMouseEvent::LeftButton ) )
  {
    mMousePos = QPoint( mouse->x(), mouse->y() );
    mMiddleButtonClickPos = QPoint( mouse->x(), mouse->y() );
    mPressedButton = mouse->button();
    mMousePressed = true;
    if ( mCaptureFpsMouseMovements )
      mIgnoreNextMouseMove = true;
    mDepthBufferIsReady = false;
    mRotationCenterCalculated = false;

    mRotationPitch = mCameraPose.pitchAngle();
    mRotationYaw = mCameraPose.headingAngle();

    mCameraPose.updateCamera( mCameraBeforeRotation.get() );

    mCameraBeforeRotation->setProjectionMatrix( mCamera->projectionMatrix() );
    mCameraBeforeRotation->setNearPlane( mCamera->nearPlane() );
    mCameraBeforeRotation->setFarPlane( mCamera->farPlane() );
    mCameraBeforeRotation->setAspectRatio( mCamera->aspectRatio() );
    mCameraBeforeRotation->setFieldOfView( mCamera->fieldOfView() );

    emit requestDepthBufferCapture();
  }
}

void QgsCameraController::onMouseReleased( Qt3DInput::QMouseEvent *mouse )
{
  Q_UNUSED( mouse )
  mPressedButton = Qt3DInput::QMouseEvent::NoButton;
  mMousePressed = false;

  mDragPointCalculated = false;
  mRotationCenterCalculated = false;
}

void QgsCameraController::onKeyPressed( Qt3DInput::QKeyEvent *event )
{
  if ( event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_QuoteLeft )
  {
    // switch navigation mode
    switch ( mCameraNavigationMode )
    {
      case NavigationMode::WalkNavigation:
        setCameraNavigationMode( NavigationMode::TerrainBasedNavigation );
        break;
      case NavigationMode::TerrainBasedNavigation:
        setCameraNavigationMode( NavigationMode::WalkNavigation );
        break;
    }
    emit navigationModeHotKeyPressed( mCameraNavigationMode );
    return;
  }

  switch ( mCameraNavigationMode )
  {
    case WalkNavigation:
    {
      onKeyPressedFlyNavigation( event );
      break;
    }

    case TerrainBasedNavigation:
    {
      onKeyPressedTerrainNavigation( event );
      break;
    }
  }
}

void QgsCameraController::onKeyPressedTerrainNavigation( Qt3DInput::QKeyEvent *event )
{
  const bool hasShift = ( event->modifiers() & Qt::ShiftModifier );
  const bool hasCtrl = ( event->modifiers() & Qt::ControlModifier );

  int tx = 0, ty = 0, tElev = 0;
  switch ( event->key() )
  {
    case Qt::Key_Left:
      tx -= 1;
      break;
    case Qt::Key_Right:
      tx += 1;
      break;

    case Qt::Key_Up:
      ty += 1;
      break;
    case Qt::Key_Down:
      ty -= 1;
      break;

    case Qt::Key_PageDown:
      tElev -= 1;
      break;
    case Qt::Key_PageUp:
      tElev += 1;
      break;
  }

  if ( tx || ty )
  {
    if ( !hasShift && !hasCtrl )
    {
      moveView( tx, ty );
    }
    else if ( hasShift && !hasCtrl )
    {
      // rotate/tilt using keyboard (camera moves as it rotates around its view center)
      tiltUpAroundViewCenter( ty );
      rotateAroundViewCenter( tx );
    }
    else if ( hasCtrl && !hasShift )
    {
      // rotate/tilt using keyboard (camera stays at one position as it rotates)
      const float diffPitch = ty;   // down key = rotating camera down
      const float diffYaw = -tx;    // right key = rotating camera to the right
      rotateCamera( diffPitch, diffYaw );
      updateCameraFromPose();
    }
  }

  if ( tElev )
  {
    QgsVector3D center = mCameraPose.centerPoint();
    center.set( center.x(), center.y() + tElev * 10, center.z() );
    mCameraPose.setCenterPoint( center );
    updateCameraFromPose();
  }
}

void QgsCameraController::onKeyPressedFlyNavigation( Qt3DInput::QKeyEvent *event )
{
  switch ( event->key() )
  {
    case Qt::Key_QuoteLeft:
    {
      // toggle mouse lock mode
      mCaptureFpsMouseMovements = !mCaptureFpsMouseMovements;
      mIgnoreNextMouseMove = true;
      if ( mCaptureFpsMouseMovements )
      {
        qApp->setOverrideCursor( QCursor( Qt::BlankCursor ) );
      }
      else
      {
        qApp->restoreOverrideCursor();
      }
      return;
    }

    case Qt::Key_Escape:
    {
      // always exit mouse lock mode
      if ( mCaptureFpsMouseMovements )
      {
        mCaptureFpsMouseMovements = false;
        mIgnoreNextMouseMove = true;
        qApp->restoreOverrideCursor();
        return;
      }
      break;
    }

    default:
      break;
  }

  if ( event->isAutoRepeat() )
    return;

  mDepressedKeys.insert( event->key() );
}

void QgsCameraController::applyFlyModeKeyMovements()
{
  const QVector3D cameraUp = mCamera->upVector().normalized();
  const QVector3D cameraFront = ( QVector3D( mCameraPose.centerPoint().x(), mCameraPose.centerPoint().y(), mCameraPose.centerPoint().z() ) - mCamera->position() ).normalized();
  const QVector3D cameraLeft = QVector3D::crossProduct( cameraUp, cameraFront );

  QVector3D cameraPosDiff( 0.0f, 0.0f, 0.0f );

  // shift = "run", ctrl = "slow walk"
  const bool shiftPressed = mDepressedKeys.contains( Qt::Key_Shift );
  const bool ctrlPressed = mDepressedKeys.contains( Qt::Key_Control );

  const double movementSpeed = mCameraMovementSpeed * ( shiftPressed ? 2 : 1 ) * ( ctrlPressed ? 0.1 : 1 );

  bool changed = false;
  if ( mDepressedKeys.contains( Qt::Key_Left ) || mDepressedKeys.contains( Qt::Key_A ) )
  {
    changed = true;
    cameraPosDiff += movementSpeed * cameraLeft;
  }

  if ( mDepressedKeys.contains( Qt::Key_Right ) || mDepressedKeys.contains( Qt::Key_D ) )
  {
    changed = true;
    cameraPosDiff += - movementSpeed * cameraLeft;
  }

  if ( mDepressedKeys.contains( Qt::Key_Up ) || mDepressedKeys.contains( Qt::Key_W ) )
  {
    changed = true;
    cameraPosDiff += movementSpeed * cameraFront;
  }

  if ( mDepressedKeys.contains( Qt::Key_Down ) || mDepressedKeys.contains( Qt::Key_S ) )
  {
    changed = true;
    cameraPosDiff += - movementSpeed * cameraFront;
  }

  // note -- vertical axis movements are slower by default then horizontal ones, as GIS projects
  // tend to have much more limited elevation range vs ground range
  static constexpr double ELEVATION_MOVEMENT_SCALE = 0.5;
  if ( mDepressedKeys.contains( Qt::Key_PageUp ) || mDepressedKeys.contains( Qt::Key_E ) )
  {
    changed = true;
    cameraPosDiff += ELEVATION_MOVEMENT_SCALE * movementSpeed * QVector3D( 0.0f, 1.0f, 0.0f );
  }

  if ( mDepressedKeys.contains( Qt::Key_PageDown ) || mDepressedKeys.contains( Qt::Key_Q ) )
  {
    changed = true;
    cameraPosDiff += ELEVATION_MOVEMENT_SCALE * - movementSpeed * QVector3D( 0.0f, 1.0f, 0.0f );
  }

  if ( changed )
    moveCameraPositionBy( cameraPosDiff );
}

void QgsCameraController::onPositionChangedFlyNavigation( Qt3DInput::QMouseEvent *mouse )
{
  const bool hasMiddleButton = ( mouse->buttons() & Qt::MiddleButton );
  const bool hasRightButton = ( mouse->buttons() & Qt::RightButton );

  const double dx = mCaptureFpsMouseMovements ? QCursor::pos().x() - mMousePos.x() : mouse->x() - mMousePos.x();
  const double dy = mCaptureFpsMouseMovements ? QCursor::pos().y() - mMousePos.y() : mouse->y() - mMousePos.y();
  mMousePos = mCaptureFpsMouseMovements ? QCursor::pos() : QPoint( mouse->x(), mouse->y() );

  if ( mIgnoreNextMouseMove )
  {
    mIgnoreNextMouseMove = false;
    return;
  }

  if ( hasMiddleButton )
  {
    // middle button drag = pan camera in place (strafe)
    const QVector3D cameraUp = mCamera->upVector().normalized();
    const QVector3D cameraFront = ( QVector3D( mCameraPose.centerPoint().x(), mCameraPose.centerPoint().y(), mCameraPose.centerPoint().z() ) - mCamera->position() ).normalized();
    const QVector3D cameraLeft = QVector3D::crossProduct( cameraUp, cameraFront );
    const QVector3D cameraPosDiff = -dx * cameraLeft - dy * cameraUp;
    moveCameraPositionBy( mCameraMovementSpeed * cameraPosDiff / 10.0 );
  }
  else if ( hasRightButton )
  {
    // right button drag = camera dolly
    const QVector3D cameraFront = ( QVector3D( mCameraPose.centerPoint().x(), mCameraPose.centerPoint().y(), mCameraPose.centerPoint().z() ) - mCamera->position() ).normalized();
    const QVector3D cameraPosDiff = dy * cameraFront;
    moveCameraPositionBy( mCameraMovementSpeed * cameraPosDiff / 5.0 );
  }
  else
  {
    if ( mCaptureFpsMouseMovements )
    {
      float diffPitch = -0.2f * dy;
      switch ( mVerticalAxisInversion )
      {
        case Always:
          diffPitch *= -1;
          break;

        case WhenDragging:
        case Never:
          break;
      }

      const float diffYaw = - 0.2f * dx;
      rotateCamera( diffPitch, diffYaw );
      updateCameraFromPose();
    }
    else if ( mouse->buttons() & Qt::LeftButton )
    {
      float diffPitch = -0.2f * dy;
      switch ( mVerticalAxisInversion )
      {
        case Always:
        case WhenDragging:
          diffPitch *= -1;
          break;

        case Never:
          break;
      }
      const float diffYaw = - 0.2f * dx;
      rotateCamera( diffPitch, diffYaw );
      updateCameraFromPose();
    }
  }

  if ( mCaptureFpsMouseMovements )
  {
    mIgnoreNextMouseMove = true;

    // reset cursor back to center of map widget
    emit setCursorPosition( QPoint( mViewport.width() / 2, mViewport.height() / 2 ) );
  }
}

void QgsCameraController::onKeyReleased( Qt3DInput::QKeyEvent *event )
{
  if ( event->isAutoRepeat() )
    return;

  mDepressedKeys.remove( event->key() );
}

void QgsCameraController::onPickerMousePressed( Qt3DRender::QPickEvent *pick )
{
  mLastPressedHeight = pick->worldIntersection().y();
}


void QgsCameraController::tiltUpAroundViewCenter( float deltaPitch )
{
  // Tilt up the view by deltaPitch around the view center (camera moves)
  float pitch = mCameraPose.pitchAngle();
  pitch -= deltaPitch;   // down key = moving camera toward terrain
  mCameraPose.setPitchAngle( pitch );
  updateCameraFromPose();
}

void QgsCameraController::rotateAroundViewCenter( float deltaYaw )
{
  // Rotate clockwise the view by deltaYaw around the view center (camera moves)
  float yaw = mCameraPose.headingAngle();
  yaw -= deltaYaw;     // right key = moving camera clockwise
  mCameraPose.setHeadingAngle( yaw );
  updateCameraFromPose();
}

void QgsCameraController::setCameraHeadingAngle( float angle )
{
  mCameraPose.setHeadingAngle( angle );
  updateCameraFromPose();
}

void QgsCameraController::moveView( float tx, float ty )
{
  const float yaw = mCameraPose.headingAngle();
  const float dist = mCameraPose.distanceFromCenterPoint();
  const float x = tx * dist * 0.02f;
  const float y = -ty * dist * 0.02f;

  // moving with keyboard - take into account yaw of camera
  const float t = sqrt( x * x + y * y );
  const float a = atan2( y, x ) - yaw * M_PI / 180;
  const float dx = cos( a ) * t;
  const float dy = sin( a ) * t;

  QgsVector3D center = mCameraPose.centerPoint();
  center.set( center.x() + dx, center.y(), center.z() + dy );
  mCameraPose.setCenterPoint( center );
  updateCameraFromPose();
}

bool QgsCameraController::willHandleKeyEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_QuoteLeft )
    return true;

  switch ( mCameraNavigationMode )
  {
    case WalkNavigation:
    {
      switch ( event->key() )
      {
        case Qt::Key_Left:
        case Qt::Key_A:
        case Qt::Key_Right:
        case Qt::Key_D:
        case Qt::Key_Up:
        case Qt::Key_W:
        case Qt::Key_Down:
        case Qt::Key_S:
        case Qt::Key_PageUp:
        case Qt::Key_E:
        case Qt::Key_PageDown:
        case Qt::Key_Q:
          return true;

        case Qt::Key_Escape:
          if ( mCaptureFpsMouseMovements )
            return true;
          break;

        default:
          break;
      }
      break;
    }

    case TerrainBasedNavigation:
    {
      switch ( event->key() )
      {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
          return true;

        default:
          break;
      }
      break;
    }
  }
  return false;
}

void QgsCameraController::depthBufferCaptured( const QImage &depthImage )
{
  mDepthBufferImage = depthImage;
  mDepthBufferIsReady = true;

  if ( mIsInZoomInState )
    handleTerrainNavigationWheelZoom();
}
