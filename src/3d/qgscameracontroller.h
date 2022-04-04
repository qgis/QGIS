/***************************************************************************
  qgscameracontroller.h
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

#ifndef QGSCAMERACONTROLLER_H
#define QGSCAMERACONTROLLER_H

#include "qgis_3d.h"

#include <QPointer>
#include <QRect>
#include <Qt3DCore/QEntity>
#include <Qt3DInput/QMouseEvent>
#include <QImage>

namespace Qt3DInput
{
  class QKeyEvent;
  class QKeyboardDevice;
  class QKeyboardHandler;
  class QMouseEvent;
  class QMouseDevice;
  class QMouseHandler;
  class QWheelEvent;
}

namespace Qt3DRender
{
  class QCamera;
  class QPickEvent;
}

#include "qgscamerapose.h"

class QDomDocument;
class QDomElement;

class QgsCameraPose;
class QgsTerrainEntity;
class QgsVector3D;

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Object that controls camera movement based on user input
 * \note Not available in Python bindings
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsCameraController : public Qt3DCore::QEntity
{
    Q_OBJECT
    Q_PROPERTY( Qt3DRender::QCamera *camera READ camera WRITE setCamera NOTIFY cameraChanged )
    Q_PROPERTY( QRect viewport READ viewport WRITE setViewport NOTIFY viewportChanged )
  public:

    //! The navigation mode used by the camera
    enum NavigationMode
    {
      TerrainBasedNavigation, //!< The default navigation based on the terrain
      WalkNavigation //!< Uses WASD keys or arrows to navigate in walking (first person) manner
    };
    Q_ENUM( NavigationMode )

    //! Vertical axis inversion options
    enum VerticalAxisInversion
    {
      Never, //!< Never invert vertical axis movements
      WhenDragging, //!< Invert vertical axis movements when dragging in first person modes
      Always, //!< Always invert vertical axis movements
    };
    Q_ENUM( VerticalAxisInversion )

  public:
    //! Constructs the camera controller with optional parent node that will take ownership
    QgsCameraController( Qt3DCore::QNode *parent = nullptr );

    //! Returns camera that is being controlled
    Qt3DRender::QCamera *camera() const { return mCamera; }
    //! Returns viewport rectangle
    QRect viewport() const { return mViewport; }

    /**
     * Returns the navigation mode used by the camera controller.
     * \since QGIS 3.18
     */
    QgsCameraController::NavigationMode cameraNavigationMode() const { return mCameraNavigationMode; }

    /**
     * Returns the camera movement speed
     * \since QGIS 3.18
     */
    double cameraMovementSpeed() const { return mCameraMovementSpeed; }

    /**
     * Sets the camera movement speed
     * \since QGIS 3.18
     */
    void setCameraMovementSpeed( double movementSpeed );

    /**
     * Returns the vertical axis inversion behavior.
     * \since QGIS 3.18
     */
    QgsCameraController::VerticalAxisInversion verticalAxisInversion() const { return mVerticalAxisInversion; }

    /**
     * Sets the vertical axis \a inversion behavior.
     * \since QGIS 3.18
     */
    void setVerticalAxisInversion( QgsCameraController::VerticalAxisInversion inversion );

    /**
     * Connects to object picker attached to terrain entity. Called internally from 3D scene.
     * This allows camera controller understand how far from the camera is the terrain under mouse cursor.
     * Also it allows adjustment of camera's view center to a point on terrain.
     */
    void setTerrainEntity( QgsTerrainEntity *te );

    //! Assigns camera that should be controlled by this class. Called internally from 3D scene.
    void setCamera( Qt3DRender::QCamera *camera );
    //! Sets viewport rectangle. Called internally from 3D canvas. Allows conversion of mouse coordinates.
    void setViewport( QRect viewport );
    //! Called internally from 3D scene when a new frame is generated. Updates camera according to keyboard/mouse input
    void frameTriggered( float dt );

    //! Move camera back to the initial position (looking down towards origin of world's coordinates)
    void resetView( float distance );

    //! Sets camera to look down towards given point in world coordinate, in given distance from plane with zero elevation
    void setViewFromTop( float worldX, float worldY, float distance, float yaw = 0 );

    //! Returns the point in the world coordinates towards which the camera is looking
    QgsVector3D lookingAtPoint() const;

    /**
     * Sets the complete camera configuration: the point towards it is looking (in 3D world coordinates), the distance
     * of the camera from the point, pitch angle in degrees (0 = looking from the top, 90 = looking from the side) and
     * yaw angle in degrees.
     * \since QGIS 3.4
     */
    void setLookingAtPoint( const QgsVector3D &point, float distance, float pitch, float yaw );

    /**
     * Sets camera pose
     * \since QGIS 3.4
     */
    void setCameraPose( const QgsCameraPose &camPose );

    /**
     * Returns camera pose
     * \since QGIS 3.4
     */
    QgsCameraPose cameraPose() const { return mCameraPose; }

    /**
     * Returns distance of the camera from the point it is looking at.
     * \since QGIS 3.4
     */
    float distance() const { return mCameraPose.distanceFromCenterPoint(); }

    /**
     * Returns pitch angle in degrees (0 = looking from the top, 90 = looking from the side).
     * The angle should range from 0 to 180.
     * \since QGIS 3.4
     */
    float pitch() const { return mCameraPose.pitchAngle(); }

    /**
     * Returns yaw angle in degrees.  Yaw value of zero means the camera is pointing towards north.
     * The angle should range from 0 to 360.
     * \since QGIS 3.4
     */
    float yaw() const { return mCameraPose.headingAngle(); }

    //! Writes camera configuration to the given DOM element
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads camera configuration from the given DOM element
    void readXml( const QDomElement &elem );

    //! Zoom the map by \a factor
    void zoom( float factor );
    //! Tilt up the view by \a deltaPitch around the view center (camera moves)
    void tiltUpAroundViewCenter( float deltaPitch );
    //! Rotate clockwise the view by \a deltaYaw around the view center (camera moves)
    void rotateAroundViewCenter( float deltaYaw );
    //! Set camera heading to \a angle (used for rotating the view)
    void setCameraHeadingAngle( float angle );
    //! Move the map by \a tx and \a ty
    void moveView( float tx, float ty );

    /**
     * Returns TRUE if the camera controller will handle the specified key \a event,
     * preventing it from being instead handled by parents of the 3D window before
     * the controller ever receives it.
     */
    bool willHandleKeyEvent( QKeyEvent *event );

  public slots:

    /**
     * Sets the navigation mode used by the camera controller.
     * \since QGIS 3.18
     */
    void setCameraNavigationMode( QgsCameraController::NavigationMode navigationMode );

    /**
     * Sets the depth buffer image used by the camera controller to calculate world position from a pixel's coordinates and depth
     * \since QGIS 3.24
     */
    void depthBufferCaptured( const QImage &depthImage );

  private:
    void rotateCamera( float diffPitch, float diffYaw );
    void updateCameraFromPose();
    void moveCameraPositionBy( const QVector3D &posDiff );

  signals:
    //! Emitted when camera has been updated
    void cameraChanged();
    //! Emitted when viewport rectangle has been updated
    void viewportChanged();
    //! Emitted when the navigation mode is changed using the hotkey ctrl + ~
    void navigationModeHotKeyPressed( QgsCameraController::NavigationMode mode );

    /**
     * Emitted whenever the camera movement speed is changed by the controller.
     */
    void cameraMovementSpeedChanged( double speed );

    /**
     * Emitted when the mouse cursor position should be moved to the specified \a point
     * on the map viewport.
     */
    void setCursorPosition( QPoint point );

    /**
     * Emitted to ask for the depth buffer image
     * \since QGIS 3.24
     */
    void requestDepthBufferCapture();

    /**
     * Emitted when the camera rotation center changes
     * \since QGIS 3.24
     */
    void cameraRotationCenterChanged( QVector3D position );

  private slots:
    void onPositionChanged( Qt3DInput::QMouseEvent *mouse );
    void onWheel( Qt3DInput::QWheelEvent *wheel );
    void onMousePressed( Qt3DInput::QMouseEvent *mouse );
    void onMouseReleased( Qt3DInput::QMouseEvent *mouse );
    void onKeyPressed( Qt3DInput::QKeyEvent *event );
    void onKeyReleased( Qt3DInput::QKeyEvent *event );
    void onPickerMousePressed( Qt3DRender::QPickEvent *pick );
    void applyFlyModeKeyMovements();

  private:
    void onKeyPressedFlyNavigation( Qt3DInput::QKeyEvent *event );
    void onKeyPressedTerrainNavigation( Qt3DInput::QKeyEvent *event );
    void onPositionChangedFlyNavigation( Qt3DInput::QMouseEvent *mouse );
    void onPositionChangedTerrainNavigation( Qt3DInput::QMouseEvent *mouse );

    void handleTerrainNavigationWheelZoom();

    double cameraCenterElevation();

    /**
     * Returns the minimum depth value in the square [px - 3, px + 3] * [py - 3, py + 3]
     * If the value is 1, the average depth of all non void pixels is returned instead.
     */
    double sampleDepthBuffer( const QImage &buffer, int px, int py );

  private:
    //! Camera that is being controlled
    Qt3DRender::QCamera *mCamera = nullptr;
    //! used for computation of translation when dragging mouse
    QRect mViewport;
    //! height of terrain when mouse button was last pressed - for camera control
    float mLastPressedHeight = 0;

    QPointer<QgsTerrainEntity> mTerrainEntity;

    //! Keeps definition of the camera's position and towards where it is looking
    QgsCameraPose mCameraPose;

    //! Last mouse position recorded
    QPoint mMousePos;
    bool mMousePressed = false;
    Qt3DInput::QMouseEvent::Buttons mPressedButton = Qt3DInput::QMouseEvent::Buttons::NoButton;

    bool mDepthBufferIsReady = false;
    QImage mDepthBufferImage;

    QPoint mMiddleButtonClickPos;
    bool mRotationCenterCalculated = false;
    QVector3D mRotationCenter;
    double mRotationDistanceFromCenter;
    double mRotationPitch = 0;
    double mRotationYaw = 0;
    std::unique_ptr< Qt3DRender::QCamera > mCameraBeforeRotation;

    QPoint mDragButtonClickPos;
    std::unique_ptr< Qt3DRender::QCamera > mCameraBeforeDrag;
    bool mDragPointCalculated = false;
    QVector3D mDragPoint;
    double mDragDepth;

    bool mIsInZoomInState = false;
    std::unique_ptr< Qt3DRender::QCamera > mCameraBeforeZoom;
    bool mZoomPointCalculated = false;
    QVector3D mZoomPoint;

    //! Delegates mouse events to the attached MouseHandler objects
    Qt3DInput::QMouseDevice *mMouseDevice = nullptr;
    Qt3DInput::QKeyboardDevice *mKeyboardDevice = nullptr;

    Qt3DInput::QMouseHandler *mMouseHandler = nullptr;
    Qt3DInput::QKeyboardHandler *mKeyboardHandler = nullptr;
    NavigationMode mCameraNavigationMode = NavigationMode::TerrainBasedNavigation;
    VerticalAxisInversion mVerticalAxisInversion = WhenDragging;
    double mCameraMovementSpeed = 5.0;

    QSet< int > mDepressedKeys;
    bool mCaptureFpsMouseMovements = false;
    bool mIgnoreNextMouseMove = false;
    QTimer *mFpsNavTimer = nullptr;

    double mCumulatedWheelY = 0;

};

#endif // QGSCAMERACONTROLLER_H
