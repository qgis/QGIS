/***************************************************************************
  qgs3dcameracontrolswidget.cpp
  --------------------------------------
  Date                 : February 2026
  Copyright            : (C) 2026 by Dominik Cindrić
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dcameracontrolswidget.h"

#include "qgis.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgscameracontroller.h"
#include "qgscoordinatereferencesystem.h"

#include <QString>

#include "moc_qgs3dcameracontrolswidget.cpp"

using namespace Qt::StringLiterals;

Qgs3DCameraControlsWidget::Qgs3DCameraControlsWidget( Qgs3DMapCanvas *canvas, QWidget *parent )
  : QWidget( parent )
  , m3DMapCanvas( canvas )
{
  setupUi( this );

  connect( mCameraX, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( const double ) {
    updateCameraLookingAt();
  } );

  connect( mCameraY, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( const double ) {
    updateCameraLookingAt();
  } );

  connect( mCameraZ, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( const double ) {
    updateCameraLookingAt();
  } );

  connect( mCameraPitch, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( const double value ) {
    QgsCameraPose pose = m3DMapCanvas->cameraController()->cameraPose();
    pose.setPitchAngle( value );
    m3DMapCanvas->cameraController()->setCameraPose( pose );
  } );

  connect( mCameraHeading, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( const double value ) {
    QgsCameraPose pose = m3DMapCanvas->cameraController()->cameraPose();
    pose.setHeadingAngle( value );
    m3DMapCanvas->cameraController()->setCameraPose( pose );
  } );

  connect( mCameraDistanceFromCenterPoint, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, [this]( const double value ) {
    QgsCameraPose pose = m3DMapCanvas->cameraController()->cameraPose();
    pose.setDistanceFromCenterPoint( value );
    m3DMapCanvas->cameraController()->setCameraPose( pose );
  } );

  mCameraX->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mCameraY->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mCameraZ->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mCameraPitch->setRange( 0.0, 180.0 );
  mCameraHeading->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mCameraDistanceFromCenterPoint->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );

  setCRSInfo();
  updateFromCamera();
}

void Qgs3DCameraControlsWidget::updateCameraLookingAt()
{
  QgsVector3D mapLookingAt( mCameraX->value(), mCameraY->value(), mCameraX->value() );
  QgsVector3D worldLookingAt = m3DMapCanvas->mapSettings()->mapToWorldCoordinates( mapLookingAt );
  worldLookingAt.setZ( mCameraZ->value() );

  m3DMapCanvas->cameraController()->setLookingAtPoint(
    worldLookingAt,
    m3DMapCanvas->cameraController()->distance(),
    m3DMapCanvas->cameraController()->pitch(),
    m3DMapCanvas->cameraController()->yaw()
  );
}

void Qgs3DCameraControlsWidget::updateFromCamera() const
{
  QgsCameraPose pose = m3DMapCanvas->cameraController()->cameraPose();
  QgsVector3D mapCoords = m3DMapCanvas->mapSettings()->worldToMapCoordinates( pose.centerPoint() );

  whileBlocking( mCameraX )->setValue( mapCoords.x() );
  whileBlocking( mCameraY )->setValue( mapCoords.y() );
  whileBlocking( mCameraZ )->setValue( mapCoords.z() );

  whileBlocking( mCameraPitch )->setValue( pose.pitchAngle() );
  whileBlocking( mCameraHeading )->setValue( pose.headingAngle() );
  whileBlocking( mCameraDistanceFromCenterPoint )->setValue( pose.distanceFromCenterPoint() );
}

void Qgs3DCameraControlsWidget::setCRSInfo() const
{
  QgsCoordinateReferenceSystem crs = m3DMapCanvas->mapSettings()->crs();
  labelCRS->setText( tr( "Coordinates in 3D map CRS" ) + u" (%1)"_s.arg( crs.userFriendlyIdentifier( Qgis::CrsIdentifierType::ShortString ) ) );
  labelCRS->setToolTip( crs.userFriendlyIdentifier( Qgis::CrsIdentifierType::MediumString ) );
}
