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

#include <Qt3DAnimation/QAnimationController>
#include <Qt3DRender/QCamera>
#include <QTimer>

Qgs3DAnimationWidget::Qgs3DAnimationWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  btnAddKeyframe->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  btnRemoveKeyframe->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );
  btnEditKeyframe->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.svg" ) ) );
  btnPlayPause->setIcon( QIcon( QgsApplication::iconPath( "mTaskRunning.svg" ) ) );

  cboKeyframe->addItem( tr( "<none>" ) );

  mAnimationTimer = new QTimer( this );
  mAnimationTimer->setInterval( 10 );
  connect( mAnimationTimer, &QTimer::timeout, this, &Qgs3DAnimationWidget::onAnimationTimer );

  mAnimationController = new Qt3DAnimation::QAnimationController( this );

  btnPlayPause->setCheckable( true );
  connect( btnPlayPause, &QToolButton::clicked, this, &Qgs3DAnimationWidget::onPlayPause );

  connect( sliderTime, &QSlider::valueChanged, this, &Qgs3DAnimationWidget::onSliderValueChanged );

  connect( cboKeyframe, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &Qgs3DAnimationWidget::onKeyframeChanged );
}

void Qgs3DAnimationWidget::setCamera( Qt3DRender::QCamera *camera )
{
  mCamera = camera;
  connect( mCamera, &Qt3DRender::QCamera::viewMatrixChanged, this, &Qgs3DAnimationWidget::onCameraViewMatrixChanged );
}

void Qgs3DAnimationWidget::setAnimation( const Qgs3DAnimationSettings &animSettings )
{
  // initialize GUI from the given animation
  cboKeyframe->clear();
  cboKeyframe->addItem( tr( "<none>" ) );
  for ( const Qgs3DAnimationSettings::Keyframe &keyframe : animSettings.keyFrames() )
  {
    cboKeyframe->addItem( QString( "%1 s" ).arg( keyframe.time ) );
    int lastIndex = cboKeyframe->count() - 1;
    cboKeyframe->setItemData( lastIndex, keyframe.time, Qt::UserRole + 1 );
    cboKeyframe->setItemData( lastIndex, keyframe.position, Qt::UserRole + 2 );
    cboKeyframe->setItemData( lastIndex, keyframe.rotation, Qt::UserRole + 3 );
  }

  initializeController( animSettings );
}

void Qgs3DAnimationWidget::initializeController( const Qgs3DAnimationSettings &animSettings )
{
  // set up animation in the controller
  Qt3DAnimation::QAnimationGroup *group = new Qt3DAnimation::QAnimationGroup;
  Qt3DAnimation::QKeyframeAnimation *animation = animSettings.createAnimation( nullptr ); // TODO: who deletes transforms?
  animation->setParent( group );
  animation->setTarget( mCamera->transform() );
  group->addAnimation( animation ); // does not delete animations later

  QVector<Qt3DAnimation::QAnimationGroup *> groups;
  groups << group;
  mAnimationController->setAnimationGroups( groups ); // does not delete groups later

  sliderTime->setMaximum( animSettings.duration() * 100 );
}

Qgs3DAnimationSettings Qgs3DAnimationWidget::animation() const
{
  Qgs3DAnimationSettings animSettings;
  Qgs3DAnimationSettings::Keyframes keyframes;
  qDebug() << "---";
  for ( int i = 1; i < cboKeyframe->count(); ++i )
  {
    Qgs3DAnimationSettings::Keyframe kf;
    kf.time = cboKeyframe->itemData( i, Qt::UserRole + 1 ).toFloat();
    kf.position = cboKeyframe->itemData( i, Qt::UserRole + 2 ).value<QVector3D>();
    kf.rotation = cboKeyframe->itemData( i, Qt::UserRole + 3 ).value<QQuaternion>();
    keyframes << kf;
    qDebug() << "keyframe" << kf.time << kf.position << kf.rotation;
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
  f1.position = mCamera->transform()->translation();
  f1.rotation = mCamera->transform()->rotation();
  f2.time = 5;
  f2.position = f1.position + QVector3D( 0, 0, f1.position.z() / 2 );
  f2.rotation = f1.rotation;
  kf << f1 << f2;
  animSettings.setKeyframes( kf );

  setAnimation( animSettings );
}

void Qgs3DAnimationWidget::onPlayPause()
{
  if ( mAnimationTimer->isActive() )
  {
    mAnimationTimer->stop();
    cboKeyframe->setEnabled( true );
  }
  else
  {
    cboKeyframe->setCurrentIndex( 0 );  // unset active keyframe
    cboKeyframe->setEnabled( false );
    mAnimationTimer->start();
  }
}

void Qgs3DAnimationWidget::onAnimationTimer()
{
  float duration = sliderTime->maximum();
  sliderTime->setValue( sliderTime->value() >= duration ? 0 : sliderTime->value() + 1 );
}

void Qgs3DAnimationWidget::onSliderValueChanged()
{
  mAnimationController->setPosition( sliderTime->value() / 100. );
}

void Qgs3DAnimationWidget::onCameraViewMatrixChanged()
{
  if ( cboKeyframe->currentIndex() <= 0 )
    return;

  // update keyframe's camera position/rotation
  int i = cboKeyframe->currentIndex();
  cboKeyframe->setItemData( i, mCamera->transform()->translation(), Qt::UserRole + 2 );
  cboKeyframe->setItemData( i, mCamera->transform()->rotation(), Qt::UserRole + 3 );

  initializeController( animation() );
}

void Qgs3DAnimationWidget::onKeyframeChanged()
{
  if ( cboKeyframe->currentIndex() <= 0 )
    return;

  // jump to the camera view of the keyframe
  float time = cboKeyframe->itemData( cboKeyframe->currentIndex(), Qt::UserRole + 1 ).toFloat();
  sliderTime->setValue( time * 100 );
}
