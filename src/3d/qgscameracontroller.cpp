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

#include "qgis.h"

#include <QDomDocument>
#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>


QgsCameraController::QgsCameraController( Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mMouseDevice( new Qt3DInput::QMouseDevice() )
  , mKeyboardDevice( new Qt3DInput::QKeyboardDevice() )
  , mMouseHandler( new Qt3DInput::QMouseHandler )
  , mLogicalDevice( new Qt3DInput::QLogicalDevice() )
  , mLeftMouseButtonAction( new Qt3DInput::QAction() )
  , mLeftMouseButtonInput( new Qt3DInput::QActionInput() )
  , mMiddleMouseButtonAction( new Qt3DInput::QAction() )
  , mMiddleMouseButtonInput( new Qt3DInput::QActionInput() )
  , mRightMouseButtonAction( new Qt3DInput::QAction() )
  , mRightMouseButtonInput( new Qt3DInput::QActionInput() )
  , mShiftAction( new Qt3DInput::QAction() )
  , mShiftInput( new Qt3DInput::QActionInput() )
  , mCtrlAction( new Qt3DInput::QAction() )
  , mCtrlInput( new Qt3DInput::QActionInput() )
  , mWheelAxis( new Qt3DInput::QAxis() )
  , mMouseWheelInput( new Qt3DInput::QAnalogAxisInput() )
  , mTxAxis( new Qt3DInput::QAxis() )
  , mTyAxis( new Qt3DInput::QAxis() )
  , mKeyboardTxPosInput( new Qt3DInput::QButtonAxisInput() )
  , mKeyboardTyPosInput( new Qt3DInput::QButtonAxisInput() )
  , mKeyboardTxNegInput( new Qt3DInput::QButtonAxisInput() )
  , mKeyboardTyNegInput( new Qt3DInput::QButtonAxisInput() )
  , mTelevAxis( new Qt3DInput::QAxis() )
  , mKeyboardTelevPosInput( new Qt3DInput::QButtonAxisInput() )
  , mKeyboardTelevNegInput( new Qt3DInput::QButtonAxisInput() )
{

  // not using QAxis + QAnalogAxisInput for mouse X,Y because
  // it is only in action when a mouse button is pressed.
  mMouseHandler->setSourceDevice( mMouseDevice );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::positionChanged,
           this, &QgsCameraController::onPositionChanged );
  addComponent( mMouseHandler );

  // TODO: keep using QAxis and QAction approach or just switch to using QMouseHandler / QKeyboardHandler?
  // it does not feel like the former approach makes anything much simpler

  // left mouse button
  mLeftMouseButtonInput->setButtons( QVector<int>() << Qt::LeftButton );
  mLeftMouseButtonInput->setSourceDevice( mMouseDevice );
  mLeftMouseButtonAction->addInput( mLeftMouseButtonInput );

  // middle mouse button
  mMiddleMouseButtonInput->setButtons( QVector<int>() << Qt::MiddleButton );
  mMiddleMouseButtonInput->setSourceDevice( mMouseDevice );
  mMiddleMouseButtonAction->addInput( mMiddleMouseButtonInput );

  // right mouse button
  mRightMouseButtonInput->setButtons( QVector<int>() << Qt::RightButton );
  mRightMouseButtonInput->setSourceDevice( mMouseDevice );
  mRightMouseButtonAction->addInput( mRightMouseButtonInput );

  // Mouse Wheel (Y)
  mMouseWheelInput->setAxis( Qt3DInput::QMouseDevice::WheelY );
  mMouseWheelInput->setSourceDevice( mMouseDevice );
  mWheelAxis->addInput( mMouseWheelInput );

  // Keyboard shift
  mShiftInput->setButtons( QVector<int>() << Qt::Key_Shift );
  mShiftInput->setSourceDevice( mKeyboardDevice );
  mShiftAction->addInput( mShiftInput );

  // Keyboard ctrl
  mCtrlInput->setButtons( QVector<int>() << Qt::Key_Control );
  mCtrlInput->setSourceDevice( mKeyboardDevice );
  mCtrlAction->addInput( mCtrlInput );

  // Keyboard Pos Tx
  mKeyboardTxPosInput->setButtons( QVector<int>() << Qt::Key_Right );
  mKeyboardTxPosInput->setScale( 1.0f );
  mKeyboardTxPosInput->setSourceDevice( mKeyboardDevice );
  mTxAxis->addInput( mKeyboardTxPosInput );

  // Keyboard Pos Ty
  mKeyboardTyPosInput->setButtons( QVector<int>() << Qt::Key_Up );
  mKeyboardTyPosInput->setScale( 1.0f );
  mKeyboardTyPosInput->setSourceDevice( mKeyboardDevice );
  mTyAxis->addInput( mKeyboardTyPosInput );

  // Keyboard Neg Tx
  mKeyboardTxNegInput->setButtons( QVector<int>() << Qt::Key_Left );
  mKeyboardTxNegInput->setScale( -1.0f );
  mKeyboardTxNegInput->setSourceDevice( mKeyboardDevice );
  mTxAxis->addInput( mKeyboardTxNegInput );

  // Keyboard Neg Ty
  mKeyboardTyNegInput->setButtons( QVector<int>() << Qt::Key_Down );
  mKeyboardTyNegInput->setScale( -1.0f );
  mKeyboardTyNegInput->setSourceDevice( mKeyboardDevice );
  mTyAxis->addInput( mKeyboardTyNegInput );

  // Keyboard Neg Telev
  mKeyboardTelevNegInput->setButtons( QVector<int>() << Qt::Key_PageDown );
  mKeyboardTelevNegInput->setScale( -1.0f );
  mKeyboardTelevNegInput->setSourceDevice( mKeyboardDevice );
  mTelevAxis->addInput( mKeyboardTelevNegInput );

  // Keyboard Pos Telev
  mKeyboardTelevPosInput->setButtons( QVector<int>() << Qt::Key_PageUp );
  mKeyboardTelevPosInput->setScale( 1.0f );
  mKeyboardTelevPosInput->setSourceDevice( mKeyboardDevice );
  mTelevAxis->addInput( mKeyboardTelevPosInput );

  mLogicalDevice->addAction( mLeftMouseButtonAction );
  mLogicalDevice->addAction( mMiddleMouseButtonAction );
  mLogicalDevice->addAction( mRightMouseButtonAction );
  mLogicalDevice->addAction( mShiftAction );
  mLogicalDevice->addAction( mCtrlAction );
  mLogicalDevice->addAxis( mWheelAxis );
  mLogicalDevice->addAxis( mTxAxis );
  mLogicalDevice->addAxis( mTyAxis );
  mLogicalDevice->addAxis( mTelevAxis );

  // Disable the logical device when the entity is disabled
  connect( this, &Qt3DCore::QEntity::enabledChanged,
           mLogicalDevice, &Qt3DInput::QLogicalDevice::setEnabled );

  addComponent( mLogicalDevice );
}

void QgsCameraController::setTerrainEntity( QgsTerrainEntity *te )
{
  mTerrainEntity = te;

  // object picker for terrain for correct map panning
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

void QgsCameraController::setViewport( const QRect &viewport )
{
  if ( mViewport == viewport )
    return;

  mViewport = viewport;
  emit viewportChanged();
}


static QVector3D unproject( const QVector3D &v, const QMatrix4x4 &modelView, const QMatrix4x4 &projection, const QRect &viewport )
{
  // Reimplementation of QVector3D::unproject() - see qtbase/src/gui/math3d/qvector3d.cpp
  // The only difference is that the original implementation uses tolerance 1e-5
  // (see qFuzzyIsNull()) as a protection against division by zero. For us it is however
  // common to get lower values (e.g. as low as 1e-8 when zoomed out to the whole Earth with web mercator).

  QMatrix4x4 inverse = QMatrix4x4( projection * modelView ).inverted();

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
  float d_x = x1 - x0;
  float d_y = y1 - y0;
  float k = ( y - y0 ) / d_y; // TODO: can we have d_y == 0 ?
  return x0 + k * d_x;
}

QPointF screen_point_to_point_on_plane( const QPointF &pt, const QRect &viewport, Qt3DRender::QCamera *camera, float y )
{
  // get two points of the ray
  QVector3D l0 = unproject( QVector3D( pt.x(), viewport.height() - pt.y(), 0 ), camera->viewMatrix(), camera->projectionMatrix(), viewport );
  QVector3D l1 = unproject( QVector3D( pt.x(), viewport.height() - pt.y(), 1 ), camera->viewMatrix(), camera->projectionMatrix(), viewport );

  QVector3D p0( 0, y, 0 ); // a point on the plane
  QVector3D n( 0, 1, 0 ); // normal of the plane
  QVector3D l = l1 - l0; // vector in the direction of the line
  float d = QVector3D::dotProduct( p0 - l0, n ) / QVector3D::dotProduct( l, n );
  QVector3D p = d * l + l0;

  return QPointF( p.x(), p.z() );
}


void QgsCameraController::rotateCamera( float diffPitch, float diffYaw )
{
  float pitch = mCameraPose.pitchAngle();
  float yaw = mCameraPose.headingAngle();

  if ( pitch + diffPitch > 180 )
    diffPitch = 180 - pitch;  // prevent going over the head
  if ( pitch + diffPitch < 0 )
    diffPitch = 0 - pitch;   // prevent going over the head

  // Is it always going to be love/hate relationship with quaternions???
  // This quaternion combines two rotations:
  // - first it undoes the previously applied rotation so we have do not have any rotation compared to world coords
  // - then it applies new rotation
  // (We can't just apply our euler angles difference because the camera may be already rotated)
  QQuaternion q = QQuaternion::fromEulerAngles( pitch + diffPitch, yaw + diffYaw, 0 ) *
                  QQuaternion::fromEulerAngles( pitch, yaw, 0 ).conjugated();

  // get camera's view vector, rotate it to get new view center
  QVector3D position = mCamera->position();
  QVector3D viewCenter = mCamera->viewCenter();
  QVector3D viewVector = viewCenter - position;
  QVector3D cameraToCenter = q * viewVector;
  viewCenter = position + cameraToCenter;

  mCameraPose.setCenterPoint( viewCenter );
  mCameraPose.setPitchAngle( pitch + diffPitch );
  mCameraPose.setHeadingAngle( yaw + diffYaw );
}


void QgsCameraController::frameTriggered( float dt )
{
  if ( mCamera == nullptr )
    return;

  QgsCameraPose oldCamPose = mCameraPose;
  float dist = mCameraPose.distanceFromCenterPoint();
  float yaw = mCameraPose.headingAngle();
  float pitch = mCameraPose.pitchAngle();
  QgsVector3D center = mCameraPose.centerPoint();

  int dx = mMousePos.x() - mLastMousePos.x();
  int dy = mMousePos.y() - mLastMousePos.y();
  mLastMousePos = mMousePos;

  double scaling = ( mCtrlAction->isActive() ? 0.1 : 1.0 );
  dist -= scaling * dist * mWheelAxis->value() * 10 * dt;

  if ( mRightMouseButtonAction->isActive() )
  {
    dist -= dist * dy * 0.01;
  }

  float tx = mTxAxis->value() * dt * dist * 1.5;
  float ty = -mTyAxis->value() * dt * dist * 1.5;
  float telev = mTelevAxis->value() * dt * 300;

  mCameraPose.setDistanceFromCenterPoint( dist );

  if ( !mShiftAction->isActive() && !mCtrlAction->isActive() && ( tx || ty ) )
  {
    // moving with keyboard - take into account yaw of camera
    float t = sqrt( tx * tx + ty * ty );
    float a = atan2( ty, tx ) - yaw * M_PI / 180;
    float dx = cos( a ) * t;
    float dy = sin( a ) * t;
    center.set( center.x() + dx, center.y(), center.z() + dy );
    mCameraPose.setCenterPoint( center );
  }

  if ( ( mLeftMouseButtonAction->isActive() && mShiftAction->isActive() ) || mMiddleMouseButtonAction->isActive() )
  {
    // rotate/tilt using mouse (camera moves as it rotates around its view center)
    pitch += dy;
    yaw -= dx / 2;
    mCameraPose.setPitchAngle( pitch );
    mCameraPose.setHeadingAngle( yaw );
  }
  else if ( mShiftAction->isActive() && ( mTxAxis->value() || mTyAxis->value() ) )
  {
    // rotate/tilt using keyboard (camera moves as it rotates around its view center)
    pitch -= mTyAxis->value();   // down key = moving camera toward terrain
    yaw -= mTxAxis->value();     // right key = moving camera clockwise
    mCameraPose.setPitchAngle( pitch );
    mCameraPose.setHeadingAngle( yaw );
  }
  else if ( mCtrlAction->isActive() && mLeftMouseButtonAction->isActive() )
  {
    // rotate/tilt using mouse (camera stays at one position as it rotates)
    float diffPitch = 0.2 * dy;
    float diffYaw = 0.2 * -dx / 2;
    rotateCamera( diffPitch, diffYaw );
  }
  else if ( mCtrlAction->isActive() && ( mTxAxis->value() || mTyAxis->value() ) )
  {
    // rotate/tilt using keyboard (camera stays at one position as it rotates)
    float diffPitch = mTyAxis->value();   // down key = rotating camera down
    float diffYaw = -mTxAxis->value();    // right key = rotating camera to the right
    rotateCamera( diffPitch, diffYaw );
  }
  else if ( mLeftMouseButtonAction->isActive() && !mShiftAction->isActive() )
  {
    // translation works as if one grabbed a point on the plane and dragged it
    // i.e. find out x,z of the previous mouse point, find out x,z of the current mouse point
    // and use the difference

    float z = mLastPressedHeight;
    QPointF p1 = screen_point_to_point_on_plane( QPointF( mMousePos - QPoint( dx, dy ) ), mViewport, mCamera, z );
    QPointF p2 = screen_point_to_point_on_plane( QPointF( mMousePos ), mViewport, mCamera, z );

    center.set( center.x() - ( p2.x() - p1.x() ), center.y(), center.z() - ( p2.y() - p1.y() ) );
    mCameraPose.setCenterPoint( center );
  }

  if ( telev != 0 )
  {
    center.set( center.x(), center.y() + telev, center.z() );
    mCameraPose.setCenterPoint( center );
  }

  if ( std::isnan( mCameraPose.centerPoint().x() ) || std::isnan( mCameraPose.centerPoint().y() ) || std::isnan( mCameraPose.centerPoint().z() ) )
  {
    // something went horribly wrong but we need to at least try to fix it somehow
    qDebug() << "camera position got NaN!";
    center.set( 0, 0, 0 );
    mCameraPose.setCenterPoint( center );
  }

  if ( mCameraPose.pitchAngle() > 180 )
    mCameraPose.setPitchAngle( 180 );  // prevent going over the head
  if ( mCameraPose.pitchAngle() < 0 )
    mCameraPose.setPitchAngle( 0 );   // prevent going over the head
  if ( mCameraPose.distanceFromCenterPoint() < 10 )
    mCameraPose.setDistanceFromCenterPoint( 10 );

  if ( mCameraPose != oldCamPose )
  {
    mCameraPose.updateCamera( mCamera );

    if ( mTerrainEntity && mCameraPose.centerPoint() != oldCamPose.centerPoint() )
    {
      // figure out our distance from terrain and update the camera's view center
      // so that camera tilting and rotation is around a point on terrain, not an point at fixed elevation
      QVector3D intersectionPoint;
      QgsRayCastingUtils::Ray3D ray = QgsRayCastingUtils::rayForCameraCenter( mCamera );
      if ( mTerrainEntity->rayIntersection( ray, intersectionPoint ) )
      {
        float dist = ( intersectionPoint - mCamera->position() ).length();
        mCameraPose.setDistanceFromCenterPoint( dist );
        mCameraPose.setCenterPoint( QgsVector3D( intersectionPoint ) );
        mCameraPose.updateCamera( mCamera );
      }
    }

    emit cameraChanged();
  }
}

void QgsCameraController::resetView( float distance )
{
  setViewFromTop( 0, 0, distance );
}

void QgsCameraController::setViewFromTop( float worldX, float worldY, float distance, float yaw )
{
  QgsCameraPose camPose;
  camPose.setCenterPoint( QgsVector3D( worldX, 0, worldY ) );
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
  mCameraPose = camPose;

  if ( mCamera )
    mCameraPose.updateCamera( mCamera );

  emit cameraChanged();
}

QDomElement QgsCameraController::writeXml( QDomDocument &doc ) const
{
  QDomElement elemCamera = doc.createElement( "camera" );
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
  float x = elem.attribute( QStringLiteral( "x" ) ).toFloat();
  float y = elem.attribute( QStringLiteral( "y" ) ).toFloat();
  float elev = elem.attribute( QStringLiteral( "elev" ) ).toFloat();
  float dist = elem.attribute( QStringLiteral( "dist" ) ).toFloat();
  float pitch = elem.attribute( QStringLiteral( "pitch" ) ).toFloat();
  float yaw = elem.attribute( QStringLiteral( "yaw" ) ).toFloat();
  setLookingAtPoint( QgsVector3D( x, elev, y ), dist, pitch, yaw );
}

void QgsCameraController::onPositionChanged( Qt3DInput::QMouseEvent *mouse )
{
  mMousePos = QPoint( mouse->x(), mouse->y() );
}

void QgsCameraController::onPickerMousePressed( Qt3DRender::QPickEvent *pick )
{
  mLastPressedHeight = pick->worldIntersection().y();
}
