/***************************************************************************
                         qgstemporalcontrollerdockwidget.cpp
                         ------------------------------
    begin                : February 2020
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

#include "qgstemporalcontrollerdockwidget.h"
#include "qgsgui.h"
#include "qgsproject.h"
#include "qgsprojecttimesettings.h"
#include "qgstemporalnavigationobject.h"
#include "qgstemporalmapsettingswidget.h"

#include "qgstemporalmapsettingsdialog.h"

QgsTemporalControllerDockWidget::QgsTemporalControllerDockWidget( const QString &name, QWidget *parent )
  : QgsDockWidget( parent )
{
  setupUi( this );
  setWindowTitle( name );

  mNavigationObject = new QgsTemporalNavigationObject( this );

  connect( mForwardButton, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::forwardButton_clicked );
  connect( mBackButton, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::backButton_clicked );
  connect( mNextButton, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::nextButton_clicked );
  connect( mPreviousButton, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::previousButton_clicked );
  connect( mStopButton, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::stopButton_clicked );

  connect( mStartDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsTemporalControllerDockWidget::updateTemporalExtent );
  connect( mEndDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsTemporalControllerDockWidget::updateTemporalExtent );
  connect( mSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsTemporalControllerDockWidget::updateFrameDuration );
  connect( mTimeStepsComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsTemporalControllerDockWidget::updateFrameDuration );
  connect( mSlider, &QSlider::valueChanged, this, &QgsTemporalControllerDockWidget::timeSlider_valueChanged );

  connect( mNavigationObject, &QgsTemporalNavigationObject::updateTemporalRange, this, &QgsTemporalControllerDockWidget::updateSlider );

  connect( mSettings, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::settings_clicked );
  connect( mSetToProjectTimeButton, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::setDatesToProjectTime );

  QgsDateTimeRange range;

  if ( QgsProject::instance()->timeSettings() )
    range = QgsProject::instance()->timeSettings()->temporalRange();
  QLocale locale;

  mStartDateTime->setDisplayFormat( locale.dateTimeFormat( QLocale::ShortFormat ) );
  mEndDateTime->setDisplayFormat( locale.dateTimeFormat( QLocale::ShortFormat ) );

  if ( range.begin().isValid() && range.end().isValid() )
  {
    mStartDateTime->setDateTime( range.begin() );
    mEndDateTime->setDateTime( range.end() );
  }

  mSetToProjectTimeButton->setToolTip( tr( "Set datetimes inputs to match project time" ) );

  for ( QgsUnitTypes::TemporalUnit u :
        {
          QgsUnitTypes::TemporalMilliseconds,
          QgsUnitTypes::TemporalSeconds,
          QgsUnitTypes::TemporalMinutes,
          QgsUnitTypes::TemporalHours,
          QgsUnitTypes::TemporalDays,
          QgsUnitTypes::TemporalWeeks,
          QgsUnitTypes::TemporalMonths,
          QgsUnitTypes::TemporalYears,
          QgsUnitTypes::TemporalDecades,
          QgsUnitTypes::TemporalCenturies
        } )
  {
    mTimeStepsComboBox->addItem( QgsUnitTypes::toString( u ), u );
  }

  // TODO: might want to choose an appropriate default unit based on the range
  mTimeStepsComboBox->setCurrentIndex( mTimeStepsComboBox->findData( QgsUnitTypes::TemporalHours ) );

  mSpinBox->setMinimum( 0.0000001 );
  mSpinBox->setSingleStep( 1 );
  mSpinBox->setValue( 1 );

  mForwardButton->setToolTip( tr( "Play" ) );
  mBackButton->setToolTip( tr( "Reverse" ) );
  mNextButton->setToolTip( tr( "Go to next frame" ) );
  mPreviousButton->setToolTip( tr( "Go to previous frame" ) );
  mStopButton->setToolTip( tr( "Pause" ) );

  updateFrameDuration();
}

void QgsTemporalControllerDockWidget::updateTemporalExtent()
{
  QgsDateTimeRange temporalExtent = QgsDateTimeRange( mStartDateTime->dateTime(),
                                    mEndDateTime->dateTime() );
  mNavigationObject->setTemporalExtents( temporalExtent );
  mSlider->setRange( 0, mNavigationObject->totalFrameCount() - 1 );
  mSlider->setValue( 0 );
}

void QgsTemporalControllerDockWidget::updateFrameDuration()
{
  mNavigationObject->setFrameDuration( QgsInterval( mSpinBox->value(), static_cast< QgsUnitTypes::TemporalUnit>( mTimeStepsComboBox->currentData().toInt() ) ) );
  mSlider->setRange( 0, mNavigationObject->totalFrameCount() - 1 );
}

void QgsTemporalControllerDockWidget::updateSlider( const QgsDateTimeRange &range )
{
  whileBlocking( mSlider )->setValue( mNavigationObject->currentFrameNumber() );
  updateRangeLabel( range );
}

void QgsTemporalControllerDockWidget::updateRangeLabel( const QgsDateTimeRange &range )
{
  QLocale locale;
  mCurrentRangeLabel->setText( tr( "%1 to %2" ).arg(
                                 range.begin().toString( locale.dateTimeFormat( QLocale::NarrowFormat ) ),
                                 range.end().toString( locale.dateTimeFormat( QLocale::NarrowFormat ) ) ) );
}

QgsTemporalController *QgsTemporalControllerDockWidget::temporalController()
{
  return mNavigationObject;
}

void QgsTemporalControllerDockWidget::settings_clicked()
{
  QgsTemporalMapSettingsDialog dialog( this );
  dialog.mapSettingsWidget()->setFrameRateValue( mNavigationObject->framesPerSeconds() );

  if ( dialog.exec() )
  {
    mNavigationObject->setFramesPerSeconds( dialog.mapSettingsWidget()->frameRateValue() );
  }
}

void QgsTemporalControllerDockWidget::timeSlider_valueChanged( int value )
{
  mNavigationObject->setCurrentFrameNumber( value );
}

void QgsTemporalControllerDockWidget::setDatesToProjectTime()
{
  QgsDateTimeRange range;
  if ( QgsProject::instance()->timeSettings() )
    range = QgsProject::instance()->timeSettings()->temporalRange();

  if ( range.begin().isValid() && range.end().isValid() )
  {
    mStartDateTime->setDateTime( range.begin() );
    mEndDateTime->setDateTime( range.end() );
  }
}

void QgsTemporalControllerDockWidget::stopButton_clicked()
{
  updateButtonsEnable( true );
  mNavigationObject->pause();
}

void QgsTemporalControllerDockWidget::forwardButton_clicked()
{
  mNavigationObject->playForward();
}

void QgsTemporalControllerDockWidget::backButton_clicked()
{
  mNavigationObject->playBackward();
}

void QgsTemporalControllerDockWidget::nextButton_clicked()
{
  mNavigationObject->next();
}

void QgsTemporalControllerDockWidget::previousButton_clicked()
{
  mNavigationObject->previous();
}

void QgsTemporalControllerDockWidget::setDateInputsEnable( bool enabled )
{
  mStartDateTime->setEnabled( enabled );
  mEndDateTime->setEnabled( enabled );
}

void QgsTemporalControllerDockWidget::updateButtonsEnable( bool enabled )
{
  mPreviousButton->setEnabled( enabled );
  mNextButton->setEnabled( enabled );
  mBackButton->setEnabled( enabled );
  mForwardButton->setEnabled( enabled );
}
