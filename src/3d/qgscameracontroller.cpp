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
#include "qgsvector3d.h"
#include "qgswindow3dengine.h"
#include "qgs3dmapscene.h"
#include "qgsterrainentity.h"
#include "qgis.h"
#include "qgs3dutils.h"

#include <QDomDocument>
#include <Qt3DRender/QCamera>
#include <Qt3DInput>

#include "qgslogger.h"

QgsCameraController::QgsCameraController( Qgs3DMapScene *scene )
  : Qt3DCore::QEntity( scene )
  , mScene( scene )
  , mCamera( scene->engine()->camera() )
  , mCameraBefore( new Qt3DRender::QCamera )
  , mMouseHandler( new Qt3DInput::QMouseHandler )
  , mKeyboardHandler( new Qt3DInput::QKeyboardHandler )
{
  mMouseHandler->setSourceDevice( new Qt3DInput::QMouseDevice() );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::positionChanged,
           this, &QgsCameraController::onPositionChanged );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::wheel,
           this, &QgsCameraController::onWheel );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::pressed,
           this, &QgsCameraController::onMousePressed );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::released,
           this, &QgsCameraController::onMouseReleased );
  addComponent( mMouseHandler );

  mKeyboardHandler->setSourceDevice( new Qt3DInput::QKeyboardDevice() );
  connect( mKeyboardHandler, &Qt3DInput::QKeyboardHandler::pressed,
           this, &QgsCameraController::onKeyPressed );
  connect( mKeyboardHandler, &Qt3DInput::QKeyboardHandler::released,
           this, &QgsCameraController::onKeyReleased );
  addComponent( mKeyboardHandler );

  // Disable the handlers when the entity is disabled
  connect( this, &Qt3DCore::QEntity::enabledChanged,
           mMouseHandler, &Qt3DInput::QMouseHandler::setEnabled );
  connect( this, &Qt3DCore::QEntity::enabledChanged,
           mKeyboardHandler, &Qt3DInput::QKeyboardHandler::setEnabled );

  mFpsNavTimer = new QTimer( this );
  mFpsNavTimer->setInterval( 10 );
  connect( mFpsNavTimer, &QTimer::timeout, this, &QgsCameraController::applyFlyModeKeyMovements );
  mFpsNavTimer->start();
}

QgsCameraController::~QgsCameraController() = default;

QWindow *QgsCameraController::window() const
{
  QgsWindow3DEngine *windowEngine = qobject_cast<QgsWindow3DEngine *>( mScene->engine() );
  return windowEngine ? windowEngine->window() : nullptr;
}

void QgsCameraController::setCameraNavigationMode( Qgis::NavigationMode navigationMode )
{
  if ( navigationMode == mCameraNavigationMode )
    return;

  mCameraNavigationMode = navigationMode;
  mIgnoreNextMouseMove = true;
  emit navigationModeChanged( mCameraNavigationMode );
}

void QgsCameraController::setCameraMovementSpeed( double movementSpeed )
{
  if ( movementSpeed == mCameraMovementSpeed )
    return;

  // If the speed becomes 0, navigation does not work anymore
  // If the speed becomes too important, only one walk can move the view far from the scene.
  mCameraMovementSpeed = std::clamp( movementSpeed, 0.05, 150.0 );
  emit cameraMovementSpeedChanged( mCameraMovementSpeed );
}

void QgsCameraController::setVerticalAxisInversion( Qgis::VerticalAxisInversion inversion )
{
  mVerticalAxisInversion = inversion;
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
  updateCameraFromPose();
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
  QgsTerrainEntity *terrain = mScene->terrainEntity();
  if ( terrain )
    camPose.setCenterPoint( QgsVector3D( worldX, terrain->terrainElevationOffset(), worldY ) );
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
  updateCameraFromPose();
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
  for ( int x = 0; x < buffer.width(); ++x )
  {
    for ( int y = 0; y < buffer.height(); ++y )
    {
      double d = Qgs3DUtils::decodeDepth( buffer.pixel( x, y ) );
      if ( d < 1 )
      {
        depth += d;
        samplesCount += 1;
      }
    }
  }

  // if the whole buffer is white, a depth cannot be computed
  if ( samplesCount == 0 )
    depth = 1.0;
  else
    depth /= samplesCount;

  return depth;
}

void QgsCameraController::updateCameraFromPose()
{
  if ( mCamera )
    mCameraPose.updateCamera( mCamera );

  emit cameraChanged();
}

void QgsCameraController::moveCameraPositionBy( const QVector3D &posDiff )
{
  mCameraPose.setCenterPoint( mCameraPose.centerPoint() + posDiff );
  updateCameraFromPose();
}

void QgsCameraController::onPositionChanged( Qt3DInput::QMouseEvent *mouse )
{
  switch ( mCameraNavigationMode )
  {
    case Qgis::NavigationMode::TerrainBased:
      onPositionChangedTerrainNavigation( mouse );
      break;

    case Qgis::NavigationMode::Walk:
      onPositionChangedFlyNavigation( mouse );
      break;
  }
}

bool QgsCameraController::screenPointToWorldPos( QPoint position, Qt3DRender::QCamera *mCameraBefore, double &depth, QVector3D &worldPosition )
{
  depth = sampleDepthBuffer( mDepthBufferImage, position.x(), position.y() );
  if ( !std::isfinite( depth ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "screenPointToWorldPos: depth is NaN or Inf. This should not happen." ), 2 );
    return false;
  }
  else
  {
    worldPosition = Qgs3DUtils::screenPointToWorldPos( position, depth, mScene->engine()->size(), mCameraBefore );
    if ( !std::isfinite( worldPosition.x() ) || !std::isfinite( worldPosition.y() ) || !std::isfinite( worldPosition.z() ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "screenPointToWorldPos: position is NaN or Inf. This should not happen." ), 2 );
      return false;
    }
    else
    {
      return true;
    }
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
    setMouseParameters( MouseOperation::RotationCenter, mMousePos );

    float scale = static_cast<float>( std::max( mScene->engine()->size().width(), mScene->engine()->size().height() ) );
    float pitchDiff = 180.0f * static_cast<float>( mouse->y() - mClickPoint.y() ) / scale;
    float yawDiff = -180.0f * static_cast<float>( mouse->x() - mClickPoint.x() ) / scale;

    if ( !mDepthBufferIsReady )
      return;

    if ( !mRotationCenterCalculated )
    {
      double depth;
      QVector3D worldPosition;
      if ( screenPointToWorldPos( mClickPoint, mCameraBefore.get(), depth, worldPosition ) )
      {
        mRotationCenter = worldPosition;
        mRotationDistanceFromCenter = ( mRotationCenter - mCameraBefore->position() ).length();
        emit cameraRotationCenterChanged( mRotationCenter );
        mRotationCenterCalculated = true;
      }
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
      QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( mClickPoint, mScene->engine()->size(), mCamera );

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
    setMouseParameters( MouseOperation::RotationCamera );
    // rotate/tilt using mouse (camera stays at one position as it rotates)
    const float diffPitch = 0.2f * dy;
    const float diffYaw = - 0.2f * dx;
    rotateCamera( diffPitch, diffYaw );
  }
  else if ( hasLeftButton && !hasShift && !hasCtrl )
  {
    // translation works as if one grabbed a point on the 3D viewer and dragged it
    setMouseParameters( MouseOperation::Translation, mMousePos );

    if ( !mDepthBufferIsReady )
      return;

    if ( !mDragPointCalculated )
    {
      double depth;
      QVector3D worldPosition;
      if ( screenPointToWorldPos( mClickPoint, mCameraBefore.get(), depth, worldPosition ) )
      {
        mDragDepth = depth;
        mDragPoint = worldPosition;
        mDragPointCalculated = true;
      }

    }

    QVector3D cameraBeforeDragPos = mCameraBefore->position();

    QVector3D moveToPosition = Qgs3DUtils::screenPointToWorldPos( mMousePos, mDragDepth, mScene->engine()->size(), mCameraBefore.get() );
    QVector3D cameraBeforeToMoveToPos = ( moveToPosition - mCameraBefore->position() ).normalized();
    QVector3D cameraBeforeToDragPointPos = ( mDragPoint - mCameraBefore->position() ).normalized();

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

    mCameraPose.setCenterPoint( mCameraBefore->viewCenter() + shiftVector );
    updateCameraFromPose();
  }
  else if ( hasLeftButton && hasShift && hasCtrl )
  {
    // change the camera elevation, similar to pageUp/pageDown
    QgsVector3D center = mCameraPose.centerPoint();
    double tElev = mMousePos.y() - mouse->y();
    center.set( center.x(), center.y() + tElev * 0.5, center.z() );
    mCameraPose.setCenterPoint( center );
    updateCameraFromPose();
  }
  else if ( hasRightButton && !hasShift && !hasCtrl )
  {
    setMouseParameters( MouseOperation::Zoom, mMousePos );
    if ( !mDepthBufferIsReady )
      return;

    if ( !mDragPointCalculated )
    {
      double depth;
      QVector3D worldPosition;
      if ( screenPointToWorldPos( mClickPoint, mCameraBefore.get(), depth, worldPosition ) )
      {
        mDragPoint = worldPosition;
        mDragPointCalculated = true;
      }
    }

    float dist = ( mCameraBefore->position() - mDragPoint ).length();

    int yOffset = 0;
    int screenHeight = mScene->engine()->size().height();
    QWindow *win = window();
    if ( win )
    {
      yOffset = win->mapToGlobal( QPoint( 0, 0 ) ).y();
      screenHeight = win->screen()->size().height();
    }

    // Applies smoothing
    if ( mMousePos.y() > mClickPoint.y() ) // zoom in
    {
      double f = ( double )( mMousePos.y() - mClickPoint.y() ) / ( double )( screenHeight - mClickPoint.y() - yOffset );
      f = std::max( 0.0, std::min( 1.0, f ) );
      f = 1 - ( std::expm1( -2 * f ) ) / ( std::expm1( -2 ) );
      dist = dist * f;
    }
    else // zoom out
    {
      double f = 1 - ( double )( mMousePos.y() + yOffset ) / ( double )( mClickPoint.y() + yOffset );
      f = std::max( 0.0, std::min( 1.0, f ) );
      f = ( std::expm1( 2 * f ) ) / ( std::expm1( 2 ) );
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
      QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( mClickPoint, mScene->engine()->size(), mCamera );
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
    double depth;
    QVector3D worldPosition;
    if ( screenPointToWorldPos( mMousePos, mCameraBefore.get(), depth, worldPosition ) )
    {
      mZoomPoint = worldPosition;
      mZoomPointCalculated = true;
    }
  }

  float f = mCumulatedWheelY / ( 120.0 * 24.0 );

  double dist = ( mZoomPoint - mCameraBefore->position() ).length();
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
    QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( QPoint( mMousePos.x(), mMousePos.y() ), mScene->engine()->size(), mCamera );
    QVector3D clickedPositionWorld = ray.origin() + dist * ray.direction();

    QVector3D shiftVector = clickedPositionWorld - mCamera->viewCenter();

    QVector3D newViewCenterWorld = camera()->viewCenter() - shiftVector;
    QVector3D newCameraPosition = camera()->position() - shiftVector;

    mCameraPose.setDistanceFromCenterPoint( ( newViewCenterWorld - newCameraPosition ).length() );
    mCameraPose.setCenterPoint( newViewCenterWorld );
    updateCameraFromPose();
  }
  mCumulatedWheelY = 0;
  setMouseParameters( MouseOperation::None );
}

void QgsCameraController::onWheel( Qt3DInput::QWheelEvent *wheel )
{
  switch ( mCameraNavigationMode )
  {
    case Qgis::NavigationMode::Walk:
    {
      const float scaling = ( ( wheel->modifiers() & Qt::ControlModifier ) != 0 ? 0.1f : 1.0f ) / 1000.f;
      setCameraMovementSpeed( mCameraMovementSpeed + mCameraMovementSpeed * scaling * wheel->angleDelta().y() );
      break;
    }

    case Qgis::NavigationMode::TerrainBased:
    {

      const float scaling = ( ( wheel->modifiers() & Qt::ControlModifier ) != 0 ? 0.5f : 5.f );

      // Apparently angleDelta needs to be accumulated
      // see: https://doc.qt.io/qt-5/qwheelevent.html#angleDelta
      mCumulatedWheelY += scaling * wheel->angleDelta().y();

      if ( mCurrentOperation != MouseOperation::ZoomWheel )
      {
        setMouseParameters( MouseOperation::ZoomWheel );
      }
      else
      {
        handleTerrainNavigationWheelZoom();
      }
      break;
    }
  }
}

void QgsCameraController::onMousePressed( Qt3DInput::QMouseEvent *mouse )
{
  mKeyboardHandler->setFocus( true );

  if ( mouse->button() == Qt3DInput::QMouseEvent::MiddleButton ||
       ( ( mouse->modifiers() & Qt::ShiftModifier ) != 0 && mouse->button() == Qt3DInput::QMouseEvent::LeftButton ) ||
       ( ( mouse->modifiers() & Qt::ControlModifier ) != 0 && mouse->button() == Qt3DInput::QMouseEvent::LeftButton ) )
  {
    mMousePos = QPoint( mouse->x(), mouse->y() );

    if ( mCaptureFpsMouseMovements )
      mIgnoreNextMouseMove = true;

    const MouseOperation operation
    {
      ( mouse->modifiers() & Qt::ControlModifier ) != 0 && mouse->button() == Qt3DInput::QMouseEvent::LeftButton  ?
      MouseOperation::RotationCamera :
      MouseOperation::RotationCenter
    };
    setMouseParameters( operation, mMousePos );
  }

  else if ( mouse->button() == Qt3DInput::QMouseEvent::LeftButton || mouse->button() == Qt3DInput::QMouseEvent::RightButton )
  {
    mMousePos = QPoint( mouse->x(), mouse->y() );

    if ( mCaptureFpsMouseMovements )
      mIgnoreNextMouseMove = true;

    const MouseOperation operation = ( mouse->button() == Qt3DInput::QMouseEvent::LeftButton ) ? MouseOperation::Translation : MouseOperation::Zoom;
    setMouseParameters( operation, mMousePos );
  }
}

void QgsCameraController::onMouseReleased( Qt3DInput::QMouseEvent *mouse )
{
  Q_UNUSED( mouse )

  setMouseParameters( MouseOperation::None );
}

void QgsCameraController::onKeyPressed( Qt3DInput::QKeyEvent *event )
{
  if ( event->modifiers() & Qt::ControlModifier && event->key() == Qt::Key_QuoteLeft )
  {
    // switch navigation mode
    switch ( mCameraNavigationMode )
    {
      case Qgis::NavigationMode::Walk:
        setCameraNavigationMode( Qgis::NavigationMode::TerrainBased );
        break;
      case Qgis::NavigationMode::TerrainBased:
        setCameraNavigationMode( Qgis::NavigationMode::Walk );
        break;
    }
    return;
  }

  switch ( mCameraNavigationMode )
  {
    case Qgis::NavigationMode::Walk:
    {
      onKeyPressedFlyNavigation( event );
      break;
    }

    case Qgis::NavigationMode::TerrainBased:
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

void QgsCameraController::walkView( double tx, double ty, double tz )
{
  const QVector3D cameraUp = mCamera->upVector().normalized();
  const QVector3D cameraFront = ( QVector3D( mCameraPose.centerPoint().x(), mCameraPose.centerPoint().y(), mCameraPose.centerPoint().z() ) - mCamera->position() ).normalized();
  const QVector3D cameraLeft = QVector3D::crossProduct( cameraUp, cameraFront );

  QVector3D cameraPosDiff( 0.0f, 0.0f, 0.0f );

  if ( tx != 0.0 )
  {
    cameraPosDiff += tx * cameraFront;
  }
  if ( ty != 0.0 )
  {
    cameraPosDiff += ty * cameraLeft;
  }
  if ( tz != 0.0 )
  {
    cameraPosDiff += tz * QVector3D( 0.0f, 1.0f, 0.0f );
  }

  moveCameraPositionBy( cameraPosDiff );
}

void QgsCameraController::applyFlyModeKeyMovements()
{
  // shift = "run", ctrl = "slow walk"
  const bool shiftPressed = mDepressedKeys.contains( Qt::Key_Shift );
  const bool ctrlPressed = mDepressedKeys.contains( Qt::Key_Control );

  const double movementSpeed = mCameraMovementSpeed * ( shiftPressed ? 2 : 1 ) * ( ctrlPressed ? 0.1 : 1 );

  bool changed = false;
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  if ( mDepressedKeys.contains( Qt::Key_Left ) || mDepressedKeys.contains( Qt::Key_A ) )
  {
    changed = true;
    y += movementSpeed;
  }

  if ( mDepressedKeys.contains( Qt::Key_Right ) || mDepressedKeys.contains( Qt::Key_D ) )
  {
    changed = true;
    y -= movementSpeed;
  }

  if ( mDepressedKeys.contains( Qt::Key_Up ) || mDepressedKeys.contains( Qt::Key_W ) )
  {
    changed = true;
    x += movementSpeed;
  }

  if ( mDepressedKeys.contains( Qt::Key_Down ) || mDepressedKeys.contains( Qt::Key_S ) )
  {
    changed = true;
    x -= movementSpeed;
  }

  // note -- vertical axis movements are slower by default then horizontal ones, as GIS projects
  // tend to have much more limited elevation range vs ground range
  static constexpr double ELEVATION_MOVEMENT_SCALE = 0.5;
  if ( mDepressedKeys.contains( Qt::Key_PageUp ) || mDepressedKeys.contains( Qt::Key_E ) )
  {
    changed = true;
    z += ELEVATION_MOVEMENT_SCALE * movementSpeed;
  }

  if ( mDepressedKeys.contains( Qt::Key_PageDown ) || mDepressedKeys.contains( Qt::Key_Q ) )
  {
    changed = true;
    z -= ELEVATION_MOVEMENT_SCALE * movementSpeed;
  }

  if ( changed )
    walkView( x, y, z );
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
        case Qgis::VerticalAxisInversion::Always:
          diffPitch *= -1;
          break;

        case Qgis::VerticalAxisInversion::WhenDragging:
        case Qgis::VerticalAxisInversion::Never:
          break;
      }

      const float diffYaw = - 0.2f * dx;
      rotateCamera( diffPitch, diffYaw );
    }
    else if ( mouse->buttons() & Qt::LeftButton )
    {
      float diffPitch = -0.2f * dy;
      switch ( mVerticalAxisInversion )
      {
        case Qgis::VerticalAxisInversion::Always:
        case Qgis::VerticalAxisInversion::WhenDragging:
          diffPitch *= -1;
          break;

        case Qgis::VerticalAxisInversion::Never:
          break;
      }
      const float diffYaw = - 0.2f * dx;
      rotateCamera( diffPitch, diffYaw );
    }
  }

  if ( mCaptureFpsMouseMovements )
  {
    mIgnoreNextMouseMove = true;

    // reset cursor back to center of map widget
    emit setCursorPosition( QPoint( mScene->engine()->size().width() / 2, mScene->engine()->size().height() / 2 ) );
  }
}

void QgsCameraController::onKeyReleased( Qt3DInput::QKeyEvent *event )
{
  if ( event->isAutoRepeat() )
    return;

  mDepressedKeys.remove( event->key() );
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
    case Qgis::NavigationMode::Walk:
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

    case Qgis::NavigationMode::TerrainBased:
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

  if ( mCurrentOperation == MouseOperation::ZoomWheel )
  {
    handleTerrainNavigationWheelZoom();
  }
}

bool QgsCameraController::isATranslationRotationSequence( MouseOperation newOperation ) const
{
  return std::find( mTranslateOrRotate.begin(), mTranslateOrRotate.end(), newOperation ) != std::end( mTranslateOrRotate ) &&
         std::find( mTranslateOrRotate.begin(), mTranslateOrRotate.end(), mCurrentOperation ) != std::end( mTranslateOrRotate );
}

void QgsCameraController::setMouseParameters( const MouseOperation &newOperation, const QPoint &clickPoint )
{
  if ( newOperation == mCurrentOperation )
  {
    return;
  }

  if ( newOperation == MouseOperation::None )
  {
    mClickPoint = QPoint();
  }
  // click point and rotation angles are updated if:
  // - it has never been computed
  // - the current and new operations are both rotation and translation
  // Indeed, if the sequence such as rotation -> zoom -> rotation updating mClickPoint on
  // the click point does not need to be updated because the relative mouse position is kept
  // during a zoom operation
  else if ( mClickPoint.isNull() || isATranslationRotationSequence( newOperation ) )
  {
    mClickPoint = clickPoint;
    mRotationPitch = mCameraPose.pitchAngle();
    mRotationYaw = mCameraPose.headingAngle();
  }
  mCurrentOperation = newOperation;
  mDepthBufferIsReady = false;
  mRotationCenterCalculated = false;
  mDragPointCalculated = false;
  mZoomPointCalculated = false;

  if ( mCurrentOperation != MouseOperation::None && mCurrentOperation != MouseOperation::RotationCamera )
  {
    emit requestDepthBufferCapture();

    mCameraBefore->setProjectionMatrix( mCamera->projectionMatrix() );
    mCameraBefore->setNearPlane( mCamera->nearPlane() );
    mCameraBefore->setFarPlane( mCamera->farPlane() );
    mCameraBefore->setAspectRatio( mCamera->aspectRatio() );
    mCameraBefore->setFieldOfView( mCamera->fieldOfView() );
    mCameraPose.updateCamera( mCameraBefore.get() );
  }
}
