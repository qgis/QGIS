#include "cameracontroller.h"

#include <Qt3DRender/QObjectPicker>
#include <Qt3DRender/QPickEvent>


CameraController::CameraController( Qt3DCore::QNode *parent )
  : Qt3DCore::QEntity( parent )
  , mLastPressedHeight( 0 )
  , mMouseDevice( new Qt3DInput::QMouseDevice() )
  , mKeyboardDevice( new Qt3DInput::QKeyboardDevice() )
  , mMouseHandler( new Qt3DInput::QMouseHandler )
  , mLogicalDevice( new Qt3DInput::QLogicalDevice() )
  , mLeftMouseButtonAction( new Qt3DInput::QAction() )
  , mLeftMouseButtonInput( new Qt3DInput::QActionInput() )
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
           this, &CameraController::onPositionChanged );
  addComponent( mMouseHandler );

  // TODO: keep using QAxis and QAction approach or just switch to using QMouseHandler / QKeyboardHandler?
  // it does not feel like the former approach makes anything much simpler

  // left mouse button
  mLeftMouseButtonInput->setButtons( QVector<int>() << Qt::LeftButton );
  mLeftMouseButtonInput->setSourceDevice( mMouseDevice );
  mLeftMouseButtonAction->addInput( mLeftMouseButtonInput );

  // Mouse Wheel (Y)
  // TODO: zoom with mouse wheel in Qt < 5.8
#if QT_VERSION >= 0x050800
  mMouseWheelInput->setAxis( Qt3DInput::QMouseDevice::WheelY );
  mMouseWheelInput->setSourceDevice( mMouseDevice );
  mWheelAxis->addInput( mMouseWheelInput );
#endif

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
  mLogicalDevice->addAction( mShiftAction );
  mLogicalDevice->addAxis( mWheelAxis );
  mLogicalDevice->addAxis( mTxAxis );
  mLogicalDevice->addAxis( mTyAxis );

  // Disable the logical device when the entity is disabled
  connect( this, &Qt3DCore::QEntity::enabledChanged,
           mLogicalDevice, &Qt3DInput::QLogicalDevice::setEnabled );

  addComponent( mLogicalDevice );
}

void CameraController::addTerrainPicker( Qt3DRender::QObjectPicker *picker )
{
  // object picker for terrain for correct map panning
  connect( picker, &Qt3DRender::QObjectPicker::pressed, this, &CameraController::onPickerMousePressed );
}

void CameraController::setCamera( Qt3DRender::QCamera *camera )
{
  if ( mCamera == camera )
    return;
  mCamera = camera;

  cd.setCamera( mCamera ); // initial setup

  // TODO: set camera's parent if not set already?
  // TODO: registerDestructionHelper (?)
  emit cameraChanged();
}

void CameraController::setViewport( const QRect &viewport )
{
  if ( mViewport == viewport )
    return;

  mViewport = viewport;
  emit viewportChanged();
}

void CameraController::setCameraData( float x, float y, float dist, float pitch, float yaw )
{
  cd.x = x;
  cd.y = y;
  cd.dist = dist;
  cd.pitch = pitch;
  cd.yaw = yaw;

  if ( mCamera )
    cd.setCamera( mCamera );
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
  QVector3D l0 = QVector3D( pt.x(), viewport.height() - pt.y(), 0 ).unproject( camera->viewMatrix(), camera->projectionMatrix(), viewport );
  QVector3D l1 = QVector3D( pt.x(), viewport.height() - pt.y(), 1 ).unproject( camera->viewMatrix(), camera->projectionMatrix(), viewport );

  QVector3D p0( 0, y, 0 ); // a point on the plane
  QVector3D n( 0, 1, 0 ); // normal of the plane
  QVector3D l = l1 - l0; // vector in the direction of the line
  float d = QVector3D::dotProduct( p0 - l0, n ) / QVector3D::dotProduct( l, n );
  QVector3D p = d * l + l0;

  return QPointF( p.x(), p.z() );
}


void CameraController::frameTriggered( float dt )
{
  if ( mCamera == nullptr )
    return;

  CamData oldCamData = cd;

  int dx = mMousePos.x() - mLastMousePos.x();
  int dy = mMousePos.y() - mLastMousePos.y();
  mLastMousePos = mMousePos;

  cd.dist -= cd.dist * mWheelAxis->value() * 10 * dt;

  float tx = mTxAxis->value() * dt * cd.dist * 1.5;
  float ty = -mTyAxis->value() * dt * cd.dist * 1.5;

  if ( tx || ty )
  {
    // moving with keyboard - take into account yaw of camera
    float t = sqrt( tx * tx + ty * ty );
    float a = atan2( ty, tx ) - cd.yaw * M_PI / 180;
    float dx = cos( a ) * t;
    float dy = sin( a ) * t;
    cd.x += dx;
    cd.y += dy;
  }

  if ( mLeftMouseButtonAction->isActive() )
  {
    if ( mShiftAction->isActive() )
    {
      cd.pitch += dy;
      cd.yaw -= dx / 2;
    }
    else
    {
      // translation works as if one grabbed a point on the plane and dragged it
      // i.e. find out x,z of the previous mouse point, find out x,z of the current mouse point
      // and use the difference

      float z = mLastPressedHeight;
      QPointF p1 = screen_point_to_point_on_plane( QPointF( mMousePos - QPoint( dx, dy ) ), mViewport, mCamera, z );
      QPointF p2 = screen_point_to_point_on_plane( QPointF( mMousePos ), mViewport, mCamera, z );

      cd.x -= p2.x() - p1.x();
      cd.y -= p2.y() - p1.y();
    }
  }

  if ( cd.pitch > 80 )
    cd.pitch = 80;  // prevent going under the plane
  if ( cd.pitch < 0 )
    cd.pitch = 0;   // prevent going over the head
  if ( cd.dist < 10 )
    cd.dist = 10;

  if ( cd != oldCamData )
  {
    cd.setCamera( mCamera );
    emit cameraChanged();
  }
}

void CameraController::resetView()
{
  setCameraData( 0, 0, 1000 );

  emit cameraChanged();
}

void CameraController::onPositionChanged( Qt3DInput::QMouseEvent *mouse )
{
  mMousePos = QPoint( mouse->x(), mouse->y() );
}

void CameraController::onPickerMousePressed( Qt3DRender::QPickEvent *pick )
{
  mLastPressedHeight = pick->worldIntersection().y();
}
