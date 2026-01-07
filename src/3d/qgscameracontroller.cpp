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

#include <cmath>

#include "qgis.h"
#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgseventtracing.h"
#include "qgslogger.h"
#include "qgsray3d.h"
#include "qgsraycastcontext.h"
#include "qgsraycasthit.h"
#include "qgsterrainentity.h"
#include "qgsvector3d.h"
#include "qgswindow3dengine.h"

#include <QDomDocument>
#include <QQuaternion>
#include <QStringLiteral>
#include <Qt3DInput>
#include <Qt3DRender/QCamera>

#include "moc_qgscameracontroller.cpp"

QgsCameraController::QgsCameraController( Qgs3DMapScene *scene )
  : Qt3DCore::QEntity( scene )
  , mScene( scene )
  , mCamera( scene->engine()->camera() )
  , mCameraBefore( new Qt3DRender::QCamera )
  , mMouseHandler( new Qt3DInput::QMouseHandler )
  , mKeyboardHandler( new Qt3DInput::QKeyboardHandler )
  , mOrigin( scene->mapSettings()->origin() )
{
  mMouseHandler->setSourceDevice( new Qt3DInput::QMouseDevice() );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::positionChanged, this, &QgsCameraController::onPositionChanged );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::wheel, this, &QgsCameraController::onWheel );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::pressed, this, &QgsCameraController::onMousePressed );
  connect( mMouseHandler, &Qt3DInput::QMouseHandler::released, this, &QgsCameraController::onMouseReleased );
  addComponent( mMouseHandler );

  // Disable the handlers when the entity is disabled
  connect( this, &Qt3DCore::QEntity::enabledChanged, mMouseHandler, &Qt3DInput::QMouseHandler::setEnabled );
  connect( this, &Qt3DCore::QEntity::enabledChanged, mKeyboardHandler, &Qt3DInput::QKeyboardHandler::setEnabled );

  mFpsNavTimer = new QTimer( this );
  mFpsNavTimer->setInterval( 10 );
  connect( mFpsNavTimer, &QTimer::timeout, this, &QgsCameraController::applyFlyModeKeyMovements );
  mFpsNavTimer->start();

  if ( mScene->mapSettings()->sceneMode() == Qgis::SceneMode::Globe )
  {
    setCameraNavigationMode( mScene->mapSettings()->cameraNavigationMode() );
    mGlobeCrsToLatLon = QgsCoordinateTransform( mScene->mapSettings()->crs(), mScene->mapSettings()->crs().toGeographicCrs(), mScene->mapSettings()->transformContext() );
  }
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

void QgsCameraController::rotateCamera( float diffPitch, float diffHeading )
{
  float newPitch = mCameraPose.pitchAngle() + diffPitch;
  float newHeading = mCameraPose.headingAngle() + diffHeading;

  newPitch = std::clamp( newPitch, 0.f, 180.f ); // prevent going over the head

  switch ( mScene->mapSettings()->sceneMode() )
  {
    case Qgis::SceneMode::Globe:
    {
      // When on a globe, we need to calculate "where is up" (the normal of a tangent plane).
      // Also it uses different axes than the standard plane-based view.
      // See QgsCameraPose::updateCameraGlobe(), we basically want to make sure
      // that after an update, the camera stays in the same spot.
      QgsVector3D viewCenterLatLon;
      try
      {
        viewCenterLatLon = mGlobeCrsToLatLon.transform( mCameraPose.centerPoint() + mOrigin );
      }
      catch ( const QgsCsException & )
      {
        QgsDebugError( u"rotateCamera: ECEF -> lat,lon transform failed!"_s );
        return;
      }
      QQuaternion qLatLon = QQuaternion::fromAxisAndAngle( QVector3D( 0, 0, 1 ), static_cast<float>( viewCenterLatLon.x() ) )
                            * QQuaternion::fromAxisAndAngle( QVector3D( 0, -1, 0 ), static_cast<float>( viewCenterLatLon.y() ) );
      QQuaternion qPitchHeading = QQuaternion::fromAxisAndAngle( QVector3D( 1, 0, 0 ), newHeading )
                                  * QQuaternion::fromAxisAndAngle( QVector3D( 0, 1, 0 ), newPitch );
      QVector3D newCameraToCenter = ( qLatLon * qPitchHeading * QVector3D( -1, 0, 0 ) ) * mCameraPose.distanceFromCenterPoint();

      mCameraPose.setCenterPoint( mCamera->position() + newCameraToCenter );
      mCameraPose.setPitchAngle( newPitch );
      mCameraPose.setHeadingAngle( newHeading );
      updateCameraFromPose();
      return;
    }

    case Qgis::SceneMode::Local:
    {
      QQuaternion q = Qgs3DUtils::rotationFromPitchHeadingAngles( newPitch, newHeading );
      QVector3D newCameraToCenter = q * QVector3D( 0, 0, -mCameraPose.distanceFromCenterPoint() );
      mCameraPose.setCenterPoint( mCamera->position() + newCameraToCenter );
      mCameraPose.setPitchAngle( newPitch );
      mCameraPose.setHeadingAngle( newHeading );
      updateCameraFromPose();
      return;
    }
  }
}

void QgsCameraController::rotateCameraAroundPivot( float newPitch, float newHeading, const QVector3D &pivotPoint )
{
  const float oldPitch = mCameraPose.pitchAngle();
  const float oldHeading = mCameraPose.headingAngle();

  newPitch = std::clamp( newPitch, 0.f, 180.f ); // prevent going over the head

  // First undo the previously applied rotation, then apply the new rotation
  // (We can't just apply our euler angles difference because the camera may be already rotated)
  const QQuaternion qNew = Qgs3DUtils::rotationFromPitchHeadingAngles( newPitch, newHeading );
  const QQuaternion qOld = Qgs3DUtils::rotationFromPitchHeadingAngles( oldPitch, oldHeading );
  const QQuaternion q = qNew * qOld.conjugated();

  const QVector3D newViewCenter = q * ( mCamera->viewCenter() - pivotPoint ) + pivotPoint;

  mCameraPose.setCenterPoint( newViewCenter );
  mCameraPose.setPitchAngle( newPitch );
  mCameraPose.setHeadingAngle( newHeading );
  updateCameraFromPose();
}

void QgsCameraController::zoomCameraAroundPivot( const QVector3D &oldCameraPosition, double zoomFactor, const QVector3D &pivotPoint )
{
  // step 1: move camera along the line connecting reference camera position and our pivot point
  QVector3D newCamPosition = pivotPoint + ( oldCameraPosition - pivotPoint ) * zoomFactor;
  double newDistance = mCameraPose.distanceFromCenterPoint() * zoomFactor;

  // step 2: using the new camera position and distance from center, calculate new view center
  QQuaternion q = Qgs3DUtils::rotationFromPitchHeadingAngles( mCameraPose.pitchAngle(), mCameraPose.headingAngle() );
  QVector3D cameraToCenter = q * QVector3D( 0, 0, -newDistance );
  QVector3D newViewCenter = newCamPosition + cameraToCenter;

  mCameraPose.setDistanceFromCenterPoint( newDistance );
  mCameraPose.setCenterPoint( newViewCenter );
  updateCameraFromPose();
}

void QgsCameraController::zoomCameraAroundPivot( const QVector3D &oldCameraPosition, double oldDistanceFromCenterPoint, double zoomFactor, const QVector3D &pivotPoint )
{
  // step 1: move camera along the line connecting reference camera position and our pivot point
  QVector3D newCamPosition = pivotPoint + ( oldCameraPosition - pivotPoint ) * zoomFactor;
  const double newDistance = oldDistanceFromCenterPoint * zoomFactor;

  // step 2: using the new camera position and distance from center, calculate new view center
  QQuaternion q = Qgs3DUtils::rotationFromPitchHeadingAngles( mCameraPose.pitchAngle(), mCameraPose.headingAngle() );
  QVector3D cameraToCenter = q * QVector3D( 0, 0, -newDistance );
  QVector3D newViewCenter = newCamPosition + cameraToCenter;

  mCameraPose.setDistanceFromCenterPoint( newDistance );
  mCameraPose.setCenterPoint( newViewCenter );
  updateCameraFromPose();
}

void QgsCameraController::frameTriggered( float dt )
{
  Q_UNUSED( dt )

  if ( mCameraChanged )
  {
    emit cameraChanged();
    mCameraChanged = false;
  }
}

void QgsCameraController::resetView( float distance )
{
  QgsPointXY extentCenter = mScene->mapSettings()->extent().center();
  QgsVector3D origin = mScene->mapSettings()->origin();
  setViewFromTop( extentCenter.x() - origin.x(), extentCenter.y() - origin.y(), distance );
}

void QgsCameraController::setViewFromTop( float worldX, float worldY, float distance, float yaw )
{
  if ( mScene->mapSettings()->sceneMode() == Qgis::SceneMode::Globe )
  {
    QgsDebugError( u"setViewFromTop() should not be used with globe!"_s );
    return;
  }

  QgsCameraPose camPose;
  QgsTerrainEntity *terrain = mScene->terrainEntity();
  const float terrainElevationOffset = terrain ? terrain->terrainElevationOffset() : 0.0f;
  camPose.setCenterPoint( QgsVector3D( worldX, worldY, terrainElevationOffset - mScene->mapSettings()->origin().z() ) );
  camPose.setDistanceFromCenterPoint( distance );
  camPose.setHeadingAngle( yaw );

  // a basic setup to make frustum depth range long enough that it does not cull everything
  mCamera->setNearPlane( distance / 2 );
  mCamera->setFarPlane( distance * 2 );
  // we force the updateCameraNearFarPlanes() in Qgs3DMapScene to properly set the planes
  setCameraPose( camPose, true );
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

QgsVector3D QgsCameraController::lookingAtMapPoint() const
{
  return lookingAtPoint() + mOrigin;
}

void QgsCameraController::setLookingAtMapPoint( const QgsVector3D &point, float distance, float pitch, float yaw )
{
  setLookingAtPoint( point - mOrigin, distance, pitch, yaw );
}

void QgsCameraController::setCameraPose( const QgsCameraPose &camPose, bool force )
{
  if ( camPose == mCameraPose && !force )
    return;

  mCameraPose = camPose;
  updateCameraFromPose();
}

QDomElement QgsCameraController::writeXml( QDomDocument &doc ) const
{
  QDomElement elemCamera = doc.createElement( u"camera"_s );
  // Save center point in map coordinates, since our world origin won't be
  // the same on loading
  QgsVector3D centerPoint = mCameraPose.centerPoint() + mOrigin;
  elemCamera.setAttribute( u"xMap"_s, centerPoint.x() );
  elemCamera.setAttribute( u"yMap"_s, centerPoint.y() );
  elemCamera.setAttribute( u"zMap"_s, centerPoint.z() );
  elemCamera.setAttribute( u"dist"_s, mCameraPose.distanceFromCenterPoint() );
  elemCamera.setAttribute( u"pitch"_s, mCameraPose.pitchAngle() );
  elemCamera.setAttribute( u"yaw"_s, mCameraPose.headingAngle() );
  return elemCamera;
}

void QgsCameraController::readXml( const QDomElement &elem, QgsVector3D savedOrigin )
{
  const float dist = elem.attribute( u"dist"_s ).toFloat();
  const float pitch = elem.attribute( u"pitch"_s ).toFloat();
  const float yaw = elem.attribute( u"yaw"_s ).toFloat();

  QgsVector3D centerPoint;
  if ( elem.hasAttribute( "xMap" ) )
  {
    // Prefer newer point saved in map coordinates ...
    const double x = elem.attribute( u"xMap"_s ).toDouble();
    const double y = elem.attribute( u"yMap"_s ).toDouble();
    const double z = elem.attribute( u"zMap"_s ).toDouble();
    centerPoint = QgsVector3D( x, y, z ) - mOrigin;
  }
  else
  {
    // ... but allow use of older origin-relative coordinates.
    const double x = elem.attribute( u"x"_s ).toDouble();
    const double y = elem.attribute( u"y"_s ).toDouble();
    const double elev = elem.attribute( u"elev"_s ).toDouble();
    centerPoint = QgsVector3D( x, elev, y ) - savedOrigin + mOrigin;
  }
  setLookingAtPoint( centerPoint, dist, pitch, yaw );
}

double QgsCameraController::sampleDepthBuffer( int px, int py )
{
  if ( !mDepthBufferIsReady )
  {
    QgsDebugError( u"Asked to sample depth buffer, but depth buffer not ready!"_s );
  }

  double depth = 1;

  if ( QWindow *win = window() )
  {
    // on high DPI screens, the mouse position is in device-independent pixels,
    // but the depth buffer is in physical pixels...
    px = static_cast<int>( px * win->devicePixelRatio() );
    py = static_cast<int>( py * win->devicePixelRatio() );
  }

  // Sample the neighbouring pixels for the closest point to the camera
  for ( int x = px - 3; x <= px + 3; ++x )
  {
    for ( int y = py - 3; y <= py + 3; ++y )
    {
      if ( mDepthBufferImage.valid( x, y ) )
      {
        depth = std::min( depth, Qgs3DUtils::decodeDepth( mDepthBufferImage.pixel( x, y ) ) );
      }
    }
  }
  return depth;
}

double QgsCameraController::depthBufferNonVoidAverage()
{
  // Cache the computed depth, since averaging over all pixels can be expensive
  if ( mDepthBufferNonVoidAverage != -1 )
    return mDepthBufferNonVoidAverage;

  // Returns the average of depth values that are not 1 (void area)
  double depth = 0;
  int samplesCount = 0;
  // Make sure we can do the cast
  Q_ASSERT( mDepthBufferImage.format() == QImage::Format_RGB32 );
  for ( int y = 0; y < mDepthBufferImage.height(); ++y )
  {
    const QRgb *line = reinterpret_cast<const QRgb *>( mDepthBufferImage.constScanLine( y ) );
    for ( int x = 0; x < mDepthBufferImage.width(); ++x )
    {
      double d = Qgs3DUtils::decodeDepth( line[x] );
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

  mDepthBufferNonVoidAverage = depth;

  return depth;
}

QgsVector3D QgsCameraController::moveGeocentricPoint( const QgsVector3D &point, double latDiff, double lonDiff )
{
  try
  {
    QgsVector3D pointLatLon = mGlobeCrsToLatLon.transform( point );
    pointLatLon.setX( pointLatLon.x() + lonDiff );
    pointLatLon.setY( std::clamp( pointLatLon.y() + latDiff, -90., 90. ) );

    return mGlobeCrsToLatLon.transform( pointLatLon, Qgis::TransformDirection::Reverse );
  }
  catch ( const QgsCsException & )
  {
    QgsDebugError( u"moveGeocentricPoint: transform failed!"_s );
    return point;
  }
}

void QgsCameraController::globeMoveCenterPoint( double latDiff, double lonDiff )
{
  const QgsVector3D viewCenter = mCameraPose.centerPoint() + mOrigin;
  const QgsVector3D newViewCenter = moveGeocentricPoint( viewCenter, latDiff, lonDiff );
  mCameraPose.setCenterPoint( newViewCenter - mOrigin );
  updateCameraFromPose();
}

void QgsCameraController::globeZoom( float factor )
{
  mCameraPose.setDistanceFromCenterPoint( mCameraPose.distanceFromCenterPoint() * factor );
  updateCameraFromPose();
}

void QgsCameraController::globeUpdatePitchAngle( float angleDiff )
{
  mCameraPose.setPitchAngle( std::clamp( mCameraPose.pitchAngle() + angleDiff, 0.f, 90.f ) );
  updateCameraFromPose();
}

void QgsCameraController::globeUpdateHeadingAngle( float angleDiff )
{
  mCameraPose.setHeadingAngle( mCameraPose.headingAngle() + angleDiff );
  updateCameraFromPose();
}

void QgsCameraController::resetGlobe( float distance, double lat, double lon )
{
  QgsVector3D mapPoint;
  try
  {
    mapPoint = mGlobeCrsToLatLon.transform( QgsVector3D( lon, lat, 0 ), Qgis::TransformDirection::Reverse );
  }
  catch ( const QgsCsException & )
  {
    QgsDebugError( u"resetGlobe: transform failed!"_s );
    return;
  }

  QgsCameraPose cp;
  cp.setCenterPoint( mapPoint - mOrigin );
  cp.setDistanceFromCenterPoint( distance );
  setCameraPose( cp );
}

void QgsCameraController::updateCameraFromPose()
{
  if ( mCamera )
  {
    if ( mScene->mapSettings()->sceneMode() == Qgis::SceneMode::Globe )
    {
      const QgsVector3D viewCenter = mCameraPose.centerPoint() + mOrigin;

      QgsVector3D viewCenterLatLon;
      try
      {
        viewCenterLatLon = mGlobeCrsToLatLon.transform( viewCenter );
      }
      catch ( const QgsCsException & )
      {
        QgsDebugError( u"updateCameraFromPose: transform failed!"_s );
        return;
      }

      mCameraPose.updateCameraGlobe( mCamera, viewCenterLatLon.y(), viewCenterLatLon.x() );
    }
    else
    {
      mCameraPose.updateCamera( mCamera );
    }
    mCameraChanged = true;
  }
}

void QgsCameraController::moveCameraPositionBy( const QVector3D &posDiff )
{
  mCameraPose.setCenterPoint( mCameraPose.centerPoint() + posDiff );
  updateCameraFromPose();
}

void QgsCameraController::onPositionChanged( Qt3DInput::QMouseEvent *mouse )
{
  if ( !mInputHandlersEnabled )
    return;

  QgsEventTracing::ScopedEvent traceEvent( u"3D"_s, u"QgsCameraController::onPositionChanged"_s );

  switch ( mCameraNavigationMode )
  {
    case Qgis::NavigationMode::TerrainBased:
      onPositionChangedTerrainNavigation( mouse );
      break;

    case Qgis::NavigationMode::Walk:
      onPositionChangedFlyNavigation( mouse );
      break;

    case Qgis::NavigationMode::GlobeTerrainBased:
      onPositionChangedGlobeTerrainNavigation( mouse );
      break;
  }
}

bool QgsCameraController::screenPointToWorldPos( QPoint position, double &depth, QVector3D &worldPosition )
{
  depth = sampleDepthBuffer( position.x(), position.y() );

  // if there's nothing around the given position, try to get just any depth
  // from the scene as a coarse approximation...
  if ( depth == 1 )
  {
    depth = depthBufferNonVoidAverage();
  }

  worldPosition = Qgs3DUtils::screenPointToWorldPos( position, depth, mScene->engine()->size(), mDepthBufferCamera.get() );
  if ( !std::isfinite( worldPosition.x() ) || !std::isfinite( worldPosition.y() ) || !std::isfinite( worldPosition.z() ) )
  {
    QgsDebugMsgLevel( u"screenPointToWorldPos: position is NaN or Inf. This should not happen."_s, 2 );
    return false;
  }

  return true;
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

  const bool hasShift = ( mouse->modifiers() & Qt3DInput::QMouseEvent::Modifiers::ShiftModifier );
  const bool hasCtrl = ( mouse->modifiers() & Qt3DInput::QMouseEvent::Modifiers::ControlModifier );
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
      if ( screenPointToWorldPos( mClickPoint, depth, worldPosition ) )
      {
        mRotationCenter = worldPosition;
        mRotationDistanceFromCenter = ( mRotationCenter - mCameraBefore->position() ).length();
        emit cameraRotationCenterChanged( mRotationCenter );
        mRotationCenterCalculated = true;
      }
    }

    rotateCameraAroundPivot( mRotationPitch + pitchDiff, mRotationYaw + yawDiff, mRotationCenter );
  }
  else if ( hasLeftButton && hasCtrl && !hasShift )
  {
    setMouseParameters( MouseOperation::RotationCamera );
    // rotate/tilt using mouse (camera stays at one position as it rotates)
    const float diffPitch = 0.2f * dy;
    const float diffYaw = -0.2f * dx;
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
      if ( screenPointToWorldPos( mClickPoint, depth, worldPosition ) )
      {
        mDragDepth = depth;
        mDragPoint = worldPosition;
        mDragPointCalculated = true;
      }
    }

    QVector3D cameraBeforeDragPos = mCameraBefore->position();

    QVector3D moveToPosition = Qgs3DUtils::screenPointToWorldPos( { mouse->x(), mouse->y() }, mDragDepth, mScene->engine()->size(), mCameraBefore.get() );
    QVector3D cameraBeforeToMoveToPos = ( moveToPosition - mCameraBefore->position() ).normalized();
    QVector3D cameraBeforeToDragPointPos = ( mDragPoint - mCameraBefore->position() ).normalized();

    // Make sure the rays are not horizontal (add small z shift if it is)
    if ( cameraBeforeToMoveToPos.z() == 0 )
    {
      cameraBeforeToMoveToPos.setZ( 0.01 );
      cameraBeforeToMoveToPos = cameraBeforeToMoveToPos.normalized();
    }

    if ( cameraBeforeToDragPointPos.z() == 0 )
    {
      cameraBeforeToDragPointPos.setZ( 0.01 );
      cameraBeforeToDragPointPos = cameraBeforeToDragPointPos.normalized();
    }

    double d1 = ( mDragPoint.z() - cameraBeforeDragPos.z() ) / cameraBeforeToMoveToPos.z();
    double d2 = ( mDragPoint.z() - cameraBeforeDragPos.z() ) / cameraBeforeToDragPointPos.z();

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
    center.set( center.x(), center.y(), center.z() + tElev * 0.5 );
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
      if ( screenPointToWorldPos( mClickPoint, depth, worldPosition ) )
      {
        mDragPoint = worldPosition;
        mDragPointCalculated = true;
      }
    }

    const double oldDist = ( QgsVector3D( mCameraBefore->position() ) - QgsVector3D( mDragPoint ) ).length();
    double newDist = oldDist;

    int yOffset = 0;
    int screenHeight = mScene->engine()->size().height();
    QWindow *win = window();
    if ( win )
    {
      yOffset = win->mapToGlobal( QPoint( 0, 0 ) ).y();
      screenHeight = win->screen()->virtualSize().height();
    }

    // Applies smoothing
    if ( mMousePos.y() > mClickPoint.y() ) // zoom in
    {
      double f = ( double ) ( mMousePos.y() - mClickPoint.y() ) / ( double ) ( screenHeight - mClickPoint.y() - yOffset );
      f = std::max( 0.0, std::min( 1.0, f ) );
      f = 1 - ( std::expm1( -2 * f ) ) / ( std::expm1( -2 ) );
      newDist = newDist * f;
    }
    else // zoom out
    {
      double f = 1 - ( double ) ( mMousePos.y() + yOffset ) / ( double ) ( mClickPoint.y() + yOffset );
      f = std::max( 0.0, std::min( 1.0, f ) );
      f = ( std::expm1( 2 * f ) ) / ( std::expm1( 2 ) );
      newDist = newDist + 2 * newDist * f;
    }

    const double zoomFactor = newDist / oldDist;
    zoomCameraAroundPivot( mCameraBefore->position(), mCameraBefore->viewCenter().distanceToPoint( mCameraBefore->position() ), zoomFactor, mDragPoint );
  }

  mMousePos = QPoint( mouse->x(), mouse->y() );
}

void QgsCameraController::onPositionChangedGlobeTerrainNavigation( Qt3DInput::QMouseEvent *mouse )
{
  const bool hasShift = ( mouse->modifiers() & Qt3DInput::QMouseEvent::Modifiers::ShiftModifier );
  const bool hasCtrl = ( mouse->modifiers() & Qt3DInput::QMouseEvent::Modifiers::ControlModifier );
  const bool hasLeftButton = ( mouse->buttons() & Qt::LeftButton );
  const bool hasMiddleButton = ( mouse->buttons() & Qt::MiddleButton );

  if ( ( hasLeftButton && hasShift && !hasCtrl ) || ( hasMiddleButton && !hasShift && !hasCtrl ) )
  {
    setMouseParameters( MouseOperation::RotationCenter, mMousePos );

    const float scale = static_cast<float>( std::max( mScene->engine()->size().width(), mScene->engine()->size().height() ) );
    const float pitchDiff = 180.0f * static_cast<float>( mouse->y() - mClickPoint.y() ) / scale;
    const float yawDiff = -180.0f * static_cast<float>( mouse->x() - mClickPoint.x() ) / scale;

    mCameraPose.setPitchAngle( mRotationPitch + pitchDiff );
    mCameraPose.setHeadingAngle( mRotationYaw + yawDiff );
    updateCameraFromPose();
    return;
  }

  if ( !( mouse->buttons() & Qt::LeftButton ) )
    return;

  // translation works as if one grabbed a point on the 3D viewer and dragged it
  setMouseParameters( MouseOperation::Translation, mMousePos );

  if ( !mDepthBufferIsReady )
    return;

  if ( !mDragPointCalculated )
  {
    double depth;
    QVector3D worldPosition;
    if ( !screenPointToWorldPos( mClickPoint, depth, worldPosition ) )
      return;

    mDragDepth = depth;
    mDragPoint = worldPosition;
    mDragPointCalculated = true;
  }

  // Approximate the globe as a sphere with a center in (0,0,0) map coords and
  // of radius the same as at startPosMap.
  const QgsVector3D startPosMap = QgsVector3D( mDragPoint ) + mOrigin;
  const double sphereRadiusMap = startPosMap.length();
  // Find the intersection of this sphere and the ray from the current clicked point.
  const QgsRay3D ray = Qgs3DUtils::rayFromScreenPoint( QPoint( mouse->x(), mouse->y() ), mScene->engine()->size(), mCameraBefore.get() );
  const QgsVector3D rayOriginMap = QgsVector3D( ray.origin() ) + mOrigin;
  // From equations of ray and sphere
  const double quadA = QVector3D::dotProduct( ray.direction(), ray.direction() );
  const double quadB = 2 * QgsVector3D::dotProduct( ray.direction(), rayOriginMap );
  const double quadC = QgsVector3D::dotProduct( rayOriginMap, rayOriginMap ) - sphereRadiusMap * sphereRadiusMap;
  const double disc = quadB * quadB - 4 * quadA * quadC;
  if ( disc < 0 )
    // Ray misses sphere
    return;
  // Distance to intersection along ray (take smaller root, closer to camera)
  const double rayDistMap = ( -quadB - sqrt( disc ) ) / ( 2 * quadA );
  if ( rayDistMap < 0 )
  {
    QgsDebugError( u"Sphere intersection result negative, canceling move"_s );
    return;
  }
  const QgsVector3D newPosMap = rayOriginMap + QgsVector3D( ray.direction() ) * rayDistMap;

  // now that we have old and new mouse position in ECEF coordinates,
  // let's figure out the difference in lat/lon angles and update the center point

  QgsVector3D oldLatLon, newLatLon;
  try
  {
    oldLatLon = mGlobeCrsToLatLon.transform( startPosMap );
    newLatLon = mGlobeCrsToLatLon.transform( newPosMap );
  }
  catch ( const QgsCsException & )
  {
    QgsDebugError( u"onPositionChangedGlobeTerrainNavigation: transform failed!"_s );
    return;
  }

  const double latDiff = oldLatLon.y() - newLatLon.y();
  const double lonDiff = oldLatLon.x() - newLatLon.x();

  const QgsVector3D newVC = moveGeocentricPoint( mMousePressViewCenter, latDiff, lonDiff );
  const QgsVector3D newVCWorld = newVC - mOrigin;

  mCameraPose.setCenterPoint( newVCWorld );
  updateCameraFromPose();
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
    if ( screenPointToWorldPos( mMousePos, depth, worldPosition ) )
    {
      mZoomPoint = worldPosition;
      mZoomPointCalculated = true;
    }
  }

  double oldDist = ( mZoomPoint - mCameraBefore->position() ).length();
  // Each step of the scroll wheel decreases distance by 20%
  double newDist = std::pow( 0.8, mCumulatedWheelY ) * oldDist;
  // Make sure we don't clip the thing we're zooming to.
  newDist = std::max( newDist, 2.0 );
  double zoomFactor = newDist / oldDist;
  // Don't change the distance too suddenly to hopefully prevent numerical instability
  zoomFactor = std::clamp( zoomFactor, 0.01, 100.0 );

  zoomCameraAroundPivot( mCameraBefore->position(), mCameraBefore->viewCenter().distanceToPoint( mCameraBefore->position() ), zoomFactor, mZoomPoint );

  mCumulatedWheelY = 0;
  setMouseParameters( MouseOperation::None );
}

void QgsCameraController::onWheel( Qt3DInput::QWheelEvent *wheel )
{
  if ( !mInputHandlersEnabled )
    return;

  switch ( mCameraNavigationMode )
  {
    case Qgis::NavigationMode::Walk:
    {
      const float scaling = ( ( wheel->modifiers() & Qt3DInput::QWheelEvent::Modifiers::ControlModifier ) != 0 ? 0.1f : 1.0f ) / 1000.f;
      setCameraMovementSpeed( mCameraMovementSpeed + mCameraMovementSpeed * scaling * wheel->angleDelta().y() );
      break;
    }

    case Qgis::NavigationMode::TerrainBased:
    {
      // Scale our variable to roughly "number of normal steps", with Ctrl
      // increasing granularity 10x
      const double scaling = ( 1.0 / 120.0 ) * ( ( wheel->modifiers() & Qt3DInput::QWheelEvent::Modifiers::ControlModifier ) != 0 ? 0.1 : 1.0 );

      // Apparently angleDelta needs to be accumulated
      // see: https://doc.qt.io/qt-5/qwheelevent.html#angleDelta
      mCumulatedWheelY += scaling * wheel->angleDelta().y();

      if ( mCurrentOperation != MouseOperation::ZoomWheel )
      {
        setMouseParameters( MouseOperation::ZoomWheel );
        // The actual zooming will happen after we get a new depth buffer
      }
      else
      {
        handleTerrainNavigationWheelZoom();
      }
      break;
    }

    case Qgis::NavigationMode::GlobeTerrainBased:
    {
      float wheelAmount = static_cast<float>( wheel->angleDelta().y() );
      float factor = abs( wheelAmount ) / 1000.f;
      float mulFactor = wheelAmount > 0 ? ( 1 - factor ) : ( 1 + factor );
      mCameraPose.setDistanceFromCenterPoint( mCameraPose.distanceFromCenterPoint() * mulFactor );
      updateCameraFromPose();
      break;
    }
  }
}

void QgsCameraController::onMousePressed( Qt3DInput::QMouseEvent *mouse )
{
  if ( !mInputHandlersEnabled )
    return;

  mKeyboardHandler->setFocus( true );

  if ( mouse->button() == Qt3DInput::QMouseEvent::MiddleButton || ( ( mouse->modifiers() & Qt3DInput::QMouseEvent::Modifiers::ShiftModifier ) != 0 && mouse->button() == Qt3DInput::QMouseEvent::LeftButton ) || ( ( mouse->modifiers() & Qt3DInput::QMouseEvent::Modifiers::ControlModifier ) != 0 && mouse->button() == Qt3DInput::QMouseEvent::LeftButton ) )
  {
    mMousePos = QPoint( mouse->x(), mouse->y() );

    if ( mCaptureFpsMouseMovements )
      mIgnoreNextMouseMove = true;

    const MouseOperation operation {
      ( mouse->modifiers() & Qt3DInput::QMouseEvent::Modifiers::ControlModifier ) != 0 && mouse->button() == Qt3DInput::QMouseEvent::LeftButton ? MouseOperation::RotationCamera : MouseOperation::RotationCenter
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
  if ( !mInputHandlersEnabled )
    return;


  setMouseParameters( MouseOperation::None );
}

bool QgsCameraController::onKeyPressedTerrainNavigation( QKeyEvent *event )
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
    default:
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
      const float diffPitch = ty; // down key = rotating camera down
      const float diffYaw = -tx;  // right key = rotating camera to the right
      rotateCamera( diffPitch, diffYaw );
    }
    return true;
  }

  if ( tElev )
  {
    QgsVector3D center = mCameraPose.centerPoint();
    center.set( center.x(), center.y(), center.z() + tElev * 10 );
    mCameraPose.setCenterPoint( center );
    return true;
  }

  return false;
}

bool QgsCameraController::onKeyPressedGlobeTerrainNavigation( QKeyEvent *event )
{
  // both move factor and zoom factor are just empirically picked numbers
  // that seem to work well (providing steps that are not too big / not too small)
  constexpr float MOVE_FACTOR = 0.000001f; // multiplied by distance to get angle
  constexpr float ZOOM_FACTOR = 0.9f;

  const bool hasShift = ( event->modifiers() & Qt::ShiftModifier );

  switch ( event->key() )
  {
    case Qt::Key_Left:
      if ( hasShift )
        globeUpdateHeadingAngle( -5 );
      else
        globeMoveCenterPoint( 0, -MOVE_FACTOR * mCameraPose.distanceFromCenterPoint() );
      return true;
    case Qt::Key_Right:
      if ( hasShift )
        globeUpdateHeadingAngle( 5 );
      else
        globeMoveCenterPoint( 0, MOVE_FACTOR * mCameraPose.distanceFromCenterPoint() );
      return true;
    case Qt::Key_Up:
      if ( hasShift )
        globeUpdatePitchAngle( -5 );
      else
        globeMoveCenterPoint( MOVE_FACTOR * mCameraPose.distanceFromCenterPoint(), 0 );
      return true;
    case Qt::Key_Down:
      if ( hasShift )
        globeUpdatePitchAngle( 5 );
      else
        globeMoveCenterPoint( -MOVE_FACTOR * mCameraPose.distanceFromCenterPoint(), 0 );
      return true;
    case Qt::Key_PageDown:
      globeZoom( ZOOM_FACTOR );
      return true;
    case Qt::Key_PageUp:
      globeZoom( 1 / ZOOM_FACTOR );
      return true;
    default:
      break;
  }
  return false;
}

static const QSet<int> walkNavigationSavedKeys = {
  Qt::Key_Left,
  Qt::Key_A,
  Qt::Key_Right,
  Qt::Key_D,
  Qt::Key_Up,
  Qt::Key_W,
  Qt::Key_Down,
  Qt::Key_S,
  Qt::Key_PageUp,
  Qt::Key_E,
  Qt::Key_PageDown,
  Qt::Key_Q,
};

bool QgsCameraController::onKeyPressedFlyNavigation( QKeyEvent *event )
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
      event->accept();
      return true;
    }

    case Qt::Key_Escape:
    {
      // always exit mouse lock mode
      if ( mCaptureFpsMouseMovements )
      {
        mCaptureFpsMouseMovements = false;
        mIgnoreNextMouseMove = true;
        qApp->restoreOverrideCursor();
        event->accept();
        return true;
      }
      break;
    }
    default:
      break;
  }

  if ( walkNavigationSavedKeys.contains( event->key() ) )
  {
    if ( !event->isAutoRepeat() )
    {
      mDepressedKeys.insert( event->key() );
    }
    event->accept();
    return true;
  }
  return false;
}

void QgsCameraController::walkView( double tx, double ty, double tz )
{
  const QVector3D cameraUp = mCamera->upVector().normalized();
  const QVector3D cameraFront = ( QVector3D( mCameraPose.centerPoint().x(), mCameraPose.centerPoint().y(), mCameraPose.centerPoint().z() ) - mCamera->position() ).normalized();
  const QVector3D cameraLeft = QVector3D::crossProduct( cameraUp, cameraFront );

  QVector3D cameraPosDiff( 0.0f, 0.0f, 0.0f );

  if ( tx != 0.0 )
  {
    cameraPosDiff += static_cast<float>( tx ) * cameraFront;
  }
  if ( ty != 0.0 )
  {
    cameraPosDiff += static_cast<float>( ty ) * cameraLeft;
  }
  if ( tz != 0.0 )
  {
    cameraPosDiff += static_cast<float>( tz ) * QVector3D( 0.0f, 0.0f, 1.0f );
  }

  moveCameraPositionBy( cameraPosDiff );
}

void QgsCameraController::applyFlyModeKeyMovements()
{
  if ( mCameraNavigationMode != Qgis::NavigationMode::Walk )
    return;

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

      const float diffYaw = -0.2f * dx;
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
      const float diffYaw = -0.2f * dx;
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

void QgsCameraController::tiltUpAroundViewCenter( float deltaPitch )
{
  // Tilt up the view by deltaPitch around the view center (camera moves)
  float pitch = mCameraPose.pitchAngle();
  pitch -= deltaPitch; // down key = moving camera toward terrain
  mCameraPose.setPitchAngle( pitch );
  updateCameraFromPose();
}

void QgsCameraController::rotateAroundViewCenter( float deltaYaw )
{
  // Rotate clockwise the view by deltaYaw around the view center (camera moves)
  float yaw = mCameraPose.headingAngle();
  yaw -= deltaYaw; // right key = moving camera clockwise
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
  center.set( center.x() + dx, center.y() - dy, center.z() );
  mCameraPose.setCenterPoint( center );
  updateCameraFromPose();
}

bool QgsCameraController::keyboardEventFilter( QKeyEvent *event )
{
  if ( !mInputHandlersEnabled )
    return false;

  if ( event->type() == QKeyEvent::Type::KeyRelease )
  {
    if ( !event->isAutoRepeat() && mDepressedKeys.contains( event->key() ) )
    {
      mDepressedKeys.remove( event->key() );
      return true;
    }
  }
  else if ( event->type() == QEvent::ShortcutOverride )
  {
    if ( event->modifiers() & Qt::ControlModifier )
    {
      switch ( event->key() )
      {
        case Qt::Key_QuoteLeft:
        {
          // switch navigation mode
          switch ( mCameraNavigationMode )
          {
            case Qgis::NavigationMode::Walk:
              setCameraNavigationMode(
                mScene->mapSettings()->sceneMode() == Qgis::SceneMode::Globe
                  ? Qgis::NavigationMode::GlobeTerrainBased
                  : Qgis::NavigationMode::TerrainBased
              );
              break;
            case Qgis::NavigationMode::TerrainBased:
            case Qgis::NavigationMode::GlobeTerrainBased:
              setCameraNavigationMode( Qgis::NavigationMode::Walk );
              break;
          }
          event->accept();
          return true;
        }

        // Make sure to sync the key combinations with strings in Qgs3DAxis::createMenu()!
        case Qt::Key_8:
          rotateCameraToNorth();
          return true;
        case Qt::Key_6:
          rotateCameraToEast();
          return true;
        case Qt::Key_2:
          rotateCameraToSouth();
          return true;
        case Qt::Key_4:
          rotateCameraToWest();
          return true;
        case Qt::Key_9:
          rotateCameraToTop();
          return true;
        case Qt::Key_3:
          rotateCameraToBottom();
          return true;
        case Qt::Key_5:
          rotateCameraToHome();
          return true;

        default:
          break;
      }
    }

    switch ( mCameraNavigationMode )
    {
      case Qgis::NavigationMode::Walk:
        return onKeyPressedFlyNavigation( event );

      case Qgis::NavigationMode::TerrainBased:
        return onKeyPressedTerrainNavigation( event );

      case Qgis::NavigationMode::GlobeTerrainBased:
        return onKeyPressedGlobeTerrainNavigation( event );
    }
  }
  return false;
}

void QgsCameraController::depthBufferCaptured( const QImage &depthImage )
{
  mDepthBufferImage = depthImage;
  mDepthBufferIsReady = true;
  mDepthBufferNonVoidAverage = -1;

  // To read distances from the captured depth buffer, we need to know the
  // camera parameters it was rendered with. This seems like the closest
  // place to save them, though I have no idea if they can't be changed
  // between the rendering and now anyway...
  mDepthBufferCamera = Qgs3DUtils::copyCamera( mCamera );

  if ( mCurrentOperation == MouseOperation::ZoomWheel )
  {
    handleTerrainNavigationWheelZoom();
  }
}

bool QgsCameraController::isATranslationRotationSequence( MouseOperation newOperation ) const
{
  return std::find( mTranslateOrRotate.begin(), mTranslateOrRotate.end(), newOperation ) != std::end( mTranslateOrRotate ) && std::find( mTranslateOrRotate.begin(), mTranslateOrRotate.end(), mCurrentOperation ) != std::end( mTranslateOrRotate );
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
    mMousePressViewCenter = mCameraPose.centerPoint() + mOrigin;
    mCameraBefore = Qgs3DUtils::copyCamera( mCamera );

    emit requestDepthBufferCapture();
  }
}

void QgsCameraController::setOrigin( const QgsVector3D &origin )
{
  QgsVector3D diff = origin - mOrigin;
  mCameraPose.setCenterPoint( mCameraPose.centerPoint() - diff );

  // update other members that depend on world coordinates
  mCameraBefore->setPosition( ( QgsVector3D( mCameraBefore->position() ) - diff ).toVector3D() );
  mCameraBefore->setViewCenter( ( QgsVector3D( mCameraBefore->viewCenter() ) - diff ).toVector3D() );
  mDragPoint = ( QgsVector3D( mDragPoint ) - diff ).toVector3D();
  mRotationCenter = ( QgsVector3D( mRotationCenter ) - diff ).toVector3D();

  mOrigin = origin;

  updateCameraFromPose();
}

void QgsCameraController::rotateToRespectingTerrain( float pitch, float yaw )
{
  QgsVector3D pos = lookingAtPoint();
  double elevation = 0.0;
  if ( mScene->mapSettings()->terrainRenderingEnabled() )
  {
    QgsDebugMsgLevel( "Checking elevation from terrain...", 2 );
    QVector3D camPos = mCamera->position();
    const QgsRay3D ray( camPos, pos.toVector3D() - camPos );
    QgsRayCastContext context;
    context.setSingleResult( true );
    context.setMaximumDistance( mCamera->farPlane() );
    const QList<QgsRayCastHit> results = mScene->terrainEntity()->rayIntersection( ray, context );

    if ( !results.isEmpty() )
    {
      elevation = results.constFirst().mapCoordinates().z() - mOrigin.z();
      QgsDebugMsgLevel( QString( "Computed elevation from terrain: %1" ).arg( elevation ), 2 );
    }
    else
    {
      QgsDebugMsgLevel( "Unable to obtain elevation from terrain", 2 );
    }
  }
  pos.set( pos.x(), pos.y(), elevation + mScene->terrainEntity()->terrainElevationOffset() );

  setLookingAtPoint( pos, ( mCamera->position() - pos.toVector3D() ).length(), pitch, yaw );
}
