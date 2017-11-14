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

#include "qgis.h"

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
  , mWheelAxis( new Qt3DInput::QAxis() )
  , mMouseWheelInput( new Qt3DInput::QAnalogAxisInput() )
  , mTxAxis( new Qt3DInput::QAxis() )
  , mTyAxis( new Qt3DInput::QAxis() )
  , mKeyboardTxPosInput( new Qt3DInput::QButtonAxisInput() )
  , mKeyboardTyPosInput( new Qt3DInput::QButtonAxisInput() )
  , mKeyboardTxNegInput( new Qt3DInput::QButtonAxisInput() )
  , mKeyboardTyNegInput( new Qt3DInput::QButtonAxisInput() )
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

  mLogicalDevice->addAction( mLeftMouseButtonAction );
  mLogicalDevice->addAction( mMiddleMouseButtonAction );
  mLogicalDevice->addAction( mRightMouseButtonAction );
  mLogicalDevice->addAction( mShiftAction );
  mLogicalDevice->addAxis( mWheelAxis );
  mLogicalDevice->addAxis( mTxAxis );
  mLogicalDevice->addAxis( mTyAxis );

  // Disable the logical device when the entity is disabled
  connect( this, &Qt3DCore::QEntity::enabledChanged,
           mLogicalDevice, &Qt3DInput::QLogicalDevice::setEnabled );

  addComponent( mLogicalDevice );
}

void QgsCameraController::addTerrainPicker( Qt3DRender::QObjectPicker *picker )
{
  // object picker for terrain for correct map panning
  connect( picker, &Qt3DRender::QObjectPicker::pressed, this, &QgsCameraController::onPickerMousePressed );
}

void QgsCameraController::setCamera( Qt3DRender::QCamera *camera )
{
  if ( mCamera == camera )
    return;
  mCamera = camera;

  mCameraData.setCamera( mCamera ); // initial setup

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

void QgsCameraController::setCameraData( float x, float y, float dist, float pitch, float yaw )
{
  mCameraData.x = x;
  mCameraData.y = y;
  mCameraData.dist = dist;
  mCameraData.pitch = pitch;
  mCameraData.yaw = yaw;

  if ( mCamera )
  {
    mCameraData.setCamera( mCamera );
  }
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


void QgsCameraController::frameTriggered( float dt )
{
  if ( mCamera == nullptr )
    return;

  CameraData oldCamData = mCameraData;

  int dx = mMousePos.x() - mLastMousePos.x();
  int dy = mMousePos.y() - mLastMousePos.y();
  mLastMousePos = mMousePos;

  mCameraData.dist -= mCameraData.dist * mWheelAxis->value() * 10 * dt;

  if ( mRightMouseButtonAction->isActive() )
  {
    mCameraData.dist -= mCameraData.dist * dy * 0.01;
  }

  float tx = mTxAxis->value() * dt * mCameraData.dist * 1.5;
  float ty = -mTyAxis->value() * dt * mCameraData.dist * 1.5;

  if ( tx || ty )
  {
    // moving with keyboard - take into account yaw of camera
    float t = sqrt( tx * tx + ty * ty );
    float a = atan2( ty, tx ) - mCameraData.yaw * M_PI / 180;
    float dx = cos( a ) * t;
    float dy = sin( a ) * t;
    mCameraData.x += dx;
    mCameraData.y += dy;
  }

  if ( ( mLeftMouseButtonAction->isActive() && mShiftAction->isActive() ) || mMiddleMouseButtonAction->isActive() )
  {
    mCameraData.pitch += dy;
    mCameraData.yaw -= dx / 2;
  }
  else if ( mLeftMouseButtonAction->isActive() && !mShiftAction->isActive() )
  {
    // translation works as if one grabbed a point on the plane and dragged it
    // i.e. find out x,z of the previous mouse point, find out x,z of the current mouse point
    // and use the difference

    float z = mLastPressedHeight;
    QPointF p1 = screen_point_to_point_on_plane( QPointF( mMousePos - QPoint( dx, dy ) ), mViewport, mCamera, z );
    QPointF p2 = screen_point_to_point_on_plane( QPointF( mMousePos ), mViewport, mCamera, z );

    mCameraData.x -= p2.x() - p1.x();
    mCameraData.y -= p2.y() - p1.y();
  }

  if ( std::isnan( mCameraData.x ) || std::isnan( mCameraData.y ) )
  {
    // something went horribly wrong but we need to at least try to fix it somehow
    qDebug() << "camera position got NaN!";
    mCameraData.x = mCameraData.y = 0;
  }

  if ( mCameraData.pitch > 80 )
    mCameraData.pitch = 80;  // prevent going under the plane
  if ( mCameraData.pitch < 0 )
    mCameraData.pitch = 0;   // prevent going over the head
  if ( mCameraData.dist < 10 )
    mCameraData.dist = 10;

  if ( mCameraData != oldCamData )
  {
    mCameraData.setCamera( mCamera );
    emit cameraChanged();
  }
}

void QgsCameraController::resetView( float distance )
{
  setCameraData( 0, 0, distance );
  // a basic setup to make frustum depth range long enough that it does not cull everything
  mCamera->setNearPlane( distance / 2 );
  mCamera->setFarPlane( distance * 2 );

  emit cameraChanged();
}

void QgsCameraController::onPositionChanged( Qt3DInput::QMouseEvent *mouse )
{
  mMousePos = QPoint( mouse->x(), mouse->y() );
}

void QgsCameraController::onPickerMousePressed( Qt3DRender::QPickEvent *pick )
{
  mLastPressedHeight = pick->worldIntersection().y();
}
