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
#include "moc_qgstemporalnavigationobject.cpp"
#include "qgis.h"
#include "qgstemporalutils.h"
#include "qgsunittypes.h"

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
    case Qgis::AnimationState::Forward:
      next();
      if ( mCurrentFrameNumber >= totalFrameCount() - 1 )
      {
        if ( mLoopAnimation )
          mCurrentFrameNumber = -1; // we don't jump immediately to frame 0, instead we delay that till the next timeout
        else
          pause();
      }
      break;

    case Qgis::AnimationState::Reverse:
      previous();
      if ( mCurrentFrameNumber <= 0 )
      {
        if ( mLoopAnimation )
          mCurrentFrameNumber = totalFrameCount(); // we don't jump immediately to real last frame..., instead we delay that till the next timeout
        else
          pause();
      }
      break;

    case Qgis::AnimationState::Idle:
      // should not happen - in an idle state the timeout won't occur
      break;
  }
}

long long QgsTemporalNavigationObject::totalMovieFrames() const
{
  return mTotalMovieFrames;
}

void QgsTemporalNavigationObject::setTotalMovieFrames( long long frames )
{
  if ( frames == mTotalMovieFrames )
    return;

  mTotalMovieFrames = frames;

  emit totalMovieFramesChanged( mTotalMovieFrames );
  emit temporalExtentsChanged( mTemporalExtents );
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
  std::unique_ptr< QgsExpressionContextScope > scope = std::make_unique< QgsExpressionContextScope >( QStringLiteral( "temporal" ) );
  scope->setVariable( QStringLiteral( "frame_rate" ), mFramesPerSecond, true );
  scope->setVariable( QStringLiteral( "frame_number" ), mCurrentFrameNumber, true );
  scope->setVariable( QStringLiteral( "frame_duration" ), mFrameDuration, true );
  scope->setVariable( QStringLiteral( "frame_timestep" ), mFrameDuration.originalDuration(), true );
  scope->setVariable( QStringLiteral( "frame_timestep_unit" ), static_cast< int >( mFrameDuration.originalUnit() ), true );
  scope->setVariable( QStringLiteral( "frame_timestep_units" ), QgsUnitTypes::toString( mFrameDuration.originalUnit() ), true );
  scope->setVariable( QStringLiteral( "animation_start_time" ), mTemporalExtents.begin(), true );
  scope->setVariable( QStringLiteral( "animation_end_time" ), mTemporalExtents.end(), true );
  scope->setVariable( QStringLiteral( "animation_interval" ), QgsInterval( mTemporalExtents.end() - mTemporalExtents.begin() ), true );
  scope->setVariable( QStringLiteral( "total_frame_count" ), totalFrameCount() );

  scope->addHiddenVariable( QStringLiteral( "frame_timestep_unit" ) );

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
  if ( mFrameDuration.originalUnit() == Qgis::TemporalUnit::IrregularStep )
  {
    if ( mAllRanges.empty() )
      return QgsDateTimeRange();

    return frame < mAllRanges.size() ? mAllRanges.at( frame ) : mAllRanges.constLast();
  }
  else
  {
    begin = QgsTemporalUtils::calculateFrameTime( start, frame, mFrameDuration );
    end = QgsTemporalUtils::calculateFrameTime( start, nextFrame, mFrameDuration );
  }

  QDateTime frameStart = begin;

  if ( mCumulativeTemporalRange )
    frameStart = start;

  return QgsDateTimeRange( frameStart, end, true, false );
}

void QgsTemporalNavigationObject::setNavigationMode( const Qgis::TemporalNavigationMode mode )
{
  if ( mNavigationMode == mode )
    return;

  mNavigationMode = mode;
  emit navigationModeChanged( mode );

  if ( !mBlockUpdateTemporalRangeSignal )
  {
    switch ( mNavigationMode )
    {
      case Qgis::TemporalNavigationMode::Animated:
        emit updateTemporalRange( dateTimeRangeForFrameNumber( mCurrentFrameNumber ) );
        break;
      case Qgis::TemporalNavigationMode::FixedRange:
        emit updateTemporalRange( mTemporalExtents );
        break;
      case Qgis::TemporalNavigationMode::Disabled:
      case Qgis::TemporalNavigationMode::Movie:
        emit updateTemporalRange( QgsDateTimeRange() );
        break;
    }
  }
}

void QgsTemporalNavigationObject::setTemporalExtents( const QgsDateTimeRange &temporalExtents )
{
  if ( mTemporalExtents == temporalExtents )
  {
    return;
  }
  const QgsDateTimeRange oldFrame = dateTimeRangeForFrameNumber( currentFrameNumber() );
  mTemporalExtents = temporalExtents;
  mCurrentFrameNumber = findBestFrameNumberForFrameStart( oldFrame.begin() );
  emit temporalExtentsChanged( mTemporalExtents );

  switch ( mNavigationMode )
  {
    case Qgis::TemporalNavigationMode::Animated:
    {
      const long long currentFrameNumber = mCurrentFrameNumber;

      // Force to emit signal if the current frame number doesn't change
      if ( currentFrameNumber == mCurrentFrameNumber && !mBlockUpdateTemporalRangeSignal )
        emit updateTemporalRange( dateTimeRangeForFrameNumber( mCurrentFrameNumber ) );
      break;
    }
    case Qgis::TemporalNavigationMode::FixedRange:
      if ( !mBlockUpdateTemporalRangeSignal )
        emit updateTemporalRange( mTemporalExtents );
      break;
    case Qgis::TemporalNavigationMode::Disabled:
    case Qgis::TemporalNavigationMode::Movie:
      break;
  }

}

QgsDateTimeRange QgsTemporalNavigationObject::temporalExtents() const
{
  return mTemporalExtents;
}

void QgsTemporalNavigationObject::setAvailableTemporalRanges( const QList<QgsDateTimeRange> &ranges )
{
  mAllRanges = ranges;
}

QList<QgsDateTimeRange> QgsTemporalNavigationObject::availableTemporalRanges() const
{
  return mAllRanges;
}

void QgsTemporalNavigationObject::setCurrentFrameNumber( long long frameNumber )
{
  if ( mCurrentFrameNumber != frameNumber )
  {
    mCurrentFrameNumber = std::max( 0LL, std::min( frameNumber, totalFrameCount() - 1 ) );
    const QgsDateTimeRange range = dateTimeRangeForFrameNumber( mCurrentFrameNumber );

    if ( !mBlockUpdateTemporalRangeSignal )
      emit updateTemporalRange( range );
  }
}

long long QgsTemporalNavigationObject::currentFrameNumber() const
{
  return mCurrentFrameNumber;
}

void QgsTemporalNavigationObject::setFrameDuration( const QgsInterval &frameDuration )
{
  if ( mFrameDuration == frameDuration )
  {
    return;
  }

  const QgsDateTimeRange oldFrame = dateTimeRangeForFrameNumber( currentFrameNumber() );
  mFrameDuration = frameDuration;

  mCurrentFrameNumber = findBestFrameNumberForFrameStart( oldFrame.begin() );
  emit temporalFrameDurationChanged( mFrameDuration );

  // forcing an update of our views
  const QgsDateTimeRange range = dateTimeRangeForFrameNumber( mCurrentFrameNumber );

  if ( !mBlockUpdateTemporalRangeSignal && mNavigationMode == Qgis::TemporalNavigationMode::Animated )
    emit updateTemporalRange( range );
}

QgsInterval QgsTemporalNavigationObject::frameDuration() const
{
  return mFrameDuration;
}

void QgsTemporalNavigationObject::setFramesPerSecond( double framesPerSeconds )
{
  if ( framesPerSeconds > 0 )
  {
    mFramesPerSecond = framesPerSeconds;
    mNewFrameTimer->setInterval( static_cast< int >( ( 1.0 / mFramesPerSecond ) * 1000 ) );
  }
}

double QgsTemporalNavigationObject::framesPerSecond() const
{
  return mFramesPerSecond;
}

void QgsTemporalNavigationObject::setTemporalRangeCumulative( bool state )
{
  if ( mCumulativeTemporalRange == state )
    return;

  mCumulativeTemporalRange = state;

  if ( !mBlockUpdateTemporalRangeSignal && mNavigationMode == Qgis::TemporalNavigationMode::Animated )
  {
    emit updateTemporalRange( dateTimeRangeForFrameNumber( mCurrentFrameNumber ) );
  }
}

bool QgsTemporalNavigationObject::temporalRangeCumulative() const
{
  return mCumulativeTemporalRange;
}

void QgsTemporalNavigationObject::play()
{
  mNewFrameTimer->start( static_cast< int >( ( 1.0 / mFramesPerSecond ) * 1000 ) );
}

void QgsTemporalNavigationObject::pause()
{
  mNewFrameTimer->stop();
  setAnimationState( Qgis::AnimationState::Idle );
}

void QgsTemporalNavigationObject::playForward()
{
  if ( mPlayBackMode == Qgis::AnimationState::Idle &&  mCurrentFrameNumber >= totalFrameCount() - 1 )
  {
    // if we are paused at the end of the video, and the user hits play, we automatically rewind and play again
    rewindToStart();
  }

  setAnimationState( Qgis::AnimationState::Forward );
  play();
}

void QgsTemporalNavigationObject::playBackward()
{
  if ( mPlayBackMode == Qgis::AnimationState::Idle &&  mCurrentFrameNumber <= 0 )
  {
    // if we are paused at the start of the video, and the user hits play, we automatically skip to end and play in reverse again
    skipToEnd();
  }

  setAnimationState( Qgis::AnimationState::Reverse );
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

long long QgsTemporalNavigationObject::totalFrameCount() const
{
  if ( mNavigationMode == Qgis::TemporalNavigationMode::Movie )
    return mTotalMovieFrames;

  if ( mFrameDuration.originalUnit() == Qgis::TemporalUnit::IrregularStep )
  {
    return mAllRanges.count();
  }
  else
  {
    const QgsInterval totalAnimationLength = mTemporalExtents.end() - mTemporalExtents.begin();
    return static_cast< long long >( std::ceil( totalAnimationLength.seconds() / mFrameDuration.seconds() ) );
  }
}

void QgsTemporalNavigationObject::setAnimationState( Qgis::AnimationState mode )
{
  if ( mode != mPlayBackMode )
  {
    mPlayBackMode = mode;
    emit stateChanged( mPlayBackMode );
  }
}

Qgis::AnimationState QgsTemporalNavigationObject::animationState() const
{
  return mPlayBackMode;
}

long long QgsTemporalNavigationObject::findBestFrameNumberForFrameStart( const QDateTime &frameStart ) const
{
  long long bestFrame = 0;
  if ( mFrameDuration.originalUnit() == Qgis::TemporalUnit::IrregularStep )
  {
    for ( const QgsDateTimeRange &range : mAllRanges )
    {
      if ( range.contains( frameStart ) )
        return bestFrame;
      else if ( range.begin() > frameStart )
        // if we've gone past the target date, go back one frame if possible
        return std::max( 0LL, bestFrame - 1 );
      bestFrame++;
    }
    return mAllRanges.count() - 1;
  }
  else
  {
    const QgsDateTimeRange testFrame = QgsDateTimeRange( frameStart, frameStart ); // creating an 'instant' Range
    // Earlier we looped from frame 0 till totalFrameCount() here, but this loop grew potentially gigantic
    long long roughFrameStart = 0;
    long long roughFrameEnd = totalFrameCount();
    // For the smaller step frames we calculate an educated guess, to prevent the loop becoming too
    // large, freezing the ui (eg having a mTemporalExtents of several months and the user selects milliseconds)
    if ( mFrameDuration.originalUnit() != Qgis::TemporalUnit::Months && mFrameDuration.originalUnit() != Qgis::TemporalUnit::Years && mFrameDuration.originalUnit() != Qgis::TemporalUnit::Decades && mFrameDuration.originalUnit() != Qgis::TemporalUnit::Centuries )
    {
      // Only if we receive a valid frameStart, that is within current mTemporalExtents
      // We tend to receive a framestart of 'now()' upon startup for example
      if ( mTemporalExtents.contains( frameStart ) )
      {
        roughFrameStart = static_cast< long long >( std::floor( QgsInterval( frameStart - mTemporalExtents.begin() ).seconds() / mFrameDuration.seconds() ) );
      }
      roughFrameEnd = roughFrameStart + 100; // just in case we miss the guess
    }
    for ( long long i = roughFrameStart; i < roughFrameEnd; ++i )
    {
      const QgsDateTimeRange range = dateTimeRangeForFrameNumber( i );
      if ( range.overlaps( testFrame ) )
      {
        bestFrame = i;
        break;
      }
    }
    return bestFrame;
  }
}
