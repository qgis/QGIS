/***************************************************************************
  qgsmediawidget.cpp

 ---------------------
 begin                : 2023.01.24
 copyright            : (C) 2023 by Mathieu Pellerin
 email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmediawidget.h"
#include "moc_qgsmediawidget.cpp"
#include "qgsapplication.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QVideoWidget>


QgsMediaWidget::QgsMediaWidget( QWidget *parent )
  : QWidget( parent )
{
  mLayout = new QVBoxLayout();
  mLayout->setContentsMargins( 0, 0, 0, 0 );

  mVideoWidget = new QVideoWidget( this );
  const int vHeight = QFontMetrics( font() ).height() * 12;
  mVideoWidget->setMinimumHeight( vHeight );
  mVideoWidget->setMaximumHeight( vHeight );
  mLayout->addWidget( mVideoWidget );

  QHBoxLayout *controlsLayout = new QHBoxLayout();
  controlsLayout->setContentsMargins( 0, 0, 0, 0 );

  mPlayButton = new QPushButton( this );
  mPlayButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
  mPlayButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPlay.svg" ) ) );
  mPlayButton->setCheckable( true );
  controlsLayout->addWidget( mPlayButton );

  mPositionSlider = new QSlider( this );
  mPositionSlider->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
  mPositionSlider->setOrientation( Qt::Horizontal );
  controlsLayout->addWidget( mPositionSlider );

  mDurationLabel = new QLabel( this );
  mDurationLabel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
  mDurationLabel->setAlignment( Qt::AlignHCenter );
  mDurationLabel->setText( QStringLiteral( "-" ) );
  QFontMetrics fm( mDurationLabel->font() );
  mDurationLabel->setMinimumWidth( fm.boundingRect( QStringLiteral( "00:00:00" ) ).width() );
  controlsLayout->addWidget( mDurationLabel );

  QWidget *controls = new QWidget();
  controls->setLayout( controlsLayout );

  mLayout->addWidget( controls );
  setLayout( mLayout );

  adjustControls();
  setControlsEnabled( false );

  mMediaPlayer.setVideoOutput( mVideoWidget );

  connect( &mMediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &QgsMediaWidget::mediaStatusChanged );
  connect( &mMediaPlayer, &QMediaPlayer::positionChanged, this, [=]() {
    mPositionSlider->setValue( static_cast<int>( mMediaPlayer.position() / 1000 ) );
  } );

  connect( mPlayButton, &QAbstractButton::clicked, this, [=]() {
#if QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 )
    if ( mMediaPlayer.playbackState() == QMediaPlayer::PlayingState )
#else
    if ( mMediaPlayer.state() == QMediaPlayer::PlayingState )
#endif
    {
      mMediaPlayer.pause();
    }
    else
    {
      mMediaPlayer.play();
    }
  } );
  connect( mPositionSlider, &QAbstractSlider::sliderReleased, this, [=]() {
    mMediaPlayer.setPosition( static_cast<qint64>( mPositionSlider->value() ) * 1000 );
  } );
}

void QgsMediaWidget::setMediaPath( const QString &path )
{
  if ( mMediaPath == path )
    return;

  mMediaPath = path;
#if QT_VERSION >= QT_VERSION_CHECK( 6, 0, 0 )
  mMediaPlayer.setSource( QUrl::fromLocalFile( path ) );
#else
  mMediaPlayer.setMedia( QUrl::fromLocalFile( path ) );
#endif
}

void QgsMediaWidget::setMode( Mode mode )
{
  if ( mMode == mode )
    return;

  mMode = mode;
  adjustControls();
}

int QgsMediaWidget::videoHeight() const
{
  return mVideoWidget->minimumHeight();
}

void QgsMediaWidget::setVideoHeight( int height )
{
  const int vHeight = height > 0 ? height : QFontMetrics( font() ).height() * 12;
  mVideoWidget->setMinimumHeight( vHeight );
  mVideoWidget->setMaximumHeight( vHeight );
}

void QgsMediaWidget::adjustControls()
{
  switch ( mMode )
  {
    case Audio:
    {
      mVideoWidget->setVisible( false );
      break;
    }
    case Video:
    {
      mVideoWidget->setVisible( true );
      break;
    }
  }
}

void QgsMediaWidget::setControlsEnabled( bool enabled )
{
  mPlayButton->setEnabled( enabled );
  mPositionSlider->setEnabled( enabled );
}

void QgsMediaWidget::mediaStatusChanged( QMediaPlayer::MediaStatus status )
{
  switch ( status )
  {
    case QMediaPlayer::LoadedMedia:
    {
      setControlsEnabled( true );
      mPositionSlider->setMinimum( 0 );
      mPositionSlider->setMaximum( static_cast<int>( mMediaPlayer.duration() / 1000 ) );
      mPositionSlider->setValue( 0 );
      int seconds = std::floor( mMediaPlayer.duration() / 1000 );
      const int hours = std::floor( seconds / 3600 );
      seconds -= hours * 3600;
      const int minutes = std::floor( seconds / 60 );
      seconds -= minutes * 60;
      mDurationLabel->setText( QStringLiteral( "%1:%2:%3" ).arg( QString::number( hours ), 2, '0' ).arg( QString::number( minutes ), 2, '0' ).arg( QString::number( seconds ), 2, '0' ) );
      break;
    }

    case QMediaPlayer::EndOfMedia:
    {
      mPlayButton->setChecked( false );
      break;
    }

    case QMediaPlayer::LoadingMedia:
    case QMediaPlayer::StalledMedia:
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
    {
      break;
    }

    case QMediaPlayer::InvalidMedia:
    {
      setControlsEnabled( false );
      mDurationLabel->setText( tr( "invalid" ) );
      break;
    }

    case QMediaPlayer::NoMedia:
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
    case QMediaPlayer::UnknownMediaStatus:
#endif
    {
      setControlsEnabled( false );
      mDurationLabel->setText( QStringLiteral( "-" ) );
      break;
    }
  }
}
