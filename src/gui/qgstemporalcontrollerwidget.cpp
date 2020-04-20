/***************************************************************************
                         qgstemporalcontrollerwidget.cpp
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

#include "qgstemporalcontrollerwidget.h"
#include "qgsgui.h"
#include "qgsproject.h"
#include "qgsprojecttimesettings.h"
#include "qgstemporalnavigationobject.h"
#include "qgstemporalmapsettingswidget.h"
#include "qgstemporalutils.h"
#include "qgsmaplayertemporalproperties.h"

QgsTemporalControllerWidget::QgsTemporalControllerWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mNavigationObject = new QgsTemporalNavigationObject( this );

  connect( mForwardButton, &QPushButton::clicked, mNavigationObject, &QgsTemporalNavigationObject::playForward );
  connect( mBackButton, &QPushButton::clicked, mNavigationObject, &QgsTemporalNavigationObject::playBackward );
  connect( mNextButton, &QPushButton::clicked, mNavigationObject, &QgsTemporalNavigationObject::next );
  connect( mPreviousButton, &QPushButton::clicked, mNavigationObject, &QgsTemporalNavigationObject::previous );
  connect( mStopButton, &QPushButton::clicked, mNavigationObject, &QgsTemporalNavigationObject::pause );
  connect( mFastForwardButton, &QPushButton::clicked, mNavigationObject, &QgsTemporalNavigationObject::skipToEnd );
  connect( mRewindButton, &QPushButton::clicked, mNavigationObject, &QgsTemporalNavigationObject::rewindToStart );
  connect( mLoopingCheckBox, &QCheckBox::toggled, this, [ = ]( bool state ) { mNavigationObject->setLooping( state ); } );

  connect( mNavigationObject, &QgsTemporalNavigationObject::stateChanged, this, [ = ]( QgsTemporalNavigationObject::AnimationState state )
  {
    mForwardButton->setChecked( state == QgsTemporalNavigationObject::Forward );
    mBackButton->setChecked( state == QgsTemporalNavigationObject::Reverse );
    mStopButton->setChecked( state == QgsTemporalNavigationObject::Idle );
  } );

  connect( mStartDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsTemporalControllerWidget::updateTemporalExtent );
  connect( mEndDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsTemporalControllerWidget::updateTemporalExtent );
  connect( mSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, &QgsTemporalControllerWidget::updateFrameDuration );
  connect( mTimeStepsComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsTemporalControllerWidget::updateFrameDuration );
  connect( mSlider, &QSlider::valueChanged, this, &QgsTemporalControllerWidget::timeSlider_valueChanged );

  mSpinBox->setClearValue( 1 );

  connect( mNavigationObject, &QgsTemporalNavigationObject::updateTemporalRange, this, &QgsTemporalControllerWidget::updateSlider );

  connect( mSettings, &QPushButton::clicked, this, &QgsTemporalControllerWidget::settings_clicked );
  connect( mSetToProjectTimeButton, &QPushButton::clicked, this, &QgsTemporalControllerWidget::setDatesToProjectTime );

  QgsDateTimeRange range;

  if ( QgsProject::instance()->timeSettings() )
    range = QgsProject::instance()->timeSettings()->temporalRange();

  mStartDateTime->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  mEndDateTime->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  if ( range.begin().isValid() && range.end().isValid() )
  {
    mStartDateTime->setDateTime( range.begin() );
    mEndDateTime->setDateTime( range.end() );
  }

  mSetToProjectTimeButton->setToolTip( tr( "Match time range to project. \n"
                                       "If a project has no explicit time range set, \n"
                                       "then the range will be calculated based on the \n"
                                       "minimum and maximum dates from any temporal-enabled layers." ) );

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
  mRewindButton->setToolTip( tr( "Rewind to start" ) );
  mFastForwardButton->setToolTip( tr( "Fast forward to end" ) );

  updateFrameDuration();

  connect( QgsProject::instance(), &QgsProject::readProject, this, &QgsTemporalControllerWidget::setWidgetStateFromProject );
  connect( QgsProject::instance(), &QgsProject::layersAdded, this, &QgsTemporalControllerWidget::onLayersAdded );
  connect( QgsProject::instance(), &QgsProject::cleared, this, &QgsTemporalControllerWidget::onProjectCleared );
}

void QgsTemporalControllerWidget::updateTemporalExtent()
{
  QgsDateTimeRange temporalExtent = QgsDateTimeRange( mStartDateTime->dateTime(),
                                    mEndDateTime->dateTime() );
  mNavigationObject->setTemporalExtents( temporalExtent );
  mSlider->setRange( 0, mNavigationObject->totalFrameCount() - 1 );
  mSlider->setValue( 0 );
}

void QgsTemporalControllerWidget::updateFrameDuration()
{
  if ( mBlockSettingUpdates )
    return;

  // save new settings into project
  QgsProject::instance()->timeSettings()->setTimeStepUnit( static_cast< QgsUnitTypes::TemporalUnit>( mTimeStepsComboBox->currentData().toInt() ) );
  QgsProject::instance()->timeSettings()->setTimeStep( mSpinBox->value() );

  mNavigationObject->setFrameDuration( QgsInterval( QgsProject::instance()->timeSettings()->timeStep(),
                                       QgsProject::instance()->timeSettings()->timeStepUnit() ) );
  mSlider->setRange( 0, mNavigationObject->totalFrameCount() - 1 );
}

void QgsTemporalControllerWidget::setWidgetStateFromProject()
{
  mBlockSettingUpdates++;
  mTimeStepsComboBox->setCurrentIndex( mTimeStepsComboBox->findData( QgsProject::instance()->timeSettings()->timeStepUnit() ) );
  mSpinBox->setValue( QgsProject::instance()->timeSettings()->timeStep() );
  mBlockSettingUpdates--;
  updateFrameDuration();

  mNavigationObject->setFramesPerSecond( QgsProject::instance()->timeSettings()->framesPerSecond() );
}

void QgsTemporalControllerWidget::onLayersAdded()
{
  if ( !mHasTemporalLayersLoaded )
  {
    QVector<QgsMapLayer *> layers = QgsProject::instance()->layers<QgsMapLayer *>();
    for ( QgsMapLayer *layer : layers )
    {
      if ( layer->temporalProperties() )
      {
        mHasTemporalLayersLoaded |= layer->temporalProperties()->isActive();

        if ( !mHasTemporalLayersLoaded )
        {
          connect( layer, &QgsMapLayer::dataSourceChanged, this, [ this, layer ]
          {
            if ( layer->isValid() && layer->temporalProperties()->isActive() && !mHasTemporalLayersLoaded )
            {
              mHasTemporalLayersLoaded = true;
              // if we are moving from zero temporal layers to non-zero temporal layers, let's set temporal extent
              this->setDatesToProjectTime();
            }
          } );
        }
      }
    }

    if ( mHasTemporalLayersLoaded )
      setDatesToProjectTime();
  }
}

void QgsTemporalControllerWidget::onProjectCleared()
{
  mHasTemporalLayersLoaded = false;
  mStartDateTime->setDateTime( QDateTime( QDate::currentDate(), QTime( 0, 0, 0, Qt::UTC ) ) );
  mEndDateTime->setDateTime( mStartDateTime->dateTime() );
  updateTemporalExtent();
}

void QgsTemporalControllerWidget::updateSlider( const QgsDateTimeRange &range )
{
  whileBlocking( mSlider )->setValue( mNavigationObject->currentFrameNumber() );
  updateRangeLabel( range );
}

void QgsTemporalControllerWidget::updateRangeLabel( const QgsDateTimeRange &range )
{
  mCurrentRangeLabel->setText( tr( "%1 to %2" ).arg(
                                 range.begin().toString( "yyyy-MM-dd HH:mm:ss" ),
                                 range.end().toString( "yyyy-MM-dd HH:mm:ss" ) ) );
}

QgsTemporalController *QgsTemporalControllerWidget::temporalController()
{
  return mNavigationObject;
}

void QgsTemporalControllerWidget::settings_clicked()
{
  QgsTemporalMapSettingsWidget *settingsWidget = new QgsTemporalMapSettingsWidget( this );
  settingsWidget->setFrameRateValue( mNavigationObject->framesPerSecond() );

  connect( settingsWidget, &QgsTemporalMapSettingsWidget::frameRateChanged, this, [ = ]( double rate )
  {
    // save new settings into project
    QgsProject::instance()->timeSettings()->setFramesPerSecond( rate );
    mNavigationObject->setFramesPerSecond( rate );
  } );
  openPanel( settingsWidget );
}

void QgsTemporalControllerWidget::timeSlider_valueChanged( int value )
{
  mNavigationObject->setCurrentFrameNumber( value );
}

void QgsTemporalControllerWidget::setDatesToProjectTime()
{
  QgsDateTimeRange range;

  // by default try taking the project's fixed temporal extent
  if ( QgsProject::instance()->timeSettings() )
    range = QgsProject::instance()->timeSettings()->temporalRange();

  // if that's not set, calculate the extent from the project's layers
  if ( !range.begin().isValid() || !range.end().isValid() )
  {
    range = QgsTemporalUtils::calculateTemporalRangeForProject( QgsProject::instance() );
  }

  if ( range.begin().isValid() && range.end().isValid() )
  {
    mStartDateTime->setDateTime( range.begin() );
    mEndDateTime->setDateTime( range.end() );
    updateTemporalExtent();
  }
}

void QgsTemporalControllerWidget::setDateInputsEnable( bool enabled )
{
  mStartDateTime->setEnabled( enabled );
  mEndDateTime->setEnabled( enabled );
}

void QgsTemporalControllerWidget::updateButtonsEnable( bool enabled )
{
  mPreviousButton->setEnabled( enabled );
  mNextButton->setEnabled( enabled );
  mBackButton->setEnabled( enabled );
  mForwardButton->setEnabled( enabled );
}
