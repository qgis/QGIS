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
      break;

    case AnimationState::Reverse:
      previous();
      break;

    case AnimationState::Idle:
      // should not happen - in an idle state the timeout won't occur
      break;
  }
}

QgsDateTimeRange QgsTemporalNavigationObject::dateTimeRangeForFrameNumber( long long frame ) const
{
  QDateTime start = mTemporalExtents.begin();

  if ( frame < 0 )
    frame = 0;
  long long nextFrame = frame + 1;

  QDateTime begin = start.addSecs( frame * mFrameDuration.seconds() );
  QDateTime end = start.addSecs( nextFrame * mFrameDuration.seconds() );

  if ( end <= mTemporalExtents.end() )
    return QgsDateTimeRange( begin, end );

  return QgsDateTimeRange( begin, mTemporalExtents.end() );
}

void QgsTemporalNavigationObject::setTemporalExtents( const QgsDateTimeRange &temporalExtents )
{
  mTemporalExtents = temporalExtents;
  setCurrentFrameNumber( 0 );
}

QgsDateTimeRange QgsTemporalNavigationObject::temporalExtents() const
{
  return mTemporalExtents;
}

void QgsTemporalNavigationObject::setCurrentFrameNumber( long long frameNumber )
{
  if ( mCurrentFrameNumber != frameNumber )
  {
    mCurrentFrameNumber = frameNumber;
    QgsDateTimeRange range = dateTimeRangeForFrameNumber( mCurrentFrameNumber );
    emit updateTemporalRange( range );
  }
}

long long QgsTemporalNavigationObject::currentFrameNumber() const
{
  return mCurrentFrameNumber;
}

void QgsTemporalNavigationObject::setFrameDuration( QgsInterval frameDuration )
{
  mFrameDuration = frameDuration;
  setCurrentFrameNumber( 0 );
}

QgsInterval QgsTemporalNavigationObject::frameDuration() const
{
  return mFrameDuration;
}

void QgsTemporalNavigationObject::setFramesPerSeconds( double framesPerSeconds )
{
  if ( framesPerSeconds > 0 )
  {
    mFramesPerSecond = framesPerSeconds;
    mNewFrameTimer->setInterval( ( 1.0 / mFramesPerSecond ) * 1000 );
  }
}

double QgsTemporalNavigationObject::framesPerSeconds() const
{
  return mFramesPerSecond;
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
  setAnimationState( AnimationState::Forward );
  play();
}

void QgsTemporalNavigationObject::playBackward()
{
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
  const long long frame = totalFrameCount();
  setCurrentFrameNumber( frame );
}

long long QgsTemporalNavigationObject::totalFrameCount()
{
  QgsInterval totalAnimationLength = mTemporalExtents.end() - mTemporalExtents.begin();
  return std::ceil( totalAnimationLength.seconds() / mFrameDuration.seconds() ) + 1;
}

void QgsTemporalNavigationObject::setAnimationState( AnimationState mode )
{
  mPlayBackMode = mode;
}

QgsTemporalNavigationObject::AnimationState QgsTemporalNavigationObject::animationState() const
{
  return mPlayBackMode;
}
