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

#include "qgsapplication.h"
#include "qgstemporalcontrollerwidget.h"
#include "moc_qgstemporalcontrollerwidget.cpp"
#include "qgsmaplayermodel.h"
#include "qgsproject.h"
#include "qgsprojecttimesettings.h"
#include "qgstemporalmapsettingswidget.h"
#include "qgstemporalutils.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsmeshlayer.h"
#include "qgsrasterlayer.h"
#include "qgsunittypes.h"

#include <QAction>
#include <QMenu>
#include <QRegularExpression>

QgsTemporalControllerWidget::QgsTemporalControllerWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  mNavigationObject = new QgsTemporalNavigationObject( this );

  mStartDateTime->setDateTimeRange( QDateTime( QDate( 1, 1, 1 ), QTime( 0, 0, 0 ) ), mStartDateTime->maximumDateTime() );
  mEndDateTime->setDateTimeRange( QDateTime( QDate( 1, 1, 1 ), QTime( 0, 0, 0 ) ), mStartDateTime->maximumDateTime() );
  mFixedRangeStartDateTime->setDateTimeRange( QDateTime( QDate( 1, 1, 1 ), QTime( 0, 0, 0 ) ), mStartDateTime->maximumDateTime() );
  mFixedRangeEndDateTime->setDateTimeRange( QDateTime( QDate( 1, 1, 1 ), QTime( 0, 0, 0 ) ), mStartDateTime->maximumDateTime() );

  auto handleOperation = [=]( Qgis::PlaybackOperation operation ) {
    switch ( operation )
    {
      case Qgis::PlaybackOperation::SkipToStart:
        mNavigationObject->rewindToStart();
        break;

      case Qgis::PlaybackOperation::PreviousFrame:
        mNavigationObject->previous();
        break;

      case Qgis::PlaybackOperation::PlayReverse:
        mNavigationObject->playBackward();
        break;

      case Qgis::PlaybackOperation::Pause:
        mNavigationObject->pause();
        break;

      case Qgis::PlaybackOperation::PlayForward:
        mNavigationObject->playForward();
        break;

      case Qgis::PlaybackOperation::NextFrame:
        mNavigationObject->next();
        break;

      case Qgis::PlaybackOperation::SkipToEnd:
        mNavigationObject->skipToEnd();
        break;
    }
  };
  connect( mAnimationController, &QgsPlaybackControllerWidget::operationTriggered, this, handleOperation );
  connect( mMovieController, &QgsPlaybackControllerWidget::operationTriggered, this, handleOperation );

  connect( mAnimationLoopingCheckBox, &QCheckBox::toggled, this, [=]( bool state ) { mNavigationObject->setLooping( state ); mMovieLoopingCheckBox->setChecked( state ); } );
  connect( mMovieLoopingCheckBox, &QCheckBox::toggled, this, [=]( bool state ) { mNavigationObject->setLooping( state );  mAnimationLoopingCheckBox->setChecked( state ); } );

  setWidgetStateFromNavigationMode( mNavigationObject->navigationMode() );
  connect( mNavigationObject, &QgsTemporalNavigationObject::navigationModeChanged, this, &QgsTemporalControllerWidget::setWidgetStateFromNavigationMode );
  connect( mNavigationObject, &QgsTemporalNavigationObject::temporalExtentsChanged, this, &QgsTemporalControllerWidget::setDates );
  connect( mNavigationObject, &QgsTemporalNavigationObject::temporalFrameDurationChanged, this, [=]( const QgsInterval &timeStep ) {
    if ( mBlockFrameDurationUpdates )
      return;

    mBlockFrameDurationUpdates++;
    updateTimeStepInputs( timeStep );
    mBlockFrameDurationUpdates--;
  } );
  connect( mNavigationOff, &QPushButton::clicked, this, &QgsTemporalControllerWidget::mNavigationOff_clicked );
  connect( mNavigationFixedRange, &QPushButton::clicked, this, &QgsTemporalControllerWidget::mNavigationFixedRange_clicked );
  connect( mNavigationAnimated, &QPushButton::clicked, this, &QgsTemporalControllerWidget::mNavigationAnimated_clicked );
  connect( mNavigationMovie, &QPushButton::clicked, this, &QgsTemporalControllerWidget::mNavigationMovie_clicked );

  connect( mNavigationObject, &QgsTemporalNavigationObject::stateChanged, this, [=]( Qgis::AnimationState state ) {
    mAnimationController->setState( state );
    mMovieController->setState( state );
  } );

  connect( mStartDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsTemporalControllerWidget::startEndDateTime_changed );
  connect( mEndDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsTemporalControllerWidget::startEndDateTime_changed );
  connect( mFixedRangeStartDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsTemporalControllerWidget::fixedRangeStartEndDateTime_changed );
  connect( mFixedRangeEndDateTime, &QDateTimeEdit::dateTimeChanged, this, &QgsTemporalControllerWidget::fixedRangeStartEndDateTime_changed );
  connect( mStepSpinBox, qOverload<double>( &QDoubleSpinBox::valueChanged ), this, &QgsTemporalControllerWidget::updateFrameDuration );
  connect( mTimeStepsComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsTemporalControllerWidget::updateFrameDuration );
  connect( mAnimationSlider, &QSlider::valueChanged, this, &QgsTemporalControllerWidget::timeSlider_valueChanged );
  connect( mMovieSlider, &QSlider::valueChanged, this, &QgsTemporalControllerWidget::timeSlider_valueChanged );

  connect( mTotalFramesSpinBox, qOverload<int>( &QSpinBox::valueChanged ), this, [=]( int frames ) {
    mNavigationObject->setTotalMovieFrames( frames );
  } );

  mStepSpinBox->setClearValue( 1 );

  connect( mNavigationObject, &QgsTemporalNavigationObject::updateTemporalRange, this, &QgsTemporalControllerWidget::updateSlider );
  connect( mNavigationObject, &QgsTemporalNavigationObject::totalMovieFramesChanged, this, &QgsTemporalControllerWidget::totalMovieFramesChanged );

  connect( mSettings, &QPushButton::clicked, this, &QgsTemporalControllerWidget::settings_clicked );

  mMapLayerModel = new QgsMapLayerModel( this );

  mRangeMenu.reset( new QMenu( this ) );

  mRangeSetToAllLayersAction = new QAction( tr( "Set to Full Range" ), mRangeMenu.get() );
  mRangeSetToAllLayersAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRefresh.svg" ) ) );
  connect( mRangeSetToAllLayersAction, &QAction::triggered, this, &QgsTemporalControllerWidget::mRangeSetToAllLayersAction_triggered );
  mRangeMenu->addAction( mRangeSetToAllLayersAction );

  mRangeSetToProjectAction = new QAction( tr( "Set to Preset Project Range" ), mRangeMenu.get() );
  connect( mRangeSetToProjectAction, &QAction::triggered, this, &QgsTemporalControllerWidget::mRangeSetToProjectAction_triggered );
  mRangeMenu->addAction( mRangeSetToProjectAction );

  mRangeMenu->addSeparator();

  mRangeLayersSubMenu.reset( new QMenu( tr( "Set to Single Layer's Range" ), mRangeMenu.get() ) );
  mRangeLayersSubMenu->setEnabled( false );
  mRangeMenu->addMenu( mRangeLayersSubMenu.get() );
  connect( mRangeMenu.get(), &QMenu::aboutToShow, this, &QgsTemporalControllerWidget::aboutToShowRangeMenu );

  mSetRangeButton->setPopupMode( QToolButton::MenuButtonPopup );
  mSetRangeButton->setMenu( mRangeMenu.get() );
  mSetRangeButton->setDefaultAction( mRangeSetToAllLayersAction );
  mFixedRangeSetRangeButton->setPopupMode( QToolButton::MenuButtonPopup );
  mFixedRangeSetRangeButton->setMenu( mRangeMenu.get() );
  mFixedRangeSetRangeButton->setDefaultAction( mRangeSetToAllLayersAction );

  connect( mExportAnimationButton, &QPushButton::clicked, this, &QgsTemporalControllerWidget::exportAnimation );
  connect( mExportMovieButton, &QPushButton::clicked, this, &QgsTemporalControllerWidget::exportAnimation );

  QgsDateTimeRange range;

  if ( QgsProject::instance()->timeSettings() )
    range = QgsProject::instance()->timeSettings()->temporalRange();

  if ( range.begin().isValid() && range.end().isValid() )
  {
    whileBlocking( mStartDateTime )->setDateTime( range.begin() );
    whileBlocking( mEndDateTime )->setDateTime( range.end() );
    whileBlocking( mFixedRangeStartDateTime )->setDateTime( range.begin() );
    whileBlocking( mFixedRangeEndDateTime )->setDateTime( range.end() );
  }

  for ( const Qgis::TemporalUnit u :
        {
          Qgis::TemporalUnit::Milliseconds,
          Qgis::TemporalUnit::Seconds,
          Qgis::TemporalUnit::Minutes,
          Qgis::TemporalUnit::Hours,
          Qgis::TemporalUnit::Days,
          Qgis::TemporalUnit::Weeks,
          Qgis::TemporalUnit::Months,
          Qgis::TemporalUnit::Years,
          Qgis::TemporalUnit::Decades,
          Qgis::TemporalUnit::Centuries,
          Qgis::TemporalUnit::IrregularStep,
        } )
  {
    mTimeStepsComboBox->addItem( u != Qgis::TemporalUnit::IrregularStep ? QgsUnitTypes::toString( u ) : tr( "source timestamps" ), static_cast<int>( u ) );
  }

  // TODO: might want to choose an appropriate default unit based on the range
  mTimeStepsComboBox->setCurrentIndex( mTimeStepsComboBox->findData( static_cast<int>( Qgis::TemporalUnit::Hours ) ) );

  // NOTE 'minimum' and 'decimals' should be in sync with the 'decimals' in qgstemporalcontrollerwidgetbase.ui
  mStepSpinBox->setDecimals( 3 );
  // minimum timestep one millisecond
  mStepSpinBox->setMinimum( 0.001 );
  mStepSpinBox->setMaximum( std::numeric_limits<int>::max() );
  mStepSpinBox->setSingleStep( 1 );
  mStepSpinBox->setValue( 1 );

  updateFrameDuration();

  connect( QgsProject::instance(), &QgsProject::readProject, this, &QgsTemporalControllerWidget::setWidgetStateFromProject );
  connect( QgsProject::instance(), &QgsProject::layersAdded, this, &QgsTemporalControllerWidget::onLayersAdded );
  connect( QgsProject::instance(), &QgsProject::cleared, this, &QgsTemporalControllerWidget::onProjectCleared );
}

bool QgsTemporalControllerWidget::applySizeConstraintsToStack() const
{
  return true;
}

void QgsTemporalControllerWidget::keyPressEvent( QKeyEvent *e )
{
  if ( ( mAnimationSlider->hasFocus() || mMovieSlider->hasFocus() ) && e->key() == Qt::Key_Space )
  {
    mAnimationController->togglePause();
    // connections will auto-sync mMovieController state!
  }
  QgsPanelWidget::keyPressEvent( e );
}

void QgsTemporalControllerWidget::aboutToShowRangeMenu()
{
  QgsDateTimeRange projectRange;
  if ( QgsProject::instance()->timeSettings() )
    projectRange = QgsProject::instance()->timeSettings()->temporalRange();
  mRangeSetToProjectAction->setEnabled( projectRange.begin().isValid() && projectRange.end().isValid() );

  mRangeLayersSubMenu->clear();
  for ( int i = 0; i < mMapLayerModel->rowCount(); ++i )
  {
    const QModelIndex index = mMapLayerModel->index( i, 0 );
    QgsMapLayer *currentLayer = mMapLayerModel->data( index, static_cast<int>( QgsMapLayerModel::CustomRole::Layer ) ).value<QgsMapLayer *>();
    if ( !currentLayer->temporalProperties() || !currentLayer->temporalProperties()->isActive() )
      continue;

    const QIcon icon = qvariant_cast<QIcon>( mMapLayerModel->data( index, Qt::DecorationRole ) );
    const QString text = mMapLayerModel->data( index, Qt::DisplayRole ).toString();
    const QgsDateTimeRange range = currentLayer->temporalProperties()->calculateTemporalExtent( currentLayer );
    if ( range.begin().isValid() && range.end().isValid() )
    {
      QAction *action = new QAction( icon, text, mRangeLayersSubMenu.get() );
      connect( action, &QAction::triggered, this, [=] {
        setDates( range );
        saveRangeToProject();
      } );
      mRangeLayersSubMenu->addAction( action );
    }
  }
  mRangeLayersSubMenu->setEnabled( !mRangeLayersSubMenu->actions().isEmpty() );
}

void QgsTemporalControllerWidget::updateTemporalExtent()
{
  // TODO - consider whether the overall time range set for animations should include the end date time or not.
  // (currently it DOES include the end date time).
  const QDateTime start = mStartDateTime->dateTime();
  const QDateTime end = mEndDateTime->dateTime();
  const bool isTimeInstant = start == end;
  const QgsDateTimeRange temporalExtent = QgsDateTimeRange( start, end, true, !isTimeInstant && mNavigationObject->navigationMode() == Qgis::TemporalNavigationMode::FixedRange ? false : true );
  mNavigationObject->setTemporalExtents( temporalExtent );
  mAnimationSlider->setRange( 0, mNavigationObject->totalFrameCount() - 1 );
  mAnimationSlider->setValue( mNavigationObject->currentFrameNumber() );
  mMovieSlider->setRange( 0, mNavigationObject->totalFrameCount() - 1 );
  mMovieSlider->setValue( mNavigationObject->currentFrameNumber() );
}

void QgsTemporalControllerWidget::updateFrameDuration()
{
  if ( mBlockSettingUpdates )
    return;

  // save new settings into project
  const Qgis::TemporalUnit unit = static_cast<Qgis::TemporalUnit>( mTimeStepsComboBox->currentData().toInt() );
  QgsProject::instance()->timeSettings()->setTimeStepUnit( unit );
  QgsProject::instance()->timeSettings()->setTimeStep( unit == Qgis::TemporalUnit::IrregularStep ? 1 : mStepSpinBox->value() );

  if ( !mBlockFrameDurationUpdates )
  {
    mNavigationObject->setFrameDuration(
      QgsInterval( QgsProject::instance()->timeSettings()->timeStep(), QgsProject::instance()->timeSettings()->timeStepUnit() )
    );
    mAnimationSlider->setValue( mNavigationObject->currentFrameNumber() );
  }
  mAnimationSlider->setRange( 0, mNavigationObject->totalFrameCount() - 1 );
  mAnimationSlider->setValue( mNavigationObject->currentFrameNumber() );
  mMovieSlider->setRange( 0, mNavigationObject->totalFrameCount() - 1 );
  mMovieSlider->setValue( mNavigationObject->currentFrameNumber() );

  if ( unit == Qgis::TemporalUnit::IrregularStep )
  {
    mStepSpinBox->setEnabled( false );
    mStepSpinBox->setValue( 1 );
    mAnimationSlider->setTickInterval( 1 );
    mAnimationSlider->setTickPosition( QSlider::TicksBothSides );
  }
  else
  {
    mStepSpinBox->setEnabled( true );
    mAnimationSlider->setTickInterval( 0 );
    mAnimationSlider->setTickPosition( QSlider::NoTicks );
  }
}

void QgsTemporalControllerWidget::setWidgetStateFromProject()
{
  mBlockSettingUpdates++;
  mTimeStepsComboBox->setCurrentIndex( mTimeStepsComboBox->findData( static_cast<int>( QgsProject::instance()->timeSettings()->timeStepUnit() ) ) );
  mStepSpinBox->setValue( QgsProject::instance()->timeSettings()->timeStep() );
  mBlockSettingUpdates--;

  bool ok = false;
  const Qgis::TemporalNavigationMode mode = static_cast<Qgis::TemporalNavigationMode>( QgsProject::instance()->readNumEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/NavigationMode" ), 0, &ok ) );
  if ( ok )
  {
    mNavigationObject->setNavigationMode( mode );
    setWidgetStateFromNavigationMode( mode );
  }
  else
  {
    mNavigationObject->setNavigationMode( Qgis::TemporalNavigationMode::Disabled );
    setWidgetStateFromNavigationMode( Qgis::TemporalNavigationMode::Disabled );
  }

  mNavigationObject->setTotalMovieFrames( QgsProject::instance()->timeSettings()->totalMovieFrames() );
  mTotalFramesSpinBox->setValue( QgsProject::instance()->timeSettings()->totalMovieFrames() );

  const QString startString = QgsProject::instance()->readEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/StartDateTime" ) );
  const QString endString = QgsProject::instance()->readEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/EndDateTime" ) );
  if ( !startString.isEmpty() && !endString.isEmpty() )
  {
    whileBlocking( mStartDateTime )->setDateTime( QDateTime::fromString( startString, Qt::ISODateWithMs ) );
    whileBlocking( mEndDateTime )->setDateTime( QDateTime::fromString( endString, Qt::ISODateWithMs ) );
    whileBlocking( mFixedRangeStartDateTime )->setDateTime( QDateTime::fromString( startString, Qt::ISODateWithMs ) );
    whileBlocking( mFixedRangeEndDateTime )->setDateTime( QDateTime::fromString( endString, Qt::ISODateWithMs ) );
  }
  else
  {
    setDatesToProjectTime( false );
  }
  updateTemporalExtent();
  updateFrameDuration();

  mNavigationObject->setFramesPerSecond( QgsProject::instance()->timeSettings()->framesPerSecond() );
  mNavigationObject->setTemporalRangeCumulative( QgsProject::instance()->timeSettings()->isTemporalRangeCumulative() );
}

void QgsTemporalControllerWidget::mNavigationOff_clicked()
{
  QgsProject::instance()->writeEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/NavigationMode" ), static_cast<int>( Qgis::TemporalNavigationMode::Disabled ) );

  mNavigationObject->setNavigationMode( Qgis::TemporalNavigationMode::Disabled );
  setWidgetStateFromNavigationMode( Qgis::TemporalNavigationMode::Disabled );
}

void QgsTemporalControllerWidget::mNavigationFixedRange_clicked()
{
  QgsProject::instance()->writeEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/NavigationMode" ), static_cast<int>( Qgis::TemporalNavigationMode::FixedRange ) );

  mNavigationObject->setNavigationMode( Qgis::TemporalNavigationMode::FixedRange );
  setWidgetStateFromNavigationMode( Qgis::TemporalNavigationMode::FixedRange );
}

void QgsTemporalControllerWidget::mNavigationAnimated_clicked()
{
  QgsProject::instance()->writeEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/NavigationMode" ), static_cast<int>( Qgis::TemporalNavigationMode::Animated ) );

  mNavigationObject->setNavigationMode( Qgis::TemporalNavigationMode::Animated );
  setWidgetStateFromNavigationMode( Qgis::TemporalNavigationMode::Animated );
}

void QgsTemporalControllerWidget::mNavigationMovie_clicked()
{
  QgsProject::instance()->writeEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/NavigationMode" ), static_cast<int>( Qgis::TemporalNavigationMode::Movie ) );

  mNavigationObject->setNavigationMode( Qgis::TemporalNavigationMode::Movie );
  setWidgetStateFromNavigationMode( Qgis::TemporalNavigationMode::Movie );
}

void QgsTemporalControllerWidget::setWidgetStateFromNavigationMode( const Qgis::TemporalNavigationMode mode )
{
  mNavigationOff->setChecked( mode == Qgis::TemporalNavigationMode::Disabled );
  mNavigationFixedRange->setChecked( mode == Qgis::TemporalNavigationMode::FixedRange );
  mNavigationAnimated->setChecked( mode == Qgis::TemporalNavigationMode::Animated );
  mNavigationMovie->setChecked( mode == Qgis::TemporalNavigationMode::Movie );

  switch ( mode )
  {
    case Qgis::TemporalNavigationMode::Disabled:
      mNavigationModeStackedWidget->setCurrentIndex( 0 );
      break;
    case Qgis::TemporalNavigationMode::FixedRange:
      mNavigationModeStackedWidget->setCurrentIndex( 1 );
      break;
    case Qgis::TemporalNavigationMode::Animated:
      mNavigationModeStackedWidget->setCurrentIndex( 2 );
      break;
    case Qgis::TemporalNavigationMode::Movie:
      mNavigationModeStackedWidget->setCurrentIndex( 3 );
      break;
  }
}

void QgsTemporalControllerWidget::onLayersAdded( const QList<QgsMapLayer *> &layers )
{
  if ( !mHasTemporalLayersLoaded )
  {
    for ( QgsMapLayer *layer : layers )
    {
      if ( layer->temporalProperties() )
      {
        mHasTemporalLayersLoaded |= layer->temporalProperties()->isActive();

        if ( !mHasTemporalLayersLoaded )
        {
          connect( layer, &QgsMapLayer::dataSourceChanged, this, [this, layer] {
            if ( layer->isValid() && layer->temporalProperties()->isActive() && !mHasTemporalLayersLoaded )
            {
              mHasTemporalLayersLoaded = true;
              firstTemporalLayerLoaded( layer );
              mNavigationObject->setAvailableTemporalRanges( QgsTemporalUtils::usedTemporalRangesForProject( QgsProject::instance() ) );
            }
          } );
        }

        firstTemporalLayerLoaded( layer );
      }
    }
  }

  mNavigationObject->setAvailableTemporalRanges( QgsTemporalUtils::usedTemporalRangesForProject( QgsProject::instance() ) );
}

void QgsTemporalControllerWidget::firstTemporalLayerLoaded( QgsMapLayer *layer )
{
  setDatesToProjectTime( true );

  if ( QgsMeshLayer *meshLayer = qobject_cast<QgsMeshLayer *>( layer ) )
  {
    mBlockFrameDurationUpdates++;
    setTimeStep( meshLayer->firstValidTimeStep() );
    mBlockFrameDurationUpdates--;
    updateFrameDuration();
  }
  else if ( QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( layer ) )
  {
    if ( rasterLayer->dataProvider() && rasterLayer->dataProvider()->temporalCapabilities() )
    {
      mBlockFrameDurationUpdates++;
      setTimeStep( rasterLayer->dataProvider()->temporalCapabilities()->defaultInterval() );
      mBlockFrameDurationUpdates--;
      updateFrameDuration();
    }
  }
}

void QgsTemporalControllerWidget::onProjectCleared()
{
  mHasTemporalLayersLoaded = false;

  mNavigationObject->setNavigationMode( Qgis::TemporalNavigationMode::Disabled );
  setWidgetStateFromNavigationMode( Qgis::TemporalNavigationMode::Disabled );

  // default to showing the last 24 hours, ending at the current date's hour, in one hour blocks...
  // it's COMPLETELY arbitrary, but better than starting with a "zero length" duration!
  const QTime startOfCurrentHour = QTime( QTime::currentTime().hour(), 0, 0 );
  const QDateTime end = QDateTime( QDate::currentDate(), startOfCurrentHour, Qt::UTC );
  const QDateTime start = end.addSecs( -24 * 60 * 60 );

  whileBlocking( mStartDateTime )->setDateTime( start );
  whileBlocking( mEndDateTime )->setDateTime( end );
  whileBlocking( mFixedRangeStartDateTime )->setDateTime( start );
  whileBlocking( mFixedRangeEndDateTime )->setDateTime( end );

  updateTemporalExtent();
  mTimeStepsComboBox->setCurrentIndex( mTimeStepsComboBox->findData( static_cast<int>( Qgis::TemporalUnit::Hours ) ) );
  mStepSpinBox->setValue( 1 );
}

void QgsTemporalControllerWidget::updateSlider( const QgsDateTimeRange &range )
{
  whileBlocking( mAnimationSlider )->setValue( mNavigationObject->currentFrameNumber() );
  whileBlocking( mMovieSlider )->setValue( mNavigationObject->currentFrameNumber() );
  updateRangeLabel( range );
}

void QgsTemporalControllerWidget::totalMovieFramesChanged( long long frames )
{
  QgsProject::instance()->timeSettings()->setTotalMovieFrames( frames );
  mTotalFramesSpinBox->setValue( frames );
  mCurrentRangeLabel->setText( tr( "Current frame: %1/%2" ).arg( mNavigationObject->currentFrameNumber() ).arg( frames ) );
}

void QgsTemporalControllerWidget::updateRangeLabel( const QgsDateTimeRange &range )
{
  QString timeFrameFormat = QStringLiteral( "yyyy-MM-dd HH:mm:ss" );
  // but if timesteps are < 1 second (as: in milliseconds), add milliseconds to the format
  if ( static_cast<Qgis::TemporalUnit>( mTimeStepsComboBox->currentData().toInt() ) == Qgis::TemporalUnit::Milliseconds )
    timeFrameFormat = QStringLiteral( "yyyy-MM-dd HH:mm:ss.zzz" );
  switch ( mNavigationObject->navigationMode() )
  {
    case Qgis::TemporalNavigationMode::Animated:
      mCurrentRangeLabel->setText( tr( "Current frame: %1 ≤ <i>t</i> &lt; %2" ).arg( range.begin().toString( timeFrameFormat ), range.end().toString( timeFrameFormat ) ) );
      break;
    case Qgis::TemporalNavigationMode::FixedRange:
      mCurrentRangeLabel->setText( tr( "Range: %1 ≤ <i>t</i> &lt; %2" ).arg( range.begin().toString( timeFrameFormat ), range.end().toString( timeFrameFormat ) ) );
      break;
    case Qgis::TemporalNavigationMode::Disabled:
      mCurrentRangeLabel->setText( tr( "Temporal navigation disabled" ) );
      break;
    case Qgis::TemporalNavigationMode::Movie:
      mCurrentRangeLabel->setText( tr( "Current frame: %1/%2" ).arg( mNavigationObject->currentFrameNumber() ).arg( mNavigationObject->totalMovieFrames() ) );
      break;
  }
}

QgsTemporalNavigationObject *QgsTemporalControllerWidget::temporalController()
{
  return mNavigationObject;
}

void QgsTemporalControllerWidget::settings_clicked()
{
  QgsTemporalMapSettingsWidget *settingsWidget = new QgsTemporalMapSettingsWidget( this );
  settingsWidget->setFrameRateValue( mNavigationObject->framesPerSecond() );
  settingsWidget->setIsTemporalRangeCumulative( mNavigationObject->temporalRangeCumulative() );

  connect( settingsWidget, &QgsTemporalMapSettingsWidget::frameRateChanged, this, [=]( double rate ) {
    // save new settings into project
    QgsProject::instance()->timeSettings()->setFramesPerSecond( rate );
    mNavigationObject->setFramesPerSecond( rate );
  } );

  connect( settingsWidget, &QgsTemporalMapSettingsWidget::temporalRangeCumulativeChanged, this, [=]( bool state ) {
    // save new settings into project
    QgsProject::instance()->timeSettings()->setIsTemporalRangeCumulative( state );
    mNavigationObject->setTemporalRangeCumulative( state );
  } );
  openPanel( settingsWidget );
}

void QgsTemporalControllerWidget::timeSlider_valueChanged( int value )
{
  mNavigationObject->setCurrentFrameNumber( value );
}

void QgsTemporalControllerWidget::startEndDateTime_changed()
{
  whileBlocking( mFixedRangeStartDateTime )->setDateTime( mStartDateTime->dateTime() );
  whileBlocking( mFixedRangeEndDateTime )->setDateTime( mEndDateTime->dateTime() );

  updateTemporalExtent();
  saveRangeToProject();
}

void QgsTemporalControllerWidget::fixedRangeStartEndDateTime_changed()
{
  whileBlocking( mStartDateTime )->setDateTime( mFixedRangeStartDateTime->dateTime() );
  whileBlocking( mEndDateTime )->setDateTime( mFixedRangeEndDateTime->dateTime() );

  updateTemporalExtent();
  saveRangeToProject();
}

void QgsTemporalControllerWidget::mRangeSetToAllLayersAction_triggered()
{
  setDatesToAllLayers();
  saveRangeToProject();
}

void QgsTemporalControllerWidget::setTimeStep( const QgsInterval &timeStep )
{
  if ( !timeStep.isValid() || timeStep.seconds() <= 0 )
    return;

  int selectedUnit = -1;
  double selectedValue = std::numeric_limits<double>::max();
  if ( timeStep.originalUnit() != Qgis::TemporalUnit::IrregularStep )
  {
    // Search the time unit the most appropriate :
    // the one that gives the smallest time step value for double spin box with round value (if possible) and/or the less signifiant digits

    int stringSize = std::numeric_limits<int>::max();
    const int precision = mStepSpinBox->decimals();
    for ( int i = 0; i < mTimeStepsComboBox->count(); ++i )
    {
      const Qgis::TemporalUnit unit = static_cast<Qgis::TemporalUnit>( mTimeStepsComboBox->itemData( i ).toInt() );
      const double value = timeStep.seconds() * QgsUnitTypes::fromUnitToUnitFactor( Qgis::TemporalUnit::Seconds, unit );
      QString string = QString::number( value, 'f', precision );

      const thread_local QRegularExpression trailingZeroRegEx = QRegularExpression( QStringLiteral( "0+$" ) );
      //remove trailing zero
      string.remove( trailingZeroRegEx );

      const thread_local QRegularExpression trailingPointRegEx = QRegularExpression( QStringLiteral( "[.]+$" ) );
      //remove last point if present
      string.remove( trailingPointRegEx );

      if ( value >= 1
           && string.size() <= stringSize // less significant digit than currently selected
           && value < selectedValue )     // less than currently selected
      {
        selectedUnit = i;
        selectedValue = value;
        stringSize = string.size();
      }
      else if ( string != '0'
                && string.size() < precision + 2 //round value (ex: 0.xx with precision=3)
                && string.size() < stringSize )  //less significant digit than currently selected
      {
        selectedUnit = i;
        selectedValue = value;
        stringSize = string.size();
      }
    }
  }
  else
  {
    selectedUnit = mTimeStepsComboBox->findData( static_cast<int>( timeStep.originalUnit() ) );
    selectedValue = 1;
  }

  if ( selectedUnit >= 0 )
  {
    mStepSpinBox->setValue( selectedValue );
    mTimeStepsComboBox->setCurrentIndex( selectedUnit );
  }

  updateFrameDuration();
}

void QgsTemporalControllerWidget::updateTimeStepInputs( const QgsInterval &timeStep )
{
  if ( !timeStep.isValid() || timeStep.seconds() <= 0.0001 )
    return;

  QString timeDisplayFormat = QStringLiteral( "yyyy-MM-dd HH:mm:ss" );
  if ( Qgis::TemporalUnit::Milliseconds == timeStep.originalUnit() )
  {
    timeDisplayFormat = QStringLiteral( "yyyy-MM-dd HH:mm:ss.zzz" );
    // very big change that you have to update the range too, as defaulting to NOT handling millis
    updateTemporalExtent();
  }
  mStartDateTime->setDisplayFormat( timeDisplayFormat );
  mEndDateTime->setDisplayFormat( timeDisplayFormat );
  mFixedRangeStartDateTime->setDisplayFormat( timeDisplayFormat );
  mFixedRangeEndDateTime->setDisplayFormat( timeDisplayFormat );

  // Only update ui when the intervals are different
  if ( timeStep == QgsInterval( mStepSpinBox->value(), static_cast<Qgis::TemporalUnit>( mTimeStepsComboBox->currentData().toInt() ) ) )
    return;

  if ( timeStep.originalUnit() != Qgis::TemporalUnit::Unknown )
  {
    mStepSpinBox->setValue( timeStep.originalDuration() );
    mTimeStepsComboBox->setCurrentIndex( mTimeStepsComboBox->findData( static_cast<int>( timeStep.originalUnit() ) ) );
  }

  updateFrameDuration();
}

void QgsTemporalControllerWidget::mRangeSetToProjectAction_triggered()
{
  setDatesToProjectTime( false );
  saveRangeToProject();
}

void QgsTemporalControllerWidget::setDates( const QgsDateTimeRange &range )
{
  if ( range.begin().isValid() && range.end().isValid() )
  {
    whileBlocking( mStartDateTime )->setDateTime( range.begin() );
    whileBlocking( mEndDateTime )->setDateTime( range.end() );
    whileBlocking( mFixedRangeStartDateTime )->setDateTime( range.begin() );
    whileBlocking( mFixedRangeEndDateTime )->setDateTime( range.end() );
    updateTemporalExtent();
  }
}

void QgsTemporalControllerWidget::setDatesToAllLayers()
{
  QgsDateTimeRange range;
  range = QgsTemporalUtils::calculateTemporalRangeForProject( QgsProject::instance() );
  mNavigationObject->setAvailableTemporalRanges( QgsTemporalUtils::usedTemporalRangesForProject( QgsProject::instance() ) );

  setDates( range );
}

void QgsTemporalControllerWidget::setDatesToProjectTime( bool tryLastStoredRange )
{
  QgsDateTimeRange range;

  if ( tryLastStoredRange )
  {
    const QString startString = QgsProject::instance()->readEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/StartDateTime" ) );
    const QString endString = QgsProject::instance()->readEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/EndDateTime" ) );
    if ( !startString.isEmpty() && !endString.isEmpty() )
    {
      range = QgsDateTimeRange( QDateTime::fromString( startString, Qt::ISODateWithMs ), QDateTime::fromString( endString, Qt::ISODateWithMs ) );
    }
  }

  // by default try taking the project's fixed temporal extent
  if ( ( !range.begin().isValid() || !range.end().isValid() ) && QgsProject::instance()->timeSettings() )
    range = QgsProject::instance()->timeSettings()->temporalRange();

  // if that's not set, calculate the extent from the project's layers
  if ( !range.begin().isValid() || !range.end().isValid() )
  {
    range = QgsTemporalUtils::calculateTemporalRangeForProject( QgsProject::instance() );
  }

  mNavigationObject->setAvailableTemporalRanges( QgsTemporalUtils::usedTemporalRangesForProject( QgsProject::instance() ) );

  setDates( range );
}

void QgsTemporalControllerWidget::saveRangeToProject()
{
  QgsProject::instance()->writeEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/StartDateTime" ), mStartDateTime->dateTime().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODateWithMs ) );
  QgsProject::instance()->writeEntry( QStringLiteral( "TemporalControllerWidget" ), QStringLiteral( "/EndDateTime" ), mEndDateTime->dateTime().toTimeSpec( Qt::OffsetFromUTC ).toString( Qt::ISODateWithMs ) );
}
