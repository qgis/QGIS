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
#include "qgslayoututils.h"
#include "qgssettingstree.h"

#include <QPushButton>
#include <QString>
#include <QTimer>

#include "moc_qgs3dcameracontrolswidget.cpp"

using namespace Qt::StringLiterals;

const QgsSettingsEntryBool *Qgs3DCameraControlsWidget::setting3DCameraControlsLiveUpdate = new QgsSettingsEntryBool( u"camera-controls-live-update"_s, QgsSettingsTree::sTree3DMap, true, u"Whether the 3D map is dynamically updated while camera controls are edited"_s );

Qgs3DCameraControlsWidget::Qgs3DCameraControlsWidget( Qgs3DMapCanvas *canvas, QWidget *parent )
  : QWidget( parent )
  , m3DMapCanvas( canvas )
{
  setupUi( this );

  mAutoApplyTimer = new QTimer( this );
  mAutoApplyTimer->setSingleShot( true );
  connect( mAutoApplyTimer, &QTimer::timeout, this, &Qgs3DCameraControlsWidget::applySettings );

  connect( mButtonBox->button( QDialogButtonBox::Apply ), &QAbstractButton::clicked, this, &Qgs3DCameraControlsWidget::applySettings );
  connect( mLiveApplyCheck, &QCheckBox::toggled, this, &Qgs3DCameraControlsWidget::liveApplyToggled );

  mLiveApplyCheck->setChecked( setting3DCameraControlsLiveUpdate->value() );
  mButtonBox->button( QDialogButtonBox::Apply )->setEnabled( !mLiveApplyCheck->isChecked() );
  if ( mLiveApplyCheck->isChecked() )
    connectLiveUpdates();

  mCameraX->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mCameraY->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mCameraZ->setRange( std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max() );
  mCameraPitch->setRange( 0.0, 180.0 );
  mCameraHeading->setRange( 0.0, 360.0 );
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

  whileBlocking( mCameraPitch )->setValue( QgsLayoutUtils::normalizedAngle( pose.pitchAngle() ) );
  whileBlocking( mCameraHeading )->setValue( QgsLayoutUtils::normalizedAngle( pose.headingAngle() ) );
  whileBlocking( mCameraDistanceFromCenterPoint )->setValue( pose.distanceFromCenterPoint() );
}

void Qgs3DCameraControlsWidget::setCRSInfo() const
{
  QgsCoordinateReferenceSystem crs = m3DMapCanvas->mapSettings()->crs();
  labelCRS->setText( tr( "Coordinates in 3D map CRS" ) + u" (%1)"_s.arg( crs.userFriendlyIdentifier( Qgis::CrsIdentifierType::ShortString ) ) );
  labelCRS->setToolTip( crs.userFriendlyIdentifier( Qgis::CrsIdentifierType::MediumString ) );
}

void Qgs3DCameraControlsWidget::autoApply()
{
  mAutoApplyTimer->start( 100 );
}

void Qgs3DCameraControlsWidget::liveApplyToggled( const bool liveUpdateEnabled )
{
  setting3DCameraControlsLiveUpdate->setValue( liveUpdateEnabled );
  mButtonBox->button( QDialogButtonBox::Apply )->setEnabled( !liveUpdateEnabled );
  if ( liveUpdateEnabled )
  {
    connectLiveUpdates();
    applySettings();
  }
  else
  {
    disconnectLiveUpdates();
  }
}

void Qgs3DCameraControlsWidget::connectLiveUpdates()
{
  connect( mCameraX, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DCameraControlsWidget::autoApply );
  connect( mCameraY, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DCameraControlsWidget::autoApply );
  connect( mCameraZ, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DCameraControlsWidget::autoApply );
  connect( mCameraPitch, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DCameraControlsWidget::autoApply );
  connect( mCameraHeading, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DCameraControlsWidget::autoApply );
  connect( mCameraDistanceFromCenterPoint, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DCameraControlsWidget::autoApply );
}

void Qgs3DCameraControlsWidget::disconnectLiveUpdates()
{
  disconnect( mCameraX, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DCameraControlsWidget::autoApply );
  disconnect( mCameraY, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DCameraControlsWidget::autoApply );
  disconnect( mCameraZ, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DCameraControlsWidget::autoApply );
  disconnect( mCameraPitch, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DCameraControlsWidget::autoApply );
  disconnect( mCameraHeading, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DCameraControlsWidget::autoApply );
  disconnect( mCameraDistanceFromCenterPoint, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DCameraControlsWidget::autoApply );
}

void Qgs3DCameraControlsWidget::applySettings()
{
  updateCameraLookingAt();

  QgsCameraPose pose = m3DMapCanvas->cameraController()->cameraPose();
  pose.setPitchAngle( static_cast< float >( mCameraPitch->value() ) );
  pose.setHeadingAngle( static_cast< float >( mCameraHeading->value() ) );
  pose.setDistanceFromCenterPoint( static_cast< float >( mCameraDistanceFromCenterPoint->value() ) );
  m3DMapCanvas->cameraController()->setCameraPose( pose );
}
