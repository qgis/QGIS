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
  : QObject( parent )
{
  mNewFrameTimer = new QTimer( this );

  connect( mNewFrameTimer, &QTimer::timeout,
           this,  qgis::overload<>::of( &QgsTemporalNavigationObject::timerTimeout ) );
}


void QgsTemporalNavigationObject::timerTimeout()
{
  if ( mPlayBackMode == PlaybackMode::Forward )
    forward();
  if ( mPlayBackMode == PlaybackMode::Reverse )
    backward();
  return;
}

QgsDateTimeRange QgsTemporalNavigationObject::dateTimeRangeForFrameNumber( long long frame ) const
{
  QDateTime start = mTemporalExtents.begin();

  long long pastFrame = frame - 1;
  if ( pastFrame < 0 )
    pastFrame = 0;

  QDateTime begin = start.addSecs( pastFrame * mFrameDuration.seconds() );
  QDateTime end = start.addSecs( frame * mFrameDuration.seconds() );

  if ( end <= mTemporalExtents.end() )
    return QgsDateTimeRange( begin, end );

  return mTemporalExtents;
}

void QgsTemporalNavigationObject::setTemporalExtents( QgsDateTimeRange temporalExtents )
{
  mTemporalExtents = temporalExtents;
}

QgsDateTimeRange QgsTemporalNavigationObject::temporalExtents() const
{
  return mTemporalExtents;
}

void QgsTemporalNavigationObject::setCurrentFrameNumber( long long frameNumber )
{
  if ( frameNumber < 0 ||
       frameNumber >= totalFrameCount() )
  {
    pause();
    return;
  }

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
}

QgsInterval QgsTemporalNavigationObject::frameDuration() const
{
  return mFrameDuration;
}

void QgsTemporalNavigationObject::setFramesPerSeconds( double framesPerSeconds )
{
  if ( framesPerSeconds > 0 )
    mFramesPerSecond = framesPerSeconds;
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
  setPlayBackMode( PlaybackMode::Idle );
}

void QgsTemporalNavigationObject::forward()
{
  setPlayBackMode( PlaybackMode::Forward );
  play();
  next();
}

void QgsTemporalNavigationObject::backward()
{
  setPlayBackMode( PlaybackMode::Reverse );
  play();
  previous();
}

void QgsTemporalNavigationObject::next()
{
  long long frame = mCurrentFrameNumber + 1;
  setCurrentFrameNumber( frame );
}

void QgsTemporalNavigationObject::previous()
{
  long long frame = mCurrentFrameNumber - 1;
  setCurrentFrameNumber( frame );
}

void QgsTemporalNavigationObject::rewindToStart()
{
  long long frame = 0;
  setCurrentFrameNumber( frame );
}

void QgsTemporalNavigationObject::skipToEnd()
{
  long long frame = totalFrameCount();
  setCurrentFrameNumber( frame );
}

long long QgsTemporalNavigationObject::totalFrameCount()
{
  QgsInterval totalAnimationLength = mTemporalExtents.end() - mTemporalExtents.begin();
  long long totalFrameCount = std::ceil( totalAnimationLength.seconds() / mFrameDuration.seconds() ) + 1;

  return totalFrameCount;
}

void QgsTemporalNavigationObject::setPlayBackMode( PlaybackMode mode )
{
  mPlayBackMode = mode;
}

QgsTemporalNavigationObject::PlaybackMode QgsTemporalNavigationObject::playBackMode() const
{
  return mPlayBackMode;
}
