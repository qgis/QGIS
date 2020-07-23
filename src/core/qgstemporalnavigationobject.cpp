/***************************************************************************
                         qgstemporalnavigationobject.cpp
                         ---------------
    begin                : March 2020
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

#include "qgstemporalnavigationobject.h"
#include "qgis.h"

QgsTemporalNavigationObject::QgsTemporalNavigationObject( QObject *parent )
  : QgsTemporalController( parent )
{
  mNewFrameTimer = new QTimer( this );

  connect( mNewFrameTimer, &QTimer::timeout,
           this, &QgsTemporalNavigationObject::timerTimeout );
}

void QgsTemporalNavigationObject::timerTimeout()
{
  switch ( mPlayBackMode )
  {
    case AnimationState::Forward:
      next();
      if ( mCurrentFrameNumber >= totalFrameCount() - 1 )
      {
        if ( mLoopAnimation )
          mCurrentFrameNumber = -1; // we don't jump immediately to frame 0, instead we delay that till the next timeout
        else
          pause();
      }
      break;

    case AnimationState::Reverse:
      previous();
      if ( mCurrentFrameNumber <= 0 )
      {
        if ( mLoopAnimation )
          mCurrentFrameNumber = totalFrameCount(); // we don't jump immediately to real last frame..., instead we delay that till the next timeout
        else
          pause();
      }
      break;

    case AnimationState::Idle:
      // should not happen - in an idle state the timeout won't occur
      break;
  }
}

bool QgsTemporalNavigationObject::isLooping() const
{
  return mLoopAnimation;
}

void QgsTemporalNavigationObject::setLooping( bool loopAnimation )
{
  mLoopAnimation = loopAnimation;
}

QgsExpressionContextScope *QgsTemporalNavigationObject::createExpressionContextScope() const
{
  std::unique_ptr< QgsExpressionContextScope > scope = qgis::make_unique< QgsExpressionContextScope >( QStringLiteral( "temporal" ) );
  scope->setVariable( QStringLiteral( "frame_rate" ), mFramesPerSecond, true );
  scope->setVariable( QStringLiteral( "frame_number" ), mCurrentFrameNumber, true );
  scope->setVariable( QStringLiteral( "frame_timestep" ), mFrameTimeStep, true );
  scope->setVariable( QStringLiteral( "frame_timestepunit" ), mFrameTimeStepUnit, true );
  scope->setVariable( QStringLiteral( "animation_start_time" ), mTemporalExtents.begin(), true );
  scope->setVariable( QStringLiteral( "animation_end_time" ), mTemporalExtents.end(), true );
  scope->setVariable( QStringLiteral( "animation_interval" ), mTemporalExtents.end() - mTemporalExtents.begin(), true );
  return scope.release();
}

QgsDateTimeRange QgsTemporalNavigationObject::dateTimeRangeForFrameNumber( long long frame ) const
{
  const QDateTime start = mTemporalExtents.begin();

  if ( frame < 0 )
    frame = 0;

  const long long nextFrame = frame + 1;

  QDateTime begin;
  QDateTime end;

  // If mFrameTimeStep is fractional, we use QgsInterval to determine the
  // duration of the frame, which uses average durations for months and years.
  // Otherwise, we use QDateTime to advance by the exact duration of the current
  // month or year. So a time step of 1.5 months will result in a duration of 45
  // days, but a time step of 1 month will result in a duration that depends upon
  // the number of days in the current month.
  if ( mFrameTimeStepIsFractional )
  {
    double duration = QgsInterval( mFrameTimeStep, mFrameTimeStepUnit ).seconds();
    begin = start.addSecs( frame * duration );
    end = start.addSecs( nextFrame * duration );
  }
  else
  {
    switch ( mFrameTimeStepUnit )
    {
      case QgsUnitTypes::TemporalUnit::TemporalMilliseconds:
        begin = start.addMSecs( frame * mFrameTimeStep );
        end = start.addMSecs( nextFrame * mFrameTimeStep );
        break;
      case QgsUnitTypes::TemporalUnit::TemporalSeconds:
        begin = start.addSecs( frame * mFrameTimeStep );
        end = start.addSecs( nextFrame * mFrameTimeStep );
        break;
      case QgsUnitTypes::TemporalUnit::TemporalMinutes:
        begin = start.addSecs( 60 * frame * mFrameTimeStep );
        end = start.addSecs( 60 * nextFrame * mFrameTimeStep );
        break;
      case QgsUnitTypes::TemporalUnit::TemporalHours:
        begin = start.addSecs( 3600 * frame * mFrameTimeStep );
        end = start.addSecs( 3600 * nextFrame * mFrameTimeStep );
        break;
      case QgsUnitTypes::TemporalUnit::TemporalDays:
        begin = start.addDays( frame * mFrameTimeStep );
        end = start.addDays( nextFrame * mFrameTimeStep );
        break;
      case QgsUnitTypes::TemporalUnit::TemporalWeeks:
        begin = start.addDays( 7 * frame * mFrameTimeStep );
        end = start.addDays( 7 * nextFrame * mFrameTimeStep );
        break;
      case QgsUnitTypes::TemporalUnit::TemporalMonths:
        begin = start.addMonths( frame * mFrameTimeStep );
        end = start.addMonths( nextFrame * mFrameTimeStep );
        break;
      case QgsUnitTypes::TemporalUnit::TemporalYears:
        begin = start.addYears( frame * mFrameTimeStep );
        end = start.addYears( nextFrame * mFrameTimeStep );
        break;
      case QgsUnitTypes::TemporalUnit::TemporalDecades:
        begin = start.addYears( 10 * frame * mFrameTimeStep );
        end = start.addYears( 10 * nextFrame * mFrameTimeStep );
        break;
      case QgsUnitTypes::TemporalUnit::TemporalCenturies:
        begin = start.addYears( 100 * frame * mFrameTimeStep );
        end = start.addYears( 100 * nextFrame * mFrameTimeStep );
        break;
      case QgsUnitTypes::TemporalUnit::TemporalUnknownUnit:
        Q_ASSERT( false );
        break;
    }
  }

  QDateTime frameStart = begin;

  if ( mCumulativeTemporalRange )
    frameStart = start;

  if ( end <= mTemporalExtents.end() )
    return QgsDateTimeRange( frameStart, end, true, false );

  return QgsDateTimeRange( frameStart, mTemporalExtents.end(), true, false );
}

void QgsTemporalNavigationObject::setNavigationMode( const NavigationMode mode )
{
  if ( mNavigationMode == mode )
    return;

  mNavigationMode = mode;
  emit navigationModeChanged( mode );

  switch ( mNavigationMode )
  {
    case Animated:
      emit updateTemporalRange( dateTimeRangeForFrameNumber( mCurrentFrameNumber ) );
      break;
    case FixedRange:
      emit updateTemporalRange( mTemporalExtents );
      break;
    case NavigationOff:
      emit updateTemporalRange( QgsDateTimeRange() );
      break;
  }
}

void QgsTemporalNavigationObject::setTemporalExtents( const QgsDateTimeRange &temporalExtents )
{
  mTemporalExtents = temporalExtents;

  switch ( mNavigationMode )
  {
    case Animated:
    {
      int currentFrameNmber = mCurrentFrameNumber;
      setCurrentFrameNumber( 0 );

      //Force to emit signal if the current frame number doesn't change
      if ( currentFrameNmber == mCurrentFrameNumber )
        emit updateTemporalRange( dateTimeRangeForFrameNumber( 0 ) );
      break;
    }
    case FixedRange:
      emit updateTemporalRange( mTemporalExtents );
      break;
    case NavigationOff:
      break;
  }
}

QgsDateTimeRange QgsTemporalNavigationObject::temporalExtents() const
{
  return mTemporalExtents;
}

void QgsTemporalNavigationObject::setCurrentFrameNumber( long long frameNumber )
{
  if ( mCurrentFrameNumber != frameNumber )
  {
    mCurrentFrameNumber = std::max( 0LL, std::min( frameNumber, totalFrameCount() - 1 ) );
    QgsDateTimeRange range = dateTimeRangeForFrameNumber( mCurrentFrameNumber );
    emit updateTemporalRange( range );
  }
}

long long QgsTemporalNavigationObject::currentFrameNumber() const
{
  return mCurrentFrameNumber;
}

void QgsTemporalNavigationObject::setFrameTimeStep( double timeStep )
{
  mFrameTimeStep = timeStep;
  double unused;
  mFrameTimeStepIsFractional = fabs( modf( mFrameTimeStep, &unused ) ) > 0.00001;
  setCurrentFrameNumber( 0 );
}

void QgsTemporalNavigationObject::setFrameTimeStepUnit( QgsUnitTypes::TemporalUnit timeStepUnit )
{
  mFrameTimeStepUnit = timeStepUnit;
  setCurrentFrameNumber( 0 );
}

double QgsTemporalNavigationObject::frameTimeStep() const
{
  return mFrameTimeStep;
}

QgsUnitTypes::TemporalUnit QgsTemporalNavigationObject::frameTimeStepUnit() const
{
  return mFrameTimeStepUnit;
}

void QgsTemporalNavigationObject::setFramesPerSecond( double framesPerSeconds )
{
  if ( framesPerSeconds > 0 )
  {
    mFramesPerSecond = framesPerSeconds;
    mNewFrameTimer->setInterval( ( 1.0 / mFramesPerSecond ) * 1000 );
  }
}

double QgsTemporalNavigationObject::framesPerSecond() const
{
  return mFramesPerSecond;
}

void QgsTemporalNavigationObject::setTemporalRangeCumulative( bool state )
{
  mCumulativeTemporalRange = state;
}

bool QgsTemporalNavigationObject::temporalRangeCumulative() const
{
  return mCumulativeTemporalRange;
}

void QgsTemporalNavigationObject::play()
{
  mNewFrameTimer->start( ( 1.0 / mFramesPerSecond ) * 1000 );
}

void QgsTemporalNavigationObject::pause()
{
  mNewFrameTimer->stop();
  setAnimationState( AnimationState::Idle );
}

void QgsTemporalNavigationObject::playForward()
{
  if ( mPlayBackMode == Idle &&  mCurrentFrameNumber >= totalFrameCount() - 1 )
  {
    // if we are paused at the end of the video, and the user hits play, we automatically rewind and play again
    rewindToStart();
  }

  setAnimationState( AnimationState::Forward );
  play();
}

void QgsTemporalNavigationObject::playBackward()
{
  if ( mPlayBackMode == Idle &&  mCurrentFrameNumber <= 0 )
  {
    // if we are paused at the start of the video, and the user hits play, we automatically skip to end and play in reverse again
    skipToEnd();
  }

  setAnimationState( AnimationState::Reverse );
  play();
}

void QgsTemporalNavigationObject::next()
{
  setCurrentFrameNumber( mCurrentFrameNumber + 1 );
}

void QgsTemporalNavigationObject::previous()
{
  setCurrentFrameNumber( mCurrentFrameNumber - 1 );
}

void QgsTemporalNavigationObject::rewindToStart()
{
  setCurrentFrameNumber( 0 );
}

void QgsTemporalNavigationObject::skipToEnd()
{
  const long long frame = totalFrameCount() - 1;
  setCurrentFrameNumber( frame );
}

long long QgsTemporalNavigationObject::totalFrameCount()
{
  QgsInterval totalAnimationLength = mTemporalExtents.end() - mTemporalExtents.begin();
  return std::floor( totalAnimationLength.seconds() / QgsInterval( mFrameTimeStep, mFrameTimeStepUnit ).seconds() ) + 1;
}

void QgsTemporalNavigationObject::setAnimationState( AnimationState mode )
{
  if ( mode != mPlayBackMode )
  {
    mPlayBackMode = mode;
    emit stateChanged( mPlayBackMode );
  }
}

QgsTemporalNavigationObject::AnimationState QgsTemporalNavigationObject::animationState() const
{
  return mPlayBackMode;
}
