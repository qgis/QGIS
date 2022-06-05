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

#include <QGridLayout>
#include <QToolButton>
#include <QObject>
#include <QHeaderView>
#include <QCheckBox>
#include "qgis.h"

Q_NOWARN_DEPRECATED_PUSH
#include "qwt_compass.h"
#include "qwt_dial_needle.h"
Q_NOWARN_DEPRECATED_POP

#include "qgsapplication.h"

#include "qgscameracontroller.h"
#include "qgs3dnavigationwidget.h"
#include "qgs3daxis.h"

#include <Qt3DRender/QCamera>

Qgs3DNavigationWidget::Qgs3DNavigationWidget( Qgs3DMapCanvas *parent ) : QWidget( parent )
{
  setupUi( this );

  mParent3DMapCanvas = parent;
  // Zoom in button
  QObject::connect(
    mZoomInButton,
    &QToolButton::clicked,
    parent,
    [ = ]
  {
    parent->cameraController()->zoom( 5 );
  }
  );

  // Zoom out button
  QObject::connect(
    mZoomOutButton,
    &QToolButton::clicked,
    parent,
    [ = ]
  {
    parent->cameraController()->zoom( -5 );
  }
  );

  // Tilt up button
  QObject::connect(
    mTiltUpButton,
    &QToolButton::clicked,
    parent,
    [ = ]
  {
    parent->cameraController()->tiltUpAroundViewCenter( 1 );
  }
  );

  // Tilt down button
  QObject::connect(
    mTiltDownButton,
    &QToolButton::clicked,
    parent,
    [ = ]
  {
    parent->cameraController()->tiltUpAroundViewCenter( -1 );
  }
  );

  // Compas
  QwtCompassMagnetNeedle *compasNeedle = new QwtCompassMagnetNeedle();
  mCompass->setNeedle( compasNeedle );

  QObject::connect(
    mCompass,
    &QwtDial::valueChanged,
    parent,
    [ = ]
  {
    parent->cameraController()->setCameraHeadingAngle( float( mCompass->value() ) );
  }
  );

  // Move up button
  QObject::connect(
    mMoveUpButton,
    &QToolButton::clicked,
    parent,
    [ = ]
  {
    parent->cameraController()->moveView( 0, 1 );
  }
  );

  // Move right button
  QObject::connect(
    mMoveRightButton,
    &QToolButton::clicked,
    parent,
    [ = ]
  {
    parent->cameraController()->moveView( 1, 0 );
  }
  );

  // Move down button
  QObject::connect(
    mMoveDownButton,
    &QToolButton::clicked,
    parent,
    [ = ]
  {
    parent->cameraController()->moveView( 0, -1 );
  }
  );

  // Move left button
  QObject::connect(
    mMoveLeftButton,
    &QToolButton::clicked,
    parent,
    [ = ]
  {
    parent->cameraController()->moveView( -1, 0 );
  }
  );

  mCameraInfoItemModel = new QStandardItemModel( this );

  mCameraInfoItemModel->appendRow( QList<QStandardItem *> { new QStandardItem( QStringLiteral( "Near plane" ) ), new QStandardItem } );
  mCameraInfoItemModel->appendRow( QList<QStandardItem *> { new QStandardItem( QStringLiteral( "Far plane" ) ), new QStandardItem } );
  mCameraInfoItemModel->appendRow( QList<QStandardItem *> { new QStandardItem( QStringLiteral( "Camera X pos" ) ), new QStandardItem } );
  mCameraInfoItemModel->appendRow( QList<QStandardItem *> { new QStandardItem( QStringLiteral( "Camera Y pos" ) ), new QStandardItem } );
  mCameraInfoItemModel->appendRow( QList<QStandardItem *> { new QStandardItem( QStringLiteral( "Camera Z pos" ) ), new QStandardItem } );
  mCameraInfoItemModel->appendRow( QList<QStandardItem *> { new QStandardItem( QStringLiteral( "Looking at X" ) ), new QStandardItem } );
  mCameraInfoItemModel->appendRow( QList<QStandardItem *> { new QStandardItem( QStringLiteral( "Looking at Y" ) ), new QStandardItem } );
  mCameraInfoItemModel->appendRow( QList<QStandardItem *> { new QStandardItem( QStringLiteral( "Looking at Z" ) ), new QStandardItem } );

  mCameraInfo->setModel( mCameraInfoItemModel );
  mCameraInfo->verticalHeader()->hide();
  mCameraInfo->horizontalHeader()->hide();
  mCameraInfo->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeMode::Stretch );

  QObject::connect( mCameraInfoCheckBox, &QCheckBox::clicked, parent, [ = ]( bool enabled ) { mCameraInfo->setVisible( enabled ); } );
}

void Qgs3DNavigationWidget::updateFromCamera()
{
  // Make sure the angle is between 0 - 359
  whileBlocking( mCompass )->setValue( fmod( mParent3DMapCanvas->cameraController()->yaw() + 360, 360 ) );

  mCameraInfoItemModel->setData( mCameraInfoItemModel->index( 0, 1 ), QStringLiteral( "%1" ).arg( mParent3DMapCanvas->cameraController()->camera()->nearPlane() ) );
  mCameraInfoItemModel->setData( mCameraInfoItemModel->index( 1, 1 ), QStringLiteral( "%1" ).arg( mParent3DMapCanvas->cameraController()->camera()->farPlane() ) );
  mCameraInfoItemModel->setData( mCameraInfoItemModel->index( 2, 1 ), QStringLiteral( "%1" ).arg( mParent3DMapCanvas->cameraController()->camera()->position().x() ) );
  mCameraInfoItemModel->setData( mCameraInfoItemModel->index( 3, 1 ), QStringLiteral( "%1" ).arg( mParent3DMapCanvas->cameraController()->camera()->position().y() ) );
  mCameraInfoItemModel->setData( mCameraInfoItemModel->index( 4, 1 ), QStringLiteral( "%1" ).arg( mParent3DMapCanvas->cameraController()->camera()->position().z() ) );
  mCameraInfoItemModel->setData( mCameraInfoItemModel->index( 5, 1 ), QStringLiteral( "%1" ).arg( mParent3DMapCanvas->cameraController()->lookingAtPoint().x() ) );
  mCameraInfoItemModel->setData( mCameraInfoItemModel->index( 6, 1 ), QStringLiteral( "%1" ).arg( mParent3DMapCanvas->cameraController()->lookingAtPoint().y() ) );
  mCameraInfoItemModel->setData( mCameraInfoItemModel->index( 7, 1 ), QStringLiteral( "%1" ).arg( mParent3DMapCanvas->cameraController()->lookingAtPoint().z() ) );
}

void Qgs3DNavigationWidget::resizeEvent( QResizeEvent *ev )
{
  QWidget::resizeEvent( ev );

  QSize size = ev->size();
  emit sizeChanged( size );
}

void Qgs3DNavigationWidget::hideEvent( QHideEvent *ev )
{
  QWidget::hideEvent( ev );
  emit sizeChanged( QSize( 0, 0 ) );
}

void Qgs3DNavigationWidget::showEvent( QShowEvent *ev )
{
  QWidget::showEvent( ev );
  emit sizeChanged( size() );
}
