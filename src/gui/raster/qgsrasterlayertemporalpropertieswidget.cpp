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
#include "qgsprojecttimesettings.h"
#include "qgsrasterdataprovidertemporalcapabilities.h"
#include "qgsrasterlayer.h"


QgsRasterLayerTemporalPropertiesWidget::QgsRasterLayerTemporalPropertiesWidget( QWidget *parent, QgsRasterLayer *layer )
  : QWidget( parent )
  , mLayer( layer )
{
  setupUi( this );
  connect( mSetEndAsStartNormalButton, &QPushButton::clicked, this, &QgsRasterLayerTemporalPropertiesWidget::setEndAsStartNormalButton_clicked );
//  connect( mSetEndAsStartReferenceButton, &QPushButton::clicked, this, &QgsRasterLayerTemporalPropertiesWidget::setEndAsStartReferenceButton_clicked );
  connect( mResetDatesButton, &QPushButton::clicked, this, &QgsRasterLayerTemporalPropertiesWidget::resetDatesButton_clicked );
  connect( mLayerRadioButton, &QRadioButton::toggled, this, &QgsRasterLayerTemporalPropertiesWidget::layerRadioButton_toggled );
  // connect( mProjectRadioButton, &QRadioButton::toggled, this, &QgsRasterLayerTemporalPropertiesWidget::projectRadioButton_toggled );
  // connect( mReferenceCheckBox, &QCheckBox::clicked, this, &QgsRasterLayerTemporalPropertiesWidget::referenceCheckBox_clicked );

//  mFetchModeComboBox->addItem( tr( "Use Whole Temporal Range" ), QgsRasterDataProviderTemporalCapabilities::MatchUsingWholeRange );
//  mFetchModeComboBox->addItem( tr( "Match to Start of Range" ), QgsRasterDataProviderTemporalCapabilities::MatchExactUsingStartOfRange );
//  mFetchModeComboBox->addItem( tr( "Match to End of Range" ), QgsRasterDataProviderTemporalCapabilities::MatchExactUsingEndOfRange );

  mSetEndAsStartNormalButton->setToolTip( tr( "Set the end datetime same as the start datetime" ) );
  //mSetEndAsStartReferenceButton->setToolTip( tr( "Set the end datetime same as the start datetime" ) );
  mResetDatesButton->setToolTip( tr( "Reset the start and end datetime inputs" ) );
  //mDisableTime->setToolTip( "Use only the date in the datetime inputs to update the temporal range" );

  init();
}

void QgsRasterLayerTemporalPropertiesWidget::init()
{
  setInputWidgetState( TemporalDimension::BiTemporal, false );
  setDateTimeInputsLimit();
  setDateTimeInputsLocale();

  mTemporalGroupBox->setChecked( mLayer->temporalProperties()->isActive() );
// mFetchModeComboBox->setCurrentIndex( mFetchModeComboBox->findData( mLayer->temporalProperties()->intervalHandlingMethod() ) );

// if ( mLayer->temporalProperties()->temporalSource() == QgsMapLayerTemporalProperties::TemporalSource::Project )
  //  mProjectRadioButton->setChecked( true );

  // updateRangeLabel( mLabel );

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
//    mStartReferenceDateTimeEdit->setEnabled( enabled );
//    mEndReferenceDateTimeEdit->setEnabled( enabled );
//    mSetEndAsStartReferenceButton->setEnabled( enabled );
  }

}

void QgsRasterLayerTemporalPropertiesWidget::setDateTimeInputsLocale()
{
  QLocale locale;
  mStartTemporalDateTimeEdit->setDisplayFormat(
    locale.dateTimeFormat( QLocale::ShortFormat ) );
  mEndTemporalDateTimeEdit->setDisplayFormat(
    locale.dateTimeFormat( QLocale::ShortFormat ) );
//  mStartReferenceDateTimeEdit->setDisplayFormat(
//    locale.dateTimeFormat( QLocale::ShortFormat ) );
//  mEndReferenceDateTimeEdit->setDisplayFormat(
//    locale.dateTimeFormat( QLocale::ShortFormat ) );
}

void QgsRasterLayerTemporalPropertiesWidget::setDateTimeInputsLimit()
{
  QgsDateTimeRange fixedRange = mLayer->temporalProperties()->fixedTemporalRange();
  QgsDateTimeRange fixedReferenceRange = mLayer->temporalProperties()->fixedReferenceTemporalRange();

  QgsDateTimeRange range = mLayer->temporalProperties()->temporalRange();
  QgsDateTimeRange referenceRange = mLayer->temporalProperties()->referenceTemporalRange();

  // Set initial date time input values to the layers temporal range only if the
  // ranges are valid

  mStartTemporalDateTimeEdit->setDateTimeRange( fixedRange.begin(), fixedRange.end() );
  mEndTemporalDateTimeEdit->setDateTimeRange( fixedRange.begin(), fixedRange.end() );
//  mStartReferenceDateTimeEdit->setDateTimeRange( fixedReferenceRange.begin(), fixedReferenceRange.end() );
//  mEndReferenceDateTimeEdit->setDateTimeRange( fixedReferenceRange.begin(), fixedReferenceRange.end() );

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
//    mStartReferenceDateTimeEdit->setDateTime( referenceRange.begin() );
//    mEndReferenceDateTimeEdit->setDateTime( referenceRange.end() );
  }
  else
  {
    if ( fixedReferenceRange.begin().isValid() && fixedReferenceRange.end().isValid() )
    {
//      mStartReferenceDateTimeEdit->setDateTime( fixedReferenceRange.begin() );
//      mEndReferenceDateTimeEdit->setDateTime( fixedReferenceRange.end() );
    }
  }
}

void QgsRasterLayerTemporalPropertiesWidget::updateRangeLabel( QLabel *label )
{
  QLocale locale;
  if ( mLayer->temporalProperties()->temporalSource() ==
       QgsMapLayerTemporalProperties::TemporalSource::Layer )
  {
    if ( mLayer->type() == QgsMapLayerType::RasterLayer )
    {
      QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *> ( mLayer );
      QgsDateTimeRange range = rasterLayer->temporalProperties()->temporalRange();

      if ( range.begin().isValid() && range.end().isValid() )
        label->setText( tr( "Current layer range: %1 to %2" ).arg(
                          range.begin().toString( locale.dateTimeFormat() ),
                          range.end().toString( locale.dateTimeFormat() ) ) );
      else
        label->setText( tr( "Layer temporal range is not set" ) );
    }
  }
  else if ( mLayer->temporalProperties()->temporalSource() ==
            QgsMapLayerTemporalProperties::TemporalSource::Project )
  {
    QgsDateTimeRange range = QgsProject::instance()->timeSettings()->temporalRange();

    if ( range.begin().isValid() && range.end().isValid() )
      label->setText( tr( "Project time range is from %1 to %2 " ).arg(
                        range.begin().toString( locale.dateTimeFormat() ),
                        range.end().toString( locale.dateTimeFormat() ) ) );
    else
      label->setText( tr( "Temporal range from the Project time settings is invalid, change it before using it here." ) );
  }
}

void QgsRasterLayerTemporalPropertiesWidget::setEndAsStartNormalButton_clicked()
{
  mEndTemporalDateTimeEdit->setDateTime( mStartTemporalDateTimeEdit->dateTime() );
  // updateRangeLabel( mLabel );
}

void QgsRasterLayerTemporalPropertiesWidget::setEndAsStartReferenceButton_clicked()
{
  //mEndReferenceDateTimeEdit->setDateTime( mStartReferenceDateTimeEdit->dateTime() );
}

void QgsRasterLayerTemporalPropertiesWidget::layerRadioButton_toggled( bool checked )
{
  if ( checked )
  {
    // updateRangeLabel( mLabel );
  }
}

void QgsRasterLayerTemporalPropertiesWidget::projectRadioButton_toggled( bool checked )
{
  if ( checked )
  {
    // updateRangeLabel( mLabel );
  }
}

void QgsRasterLayerTemporalPropertiesWidget::resetDatesButton_clicked()
{
  QgsDateTimeRange layerFixedRange;

  layerFixedRange = mLayer->temporalProperties()->fixedTemporalRange();
  if ( layerFixedRange.begin().isValid() && layerFixedRange.end().isValid() )
  {
    mStartTemporalDateTimeEdit->setDateTime( layerFixedRange.begin() );
    mEndTemporalDateTimeEdit->setDateTime( layerFixedRange.end() );
  }
  // else
  // mLabel->setText( tr( "Cannot reset dates - no temporal metadata "
  //  "is available for this layer" ) );
}

void QgsRasterLayerTemporalPropertiesWidget::referenceCheckBox_clicked()
{
//  if ( mReferenceCheckBox->isChecked() )
//    setInputWidgetState( TemporalDimension::BiTemporal, true );
//  else
//    setInputWidgetState( TemporalDimension::BiTemporal, false );
}

void QgsRasterLayerTemporalPropertiesWidget::saveTemporalProperties()
{
  mLayer->temporalProperties()->setIsActive( mTemporalGroupBox->isChecked() );

  if ( mLayerRadioButton->isChecked() )
  {
    QgsDateTimeRange normalRange = QgsDateTimeRange( mStartTemporalDateTimeEdit->dateTime(),
                                   mEndTemporalDateTimeEdit->dateTime() );
    mLayer->temporalProperties()->setTemporalRange( normalRange );
    mLayer->temporalProperties()->setTemporalSource( QgsMapLayerTemporalProperties::TemporalSource::Layer );
  }
//  else if ( mProjectRadioButton->isChecked() )
//  {
//    QgsDateTimeRange projectRange;

//    if ( QgsProject::instance()->timeSettings() )
//      projectRange = QgsProject::instance()->timeSettings()->temporalRange();

//    if ( !projectRange.begin().isValid() || !projectRange.end().isValid() )
//      return;

//    mLayer->temporalProperties()->setTemporalRange( projectRange );
//    mLayer->temporalProperties()->setTemporalSource( QgsMapLayerTemporalProperties::TemporalSource::Project );
//  }

  //mLayer->temporalProperties()->setIntervalHandlingMethod( static_cast< QgsRasterDataProviderTemporalCapabilities::IntervalHandlingMethod >( mFetchModeComboBox->currentData().toInt() ) );

//  if ( mReferenceCheckBox->isChecked() )
//  {
//    QgsDateTimeRange referenceRange = QgsDateTimeRange( mReferenceDateTimeEdit->dateTime(),
//                                      mReferenceDateTimeEdit->dateTime() );
//    mLayer->temporalProperties()->setReferenceTemporalRange( referenceRange );
//  }
}
