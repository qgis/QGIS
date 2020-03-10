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
  settingsDialog();

  connect( mForwardButton, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::forwardButton_clicked );
  connect( mBackButton, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::backButton_clicked );
  connect( mNextButton, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::nextButton_clicked );
  connect( mPreviousButton, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::previousButton_clicked );
  connect( mStopButton, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::stopButton_clicked );

  connect( mStartDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsTemporalControllerDockWidget::updateTemporalExtent );
  connect( mEndDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsTemporalControllerDockWidget::updateTemporalExtent );
  connect( mSpinBox, qgis::overload<int>::of( &QSpinBox::valueChanged ), this, &QgsTemporalControllerDockWidget::updateFrameDuration );
  connect( mTimeStepsComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsTemporalControllerDockWidget::updateFrameDuration );
  connect( mSlider, &QSlider::valueChanged, this, &QgsTemporalControllerDockWidget::timeSlider_valueChanged );

  connect( mNavigationObject, &QgsTemporalNavigationObject::updateTemporalRange, this, &QgsTemporalControllerDockWidget::updateSlider );

  connect( mSettingsDialog->mapSettingsWidget(), &QgsTemporalMapSettingsWidget::frameRateChanged, this, &QgsTemporalControllerDockWidget::updateFrameRate );

  connect( mSettings, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::settings_clicked );
  connect( mSetToProjectTimeButton, &QPushButton::clicked, this, &QgsTemporalControllerDockWidget::setDatesToProjectTime );

  init();
}

QgsTemporalControllerDockWidget::~QgsTemporalControllerDockWidget()
{
}

void QgsTemporalControllerDockWidget::init()
{
  QgsDateTimeRange range;

  if ( QgsProject::instance()->timeSettings() )
    range = QgsProject::instance()->timeSettings()->temporalRange();
  QLocale locale;

  mStartDateTime->setDisplayFormat( locale.dateTimeFormat() );
  mEndDateTime->setDisplayFormat( locale.dateTimeFormat() );

  if ( range.begin().isValid() && range.end().isValid() )
  {
    mStartDateTime->setDateTime( range.begin() );
    mEndDateTime->setDateTime( range.end() );
  }

  mSetToProjectTimeButton->setToolTip( tr( "Set to project time" ) );

  QStringList listSteps = ( QStringList() << tr( "Seconds" ) << tr( "Minutes" ) << tr( "Hours" )
                            << tr( "Days" ) <<
                            tr( "Months" ) << ( "Years" ) );

  mTimeStepsComboBox->addItems( listSteps );
  mTimeStepsComboBox->setCurrentIndex( 2 );

  mSpinBox->setMinimum( 1 );
  mSpinBox->setSingleStep( 1 );
  mSpinBox->setValue( 1 );
  mSpinBox->setEnabled( true );

  updateFrameRate();
  updateTemporalExtent();
  updateFrameDuration();
}

void QgsTemporalControllerDockWidget::updateFrameRate()
{
  if ( mSettingsDialog && mSettingsDialog->mapSettingsWidget() )
    mNavigationObject->setFramesPerSeconds(
      mSettingsDialog->mapSettingsWidget()->frameRateValue() );
}

void QgsTemporalControllerDockWidget::updateTemporalExtent()
{
  QgsDateTimeRange temporaExtent = QgsDateTimeRange( mStartDateTime->dateTime(),
                                   mEndDateTime->dateTime() );
  mNavigationObject->setTemporalExtents( temporaExtent );
  mSlider->setRange( 0, mNavigationObject->totalFrameCount() - 1 );
}

void QgsTemporalControllerDockWidget::updateFrameDuration()
{
  mNavigationObject->setFrameDuration( interval( mTimeStepsComboBox->currentText(),
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

QgsTemporalNavigationObject *QgsTemporalControllerDockWidget::temporalController()
{
  return mNavigationObject;
}

void QgsTemporalControllerDockWidget::settingsDialog()
{
  mSettingsDialog = new QgsTemporalMapSettingsDialog( this );
}

void QgsTemporalControllerDockWidget::settings_clicked()
{
  mSettingsDialog->setVisible( !mSettingsDialog->isVisible() );
}

void QgsTemporalControllerDockWidget::timeSlider_valueChanged( int value )
{
  mNavigationObject->setCurrentFrameNumber( value );
}

QgsInterval QgsTemporalControllerDockWidget::interval( QString time, int value )
{
  QgsInterval interval;

  if ( time == QString( "Seconds" ) )
  {
    interval.setSeconds( value );
  }
  if ( time == QString( "Minutes" ) )
  {
    interval.setMinutes( value );
  }
  if ( time == QString( "Hours" ) )
  {
    interval.setHours( value );
  }
  if ( time == QString( "Days" ) )
  {
    interval.setDays( value );
  }
  if ( time == QString( "Months" ) )
  {
    interval.setMonths( value );
  }
  if ( time == QString( "Years" ) )
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
  mNavigationObject->forward();
}

void QgsTemporalControllerDockWidget::backButton_clicked()
{
  mNavigationObject->backward();
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
