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

#include <Qt3DCore/QEntity>
#include <Qt3DInput>
#include <Qt3DRender>

#include "qgscamerapose.h"

class QDomDocument;
class QDomElement;

class QgsCameraPose;
class QgsTerrainEntity;
class QgsVector3D;

/**
 * \ingroup 3d
 * Object that controls camera movement based on user input
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsCameraController : public Qt3DCore::QEntity
{
    Q_OBJECT
    Q_PROPERTY( Qt3DRender::QCamera *camera READ camera WRITE setCamera NOTIFY cameraChanged )
    Q_PROPERTY( QRect viewport READ viewport WRITE setViewport NOTIFY viewportChanged )
  public:
    //! Constructs the camera controller with optional parent node that will take ownership
    QgsCameraController( Qt3DCore::QNode *parent = nullptr );

    //! Returns camera that is being controlled
    Qt3DRender::QCamera *camera() const { return mCamera; }
    //! Returns viewport rectangle
    QRect viewport() const { return mViewport; }

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

  private:
    void rotateCamera( float diffPitch, float diffYaw );

  signals:
    //! Emitted when camera has been updated
    void cameraChanged();
    //! Emitted when viewport rectangle has been updated
    void viewportChanged();

  private slots:
    void onPositionChanged( Qt3DInput::QMouseEvent *mouse );
    void onPickerMousePressed( Qt3DRender::QPickEvent *pick );

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
    //! Mouse position used in the previous frame
    QPoint mLastMousePos;

    //! Delegates mouse events to the attached MouseHandler objects
    Qt3DInput::QMouseDevice *mMouseDevice = nullptr;

    Qt3DInput::QKeyboardDevice *mKeyboardDevice = nullptr;

    Qt3DInput::QMouseHandler *mMouseHandler = nullptr;

    /**
     * Allows us to define a set of actions that we wish to use
     * (it is a component that can be attached to 3D scene)
     */
    Qt3DInput::QLogicalDevice *mLogicalDevice = nullptr;

    Qt3DInput::QAction *mLeftMouseButtonAction = nullptr;
    Qt3DInput::QActionInput *mLeftMouseButtonInput = nullptr;

    Qt3DInput::QAction *mMiddleMouseButtonAction = nullptr;
    Qt3DInput::QActionInput *mMiddleMouseButtonInput = nullptr;

    Qt3DInput::QAction *mRightMouseButtonAction = nullptr;
    Qt3DInput::QActionInput *mRightMouseButtonInput = nullptr;

    Qt3DInput::QAction *mShiftAction = nullptr;
    Qt3DInput::QActionInput *mShiftInput = nullptr;

    Qt3DInput::QAction *mCtrlAction = nullptr;
    Qt3DInput::QActionInput *mCtrlInput = nullptr;

    Qt3DInput::QAxis *mWheelAxis = nullptr;
    Qt3DInput::QAnalogAxisInput *mMouseWheelInput = nullptr;

    Qt3DInput::QAxis *mTxAxis = nullptr;
    Qt3DInput::QAxis *mTyAxis = nullptr;
    Qt3DInput::QButtonAxisInput *mKeyboardTxPosInput = nullptr;
    Qt3DInput::QButtonAxisInput *mKeyboardTyPosInput = nullptr;
    Qt3DInput::QButtonAxisInput *mKeyboardTxNegInput = nullptr;
    Qt3DInput::QButtonAxisInput *mKeyboardTyNegInput = nullptr;

    Qt3DInput::QAxis *mTelevAxis = nullptr;
    Qt3DInput::QButtonAxisInput *mKeyboardTelevPosInput = nullptr;
    Qt3DInput::QButtonAxisInput *mKeyboardTelevNegInput = nullptr;
};

#endif // QGSCAMERACONTROLLER_H
