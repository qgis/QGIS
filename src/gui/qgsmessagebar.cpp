/***************************************************************************
                          qgsmessagebar.cpp  -  description
                             -------------------
    begin                : June 2012
    copyright            : (C) 2012 by Giuseppe Sucameli
    email                : sucameli at faunalia dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsapplication.h"

#include <QWidget>
#include <QPalette>
#include <QStackedWidget>
#include <QProgressBar>
#include <QToolButton>
#include <QTimer>
#include <QGridLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QLabel>

QgsMessageBar::QgsMessageBar( QWidget *parent )
    : QFrame( parent ), mCurrentItem( NULL )
{
  QPalette pal = palette();
  pal.setBrush( backgroundRole(), pal.window() );
  setPalette( pal );
  setAutoFillBackground( true );
  setFrameShape( QFrame::StyledPanel );
  setFrameShadow( QFrame::Plain );

  mLayout = new QGridLayout( this );
  mLayout->setContentsMargins( 9, 1, 9, 1 );
  setLayout( mLayout );

  mCountProgress = new QProgressBar( this );
  mCountStyleSheet = QString( "QProgressBar { border: 1px solid rgba(0, 0, 0, 75%);"
                              " border-radius: 2px; background: rgba(0, 0, 0, 0);"
                              " image: url(:/images/themes/default/%1) }"
                              "QProgressBar::chunk { background-color: rgba(0, 0, 0, 30%); width: 5px; }" );

  mCountProgress->setStyleSheet( mCountStyleSheet.arg( "mIconTimerPause.png" ) );
  mCountProgress->setObjectName( "mCountdown" );
  mCountProgress->setFixedSize( 25, 14 );
  mCountProgress->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  mCountProgress->setTextVisible( false );
  mCountProgress->setRange( 0, 5 );
  mCountProgress->setHidden( true );
  mLayout->addWidget( mCountProgress, 0, 0, 1, 1 );

  mItemCount = new QLabel( this );
  mItemCount->setObjectName( "mItemCount" );
  mItemCount->setToolTip( tr( "Remaining messages" ) );
  mItemCount->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
  mLayout->addWidget( mItemCount, 0, 2, 1, 1 );

  mCloseMenu = new QMenu( this );
  mCloseMenu->setObjectName( "mCloseMenu" );
  mActionCloseAll = new QAction( tr( "Close all" ), this );
  mCloseMenu->addAction( mActionCloseAll );
  connect( mActionCloseAll, SIGNAL( triggered() ), this, SLOT( clearWidgets() ) );

  mCloseBtn = new QToolButton( this );
  mCloseMenu->setObjectName( "mCloseMenu" );
  mCloseBtn->setToolTip( tr( "Close" ) );
  mCloseBtn->setMinimumWidth( 40 );
  mCloseBtn->setStyleSheet(
    "QToolButton { background-color: rgba(255, 255, 255, 0); } "
    "QToolButton::menu-indicator { subcontrol-position: right bottom; subcontrol-origin: padding; bottom: 2px; }" );
  mCloseBtn->setCursor( Qt::PointingHandCursor );
  mCloseBtn->setIcon( QgsApplication::getThemeIcon( "/mIconClose.png" ) );
  mCloseBtn->setIconSize( QSize( 18, 18 ) );
  mCloseBtn->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );
  mCloseBtn->setMenu( mCloseMenu );
  connect( mCloseBtn, SIGNAL( clicked() ), this, SLOT( popWidget() ) );
  mLayout->addWidget( mCloseBtn, 0, 3, 1, 1 );

  mCountdownTimer = new QTimer( this );
  mCountdownTimer->setInterval( 1000 );
  connect( mCountdownTimer, SIGNAL( timeout() ), this, SLOT( updateCountdown() ) );

  connect( this, SIGNAL( widgetAdded( QgsMessageBarItem* ) ), this, SLOT( updateItemCount() ) );
  connect( this, SIGNAL( widgetRemoved( QgsMessageBarItem* ) ), this, SLOT( updateItemCount() ) );

  // start hidden
  setVisible( false );
}

QgsMessageBar::~QgsMessageBar()
{
}

void QgsMessageBar::mousePressEvent( QMouseEvent * e )
{
  if ( mCountProgress == childAt( e->pos() ) && e->button() == Qt::LeftButton )
  {
    if ( mCountdownTimer->isActive() )
    {
      mCountdownTimer->stop();
      mCountProgress->setStyleSheet( mCountStyleSheet.arg( "mIconTimerContinue.png" ) );
    }
    else
    {
      mCountdownTimer->start();
      mCountProgress->setStyleSheet( mCountStyleSheet.arg( "mIconTimerPause.png" ) );
    }
  }
}

void QgsMessageBar::popItem( QgsMessageBarItem *item )
{
  Q_ASSERT( item );

  if ( item != mCurrentItem && !mItems.contains( item ) )
    return;

  if ( item == mCurrentItem )
  {
    if ( mCurrentItem )
    {
      QWidget *widget = dynamic_cast<QWidget*>( mCurrentItem );
      mLayout->removeWidget( widget );
      mCurrentItem->hide();
      disconnect( mCurrentItem, SIGNAL( styleChanged( QString ) ), this, SLOT( setStyleSheet( QString ) ) );
      delete mCurrentItem;
      mCurrentItem = 0;
    }

    if ( !mItems.isEmpty() )
    {
      showItem( mItems.first() );
    }
    else
    {
      hide();
    }
  }
  else
  {
    mItems.removeOne( item );
  }

  emit widgetRemoved( item );
}

bool QgsMessageBar::popWidget( QgsMessageBarItem *item )
{
  if ( !item || !mCurrentItem )
    return false;

  if ( item == mCurrentItem )
  {
    popItem( mCurrentItem );
    return true;
  }

  foreach ( QgsMessageBarItem *existingItem, mItems )
  {
    if ( existingItem == item )
    {
      mItems.removeOne( existingItem );
      delete existingItem;
      return true;
    }
  }

  return false;
}

bool QgsMessageBar::popWidget()
{
  if ( !mCurrentItem )
    return false;

  resetCountdown();

  QgsMessageBarItem *item = mCurrentItem;
  popItem( item );

  return true;
}

bool QgsMessageBar::clearWidgets()
{
  if ( !mCurrentItem && mItems.empty() )
    return true;

  while ( mItems.count() > 0 )
  {
    popWidget();
  }
  popWidget();

  return !mCurrentItem && mItems.empty();
}

void QgsMessageBar::showItem( QgsMessageBarItem *item )
{
  Q_ASSERT( item );

  if ( mCurrentItem != 0 )
    disconnect( mCurrentItem, SIGNAL( styleChanged( QString ) ), this, SLOT( setStyleSheet( QString ) ) );

  if ( item == mCurrentItem )
    return;

  if ( mItems.contains( item ) )
    mItems.removeOne( item );

  if ( mCurrentItem )
  {
    mItems.prepend( mCurrentItem );
    mLayout->removeWidget( mCurrentItem );
    mCurrentItem->hide();
  }

  mCurrentItem = item;
  mLayout->addWidget( item, 0, 1, 1, 1 );
  mCurrentItem->show();

  if ( item->duration() > 0 )
  {
    mCountProgress->setRange( 0, item->duration() );
    mCountProgress->setValue( item->duration() );
    mCountProgress->setVisible( true );
    mCountdownTimer->start();
  }

  connect( mCurrentItem, SIGNAL( styleChanged( QString ) ), this, SLOT( setStyleSheet( QString ) ) );
  setStyleSheet( item->getStyleSheet() );
  show();

  emit widgetAdded( item );
}

void QgsMessageBar::pushItem( QgsMessageBarItem *item )
{
  resetCountdown();
  // avoid duplicated widget
  popWidget( item );
  showItem( item );
}

QgsMessageBarItem* QgsMessageBar::pushWidget( QWidget *widget , QgsMessageBar::MessageLevel level, int duration )
{
  QgsMessageBarItem *item;
  item = dynamic_cast<QgsMessageBarItem*>( widget );
  if ( item )
  {
    item->setLevel( level )->setDuration( duration );
  }
  else
  {
    item = new QgsMessageBarItem( widget, level, duration );
  }
  pushItem( item );
  return item;
}

void QgsMessageBar::pushMessage( const QString &title, const QString &text, QgsMessageBar::MessageLevel level, int duration )
{
  QgsMessageBarItem *item = new QgsMessageBarItem( title, text, level, duration );
  pushItem( item );
}

QgsMessageBarItem* QgsMessageBar::createMessage( const QString &text, QWidget *parent )
{
  QgsMessageBarItem* item = new QgsMessageBarItem( text, INFO, 0, parent );
  return item;
}

QgsMessageBarItem* QgsMessageBar::createMessage( const QString &title, const QString &text, QWidget *parent )
{
  return new QgsMessageBarItem( title, text, QgsMessageBar::INFO, 0, parent );
}

QgsMessageBarItem* QgsMessageBar::createMessage( QWidget *widget, QWidget *parent )
{
  return new QgsMessageBarItem( widget, INFO, 0, parent );
}

void QgsMessageBar::updateCountdown()
{
  if ( !mCountdownTimer->isActive() )
  {
    resetCountdown();
    return;
  }
  if ( mCountProgress->value() < 2 )
  {
    popWidget();
  }
  else
  {
    mCountProgress->setValue( mCountProgress->value() - 1 );
  }
}

void QgsMessageBar::resetCountdown()
{
  if ( mCountdownTimer->isActive() )
    mCountdownTimer->stop();

  mCountProgress->setStyleSheet( mCountStyleSheet.arg( "mIconTimerPause.png" ) );
  mCountProgress->setVisible( false );
}

void QgsMessageBar::updateItemCount()
{
  mItemCount->setText( mItems.count() > 0 ? tr( "%n more", "unread messages", mItems.count() ) : QString( "" ) );
}
