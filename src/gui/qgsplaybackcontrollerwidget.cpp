/***************************************************************************
   qgsplaybackcontrollerwidget.cpp
    --------------------------------------
    begin                : November 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsplaybackcontrollerwidget.h"
#include "moc_qgsplaybackcontrollerwidget.cpp"

QgsPlaybackControllerWidget::QgsPlaybackControllerWidget( QWidget *parent )
  : QWidget { parent }
{
  setupUi( this );

  mPauseButton->setChecked( true );

  mForwardButton->setToolTip( tr( "Play" ) );
  mBackButton->setToolTip( tr( "Reverse" ) );
  mNextButton->setToolTip( tr( "Go to next frame" ) );
  mPreviousButton->setToolTip( tr( "Go to previous frame" ) );
  mPauseButton->setToolTip( tr( "Pause" ) );
  mRewindButton->setToolTip( tr( "Rewind to start" ) );
  mFastForwardButton->setToolTip( tr( "Fast forward to end" ) );

  connect( mForwardButton, &QPushButton::clicked, this, &QgsPlaybackControllerWidget::togglePlayForward );
  connect( mBackButton, &QPushButton::clicked, this, &QgsPlaybackControllerWidget::togglePlayBackward );
  connect( mPauseButton, &QPushButton::clicked, this, &QgsPlaybackControllerWidget::pause );
  connect( mNextButton, &QPushButton::clicked, this, &QgsPlaybackControllerWidget::next );
  connect( mPreviousButton, &QPushButton::clicked, this, &QgsPlaybackControllerWidget::previous );
  connect( mFastForwardButton, &QPushButton::clicked, this, &QgsPlaybackControllerWidget::skipToEnd );
  connect( mRewindButton, &QPushButton::clicked, this, &QgsPlaybackControllerWidget::rewindToStart );
}

QPushButton *QgsPlaybackControllerWidget::button( Qgis::PlaybackOperation operation )
{
  switch ( operation )
  {
    case Qgis::PlaybackOperation::SkipToStart:
      return mRewindButton;
    case Qgis::PlaybackOperation::PreviousFrame:
      return mPreviousButton;
    case Qgis::PlaybackOperation::PlayReverse:
      return mBackButton;
    case Qgis::PlaybackOperation::Pause:
      return mPauseButton;
    case Qgis::PlaybackOperation::PlayForward:
      return mForwardButton;
    case Qgis::PlaybackOperation::NextFrame:
      return mNextButton;
    case Qgis::PlaybackOperation::SkipToEnd:
      return mFastForwardButton;
  }
  BUILTIN_UNREACHABLE
}

void QgsPlaybackControllerWidget::setState( Qgis::AnimationState state )
{
  if ( state == mAnimationState )
    return;

  mAnimationState = state;
  whileBlocking( mForwardButton )->setChecked( mAnimationState == Qgis::AnimationState::Forward );
  whileBlocking( mBackButton )->setChecked( mAnimationState == Qgis::AnimationState::Reverse );
  whileBlocking( mPauseButton )->setChecked( mAnimationState == Qgis::AnimationState::Idle );

  switch ( mAnimationState )
  {
    case Qgis::AnimationState::Idle:
      emit operationTriggered( Qgis::PlaybackOperation::Pause );
      break;
    case Qgis::AnimationState::Forward:
      emit operationTriggered( Qgis::PlaybackOperation::PlayForward );
      break;
    case Qgis::AnimationState::Reverse:
      emit operationTriggered( Qgis::PlaybackOperation::PlayReverse );
      break;
  }
}

void QgsPlaybackControllerWidget::togglePause()
{
  switch ( mAnimationState )
  {
    case Qgis::AnimationState::Idle:
      setState( mPlayingForward ? Qgis::AnimationState::Forward : Qgis::AnimationState::Reverse );
      break;
    case Qgis::AnimationState::Forward:
    case Qgis::AnimationState::Reverse:
      setState( Qgis::AnimationState::Idle );
      break;
  }
}

void QgsPlaybackControllerWidget::togglePlayForward()
{
  mPlayingForward = true;

  if ( mAnimationState != Qgis::AnimationState::Forward )
  {
    mPauseButton->setChecked( false );
    mBackButton->setChecked( false );
    mForwardButton->setChecked( true );

    mAnimationState = Qgis::AnimationState::Forward;
    emit operationTriggered( Qgis::PlaybackOperation::PlayForward );
  }
  else
  {
    mPauseButton->setChecked( true );
    mForwardButton->setChecked( false );

    mAnimationState = Qgis::AnimationState::Idle;
    emit operationTriggered( Qgis::PlaybackOperation::Pause );
  }
}

void QgsPlaybackControllerWidget::togglePlayBackward()
{
  mPlayingForward = false;

  if ( mAnimationState != Qgis::AnimationState::Reverse )
  {
    mPauseButton->setChecked( false );
    mBackButton->setChecked( true );
    mForwardButton->setChecked( false );

    mAnimationState = Qgis::AnimationState::Reverse;
    emit operationTriggered( Qgis::PlaybackOperation::PlayReverse );
  }
  else
  {
    mPauseButton->setChecked( true );
    mBackButton->setChecked( false );

    mAnimationState = Qgis::AnimationState::Idle;
    emit operationTriggered( Qgis::PlaybackOperation::Pause );
  }
}

void QgsPlaybackControllerWidget::pause()
{
  if ( mAnimationState != Qgis::AnimationState::Idle )
  {
    mPauseButton->setChecked( true );
    mBackButton->setChecked( false );
    mForwardButton->setChecked( false );

    mAnimationState = Qgis::AnimationState::Idle;
    emit operationTriggered( Qgis::PlaybackOperation::Pause );
  }
  else
  {
    mBackButton->setChecked( !mPlayingForward );
    mForwardButton->setChecked( mPlayingForward );
    mAnimationState = mPlayingForward ? Qgis::AnimationState::Forward : Qgis::AnimationState::Reverse;
    if ( mPlayingForward )
    {
      emit operationTriggered( Qgis::PlaybackOperation::PlayForward );
    }
    else
    {
      emit operationTriggered( Qgis::PlaybackOperation::PlayReverse );
    }
  }
}

void QgsPlaybackControllerWidget::next()
{
  emit operationTriggered( Qgis::PlaybackOperation::NextFrame );
}

void QgsPlaybackControllerWidget::previous()
{
  emit operationTriggered( Qgis::PlaybackOperation::PreviousFrame );
}

void QgsPlaybackControllerWidget::skipToEnd()
{
  emit operationTriggered( Qgis::PlaybackOperation::SkipToEnd );
}

void QgsPlaybackControllerWidget::rewindToStart()
{
  emit operationTriggered( Qgis::PlaybackOperation::SkipToStart );
}
