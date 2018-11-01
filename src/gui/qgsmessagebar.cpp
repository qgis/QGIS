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
#include "qgsmessagelog.h"
#include "qgsmessageviewer.h"

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
  : QFrame( parent )

{
  QPalette pal = palette();
  pal.setBrush( backgroundRole(), pal.window() );
  setPalette( pal );
  setAutoFillBackground( true );
  setFrameShape( QFrame::StyledPanel );
  setFrameShadow( QFrame::Plain );

  mLayout = new QGridLayout( this );
  const int xMargin = std::max( 9.0, Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 0.45 );
  const int yMargin = std::max( 1.0, Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 0.05 );
  mLayout->setContentsMargins( xMargin, yMargin, xMargin, yMargin );
  setLayout( mLayout );

  mCountProgress = new QProgressBar( this );
  mCountStyleSheet = QString( "QProgressBar { border: 1px solid rgba(0, 0, 0, 75%);"
                              " border-radius: 2px; background: rgba(0, 0, 0, 0);"
                              " image: url(:/images/themes/default/%1) }"
                              "QProgressBar::chunk { background-color: rgba(0, 0, 0, 30%); width: 5px; }" );

  mCountProgress->setStyleSheet( mCountStyleSheet.arg( QStringLiteral( "mIconTimerPause.svg" ) ) );
  mCountProgress->setObjectName( QStringLiteral( "mCountdown" ) );
  const int barWidth = std::max( 25.0, Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 1.25 );
  const int barHeight = std::max( 14.0, Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 0.7 );
  mCountProgress->setFixedSize( barWidth, barHeight );
  mCountProgress->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
  mCountProgress->setTextVisible( false );
  mCountProgress->setRange( 0, 5 );
  mCountProgress->setHidden( true );
  mLayout->addWidget( mCountProgress, 0, 0, 1, 1 );

  mItemCount = new QLabel( this );
  mItemCount->setObjectName( QStringLiteral( "mItemCount" ) );
  mItemCount->setToolTip( tr( "Remaining messages" ) );
  mItemCount->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
  mLayout->addWidget( mItemCount, 0, 2, 1, 1 );

  mCloseMenu = new QMenu( this );
  mCloseMenu->setObjectName( QStringLiteral( "mCloseMenu" ) );
  mActionCloseAll = new QAction( tr( "Close All" ), this );
  mCloseMenu->addAction( mActionCloseAll );
  connect( mActionCloseAll, &QAction::triggered, this, &QgsMessageBar::clearWidgets );

  mCloseBtn = new QToolButton( this );
  mCloseMenu->setObjectName( QStringLiteral( "mCloseMenu" ) );
  mCloseBtn->setToolTip( tr( "Close" ) );
  mCloseBtn->setMinimumWidth( 40 );
  mCloseBtn->setStyleSheet(
    "QToolButton { background-color: rgba(0, 0, 0, 0); }"
    "QToolButton::menu-button { background-color: rgba(0, 0, 0, 0); }" );
  mCloseBtn->setCursor( Qt::PointingHandCursor );
  mCloseBtn->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconClose.svg" ) ) );

  const int iconSize = std::max( 18.0, Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 0.9 );
  mCloseBtn->setIconSize( QSize( iconSize, iconSize ) );
  mCloseBtn->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );
  mCloseBtn->setMenu( mCloseMenu );
  mCloseBtn->setPopupMode( QToolButton::MenuButtonPopup );
  connect( mCloseBtn, &QAbstractButton::clicked, this, static_cast < bool ( QgsMessageBar::* )() > ( &QgsMessageBar::popWidget ) );
  mLayout->addWidget( mCloseBtn, 0, 3, 1, 1 );

  mCountdownTimer = new QTimer( this );
  mCountdownTimer->setInterval( 1000 );
  connect( mCountdownTimer, &QTimer::timeout, this, &QgsMessageBar::updateCountdown );

  connect( this, &QgsMessageBar::widgetAdded, this, &QgsMessageBar::updateItemCount );
  connect( this, &QgsMessageBar::widgetRemoved, this, &QgsMessageBar::updateItemCount );

  // start hidden
  setVisible( false );
}

void QgsMessageBar::mousePressEvent( QMouseEvent *e )
{
  if ( mCountProgress == childAt( e->pos() ) && e->button() == Qt::LeftButton )
  {
    if ( mCountdownTimer->isActive() )
    {
      mCountdownTimer->stop();
      mCountProgress->setStyleSheet( mCountStyleSheet.arg( QStringLiteral( "mIconTimerContinue.svg" ) ) );
    }
    else
    {
      mCountdownTimer->start();
      mCountProgress->setStyleSheet( mCountStyleSheet.arg( QStringLiteral( "mIconTimerPause.svg" ) ) );
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
      QWidget *widget = mCurrentItem;
      mLayout->removeWidget( widget );
      mCurrentItem->hide();
      disconnect( mCurrentItem, &QgsMessageBarItem::styleChanged, this, &QWidget::setStyleSheet );
      mCurrentItem->deleteLater();
      mCurrentItem = nullptr;
    }

    if ( !mItems.isEmpty() )
    {
      showItem( mItems.at( 0 ) );
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

  Q_FOREACH ( QgsMessageBarItem *existingItem, mItems )
  {
    if ( existingItem == item )
    {
      mItems.removeOne( existingItem );
      existingItem->deleteLater();
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

  while ( !mItems.isEmpty() )
  {
    popWidget();
  }
  popWidget();

  return !mCurrentItem && mItems.empty();
}

void QgsMessageBar::pushSuccess( const QString &title, const QString &message )
{
  pushMessage( title, message, Qgis::Success );
}

void QgsMessageBar::pushInfo( const QString &title, const QString &message )
{
  pushMessage( title, message, Qgis::Info );
}

void QgsMessageBar::pushWarning( const QString &title, const QString &message )
{
  pushMessage( title, message, Qgis::Warning );
}

void QgsMessageBar::pushCritical( const QString &title, const QString &message )
{
  pushMessage( title, message, Qgis::Critical );
}

void QgsMessageBar::showItem( QgsMessageBarItem *item )
{
  Q_ASSERT( item );

  if ( mCurrentItem )
    disconnect( mCurrentItem, &QgsMessageBarItem::styleChanged, this, &QWidget::setStyleSheet );

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

  connect( mCurrentItem, &QgsMessageBarItem::styleChanged, this, &QWidget::setStyleSheet );
  setStyleSheet( item->getStyleSheet() );
  show();

  emit widgetAdded( item );
}

void QgsMessageBar::pushItem( QgsMessageBarItem *item )
{
  resetCountdown();

  item->mMessageBar = this;

  // avoid duplicated widget
  popWidget( item );
  showItem( item );

  // Log all messages that are sent to the message bar into the message log so the
  // user can get them back easier.
  QString formattedTitle = QStringLiteral( "%1 : %2" ).arg( item->title(), item->text() );
  QgsMessageLog::logMessage( formattedTitle, tr( "Messages" ), item->level() );
}

QgsMessageBarItem *QgsMessageBar::pushWidget( QWidget *widget, Qgis::MessageLevel level, int duration )
{
  QgsMessageBarItem *item = nullptr;
  item = dynamic_cast<QgsMessageBarItem *>( widget );
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

void QgsMessageBar::pushMessage( const QString &title, const QString &text, Qgis::MessageLevel level, int duration )
{
  // keep the number of items in the message bar to a maximum of 20, avoids flooding (and freezing) of the main window
  if ( mItems.count() > 20 )
    mItems.removeFirst();

  // block duplicate items, avoids flooding (and freezing) of the main window
  for ( auto it = mItems.constBegin(); it != mItems.constEnd(); ++it )
  {
    if ( level == ( *it )->level() && title == ( *it )->title() && text == ( *it )->text() )
      return;
  }

  QgsMessageBarItem *item = new QgsMessageBarItem( title, text, level, duration );
  pushItem( item );
}

void QgsMessageBar::pushMessage( const QString &title, const QString &text, const QString &showMore, Qgis::MessageLevel level, int duration )
{
  QgsMessageViewer *mv = new QgsMessageViewer();
  mv->setWindowTitle( title );
  mv->setMessageAsPlainText( text + "\n\n" + showMore );

  QToolButton *showMoreButton = new QToolButton();
  QAction *act = new QAction( showMoreButton );
  act->setText( tr( "Show more" ) );
  showMoreButton->setStyleSheet( QStringLiteral( "background-color: rgba(255, 255, 255, 0); color: black; text-decoration: underline;" ) );
  showMoreButton->setCursor( Qt::PointingHandCursor );
  showMoreButton->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
  showMoreButton->addAction( act );
  showMoreButton->setDefaultAction( act );
  connect( showMoreButton, &QToolButton::triggered, mv, &QDialog::exec );
  connect( showMoreButton, &QToolButton::triggered, showMoreButton, &QObject::deleteLater );

  QgsMessageBarItem *item = new QgsMessageBarItem(
    title,
    text,
    showMoreButton,
    level,
    duration );
  pushItem( item );
}

QgsMessageBarItem *QgsMessageBar::createMessage( const QString &text, QWidget *parent )
{
  QgsMessageBarItem *item = new QgsMessageBarItem( text, Qgis::Info, 0, parent );
  return item;
}

QgsMessageBarItem *QgsMessageBar::createMessage( const QString &title, const QString &text, QWidget *parent )
{
  return new QgsMessageBarItem( title, text, Qgis::Info, 0, parent );
}

QgsMessageBarItem *QgsMessageBar::createMessage( QWidget *widget, QWidget *parent )
{
  return new QgsMessageBarItem( widget, Qgis::Info, 0, parent );
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

  mCountProgress->setStyleSheet( mCountStyleSheet.arg( QStringLiteral( "mIconTimerPause.svg" ) ) );
  mCountProgress->setVisible( false );
}

void QgsMessageBar::updateItemCount()
{
  mItemCount->setText( !mItems.isEmpty() ? tr( "%n more", "unread messages", mItems.count() ) : QString() );

  // do not show the down arrow for opening menu with "close all" if there is just one message
  mCloseBtn->setMenu( !mItems.isEmpty() ? mCloseMenu : nullptr );
  mCloseBtn->setPopupMode( !mItems.isEmpty() ? QToolButton::MenuButtonPopup : QToolButton::DelayedPopup );
}
