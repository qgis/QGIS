/***************************************************************************
                         qgstemporalvcrdockwidget.cpp
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

#include "qgstemporalvcrdockwidget.h"
#include "qgsgui.h"
#include "qgsproject.h"
#include "qgsprojecttimesettings.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include "qgstemporalnavigationobject.h"
#include "qgstemporalmapsettingswidget.h"

#include "qgstemporalmapsettingsdialog.h"

QgsTemporalVcrDockWidget::QgsTemporalVcrDockWidget( const QString &name, QWidget *parent )
  : QgsDockWidget( parent )
{
  setupUi( this );
  setWindowTitle( name );

  mNavigationObject = new QgsTemporalNavigationObject();
  settingsDialog();

  connect( mForwardButton, &QPushButton::clicked, this, &QgsTemporalVcrDockWidget::forwardButton_clicked );
  connect( mBackButton, &QPushButton::clicked, this, &QgsTemporalVcrDockWidget::backButton_clicked );
  connect( mNextButton, &QPushButton::clicked, this, &QgsTemporalVcrDockWidget::nextButton_clicked );
  connect( mPreviousButton, &QPushButton::clicked, this, &QgsTemporalVcrDockWidget::previousButton_clicked );
  connect( mStopButton, &QPushButton::clicked, this, &QgsTemporalVcrDockWidget::stopButton_clicked );
  connect( mSpinBox, qgis::overload<int>::of( &QSpinBox::valueChanged ), this, &QgsTemporalVcrDockWidget::spinBox_valueChanged );
  connect( mTimeStepsComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsTemporalVcrDockWidget::timeStepsComboBox_currentIndexChanged );
  connect( mModeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsTemporalVcrDockWidget::modeComboBox_currentIndexChanged );
  connect( mTimeSlider, &QSlider::valueChanged, this, &QgsTemporalVcrDockWidget::timeSlider_valueChanged );
  connect( mSettings, &QPushButton::clicked, this, &QgsTemporalVcrDockWidget::settings_clicked );

  connect( mStartDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsTemporalVcrDockWidget::startDateTime_changed );
  connect( mEndDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsTemporalVcrDockWidget::endDateTime_changed );

  mTimer = new QTimer( this );

  connect( mTimer, &QTimer::timeout,
           this,  qgis::overload<>::of( &QgsTemporalVcrDockWidget::timerTimeout ) );

  init();
}

QgsTemporalVcrDockWidget::~QgsTemporalVcrDockWidget()
{
  mTimer->stop();
  delete mNavigationObject;
  delete mSettingsDialog;
}

void QgsTemporalVcrDockWidget::init()
{
  QgsDateTimeRange range;

  if ( QgsProject::instance()->timeSettings() )
    range = QgsProject::instance()->timeSettings()->temporalRange();

  if ( range.begin().isValid() && range.end().isValid() )
  {
    mStartDateTime->setDateTime( range.begin() );
    mEndDateTime->setDateTime( range.end() );
  }

  QStringList listSteps = ( QStringList() << tr( "Minutes" ) << tr( "Hours" ) << tr( "Days" ) <<
                            tr( "Months" ) << ( "Years" ) );

  mTimeStepsComboBox->addItems( listSteps );
  mTimeStepsComboBox->setCurrentIndex( 2 );

  QStringList listMode = ( QStringList() << tr( "Nearest Previous Product" ) <<
                           tr( "Snapshot" ) <<
                           tr( "Composite" ) );

  mModeComboBox->addItems( listMode );
  mModeComboBox->setCurrentIndex( 1 );

  mSpinBox->setMinimum( 1 );
  mSpinBox->setSingleStep( 1 );
  mSpinBox->setValue( 1 );
  mSpinBox->setEnabled( true );

  setSliderRange();
}

void QgsTemporalVcrDockWidget::settingsDialog()
{
  mSettingsDialog = new QgsTemporalMapSettingsDialog( this );

}

void QgsTemporalVcrDockWidget::settings_clicked()
{
  mSettingsDialog->setVisible( !mSettingsDialog->isVisible() );
}

void QgsTemporalVcrDockWidget::timerTimeout()
{
  if ( mNavigationObject->navigationStatus() ==
       QgsTemporalNavigationObject::NavigationStatus::Forward )
    forwardButton_clicked();
  if ( mNavigationObject->navigationStatus() ==
       QgsTemporalNavigationObject::NavigationStatus::BackWard )
    backButton_clicked();
}

void QgsTemporalVcrDockWidget::timeSlider_valueChanged( int value )
{
  if ( value < mNavigationObject->dateTimes().size() )
  {
    QDateTime dateTime = QDateTime( mNavigationObject->dateTimes().at( value ) );
    mCurrentDateTime->setText( tr( "Current time: %1 " ).arg(
                                 dateTime.toString() ) );
    int timeStep = mSpinBox->value();
    QString time = mTimeStepsComboBox->currentText();

    mNavigationObject->updateLayersTemporalRange( QDateTime( dateTime ), time, timeStep );
  }
}

void QgsTemporalVcrDockWidget::spinBox_valueChanged( int value )
{
  Q_UNUSED( value );
  setSliderRange();
}

void QgsTemporalVcrDockWidget::timeStepsComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  setSliderRange();
}

void QgsTemporalVcrDockWidget::modeComboBox_currentIndexChanged( int index )
{
  if ( index == 0 )
    mNavigationObject->setMode( QgsTemporalNavigationObject::Mode::NearestPreviousProduct );
  if ( index == 1 )
    mNavigationObject->setMode( QgsTemporalNavigationObject::Mode::Snapshot );
  if ( index == 2 )
    mNavigationObject->setMode( QgsTemporalNavigationObject::Mode::Composite );
}

void QgsTemporalVcrDockWidget::setSliderRange()
{
  mTimeSlider->setMinimum( 0 );
  int max = 0;

  QDateTime start = mStartDateTime->dateTime();
  QDateTime end = mEndDateTime->dateTime();

  int timeStep = mSpinBox->value();
  QString time = mTimeStepsComboBox->currentText();

  QList<QDateTime> dateTimes;

  // Update list of dates to navigate while setting the slider range
  mNavigationObject->setDateTimes( {} );

  if ( start <= end )
    dateTimes.append( start );

  while ( start < end )
  {
    start = mNavigationObject->addToDateTime( start, time, timeStep );
    max++;

    if ( start > end )
      start = end;
    dateTimes.append( start );
  }
  mNavigationObject->setDateTimes( dateTimes );
  mTimeSlider->setMaximum( max );
}

void QgsTemporalVcrDockWidget::startDateTime_changed( const QDateTime &datetime )
{
  Q_UNUSED( datetime )
  setSliderRange();
}

void QgsTemporalVcrDockWidget::endDateTime_changed( const QDateTime &datetime )
{
  Q_UNUSED( datetime )
  setSliderRange();
}

void QgsTemporalVcrDockWidget::updateDatesLabels( bool useProjectTime )
{
  if ( useProjectTime )
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
}

void QgsTemporalVcrDockWidget::stopButton_clicked()
{
  if ( mNavigationObject->isPlaying() )
  {
    updateButtonsEnable( false );
    mNavigationObject->setIsPlaying( false );
  }
  updateButtonsEnable( true );
  mNavigationObject->setNavigationStatus( QgsTemporalNavigationObject::NavigationStatus::Idle );
}

void QgsTemporalVcrDockWidget::forwardButton_clicked()
{
  if ( QgsProject::instance()->timeSettings() )
  {
    updateButtonsEnable( false );
    mNavigationObject->setNavigationStatus( QgsTemporalNavigationObject::NavigationStatus::Forward );
    setDateInputsEnable( false );

    QDateTime begin = QDateTime( mStartDateTime->dateTime() );
    QDateTime end = QDateTime( mEndDateTime->dateTime() );

    mNavigationObject->setIsPlaying( true );

    if ( begin.isValid() && end.isValid() )
    {
      if ( ( mTimeSlider->value() + 1 ) < mNavigationObject->dateTimes().size() )
      {
        mTimeSlider->setValue( mTimeSlider->value() + 1 );
        mTimer->start( mSettingsDialog->frameRateValue() );
      }
      updateButtonsEnable( true );
      setDateInputsEnable( true );
    }
  }
}

void QgsTemporalVcrDockWidget::backButton_clicked()
{
  if ( QgsProject::instance()->timeSettings() )
  {
    QDateTime begin = QDateTime( mStartDateTime->dateTime() );
    QDateTime end = QDateTime( mEndDateTime->dateTime() );

    updateButtonsEnable( false );

    mNavigationObject->setIsPlaying( true );
    mNavigationObject->setNavigationStatus( QgsTemporalNavigationObject::NavigationStatus::BackWard );

    if ( begin.isValid() && end.isValid() )
    {
      if ( ( mTimeSlider->value() - 1 ) >= 0 )
      {
        mTimeSlider->setValue( mTimeSlider->value() - 1 );
        mTimer->start( 1000 );
      }
      updateButtonsEnable( true );
    }
  }
}

void QgsTemporalVcrDockWidget::nextButton_clicked()
{
  if ( QgsProject::instance()->timeSettings() )
  {
    QDateTime begin = QDateTime( mStartDateTime->dateTime() );
    QDateTime end = QDateTime( mEndDateTime->dateTime() );
    updateButtonsEnable( false );

    mNavigationObject->setIsPlaying( true );
    mNavigationObject->setNavigationStatus( QgsTemporalNavigationObject::NavigationStatus::Next );

    if ( begin.isValid() && end.isValid() )
    {
      if ( ( mTimeSlider->value() + 1 ) < mNavigationObject->dateTimes().size() )
      {
        mTimeSlider->setValue( mTimeSlider->value() + 1 );
        mTimer->start( 1000 );
      }
      updateButtonsEnable( true );
    }
    mNavigationObject->setNavigationStatus( QgsTemporalNavigationObject::NavigationStatus::Idle );
  }
}

void QgsTemporalVcrDockWidget::previousButton_clicked()
{
  if ( QgsProject::instance()->timeSettings() )
  {
    QDateTime begin = QDateTime( mStartDateTime->dateTime() );
    QDateTime end = QDateTime( mEndDateTime->dateTime() );

    updateButtonsEnable( false );

    mNavigationObject->setIsPlaying( true );
    mNavigationObject->setNavigationStatus( QgsTemporalNavigationObject::NavigationStatus::BackWard );

    if ( begin.isValid() && end.isValid() )
    {
      if ( ( mTimeSlider->value() - 1 ) >= 0 )
      {
        mTimeSlider->setValue( mTimeSlider->value() - 1 );
        mTimer->start( 1000 );
      }
      updateButtonsEnable( true );
    }
    mNavigationObject->setNavigationStatus( QgsTemporalNavigationObject::NavigationStatus::Idle );
  }
}

void QgsTemporalVcrDockWidget::setDateInputsEnable( bool enabled )
{
  mStartDateTime->setEnabled( enabled );
  mEndDateTime->setEnabled( enabled );
}

void QgsTemporalVcrDockWidget::updateButtonsEnable( bool enabled )
{
  mPreviousButton->setEnabled( enabled );
  mNextButton->setEnabled( enabled );
  mBackButton->setEnabled( enabled );
  mForwardButton->setEnabled( enabled );
}
