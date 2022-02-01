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

#include <Qt3DRender/QCamera>

Qgs3DNavigationWidget::Qgs3DNavigationWidget( Qgs3DMapCanvas *parent ) : QWidget( parent )
{
  mParent3DMapCanvas = parent;
  // Zoom in button
  mZoomInButton = new QToolButton( this );
  mZoomInButton->setToolTip( tr( "Zoom In" ) );
  mZoomInButton->setAutoRepeat( true );
  mZoomInButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ) );
  mZoomInButton->setAutoRaise( true );

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
  mZoomOutButton = new QToolButton( this );
  mZoomOutButton->setToolTip( tr( "Zoom Out" ) );
  mZoomOutButton->setAutoRepeat( true );
  mZoomOutButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomOut.svg" ) ) );
  mZoomOutButton->setAutoRaise( true );

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
  mTiltUpButton = new QToolButton( this );
  mTiltUpButton->setToolTip( tr( "Tilt Up" ) );
  mTiltUpButton->setAutoRepeat( true );
  mTiltUpButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionTiltUp.svg" ) ) );
  mTiltUpButton->setAutoRaise( true );

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
  mTiltDownButton = new QToolButton( this );
  mTiltDownButton->setToolTip( tr( "Tilt Down" ) );
  mTiltDownButton->setAutoRepeat( true );
  mTiltDownButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionTiltDown.svg" ) ) );
  mTiltDownButton->setAutoRaise( true );

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
  mCompas = new QwtCompass( this );
  mCompas->setToolTip( tr( "Rotate view" ) );
  mCompas->setWrapping( true );
  mCompas->setNeedle( compasNeedle );

  QObject::connect(
    mCompas,
    &QwtDial::valueChanged,
    parent,
    [ = ]
  {
    parent->cameraController()->setCameraHeadingAngle( float( mCompas->value() ) );
  }
  );

  // Move up button
  mMoveUpButton = new QToolButton( this );
  mMoveUpButton->setToolTip( tr( "Move up" ) );
  mMoveUpButton->setAutoRepeat( true );
  mMoveUpButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionArrowUp.svg" ) ) );
  mMoveUpButton->setAutoRaise( true );

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
  mMoveRightButton = new QToolButton( this );
  mMoveRightButton->setToolTip( tr( "Move right" ) );
  mMoveRightButton->setAutoRepeat( true );
  mMoveRightButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionArrowRight.svg" ) ) );
  mMoveRightButton->setAutoRaise( true );

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
  mMoveDownButton = new QToolButton( this );
  mMoveDownButton->setToolTip( tr( "Move down" ) );
  mMoveDownButton->setAutoRepeat( true );
  mMoveDownButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionArrowDown.svg" ) ) );
  mMoveDownButton->setAutoRaise( true );

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
  mMoveLeftButton = new QToolButton( this );
  mMoveLeftButton->setToolTip( tr( "Move left" ) );
  mMoveLeftButton->setAutoRepeat( true );
  mMoveLeftButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionArrowLeft.svg" ) ) );
  mMoveLeftButton->setAutoRaise( true );

  QObject::connect(
    mMoveLeftButton,
    &QToolButton::clicked,
    parent,
    [ = ]
  {
    parent->cameraController()->moveView( -1, 0 );
  }
  );

  mCameraInfo = new QTableView( this );
  mCameraInfo->setEditTriggers( QAbstractItemView::NoEditTriggers );

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

  QGridLayout *gridLayout = new QGridLayout( this );
  gridLayout->addWidget( mTiltUpButton, 0, 0 );
  gridLayout->addWidget( mTiltDownButton, 3, 0 );
  gridLayout->addWidget( mZoomInButton, 0, 3 );
  gridLayout->addWidget( mZoomOutButton, 3, 3 );
  gridLayout->addWidget( mCompas, 1, 1, 2, 2 );
  gridLayout->addWidget( mMoveUpButton, 0, 1, 1, 2, Qt::AlignCenter );
  gridLayout->addWidget( mMoveRightButton, 1, 3, 2, 1, Qt::AlignCenter );
  gridLayout->addWidget( mMoveDownButton, 3, 1, 1, 2, Qt::AlignCenter );
  gridLayout->addWidget( mMoveLeftButton, 1, 0, 2, 1, Qt::AlignCenter );

  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget( mCameraInfo );
  mCameraInfo->setVisible( false );

  QCheckBox *cameraInfoCheckBox = new QCheckBox( this );
  cameraInfoCheckBox->setText( tr( "Show camera info (for debugging)" ) );
  cameraInfoCheckBox->setChecked( false );
  QObject::connect( cameraInfoCheckBox, &QCheckBox::clicked, parent, [ = ]( bool enabled ) { mCameraInfo->setVisible( enabled ); } );

  gridLayout->addWidget( cameraInfoCheckBox, 4, 0, 1, 4, Qt::AlignLeft );
  gridLayout->addLayout( layout, 5, 0, 1, 4, Qt::AlignCenter );

  gridLayout->setAlignment( Qt::AlignTop );
}

void Qgs3DNavigationWidget::updateFromCamera()
{
  // Make sure the angle is between 0 - 359
  whileBlocking( mCompas )->setValue( fmod( mParent3DMapCanvas->cameraController()->yaw() + 360, 360 ) );

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
