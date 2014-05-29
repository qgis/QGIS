/***************************************************************************
   qgsscalerangewidget.cpp
    --------------------------------------
   Date                 : 25.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsscalerangewidget.h"
#include "qgsapplication.h"
#include "qgsproject.h"


QgsScaleRangeWidget::QgsScaleRangeWidget( QWidget *parent )
    : QWidget( parent )
    , mCanvas( 0 )
    , mMaximumScaleSetCurrentPushButton( 0 )
    , mMinimumScaleSetCurrentPushButton( 0 )
{
  mLayout = new QGridLayout( this );
  mLayout->setContentsMargins( 0, 0, 0, 0 );

  QLabel* minLbl = new QLabel( tr( "Minimum\n(exclusive)" ), this );
  minLbl->setWordWrap( true );
  minLbl->setAlignment( Qt::AlignTop );
  minLbl->setToolTip( tr( "Minimum scale, i.e. maximum scale denominator. "
                          "This limit is exclusive, that means the layer will not be displayed on this scale." ) );
  QLabel* maxLbl = new QLabel( tr( "Maximum\n(inclusive)" ), this );
  maxLbl->setWordWrap( true );
  maxLbl->setAlignment( Qt::AlignTop );
  maxLbl->setToolTip( tr( "Maximum scale, i.e. minimum scale denominator. "
                          "This limit is inclusive, that means the layer will be displayed on this scale." ) );

  mMinimumScaleIconLabel = new QLabel( this );
  mMinimumScaleIconLabel->setPixmap( QgsApplication::getThemePixmap( "/mActionZoomOut.svg" ) );
  mMaximumScaleIconLabel = new QLabel( this );
  mMaximumScaleIconLabel->setPixmap( QgsApplication::getThemePixmap( "/mActionZoomIn.svg" ) );

  mMinimumScaleComboBox = new QgsScaleComboBox( this );
  mMaximumScaleComboBox = new QgsScaleComboBox( this );
  reloadProjectScales();
  // add start, add comprehension of scales by settings fake ordered values
  mMinimumScaleComboBox->setCurrentIndex( 2 );
  mMaximumScaleComboBox->setCurrentIndex( mMinimumScaleComboBox->currentIndex() + 2 );

  mLayout->addWidget( minLbl, 0, 0, 2, 1 );
  mLayout->addWidget( mMinimumScaleIconLabel, 0, 1 );
  mLayout->addWidget( mMinimumScaleComboBox, 0, 2 );
  mLayout->addWidget( maxLbl, 0, 3, 2, 1 );
  mLayout->addWidget( mMaximumScaleIconLabel, 0, 4 );
  mLayout->addWidget( mMaximumScaleComboBox, 0, 5 );

  mLayout->setColumnStretch( 0, 0 );
  mLayout->setColumnStretch( 1, 0 );
  mLayout->setColumnStretch( 2, 3 );
  mLayout->setColumnStretch( 3, 0 );
  mLayout->setColumnStretch( 4, 0 );
  mLayout->setColumnStretch( 5, 3 );
}

QgsScaleRangeWidget::~QgsScaleRangeWidget()
{
}

void QgsScaleRangeWidget::reloadProjectScales()
{
  bool projectScales = QgsProject::instance()->readBoolEntry( "Scales", "/useProjectScales" );
  if ( projectScales )
  {
    QStringList scalesList = QgsProject::instance()->readListEntry( "Scales", "/ScalesList" );
    mMinimumScaleComboBox->updateScales( scalesList );
    mMaximumScaleComboBox->updateScales( scalesList );
  }
}

void QgsScaleRangeWidget::setMapCanvas( QgsMapCanvas *mapCanvas )
{
  if ( mMinimumScaleSetCurrentPushButton )
  {
    delete mMinimumScaleSetCurrentPushButton;
    mMinimumScaleSetCurrentPushButton = 0;
  }
  if ( mMaximumScaleSetCurrentPushButton )
  {
    delete mMaximumScaleSetCurrentPushButton;
    mMaximumScaleSetCurrentPushButton = 0;
  }

  if ( !mapCanvas )
    return;

  mCanvas = mapCanvas;

  mMinimumScaleSetCurrentPushButton = new QPushButton( tr( "current" ), this );
  connect( mMinimumScaleSetCurrentPushButton, SIGNAL( clicked() ), this, SLOT( setMinScaleFromCanvas() ) );
  mMaximumScaleSetCurrentPushButton = new QPushButton( tr( "current" ), this );
  connect( mMaximumScaleSetCurrentPushButton, SIGNAL( clicked() ), this, SLOT( setMaxScaleFromCanvas() ) );

  mLayout->addWidget( mMinimumScaleSetCurrentPushButton, 1, 2 );
  mLayout->addWidget( mMaximumScaleSetCurrentPushButton, 1, 5 );
}

void QgsScaleRangeWidget::setMinimumScale( double scale )
{
  mMinimumScaleComboBox->setScale( scale );
}

double QgsScaleRangeWidget::minimumScale()
{
  return mMinimumScaleComboBox->scale();
}

void QgsScaleRangeWidget::setMaximumScale( double scale )
{
  mMaximumScaleComboBox->setScale( scale );
}

double QgsScaleRangeWidget::maximumScale()
{
  return mMaximumScaleComboBox->scale();
}

double QgsScaleRangeWidget::minimumScaleDenom()
{
  return qRound( 1.0 / maximumScale() );
}

double QgsScaleRangeWidget::maximumScaleDenom()
{
  return qRound( 1.0 / minimumScale() );
}

void QgsScaleRangeWidget::setScaleRange( double min, double max )
{
  setMaximumScale( max );
  setMinimumScale( min );
}

void QgsScaleRangeWidget::setMinScaleFromCanvas()
{
  mMinimumScaleComboBox->setScale( 1.0 / mCanvas->mapSettings().scale() );
}

void QgsScaleRangeWidget::setMaxScaleFromCanvas()
{
  mMaximumScaleComboBox->setScale( 1.0 / mCanvas->mapSettings().scale() );
}


