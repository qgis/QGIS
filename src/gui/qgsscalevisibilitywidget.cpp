/***************************************************************************
   qgsscalevisibilitywidget.cpp
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

#include "qgsscalevisibilitywidget.h"
#include "qgsapplication.h"
#include "qgsproject.h"

QgsScaleVisibilityWidget::QgsScaleVisibilityWidget( QWidget *parent )
    : QWidget( parent )
    , mCanvas( 0 )
{
  setupUi( this );

  mMaximumScaleIconLabel->setPixmap( QgsApplication::getThemePixmap( "/mActionZoomIn.svg" ) );
  mMinimumScaleIconLabel->setPixmap( QgsApplication::getThemePixmap( "/mActionZoomOut.svg" ) );

  mMinimumScaleSetCurrentPushButton->hide();
  mMaximumScaleSetCurrentPushButton->hide();
}

QgsScaleVisibilityWidget::~QgsScaleVisibilityWidget()
{
}

void QgsScaleVisibilityWidget::showEvent( QShowEvent * )
{
  bool projectScales = QgsProject::instance()->readBoolEntry( "Scales", "/useProjectScales" );
  if ( projectScales )
  {
    QStringList scalesList = QgsProject::instance()->readListEntry( "Scales", "/ScalesList" );
    mMinimumScaleComboBox->updateScales( scalesList );
    mMaximumScaleComboBox->updateScales( scalesList );
  }
}

void QgsScaleVisibilityWidget::setMapCanvas( QgsMapCanvas *mapCanvas )
{
  if ( !mapCanvas )
    return;

  mCanvas = mapCanvas;
  mMinimumScaleSetCurrentPushButton->show();
  mMaximumScaleSetCurrentPushButton->show();
}

void QgsScaleVisibilityWidget::setMinimumScale( double scale )
{
  mMinimumScaleComboBox->setScale( scale );
}

double QgsScaleVisibilityWidget::minimumScale()
{
  return mMinimumScaleComboBox->scale();
}

void QgsScaleVisibilityWidget::setMaximumScale( double scale )
{
  mMaximumScaleComboBox->setScale( scale );
}

double QgsScaleVisibilityWidget::maximumScale()
{
  return mMaximumScaleComboBox->scale();
}

void QgsScaleVisibilityWidget::on_mMinimumScaleSetCurrentPushButton_clicked()
{
  mMinimumScaleComboBox->setScale( 1.0 / mCanvas->mapSettings().scale() );
}

void QgsScaleVisibilityWidget::on_mMaximumScaleSetCurrentPushButton_clicked()
{
  mMaximumScaleComboBox->setScale( 1.0 / mCanvas->mapSettings().scale() );
}

void QgsScaleVisibilityWidget::setFromLayer( QgsMapLayer *layer )
{
  mMinimumScaleComboBox->setScale( 1.0 / layer->minimumScale() );
  mMaximumScaleComboBox->setScale( 1.0 / layer->maximumScale() );
}
