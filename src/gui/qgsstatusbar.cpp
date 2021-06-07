/***************************************************************************
                         qgsstatusbar.cpp
                         ----------------
    begin                : May 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsstatusbar.h"
#include <QLayout>
#include <QLineEdit>
#include <QPalette>
#include <QTimer>
#include <QEvent>
#include <QStatusBar>

QgsStatusBar::QgsStatusBar( QWidget *parent )
  : QWidget( parent )
{
  mLayout = new QHBoxLayout();
  mLayout->setContentsMargins( 2, 0, 2, 0 );
  mLayout->setSpacing( 6 );

  mLineEdit = new QLineEdit( QString() );
  mLineEdit->setDisabled( true );
  mLineEdit->setFrame( false );
  mLineEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
  QPalette palette = mLineEdit->palette();
  palette.setColor( QPalette::Disabled, QPalette::Text, QPalette::WindowText );
  mLineEdit->setPalette( palette );
  mLineEdit->setStyleSheet( QStringLiteral( "* { border: 0; background-color: rgba(0, 0, 0, 0); }" ) );
  mLayout->addWidget( mLineEdit, 10 );
  setLayout( mLayout );
}

void QgsStatusBar::addPermanentWidget( QWidget *widget, int stretch, Anchor anchor )
{
  switch ( anchor )
  {
    case AnchorLeft:
      mLayout->insertWidget( 0, widget, stretch, Qt::AlignLeft );
      break;

    case AnchorRight:
      mLayout->addWidget( widget, stretch, Qt::AlignLeft );
      break;
  }
}

void QgsStatusBar::removeWidget( QWidget *widget )
{
  mLayout->removeWidget( widget );
}

QString QgsStatusBar::currentMessage() const
{
  return mLineEdit->text();
}

void QgsStatusBar::showMessage( const QString &text, int timeout )
{
  mLineEdit->setText( text );
  mLineEdit->setCursorPosition( 0 );
  if ( timeout > 0 )
  {
    if ( !mTempMessageTimer )
    {
      mTempMessageTimer = new QTimer( this );
      connect( mTempMessageTimer, &QTimer::timeout, this, &QgsStatusBar::clearMessage );
    }
    mTempMessageTimer->start( timeout );
  }
  else if ( mTempMessageTimer )
  {
    delete mTempMessageTimer;
    mTempMessageTimer = nullptr;
  }
}

void QgsStatusBar::clearMessage()
{
  mLineEdit->setText( QString() );
}

void QgsStatusBar::setParentStatusBar( QStatusBar *statusBar )
{
  if ( mParentStatusBar )
    mParentStatusBar->disconnect( mShowMessageConnection );

  mParentStatusBar = statusBar;

  if ( mParentStatusBar )
    mShowMessageConnection = connect( mParentStatusBar, &QStatusBar::messageChanged, this, [this]( const QString & message ) { showMessage( message ); } );
}

void QgsStatusBar::changeEvent( QEvent *event )
{
  QWidget::changeEvent( event );

  if ( event->type() == QEvent::FontChange )
  {
    mLineEdit->setFont( font() );
  }
}
