/***************************************************************************
  qgs3danimationwidget.cpp
  --------------------------------------
  Date                 : July 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3danimationwidget.h"

#include "qgs3danimationsettings.h"
#include "qgsapplication.h"
#include "qgscameracontroller.h"
#include "qgs3danimationexportdialog.h"
#include "qgs3dmapsettings.h"
#include "qgsoffscreen3dengine.h"
#include "qgs3dmapscene.h"
#include "qgs3dutils.h"
#include "qgsfeedback.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QProgressDialog>

Qgs3DAnimationWidget::Qgs3DAnimationWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  btnAddKeyframe->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  btnRemoveKeyframe->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );
  btnEditKeyframe->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.svg" ) ) );
  btnPlayPause->setIcon( QIcon( QgsApplication::iconPath( "mTaskRunning.svg" ) ) );
  btnDuplicateKeyframe->setIcon( QIcon( QgsApplication::iconPath( "mActionEditCopy.svg" ) ) );
  btnExportAnimation->setIcon( QIcon( QgsApplication::iconPath( "mActionFileSave.svg" ) ) );
  cboKeyframe->addItem( tr( "<none>" ) );

  mAnimationTimer = new QTimer( this );
  mAnimationTimer->setInterval( 10 );
  connect( mAnimationTimer, &QTimer::timeout, this, &Qgs3DAnimationWidget::onAnimationTimer );

  connect( btnAddKeyframe, &QToolButton::clicked, this, &Qgs3DAnimationWidget::onAddKeyframe );
  connect( btnRemoveKeyframe, &QToolButton::clicked, this, &Qgs3DAnimationWidget::onRemoveKeyframe );
  connect( btnEditKeyframe, &QToolButton::clicked, this, &Qgs3DAnimationWidget::onEditKeyframe );
  connect( btnDuplicateKeyframe, &QToolButton::clicked, this, &Qgs3DAnimationWidget::onDuplicateKeyframe );
  connect( btnExportAnimation, &QToolButton::clicked, this, &Qgs3DAnimationWidget::onExportAnimation );
  connect( cboInterpolation, qOverload<int>( &QComboBox::currentIndexChanged ), this, &Qgs3DAnimationWidget::onInterpolationChanged );

  btnPlayPause->setCheckable( true );
  connect( btnPlayPause, &QToolButton::clicked, this, &Qgs3DAnimationWidget::onPlayPause );

  connect( sliderTime, &QSlider::valueChanged, this, &Qgs3DAnimationWidget::onSliderValueChanged );

  connect( cboKeyframe, qOverload<int>( &QComboBox::currentIndexChanged ), this, &Qgs3DAnimationWidget::onKeyframeChanged );
}

void Qgs3DAnimationWidget::setCameraController( QgsCameraController *cameraController )
{
  mCameraController = cameraController;
  connect( mCameraController, &QgsCameraController::cameraChanged, this, &Qgs3DAnimationWidget::onCameraChanged );
}


void Qgs3DAnimationWidget::setAnimation( const Qgs3DAnimationSettings &animSettings )
{
  whileBlocking( cboInterpolation )->setCurrentIndex( animSettings.easingCurve().type() );

  // initialize GUI from the given animation
  cboKeyframe->clear();
  cboKeyframe->addItem( tr( "<none>" ) );
  for ( const Qgs3DAnimationSettings::Keyframe &keyframe : animSettings.keyFrames() )
  {
    cboKeyframe->addItem( QStringLiteral( "%1 s" ).arg( keyframe.time ) );
    const int lastIndex = cboKeyframe->count() - 1;
    cboKeyframe->setItemData( lastIndex, QVariant::fromValue<Qgs3DAnimationSettings::Keyframe>( keyframe ), Qt::UserRole + 1 );
  }

  initializeController( animSettings );
}

void Qgs3DAnimationWidget::initializeController( const Qgs3DAnimationSettings &animSettings )
{
  mAnimationSettings.reset( new Qgs3DAnimationSettings( animSettings ) );

  sliderTime->setMaximum( animSettings.duration() * 100 );
}

Qgs3DAnimationSettings Qgs3DAnimationWidget::animation() const
{
  Qgs3DAnimationSettings animSettings;
  animSettings.setEasingCurve( QEasingCurve( ( QEasingCurve::Type ) cboInterpolation->currentIndex() ) );
  Qgs3DAnimationSettings::Keyframes keyframes;
  for ( int i = 1; i < cboKeyframe->count(); ++i )
  {
    Qgs3DAnimationSettings::Keyframe kf;
    kf = cboKeyframe->itemData( i, Qt::UserRole + 1 ).value<Qgs3DAnimationSettings::Keyframe>();
    keyframes << kf;
  }
  animSettings.setKeyframes( keyframes );
  return animSettings;
}

void Qgs3DAnimationWidget::setDefaultAnimation()
{
  Qgs3DAnimationSettings animSettings;
  Qgs3DAnimationSettings::Keyframes kf;
  Qgs3DAnimationSettings::Keyframe f1, f2;
  f1.time = 0;
  f1.point = mCameraController->lookingAtPoint();
  f1.dist = mCameraController->distance();
  f1.pitch = mCameraController->pitch();
  f1.yaw = mCameraController->yaw();

  f2.time = 5;
  f2.point = f1.point;
  f2.dist = f1.dist * 2;
  f2.pitch = f1.pitch;
  f2.yaw = f1.yaw;

  kf << f1 << f2;
  animSettings.setKeyframes( kf );

  setAnimation( animSettings );
}

void Qgs3DAnimationWidget::setEditControlsEnabled( bool enabled )
{
  cboKeyframe->setEnabled( enabled );
  btnAddKeyframe->setEnabled( enabled );
  cboInterpolation->setEnabled( enabled );
}

void Qgs3DAnimationWidget::setMap( Qgs3DMapSettings *map )
{
  mMap = map;
}

void Qgs3DAnimationWidget::onPlayPause()
{
  if ( mAnimationTimer->isActive() )
  {
    mAnimationTimer->stop();
    setEditControlsEnabled( true );
  }
  else
  {
    if ( sliderTime->value() >= sliderTime->maximum() )
    {
      sliderTime->setValue( 0 );
    }

    cboKeyframe->setCurrentIndex( 0 ); // unset active keyframe
    setEditControlsEnabled( false );
    mAnimationTimer->start();
  }
}

void Qgs3DAnimationWidget::onAnimationTimer()
{
  if ( sliderTime->value() >= sliderTime->maximum() )
  {
    if ( mLoopingCheckBox->isChecked() )
      sliderTime->setValue( 0 );
    else
    {
      // stop playback
      onPlayPause();
      btnPlayPause->setChecked( false );
    }
  }
  else
  {
    sliderTime->setValue( sliderTime->value() + 1 );
  }
}

void Qgs3DAnimationWidget::onExportAnimation()
{
  if ( !mMap || !mAnimationSettings )
    QMessageBox::warning( this, tr( "Export Animation" ), tr( "Unable to export 3D animation" ) );

  Qgs3DAnimationExportDialog dialog;
  if ( dialog.exec() == QDialog::Accepted )
  {
    QgsFeedback progressFeedback;

    QProgressDialog progressDialog( tr( "Exporting frames..." ), tr( "Abort" ), 0, 100, this );
    progressDialog.setWindowModality( Qt::WindowModal );
    QString error;

    connect( &progressFeedback, &QgsFeedback::progressChanged, this,
             [&progressDialog, &progressFeedback]
    {
      progressDialog.setValue( static_cast<int>( progressFeedback.progress() ) );
      QCoreApplication::processEvents();
    } );

    connect( &progressDialog, &QProgressDialog::canceled, &progressFeedback, &QgsFeedback::cancel );

    const bool success = Qgs3DUtils::exportAnimation(
                           animation(),
                           *mMap,
                           dialog.fps(),
                           dialog.outputDirectory(),
                           dialog.fileNameExpression(),
                           dialog.frameSize(),
                           error,
                           &progressFeedback );

    progressDialog.hide();
    if ( !success )
    {
      QMessageBox::warning( this, tr( "Export Animation" ), error );
      return;
    }
  }
}


void Qgs3DAnimationWidget::onSliderValueChanged()
{
  // make sure we do not have an active keyframe
  if ( cboKeyframe->currentIndex() != 0 )
    cboKeyframe->setCurrentIndex( 0 );

  const Qgs3DAnimationSettings::Keyframe kf = mAnimationSettings->interpolate( sliderTime->value() / 100. );
  mCameraController->setLookingAtPoint( kf.point, kf.dist, kf.pitch, kf.yaw );
}

void Qgs3DAnimationWidget::onCameraChanged()
{
  if ( cboKeyframe->currentIndex() <= 0 )
    return;

  // update keyframe's camera position/rotation
  const int i = cboKeyframe->currentIndex();
  Qgs3DAnimationSettings::Keyframe kf = cboKeyframe->itemData( i, Qt::UserRole + 1 ).value<Qgs3DAnimationSettings::Keyframe>();
  kf.point = mCameraController->lookingAtPoint();
  kf.dist = mCameraController->distance();
  kf.pitch = mCameraController->pitch();
  kf.yaw = mCameraController->yaw();
  cboKeyframe->setItemData( i, QVariant::fromValue<Qgs3DAnimationSettings::Keyframe>( kf ), Qt::UserRole + 1 );

  initializeController( animation() );
}

void Qgs3DAnimationWidget::onKeyframeChanged()
{
  const bool hasKeyframe = cboKeyframe->currentIndex() > 0;
  btnRemoveKeyframe->setEnabled( hasKeyframe );
  btnEditKeyframe->setEnabled( hasKeyframe );
  btnDuplicateKeyframe->setEnabled( hasKeyframe );

  if ( !hasKeyframe )
    return;

  // jump to the camera view of the keyframe
  const Qgs3DAnimationSettings::Keyframe kf = cboKeyframe->itemData( cboKeyframe->currentIndex(), Qt::UserRole + 1 ).value<Qgs3DAnimationSettings::Keyframe>();

  whileBlocking( sliderTime )->setValue( kf.time * 100 );
  mCameraController->setLookingAtPoint( kf.point, kf.dist, kf.pitch, kf.yaw );
}

int Qgs3DAnimationWidget::findIndexForKeyframe( float time )
{
  int newIndex = 0;
  for ( const Qgs3DAnimationSettings::Keyframe &keyframe : mAnimationSettings->keyFrames() )
  {
    if ( keyframe.time > time )
      break;
    newIndex++;
  }
  return newIndex;
}

float Qgs3DAnimationWidget::askForKeyframeTime( float defaultTime, bool *ok )
{
  const double t = QInputDialog::getDouble( this, tr( "Keyframe time" ), tr( "Keyframe time [seconds]:" ), defaultTime, 0, 9999, 2, ok );
  if ( !*ok )
    return 0;

  // figure out position of this keyframe
  for ( const Qgs3DAnimationSettings::Keyframe &keyframe : mAnimationSettings->keyFrames() )
  {
    if ( keyframe.time == t )
    {
      QMessageBox::warning( this, tr( "Keyframe time" ), tr( "There is already a keyframe at the given time" ) );
      *ok = false;
      return 0;
    }
  }

  *ok = true;
  return t;
}

void Qgs3DAnimationWidget::onAddKeyframe()
{
  bool ok;
  const float t = askForKeyframeTime( sliderTime->value() / 100., &ok );
  if ( !ok )
    return;

  const int index = findIndexForKeyframe( t );

  Qgs3DAnimationSettings::Keyframe kf;
  kf.time = t;
  kf.point = mCameraController->lookingAtPoint();
  kf.dist = mCameraController->distance();
  kf.pitch = mCameraController->pitch();
  kf.yaw = mCameraController->yaw();

  cboKeyframe->insertItem( index + 1, QStringLiteral( "%1 s" ).arg( kf.time ) );
  cboKeyframe->setItemData( index + 1, QVariant::fromValue<Qgs3DAnimationSettings::Keyframe>( kf ), Qt::UserRole + 1 );

  initializeController( animation() );

  cboKeyframe->setCurrentIndex( index + 1 );
}

void Qgs3DAnimationWidget::onRemoveKeyframe()
{
  const int index = cboKeyframe->currentIndex();
  if ( index <= 0 )
    return;

  cboKeyframe->setCurrentIndex( 0 );
  cboKeyframe->removeItem( index );

  initializeController( animation() );
}

void Qgs3DAnimationWidget::onEditKeyframe()
{
  const int index = cboKeyframe->currentIndex();
  if ( index <= 0 )
    return;

  Qgs3DAnimationSettings::Keyframe kf = cboKeyframe->itemData( index, Qt::UserRole + 1 ).value<Qgs3DAnimationSettings::Keyframe>();

  bool ok;
  const float t = askForKeyframeTime( kf.time, &ok );
  if ( !ok )
    return;

  cboKeyframe->setCurrentIndex( 0 );
  cboKeyframe->removeItem( index );

  initializeController( animation() );

  // figure out position of this keyframe
  const int newIndex = findIndexForKeyframe( t );

  kf.time = t;

  cboKeyframe->insertItem( newIndex + 1, QStringLiteral( "%1 s" ).arg( kf.time ) );
  cboKeyframe->setItemData( newIndex + 1, QVariant::fromValue<Qgs3DAnimationSettings::Keyframe>( kf ), Qt::UserRole + 1 );

  initializeController( animation() );

  cboKeyframe->setCurrentIndex( newIndex + 1 );
}

void Qgs3DAnimationWidget::onDuplicateKeyframe()
{
  const int index = cboKeyframe->currentIndex();
  if ( index <= 0 )
    return;

  Qgs3DAnimationSettings::Keyframe kf = cboKeyframe->itemData( index, Qt::UserRole + 1 ).value<Qgs3DAnimationSettings::Keyframe>();

  bool ok;
  const float t = askForKeyframeTime( kf.time, &ok );
  if ( !ok )
    return;

  // figure out position of this keyframe
  const int newIndex = findIndexForKeyframe( t );

  kf.time = t;

  cboKeyframe->insertItem( newIndex + 1, QStringLiteral( "%1 s" ).arg( kf.time ) );
  cboKeyframe->setItemData( newIndex + 1, QVariant::fromValue<Qgs3DAnimationSettings::Keyframe>( kf ), Qt::UserRole + 1 );

  initializeController( animation() );

  cboKeyframe->setCurrentIndex( newIndex + 1 );
}

void Qgs3DAnimationWidget::onInterpolationChanged()
{
  initializeController( animation() );

  if ( cboKeyframe->currentIndex() <= 0 )
    onSliderValueChanged();
}
