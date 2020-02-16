/***************************************************************************
                         qgstemporallayerwidget.cpp
                         ------------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstemporallayerwidget.h"
#include "qgsgui.h"
#include "qgsproject.h"

QgsTemporalLayerWidget::QgsTemporalLayerWidget( QWidget *parent, QgsMapLayer *layer )
  : QWidget( parent )
  , mLayer( layer )
{
  setupUi( this );
  connect( mSetEndAsStartNormalButton, &QPushButton::clicked, this, &QgsTemporalLayerWidget::setEndAsStartNormalButton_clicked );
  connect( mSetEndAsStartReferenceButton, &QPushButton::clicked, this, &QgsTemporalLayerWidget::setEndAsStartReferenceButton_clicked );
  connect( mLayerRadioButton, &QRadioButton::clicked, this, &QgsTemporalLayerWidget::layerRadioButton_clicked );
  connect( mProjectRadioButton, &QRadioButton::clicked, this, &QgsTemporalLayerWidget::projectRadioButton_clicked );
  connect( mReferenceCheckBox, &QCheckBox::clicked, this, &QgsTemporalLayerWidget::referenceCheckBox_clicked );

  init();
}

void QgsTemporalLayerWidget::setEndAsStartNormalButton_clicked()
{
  mEndTemporalDateTimeEdit->setDateTime( mStartTemporalDateTimeEdit->dateTime() );
  // Update current selection label
  updateRangeLabel( "layer", mRangeLabel );
}

void QgsTemporalLayerWidget::setEndAsStartReferenceButton_clicked()
{
  mEndReferenceDateTimeEdit->setDateTime( mStartReferenceDateTimeEdit->dateTime() );
}

void QgsTemporalLayerWidget::layerRadioButton_clicked()
{
  if ( mLayerRadioButton->isChecked() )
  {
    setInputWidgetState( "normal", true );
    updateRangeLabel( "layer", mRangeLabel );
    mProjectRadioButton->setChecked( false );
  }
  else
    setInputWidgetState( "normal", false );
}

void QgsTemporalLayerWidget::projectRadioButton_clicked()
{
  if ( mProjectRadioButton->isChecked() )
  {
    mLayerRadioButton->setChecked( false );
    setInputWidgetState( "normal", false );
    updateRangeLabel( "project", mRangeLabel );
  }
}

void QgsTemporalLayerWidget::referenceCheckBox_clicked()
{
  if ( mReferenceCheckBox->isChecked() )
    setInputWidgetState( "reference", true );
  else
    setInputWidgetState( "reference", false );
}

void QgsTemporalLayerWidget::init()
{
  mLayerRadioButton->setChecked( true );
  setInputWidgetState( "reference", false );
  setDateTimeInputsLimit();

  updateRangeLabel( "layer", mRangeLabel );
}

void QgsTemporalLayerWidget::setDateTimeInputsLimit()
{
  QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( mLayer );
  QgsDateTimeRange range = layer->dataProvider()->temporalProperties()->fixedTemporalRange();
  QgsDateTimeRange referenceRange = layer->dataProvider()->temporalProperties()->fixedReferenceTemporalRange();

  if ( range.begin().isValid() && range.end().isValid() )
  {
    mStartTemporalDateTimeEdit->setDateTime( range.begin() );
    mEndTemporalDateTimeEdit->setDateTime( range.end() );
  }
  if ( referenceRange.begin().isValid() && referenceRange.end().isValid() )
  {
    mStartReferenceDateTimeEdit->setDateTime( referenceRange.begin() );
    mEndReferenceDateTimeEdit->setDateTime( referenceRange.end() );
  }
}

void QgsTemporalLayerWidget::setInputWidgetState( QString type, bool enabled )
{
  if ( type == "normal" )
  {
    mStartTemporalDateTimeEdit->setEnabled( enabled );
    mEndTemporalDateTimeEdit->setEnabled( enabled );
    mSetEndAsStartNormalButton->setEnabled( enabled );
  }
  else if ( type == "reference" )
  {
    mStartReferenceDateTimeEdit->setEnabled( enabled );
    mEndReferenceDateTimeEdit->setEnabled( enabled );
    mSetEndAsStartReferenceButton->setEnabled( enabled );
  }
}

void QgsTemporalLayerWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mCanvas = canvas;
}

void QgsTemporalLayerWidget::saveTemporalProperties()
{
  if ( mLayerRadioButton->isChecked() )
  {
    if ( mLayer->type() == QgsMapLayerType::RasterLayer &&
         mLayer->providerType() == "wms" )
    {
      QgsDateTimeRange normalRange = QgsDateTimeRange( mStartTemporalDateTimeEdit->dateTime(),
                                     mEndTemporalDateTimeEdit->dateTime() );
      QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( mLayer );
      rasterLayer->dataProvider()->temporalProperties()->setTemporalRange( normalRange );

      if ( mDisableTime->isChecked() )
        rasterLayer->dataProvider()->temporalProperties()->setEnableTime( false );
      else
        rasterLayer->dataProvider()->temporalProperties()->setEnableTime( true );

      // Update current selection label
      updateRangeLabel( "layer", mRangeLabel );

      if ( mReferenceCheckBox->isChecked() )
      {
        QgsDateTimeRange referenceRange = QgsDateTimeRange( mStartReferenceDateTimeEdit->dateTime(),
                                          mEndReferenceDateTimeEdit->dateTime() );
        rasterLayer->dataProvider()->temporalProperties()->setReferenceTemporalRange( referenceRange );
      }
    }

    if ( mProjectRadioButton->isChecked() )
    {
      if ( mLayer->type() == QgsMapLayerType::RasterLayer &&
           mLayer->providerType() == "wms" )
      {
        QgsDateTimeRange projectRange = mCanvas->mapSettings().temporalRange();
        QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *> ( mLayer );
        rasterLayer->dataProvider()->temporalProperties()->setTemporalRange( projectRange );

      }
    }
  }
}

void QgsTemporalLayerWidget::updateRangeLabel( QString type, QLabel *label )
{
  if ( type == "layer" )
    label->setText( tr( "Current selection range: %1 to %2" ).arg(
                      mStartReferenceDateTimeEdit->dateTime().toString(),
                      mEndReferenceDateTimeEdit->dateTime().toString() ) );
  else if ( type == "project" )
  {
    QgsDateTimeRange range = mCanvas->mapSettings().temporalRange();

    if ( range.begin().isValid() && range.end().isValid() )
      label->setText( tr( "Current selection range: %1 to %2 from the project settings" ).arg(
                        range.begin().toString(),
                        range.end().toString() ) );
    else
      label->setText( tr( "Date time range from the project is invalid, change it in Project options before using it here." ) );
  }
}


