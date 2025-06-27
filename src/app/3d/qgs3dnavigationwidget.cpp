/***************************************************************************
  qgs3dnavigationwidget.cpp
  --------------------------------------
  Date                 : June 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QToolButton>
#include <QObject>
#include <QHeaderView>
#include <QCheckBox>
#include <QStandardItemModel>
#include "qgis.h"

Q_NOWARN_DEPRECATED_PUSH
#include "qwt_compass.h"
#include "qwt_dial_needle.h"
Q_NOWARN_DEPRECATED_POP

#include "qgscameracontroller.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapsettings.h"
#include "qgs3dnavigationwidget.h"
#include "moc_qgs3dnavigationwidget.cpp"

#include <Qt3DRender/QCamera>

Qgs3DNavigationWidget::Qgs3DNavigationWidget( Qgs3DMapCanvas *canvas, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  constexpr float MOVE_FACTOR = 0.000001f; // multiplied by distance to get angle
  constexpr float ZOOM_FACTOR = 0.9f;

  m3DMapCanvas = canvas;
  // Zoom in button
  QObject::connect(
    mZoomInButton,
    &QToolButton::clicked,
    m3DMapCanvas,
    [this] {
      QgsCameraController *controller = m3DMapCanvas->cameraController();
      if ( m3DMapCanvas->mapSettings()->sceneMode() == Qgis::SceneMode::Globe )
        controller->globeZoom( ZOOM_FACTOR );
      else
        controller->zoom( 5 );
    }
  );

  // Zoom out button
  QObject::connect(
    mZoomOutButton,
    &QToolButton::clicked,
    m3DMapCanvas,
    [this] {
      QgsCameraController *controller = m3DMapCanvas->cameraController();
      if ( m3DMapCanvas->mapSettings()->sceneMode() == Qgis::SceneMode::Globe )
        controller->globeZoom( 1 / ZOOM_FACTOR );
      else
        controller->zoom( -5 );
    }
  );

  // Tilt up button
  QObject::connect(
    mTiltUpButton,
    &QToolButton::clicked,
    m3DMapCanvas,
    [this] {
      m3DMapCanvas->cameraController()->tiltUpAroundViewCenter( 1 );
    }
  );

  // Tilt down button
  QObject::connect(
    mTiltDownButton,
    &QToolButton::clicked,
    m3DMapCanvas,
    [this] {
      m3DMapCanvas->cameraController()->tiltUpAroundViewCenter( -1 );
    }
  );

  // Compas
  QwtCompassMagnetNeedle *compasNeedle = new QwtCompassMagnetNeedle();
  mCompass->setNeedle( compasNeedle );

  QObject::connect(
    mCompass,
    &QwtDial::valueChanged,
    m3DMapCanvas,
    [this] {
      m3DMapCanvas->cameraController()->setCameraHeadingAngle( float( mCompass->value() ) );
    }
  );

  // Move up button
  QObject::connect(
    mMoveUpButton,
    &QToolButton::clicked,
    m3DMapCanvas,
    [this] {
      QgsCameraController *controller = m3DMapCanvas->cameraController();
      if ( m3DMapCanvas->mapSettings()->sceneMode() == Qgis::SceneMode::Globe )
        controller->globeMoveCenterPoint( MOVE_FACTOR * controller->cameraPose().distanceFromCenterPoint(), 0 );
      else
        controller->moveView( 0, 1 );
    }
  );

  // Move right button
  QObject::connect(
    mMoveRightButton,
    &QToolButton::clicked,
    m3DMapCanvas,
    [this] {
      QgsCameraController *controller = m3DMapCanvas->cameraController();
      if ( m3DMapCanvas->mapSettings()->sceneMode() == Qgis::SceneMode::Globe )
        controller->globeMoveCenterPoint( 0, MOVE_FACTOR * controller->cameraPose().distanceFromCenterPoint() );
      else
        controller->moveView( 1, 0 );
    }
  );

  // Move down button
  QObject::connect(
    mMoveDownButton,
    &QToolButton::clicked,
    m3DMapCanvas,
    [this] {
      QgsCameraController *controller = m3DMapCanvas->cameraController();
      if ( m3DMapCanvas->mapSettings()->sceneMode() == Qgis::SceneMode::Globe )
        controller->globeMoveCenterPoint( -MOVE_FACTOR * controller->cameraPose().distanceFromCenterPoint(), 0 );
      else
        controller->moveView( 0, -1 );
    }
  );

  // Move left button
  QObject::connect(
    mMoveLeftButton,
    &QToolButton::clicked,
    m3DMapCanvas,
    [this] {
      QgsCameraController *controller = m3DMapCanvas->cameraController();
      if ( m3DMapCanvas->mapSettings()->sceneMode() == Qgis::SceneMode::Globe )
        controller->globeMoveCenterPoint( 0, -MOVE_FACTOR * controller->cameraPose().distanceFromCenterPoint() );
      else
        m3DMapCanvas->cameraController()->moveView( -1, 0 );
    }
  );
}
