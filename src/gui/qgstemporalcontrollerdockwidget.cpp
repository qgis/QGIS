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

  init();
}

void QgsTemporalControllerDockWidget::init()
{
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

  mTimeStepsComboBox->addItem( tr( "Seconds" ), QgsTemporalControllerDockWidget::Seconds );
  mTimeStepsComboBox->addItem( tr( "Minutes" ), QgsTemporalControllerDockWidget::Minutes );
  mTimeStepsComboBox->addItem( tr( "Hours" ), QgsTemporalControllerDockWidget::Hours );
  mTimeStepsComboBox->addItem( tr( "Days" ), QgsTemporalControllerDockWidget::Days );
  mTimeStepsComboBox->addItem( tr( "Months" ), QgsTemporalControllerDockWidget::Months );
  mTimeStepsComboBox->addItem( tr( "Years" ), QgsTemporalControllerDockWidget::Years );

  mTimeStepsComboBox->setCurrentIndex( mTimeStepsComboBox->findData(
                                         QgsTemporalControllerDockWidget::Hours ) );

  mSpinBox->setMinimum( 0.0000001 );
  mSpinBox->setSingleStep( 1 );
  mSpinBox->setValue( 1 );
  mSpinBox->setEnabled( true );

  mForwardButton->setToolTip( tr( "Play" ) );
  mBackButton->setToolTip( tr( "Reverse" ) );
  mNextButton->setToolTip( tr( "Go to next frame" ) );
  mPreviousButton->setToolTip( tr( "Go to previous frame" ) );
  mStopButton->setToolTip( tr( "Pause" ) );

  updateTemporalExtent();
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
  mNavigationObject->setFrameDuration( interval( mTimeStepsComboBox->currentData().toInt(),
                                       mSpinBox->value() ) );
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
  mCurrentRangeLabel->setText( tr( "Current temporal range is from %1 to %2" ).arg(
                                 range.begin().toString( locale.dateTimeFormat() ),
                                 range.end().toString( locale.dateTimeFormat() ) ) );
}

QgsTemporalController *QgsTemporalControllerDockWidget::temporalController()
{
  return mNavigationObject;
}

void QgsTemporalControllerDockWidget::settings_clicked()
{

  QgsTemporalMapSettingsDialog *dialog =  new QgsTemporalMapSettingsDialog( this );
  dialog->setAttribute( Qt::WA_DeleteOnClose );

  if ( dialog->mapSettingsWidget() )
    dialog->mapSettingsWidget()->setFrameRateValue(
      mNavigationObject->framesPerSeconds() );

  dialog->setVisible( true );

  connect( dialog->mapSettingsWidget(), &QgsTemporalMapSettingsWidget::frameRateChanged, this, [ this, dialog ]()
  {
    mNavigationObject->setFramesPerSeconds( dialog->mapSettingsWidget()->frameRateValue() );
  } );
}

void QgsTemporalControllerDockWidget::timeSlider_valueChanged( int value )
{
  mNavigationObject->setCurrentFrameNumber( value );
}

QgsInterval QgsTemporalControllerDockWidget::interval( int time, double value )
{
  QgsInterval interval;

  if ( time == QgsTemporalControllerDockWidget::Seconds )
  {
    interval.setSeconds( value );
  }
  if ( time == QgsTemporalControllerDockWidget::Minutes )
  {
    interval.setMinutes( value );
  }
  if ( time == QgsTemporalControllerDockWidget::Hours )
  {
    interval.setHours( value );
  }
  if ( time == QgsTemporalControllerDockWidget::Days )
  {
    interval.setDays( value );
  }
  if ( time == QgsTemporalControllerDockWidget::Months )
  {
    interval.setMonths( value );
  }
  if ( time == QgsTemporalControllerDockWidget::Years )
  {
    interval.setYears( value );
  }

  return interval;
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
