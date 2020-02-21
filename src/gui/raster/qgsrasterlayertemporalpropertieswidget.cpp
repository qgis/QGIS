/***************************************************************************
                         qgsrasterlayertemporalpropertieswidget.cpp
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

#include "qgsrasterlayertemporalpropertieswidget.h"
#include "qgsgui.h"
#include "qgsproject.h"

QgsRasterLayerTemporalPropertiesWidget::QgsRasterLayerTemporalPropertiesWidget( QWidget *parent, QgsMapLayer *layer )
  : QWidget( parent )
  , mLayer( layer )
{
  setupUi( this );
  connect( mSetEndAsStartNormalButton, &QPushButton::clicked, this, &QgsRasterLayerTemporalPropertiesWidget::setEndAsStartNormalButton_clicked );
  connect( mSetEndAsStartReferenceButton, &QPushButton::clicked, this, &QgsRasterLayerTemporalPropertiesWidget::setEndAsStartReferenceButton_clicked );
  connect( mLayerRadioButton, &QRadioButton::clicked, this, &QgsRasterLayerTemporalPropertiesWidget::layerRadioButton_clicked );
  connect( mProjectRadioButton, &QRadioButton::clicked, this, &QgsRasterLayerTemporalPropertiesWidget::projectRadioButton_clicked );
  connect( mReferenceCheckBox, &QCheckBox::clicked, this, &QgsRasterLayerTemporalPropertiesWidget::referenceCheckBox_clicked );

  init();
}

void QgsRasterLayerTemporalPropertiesWidget::setEndAsStartNormalButton_clicked()
{
  mEndTemporalDateTimeEdit->setDateTime( mStartTemporalDateTimeEdit->dateTime() );
  // Update current selection label
  updateRangeLabel( TemporalRangeSource::Layer, mRangeLabel );
}

void QgsRasterLayerTemporalPropertiesWidget::setEndAsStartReferenceButton_clicked()
{
  mEndReferenceDateTimeEdit->setDateTime( mStartReferenceDateTimeEdit->dateTime() );
}

void QgsRasterLayerTemporalPropertiesWidget::layerRadioButton_clicked()
{
  if ( mLayerRadioButton->isChecked() )
  {
    setInputWidgetState( TemporalDimension::NormalTemporal, true );
    updateRangeLabel( TemporalRangeSource::Layer, mRangeLabel );
    mProjectRadioButton->setChecked( false );
  }
  else
    setInputWidgetState( TemporalDimension::NormalTemporal, false );
}

void QgsRasterLayerTemporalPropertiesWidget::projectRadioButton_clicked()
{
  if ( mProjectRadioButton->isChecked() )
  {
    mLayerRadioButton->setChecked( false );
    setInputWidgetState( TemporalDimension::NormalTemporal, false );
    updateRangeLabel( TemporalRangeSource::Project, mRangeLabel );
  }
}

void QgsRasterLayerTemporalPropertiesWidget::referenceCheckBox_clicked()
{
  if ( mReferenceCheckBox->isChecked() )
    setInputWidgetState( TemporalDimension::BiTemporal, true );
  else
    setInputWidgetState( TemporalDimension::BiTemporal, false );
}

void QgsRasterLayerTemporalPropertiesWidget::init()
{
  mLayerRadioButton->setChecked( true );
  setInputWidgetState( TemporalDimension::BiTemporal, false );
  setDateTimeInputsLimit();

  updateRangeLabel( TemporalRangeSource::Layer, mRangeLabel );
}

void QgsRasterLayerTemporalPropertiesWidget::setDateTimeInputsLimit()
{
  QgsRasterLayer *layer = qobject_cast<QgsRasterLayer *>( mLayer );
  QgsDateTimeRange fixedRange = layer->dataProvider()->temporalCapabilities()->fixedTemporalRange();
  QgsDateTimeRange fixedReferenceRange = layer->dataProvider()->temporalCapabilities()->fixedReferenceTemporalRange();

  QgsDateTimeRange range = layer->dataProvider()->temporalCapabilities()->temporalRange();
  QgsDateTimeRange referenceRange = layer->dataProvider()->temporalCapabilities()->temporalRange();

  // Set initial date time input values to the layers temporal range only if the
  // ranges are valid
  if ( range.begin().isValid() && range.end().isValid() )
  {
    mStartTemporalDateTimeEdit->setDateTime( range.begin() );
    mEndTemporalDateTimeEdit->setDateTime( range.end() );
  }
  else
  {
    if ( fixedRange.begin().isValid() && fixedRange.end().isValid() )
    {
      mStartTemporalDateTimeEdit->setDateTime( fixedRange.begin() );
      mEndTemporalDateTimeEdit->setDateTime( fixedRange.end() );
    }
  }

  if ( referenceRange.begin().isValid() && referenceRange.end().isValid() )
  {
    mStartTemporalDateTimeEdit->setDateTime( referenceRange.begin() );
    mEndTemporalDateTimeEdit->setDateTime( referenceRange.end() );
  }
  else
  {
    if ( fixedReferenceRange.begin().isValid() && fixedReferenceRange.end().isValid() )
    {
      mStartReferenceDateTimeEdit->setDateTime( fixedReferenceRange.begin() );
      mEndReferenceDateTimeEdit->setDateTime( fixedReferenceRange.end() );
    }
  }
}

void QgsRasterLayerTemporalPropertiesWidget::setInputWidgetState( TemporalDimension dimension, bool enabled )
{
  if ( dimension == TemporalDimension::NormalTemporal )
  {
    mStartTemporalDateTimeEdit->setEnabled( enabled );
    mEndTemporalDateTimeEdit->setEnabled( enabled );
    mSetEndAsStartNormalButton->setEnabled( enabled );
  }
  else if ( dimension == TemporalDimension::BiTemporal )
  {
    mStartReferenceDateTimeEdit->setEnabled( enabled );
    mEndReferenceDateTimeEdit->setEnabled( enabled );
    mSetEndAsStartReferenceButton->setEnabled( enabled );
  }
}

void QgsRasterLayerTemporalPropertiesWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mCanvas = canvas;
}

void QgsRasterLayerTemporalPropertiesWidget::saveTemporalProperties()
{
  if ( mLayerRadioButton->isChecked() )
  {
    if ( mLayer->type() == QgsMapLayerType::RasterLayer &&
         mLayer->providerType() == "wms" )
    {
      QgsDateTimeRange normalRange = QgsDateTimeRange( mStartTemporalDateTimeEdit->dateTime(),
                                     mEndTemporalDateTimeEdit->dateTime() );
      QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( mLayer );
      rasterLayer->dataProvider()->temporalCapabilities()->setTemporalRange( normalRange );

      if ( mDisableTime->isChecked() )
        rasterLayer->dataProvider()->temporalCapabilities()->setEnableTime( false );
      else
        rasterLayer->dataProvider()->temporalCapabilities()->setEnableTime( true );

      // Update current selection label
      updateRangeLabel( TemporalRangeSource::Layer, mRangeLabel );

      if ( mReferenceCheckBox->isChecked() )
      {
        QgsDateTimeRange referenceRange = QgsDateTimeRange( mStartReferenceDateTimeEdit->dateTime(),
                                          mEndReferenceDateTimeEdit->dateTime() );
        rasterLayer->dataProvider()->temporalCapabilities()->setReferenceTemporalRange( referenceRange );
        rasterLayer->dataProvider()->temporalCapabilities()->setReferenceEnable( true );
      }
      else
        rasterLayer->dataProvider()->temporalCapabilities()->setReferenceEnable( false );
    }

    if ( mProjectRadioButton->isChecked() )
    {
      if ( mLayer->type() == QgsMapLayerType::RasterLayer &&
           mLayer->providerType() == "wms" )
      {
        QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *> ( mLayer );
//      rasterLayer->dataProvider()->temporalCapabilities()->setTemporalRange( projectRange );

        if ( mReferenceCheckBox->isChecked() )
          rasterLayer->dataProvider()->temporalCapabilities()->setReferenceEnable( true );
        else
          rasterLayer->dataProvider()->temporalCapabilities()->setReferenceEnable( false );
      }
    }
  }
}

void QgsRasterLayerTemporalPropertiesWidget::updateRangeLabel( TemporalRangeSource source, QLabel *label )
{
  if ( source == TemporalRangeSource::Layer )
  {
    if ( mLayer->type() == QgsMapLayerType::RasterLayer &&
         mLayer->providerType() == "wms" )
    {
      QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *> ( mLayer );
      QgsDateTimeRange range = rasterLayer->dataProvider()->temporalCapabilities()->temporalRange();

      if ( range.begin().isValid() && range.end().isValid() )
        label->setText( tr( "Current layer range: %1 to %2" ).arg(
                          range.begin().toString(),
                          range.end().toString() ) );
      else
        label->setText( tr( "Layer temporal range is not set" ) );
    }
  }
  else if ( source == TemporalRangeSource::Project )
  {
//    QgsDateTimeRange range = QgsProject::instance()->timeSettings()->temporalRange();

//    if ( range.begin().isValid() && range.end().isValid() )
//      label->setText( tr( "Current selection range: %1 to %2 from the project settings" ).arg(
//                        range.begin().toString(),
//                        range.end().toString() ) );
//    else
//      label->setText( tr( "Date time range from the project is invalid, change it in Project options before using it here." ) );
  }
}


