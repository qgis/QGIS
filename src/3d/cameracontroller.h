#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include "qgis_3d.h"

#include <Qt3DCore/QEntity>
#include <Qt3DInput>
#include <Qt3DRender>


class _3D_EXPORT CameraController : public Qt3DCore::QEntity
{
    Q_OBJECT
    Q_PROPERTY( Qt3DRender::QCamera *camera READ camera WRITE setCamera NOTIFY cameraChanged )
    Q_PROPERTY( QRect viewport READ viewport WRITE setViewport NOTIFY viewportChanged )
  public:
    CameraController( Qt3DCore::QNode *parent = nullptr );

    Qt3DRender::QCamera *camera() const { return mCamera; }
    QRect viewport() const { return mViewport; }

    void addTerrainPicker( Qt3DRender::QObjectPicker *picker );

    void setCamera( Qt3DRender::QCamera *camera );
    void setViewport( const QRect &viewport );

    void setCameraData( float x, float y, float dist, float pitch = 0, float yaw = 0 );

    void frameTriggered( float dt );

    //! Move camera back to the initial position (looking down towards origin of world's coordinates)
    void resetView( float distance );

  signals:
    void cameraChanged();
    void viewportChanged();

  private slots:
    void onPositionChanged( Qt3DInput::QMouseEvent *mouse );
    void onPickerMousePressed( Qt3DRender::QPickEvent *pick );

  private:
    //! Camera that is being controlled
    Qt3DRender::QCamera *mCamera;
    //! used for computation of translation when dragging mouse
    QRect mViewport;
    //! height of terrain when mouse button was last pressed - for camera control
    float mLastPressedHeight;

    struct CamData
    {
      float x = 0, y = 0;  // ground point towards which the camera is looking
      float dist = 40;  // distance of camera from the point it is looking at
      float pitch = 0; // aircraft nose up/down (0 = looking straight down to the plane)
      float yaw = 0;   // aircraft nose left/right

      bool operator==( const CamData &other ) const
      {
        return x == other.x && y == other.y && dist == other.dist && pitch == other.pitch && yaw == other.yaw;
      }
      bool operator!=( const CamData &other ) const
      {
        return !operator==( other );
      }

      void setCamera( Qt3DRender::QCamera *camera )
      {
        // basic scene setup:
        // - x grows to the right
        // - z grows to the bottom
        // - y grows towards camera
        // so a point on the plane (x',y') is transformed to (x,-z) in our 3D world
        camera->setUpVector( QVector3D( 0, 0, -1 ) );
        camera->setPosition( QVector3D( x, dist, y ) );
        camera->setViewCenter( QVector3D( x, 0, y ) );
        camera->rotateAboutViewCenter( QQuaternion::fromEulerAngles( pitch, yaw, 0 ) );
      }

    };

    CamData cd;

    //! Last mouse position recorded
    QPoint mMousePos;
    //! Mouse position used in the previous frame
    QPoint mLastMousePos;

    //! Delegates mouse events to the attached MouseHandler objects
    Qt3DInput::QMouseDevice *mMouseDevice;

    Qt3DInput::QKeyboardDevice *mKeyboardDevice;

    Qt3DInput::QMouseHandler *mMouseHandler;

    //! Allows us to define a set of actions that we wish to use
    //! (it is a component that can be attached to 3D scene)
    Qt3DInput::QLogicalDevice *mLogicalDevice;

    Qt3DInput::QAction *mLeftMouseButtonAction;
    Qt3DInput::QActionInput *mLeftMouseButtonInput;

    Qt3DInput::QAction *mMiddleMouseButtonAction;
    Qt3DInput::QActionInput *mMiddleMouseButtonInput;

    Qt3DInput::QAction *mRightMouseButtonAction;
    Qt3DInput::QActionInput *mRightMouseButtonInput;

    Qt3DInput::QAction *mShiftAction;
    Qt3DInput::QActionInput *mShiftInput;

    Qt3DInput::QAxis *mWheelAxis;
    Qt3DInput::QAnalogAxisInput *mMouseWheelInput;

    Qt3DInput::QAxis *mTxAxis;
    Qt3DInput::QAxis *mTyAxis;
    Qt3DInput::QButtonAxisInput *mKeyboardTxPosInput;
    Qt3DInput::QButtonAxisInput *mKeyboardTyPosInput;
    Qt3DInput::QButtonAxisInput *mKeyboardTxNegInput;
    Qt3DInput::QButtonAxisInput *mKeyboardTyNegInput;
};

#endif // CAMERACONTROLLER_H
