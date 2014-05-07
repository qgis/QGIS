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
{
  mLayout = new QGridLayout( this );
  mLayout->setContentsMargins( 0, 0, 0, 0 );

  QLabel* maxLbl = new QLabel( tr("Maximimum (inclusive)"), this);
  QLabel* minLbl = new QLabel( tr("Minimimum (exclusive)"), this);

  mMaximumScaleIconLabel = new QLabel(this);
  mMaximumScaleIconLabel->setPixmap( QgsApplication::getThemePixmap( "/mActionZoomIn.svg" ) );
  mMinimumScaleIconLabel = new QLabel(this);
  mMinimumScaleIconLabel->setPixmap( QgsApplication::getThemePixmap( "/mActionZoomOut.svg" ) );

  mMaximumScaleComboBox = new QgsScaleComboBox(this);
  mMinimumScaleComboBox = new QgsScaleComboBox(this);

  mLayout->addWidget(maxLbl, 0, 0);
  mLayout->addWidget(mMaximumScaleIconLabel, 0, 1);
  mLayout->addWidget(mMaximumScaleComboBox, 0, 2);
  mLayout->addWidget(minLbl, 0, 3);
  mLayout->addWidget(mMinimumScaleIconLabel, 0, 4);
  mLayout->addWidget(mMinimumScaleComboBox, 0, 5);
}

QgsScaleRangeWidget::~QgsScaleRangeWidget()
{
}

void QgsScaleRangeWidget::showEvent( QShowEvent * )
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
  if (mMinimumScaleSetCurrentPushButton)
    delete mMinimumScaleSetCurrentPushButton;
  if (mMaximumScaleSetCurrentPushButton)
    delete mMaximumScaleSetCurrentPushButton;
  if ( !mapCanvas )
    return;

  mCanvas = mapCanvas;

  mMaximumScaleSetCurrentPushButton = new QToolButton();
  connect( mMaximumScaleSetCurrentPushButton, SIGNAL(clicked()), this, SLOT(setMaxScaleFromCanvas()));
  mMinimumScaleSetCurrentPushButton = new QToolButton();
  connect( mMinimumScaleSetCurrentPushButton, SIGNAL(clicked()), this, SLOT(setMinScaleFromCanvas()));

  mLayout->addWidget(mMaximumScaleSetCurrentPushButton, 1,2);
  mLayout->addWidget(mMinimumScaleSetCurrentPushButton, 1,5);
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

void QgsScaleRangeWidget::setMinScaleFromCanvas()
{
  mMinimumScaleComboBox->setScale( 1.0 / mCanvas->mapSettings().scale() );
}

void QgsScaleRangeWidget::setMaxScaleFromCanvas()
{
  mMaximumScaleComboBox->setScale( 1.0 / mCanvas->mapSettings().scale() );
}

void QgsScaleRangeWidget::setFromLayer( QgsMapLayer *layer )
{
  mMinimumScaleComboBox->setScale( 1.0 / layer->minimumScale() );
  mMaximumScaleComboBox->setScale( 1.0 / layer->maximumScale() );
}
