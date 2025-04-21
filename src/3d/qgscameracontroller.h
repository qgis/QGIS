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
#include "qgscamerapose.h"
#include "qgscoordinatetransform.h"

#include <Qt3DCore/QEntity>
#include <Qt3DInput/QMouseEvent>
#include <QImage>

#ifndef SIP_RUN
namespace Qt3DInput
{
  class QKeyEvent;
  class QKeyboardHandler;
  class QMouseEvent;
  class QMouseHandler;
  class QWheelEvent;
} // namespace Qt3DInput

namespace Qt3DRender
{
  class QCamera;
}

#endif

class QDomDocument;
class QDomElement;

class QgsCameraPose;
class QgsVector3D;
class QgsWindow3DEngine;
class Qgs3DMapScene;

/**
 * \ingroup qgis_3d
 * \brief Object that controls camera movement based on user input.
 */
#ifndef SIP_RUN
class _3D_EXPORT QgsCameraController : public Qt3DCore::QEntity
{
#else
class _3D_EXPORT QgsCameraController : public QObject
{
#endif

    Q_OBJECT
  public:
    //! Constructs the camera controller with optional parent node that will take ownership
    QgsCameraController( Qgs3DMapScene *scene ) SIP_SKIP;
    ~QgsCameraController() override;

#ifndef SIP_RUN

    /**
     * Returns camera that is being controlled
     * \note Not available in Python bindings
     */
    Qt3DRender::QCamera *camera() const { return mCamera; }
#endif

    /**
     * Returns the navigation mode used by the camera controller.
     * \since QGIS 3.18
     */
    Qgis::NavigationMode cameraNavigationMode() const { return mCameraNavigationMode; }

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
    Qgis::VerticalAxisInversion verticalAxisInversion() const { return mVerticalAxisInversion; }

    /**
     * Sets the vertical axis \a inversion behavior.
     * \since QGIS 3.18
     */
    void setVerticalAxisInversion( Qgis::VerticalAxisInversion inversion );

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
     * Returns the point in the map coordinates towards which the camera is looking
     * \since QGIS 3.44
     */
    QgsVector3D lookingAtMapPoint() const;

    /**
     * Sets camera configuration like setLookingAtPoint(), but the point is given in map coordinates
     * \since QGIS 3.44
     */
    void setLookingAtMapPoint( const QgsVector3D &point, float distance, float pitch, float yaw );

    /**
     * Sets camera pose
     * \since QGIS 3.4
     */
    void setCameraPose( const QgsCameraPose &camPose, bool force = false );

    /**
     * Returns camera pose
     * \since QGIS 3.4
     */
    QgsCameraPose cameraPose() const { return mCameraPose; }

    /**
     * Returns distance of the camera from the point it is looking at.
     * The value should not be smaller than 10.
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
     * Walks into the map by \a tx, \a ty, and \a tz
     * \since QGIS 3.30
     */
    void walkView( double tx, double ty, double tz );

    /**
     * Rotates the camera on itself.
     * \param diffPitch the pitch difference
     * \param diffYaw the yaw difference
     * \since QGIS 3.30
     */
    void rotateCamera( float diffPitch, float diffYaw );

    /**
     * Rotates the camera around the pivot point (in world coordinates)
     * to the given new pitch and heading angle.
     * \since QGIS 3.42
     */
    void rotateCameraAroundPivot( float newPitch, float newHeading, const QVector3D &pivotPoint );

    /**
     * Zooms camera by given zoom factor (>1 one means zoom in)
     * while keeping the pivot point (given in world coordinates) at the
     * same screen coordinates after the zoom.
     * \since QGIS 3.42
     */
    void zoomCameraAroundPivot( const QVector3D &oldCameraPosition, double zoomFactor, const QVector3D &pivotPoint );

    /**
     * Returns TRUE if the camera controller will handle the specified key \a event,
     * preventing it from being instead handled by parents of the 3D window before
     * the controller ever receives it.
     */
    bool willHandleKeyEvent( QKeyEvent *event );

    /**
     * Reacts to the shift of origin of the scene, updating camera pose and
     * any other member variables so that the origin stays at the same position
     * relative to other entities.
     * \since QGIS 3.42
     */
    void setOrigin( const QgsVector3D &origin );

    /**
     * Sets whether the camera controller responds to mouse and keyboard events
     * \since QGIS 3.42
     */
    void setInputHandlersEnabled( bool enable ) { mInputHandlersEnabled = enable; }

    /**
     * Returns whether the camera controller responds to mouse and keyboard events
     * \since QGIS 3.44
     */
    bool hasInputHandlersEnabled() const { return mInputHandlersEnabled; }

    /**
     * Orbits camera around the globe by the specified amount given as the difference
     * in latitude/longitude angles (in degrees)
     * \note the final latitude gets clamped to 0-90 degrees, while the longitude does not get clamped
     * \since QGIS 3.44
     */
    void globeMoveCenterPoint( double latDiff, double lonDiff );

    /**
     * Moves camera closer or further away from the globe. Use factor greater than one
     * to zoom in, or less than one to zoom out.
     * \since QGIS 3.44
     */
    void globeZoom( float factor );

    /**
     * Updates pitch angle by the specified amount given as the angular difference in degrees
     * \since QGIS 3.44
     */
    void globeUpdatePitchAngle( float angleDiff );

    /**
     * Updates heading angle by the specified amount given as the angular difference in degrees
     * \since QGIS 3.44
     */
    void globeUpdateHeadingAngle( float angleDiff );

    /**
     * Resets view of the globe to look at a particular location given as latitude
     * and longitude (in degrees) and distance from that point on ellipsoid
     * \since QGIS 3.44
     */
    void resetGlobe( float distance, double lat = 0, double lon = 0 );

    /**
     * Returns the origin of the scene in map coordinates
     * \since QGIS 3.44
     */
    const QgsVector3D origin() const { return mOrigin; }

  public slots:

    /**
     * Sets the navigation mode used by the camera controller.
     * \since QGIS 3.18
     */
    void setCameraNavigationMode( Qgis::NavigationMode navigationMode );

    /**
     * Sets the depth buffer image used by the camera controller to calculate world position from a pixel's coordinates and depth
     * \since QGIS 3.24
     */
    void depthBufferCaptured( const QImage &depthImage );

  private:
#ifdef SIP_RUN
    QgsCameraController();
    QgsCameraController( const QgsCameraController &other );
#endif

    void updateCameraFromPose();
    void moveCameraPositionBy( const QVector3D &posDiff );
    //! Returns a pointer to the scene's engine's window or nullptr if engine is QgsOffscreen3DEngine
    QWindow *window() const;

    //! List of possible operations with the mouse in TerrainBased navigation
    enum class MouseOperation
    {
      None = 0,       // no operation
      Translation,    // left button pressed, no modifier
      RotationCamera, // left button pressed + ctrl modifier
      RotationCenter, // left button pressed + shift modifier
      Zoom,           // right button pressed
      ZoomWheel       // mouse wheel scroll
    };

    // This list gathers all the rotation and translation operations.
    // It is used to update the appropriate parameters when successive
    // translation and rotation happen.
    const QList<MouseOperation> mTranslateOrRotate = {
      MouseOperation::Translation,
      MouseOperation::RotationCamera,
      MouseOperation::RotationCenter
    };

    // check that current sequence (current operation and new operation) is a rotation or translation
    bool isATranslationRotationSequence( MouseOperation newOperation ) const;

    void setMouseParameters( const MouseOperation &newOperation, const QPoint &clickPoint = QPoint() );

  signals:
    //! Emitted when camera has been updated
    void cameraChanged();

    //! Emitted when the navigation mode is changed using the hotkey ctrl + ~
    void navigationModeChanged( Qgis::NavigationMode mode );

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
    void applyFlyModeKeyMovements();

  private:
    void onKeyPressedFlyNavigation( Qt3DInput::QKeyEvent *event );
    void onKeyPressedTerrainNavigation( Qt3DInput::QKeyEvent *event );
    void onKeyPressedGlobeTerrainNavigation( Qt3DInput::QKeyEvent *event );
    void onPositionChangedFlyNavigation( Qt3DInput::QMouseEvent *mouse );
    void onPositionChangedTerrainNavigation( Qt3DInput::QMouseEvent *mouse );
    void onPositionChangedGlobeTerrainNavigation( Qt3DInput::QMouseEvent *mouse );

    void handleTerrainNavigationWheelZoom();

    /**
     * Returns the minimum depth value in the square [px - 3, px + 3] * [py - 3, py + 3]
     * Returned depth is in range [0..1] and it is returned as it was written to the
     * depth buffer (not linearized, see Qgs3DUtils::screenPointToWorldPos() for conversion
     * to linear depth). Returned value 1 means there void around that pixel (no 3D objects).
     */
    double sampleDepthBuffer( int px, int py );

    // Returns the average depth of all non void pixels
    double depthBufferNonVoidAverage();

#ifndef SIP_RUN
    //! Converts screen point to world position
    bool screenPointToWorldPos( QPoint position, double &depth, QVector3D &worldPosition );
#endif

    // Moves given point (in ECEF) by specified lat/lon angle difference (in degrees) and returns new ECEF point
    QgsVector3D moveGeocentricPoint( const QgsVector3D &point, double latDiff, double lonDiff );

    //! The 3d scene the controller uses
    Qgs3DMapScene *mScene = nullptr;

    //! Camera that is being controlled
    Qt3DRender::QCamera *mCamera = nullptr;

    //! Keeps definition of the camera's position and towards where it is looking
    QgsCameraPose mCameraPose;

    //! Last mouse position recorded
    QPoint mMousePos;

    //! click point for a rotation or a translation
    QPoint mClickPoint;

    // false when no depth buffer captured or new capture requested and not yet done.
    bool mDepthBufferIsReady = false;
    QImage mDepthBufferImage;
    // -1 when unset
    // TODO: Change to std::optional<double>
    double mDepthBufferNonVoidAverage = -1;
    // nullptr when !mDepthBufferIsReady
    std::unique_ptr<Qt3DRender::QCamera> mDepthBufferCamera;

    std::unique_ptr<Qt3DRender::QCamera> mCameraBefore;

    bool mRotationCenterCalculated = false;
    QVector3D mRotationCenter;
    double mRotationDistanceFromCenter = 0;
    float mRotationPitch = 0;
    float mRotationYaw = 0;

    bool mDragPointCalculated = false;
    QVector3D mDragPoint;
    double mDragDepth;

    bool mZoomPointCalculated = false;
    QVector3D mZoomPoint;

    // used for globe
    QgsVector3D mMousePressViewCenter;
    QgsCoordinateTransform mGlobeCrsToLatLon;

    Qt3DInput::QMouseHandler *mMouseHandler = nullptr;
    Qt3DInput::QKeyboardHandler *mKeyboardHandler = nullptr;
    bool mInputHandlersEnabled = true;
    Qgis::NavigationMode mCameraNavigationMode = Qgis::NavigationMode::TerrainBased;
    Qgis::VerticalAxisInversion mVerticalAxisInversion = Qgis::VerticalAxisInversion::WhenDragging;
    double mCameraMovementSpeed = 5.0;

    QSet<int> mDepressedKeys;
    bool mCaptureFpsMouseMovements = false;
    bool mIgnoreNextMouseMove = false;
    QTimer *mFpsNavTimer = nullptr;

    double mCumulatedWheelY = 0;

    MouseOperation mCurrentOperation = MouseOperation::None;

    // 3D world's origin in map coordinates
    QgsVector3D mOrigin;

    //! Did camera change since last frame? Need to know if we should emit cameraChanged().
    bool mCameraChanged = false;

    // To test the cameracontroller
    friend class TestQgs3DRendering;
    friend class TestQgs3DCameraController;
};

#endif // QGSCAMERACONTROLLER_H
