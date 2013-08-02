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
#include "qgsapplication.h"

#include <QWidget>
#include <QPalette>
#include <QStackedWidget>
#include <QLabel>
#include <QProgressBar>
#include <QToolButton>
#include <QTimer>
#include <QGridLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QTextEdit>


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
  mCountProgress->setObjectName( "mCountProgress" );
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

  connect( this, SIGNAL( widgetAdded( QWidget* ) ), this, SLOT( updateItemCount() ) );
  connect( this, SIGNAL( widgetRemoved( QWidget* ) ), this, SLOT( updateItemCount() ) );

  // start hidden
  setVisible( false );
}

QgsMessageBar::~QgsMessageBar()
{
}

void QgsMessageBar::mousePressEvent( QMouseEvent * e )
{
  // stop/start mCountdownTimer
  QProgressBar *pb = static_cast<QProgressBar *>( childAt( e->pos() ) );
  if ( pb && pb->objectName() == QString( "mCountdown" ) && e->button() == Qt::LeftButton )
  {
    if ( mCountdownTimer->isActive() )
    {
      mCountdownTimer->stop();
      pb->setStyleSheet( mCountStyleSheet.arg( "mIconTimerContinue.png" ) );
    }
    else
    {
      mCountdownTimer->start();
      pb->setStyleSheet( mCountStyleSheet.arg( "mIconTimerPause.png" ) );
    }
  }
}

void QgsMessageBar::popItem( QgsMessageBarItem *item )
{
  Q_ASSERT( item );

  if ( item != mCurrentItem && !mList.contains( item ) )
    return;

  if ( item == mCurrentItem )
  {
    if ( mCurrentItem )
    {
      mLayout->removeWidget( mCurrentItem->widget() );
      mCurrentItem->widget()->hide();
      if ( mCurrentItem->widget()->parent() == this )
      {
        delete mCurrentItem->widget();
      }
      delete mCurrentItem;
      mCurrentItem = 0;
    }

    if ( !mList.isEmpty() )
    {
      pushItem( mList.first() );
    }
    else
    {
      hide();
    }
  }
  else
  {
    mList.removeOne( item );
  }

  emit widgetRemoved( item->widget() );
}

bool QgsMessageBar::popWidget( QWidget *widget )
{
  if ( !widget || !mCurrentItem )
    return false;

  if ( widget == mCurrentItem->widget() )
  {
    popItem( mCurrentItem );
    return true;
  }

  foreach ( QgsMessageBarItem *item, mList )
  {
    if ( item->widget() == widget )
    {
      mList.removeOne( item );
      if ( item->widget()->parent() == this )
      {
        delete item->widget();
      }
      delete item;
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
  if ( !mCurrentItem && mList.empty() )
    return true;

  while ( mList.count() > 0 )
  {
    popWidget();
  }
  popWidget();

  return !mCurrentItem && mList.empty();
}

void QgsMessageBar::pushItem( QgsMessageBarItem *item )
{
  Q_ASSERT( item );

  if ( item == mCurrentItem )
    return;

  if ( mList.contains( item ) )
    mList.removeOne( item );

  if ( mCurrentItem )
  {
    mList.prepend( mCurrentItem );
    mLayout->removeWidget( mCurrentItem->widget() );
    mCurrentItem->widget()->hide();
  }

  mCurrentItem = item;
  mLayout->addWidget( item->widget(), 0, 1, 1, 1 );
  mCurrentItem->widget()->show();

  if ( item->duration() > 0 )
  {
    mCountProgress->setRange( 0, item->duration() );
    mCountProgress->setValue( item->duration() );
    mCountProgress->setVisible( true );
    mCountdownTimer->start();
  }

  setStyleSheet( item->styleSheet() );
  show();

  emit widgetAdded( item->widget() );
}

void QgsMessageBar::pushWidget( QWidget *widget, MessageLevel level, int duration )
{
  resetCountdown();

  QString stylesheet;
  if ( level >= CRITICAL )
  {
    stylesheet = "QgsMessageBar { background-color: #d65253; border: 1px solid #9b3d3d; } "
                 "QLabel,QTextEdit { color: white; } ";
  }
  else if ( level == WARNING )
  {
    stylesheet = "QgsMessageBar { background-color: #ffc800; border: 1px solid #e0aa00; } "
                 "QLabel,QTextEdit { color: black; } ";
  }
  else if ( level <= INFO )
  {
    stylesheet = "QgsMessageBar { background-color: #e7f5fe; border: 1px solid #b9cfe4; } "
                 "QLabel,QTextEdit { color: #2554a1; } ";
  }
  stylesheet += "QLabel#mItemCount { font-style: italic; }";
  pushWidget( widget, stylesheet, duration );
}

void QgsMessageBar::pushWidget( QWidget *widget, const QString &styleSheet, int duration )
{
  if ( !widget )
    return;

  // avoid duplicated widget
  popWidget( widget );

  pushItem( new QgsMessageBarItem( widget, styleSheet, duration ) );
}

QWidget* QgsMessageBar::createMessage( const QString &title, const QString &text, const QIcon &icon, QWidget *parent )
{
  QWidget *widget = new QWidget( parent );

  QHBoxLayout *layout = new QHBoxLayout( widget );
  layout->setContentsMargins( 0, 0, 0, 0 );

  if ( !icon.isNull() )
  {
    QLabel *lblIcon = new QLabel( widget );
    lblIcon->setPixmap( icon.pixmap( 24 ) );
    layout->addWidget( lblIcon );
  }

  QTextEdit *msgText = new QTextEdit( widget );
  msgText->setObjectName( "mMsgText" );
  QString content = text;
  if ( !title.isEmpty() )
  {
    // add ':' to end of title
    QString t = title.trimmed();
    if ( !t.endsWith( ":" ) )
      t += ": ";
    content.prepend( QString( "<b>" ) + t + "</b>" );
  }
  msgText->setText( content );
  msgText->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
  msgText->setReadOnly( true );
  msgText->setFrameShape( QFrame::NoFrame );
  // stylesheet set here so Qt-style substitued scrollbar arrows can show within limited height
  // adjusts to height of font set in app options
  msgText->setStyleSheet( "* { background-color: rgba(0,0,0,0); margin-top: 0.25em; max-height: 1.75em; min-height: 1.75em; } "
                          "QScrollBar::add-page,QScrollBar::sub-page,QScrollBar::handle { background-color: rgba(0,0,0,0); color: rgba(0,0,0,0); }" );
  layout->addWidget( msgText );

  return widget;
}

void QgsMessageBar::pushMessage( const QString &title, const QString &text, MessageLevel level, int duration )
{
  QString msgIcon( "/mIconInfo.png" );
  switch ( level )
  {
    case QgsMessageBar::CRITICAL:
      msgIcon = QString( "/mIconCritical.png" );
      break;
    case QgsMessageBar::WARNING:
      msgIcon = QString( "/mIconWarn.png" );
      break;
    default:
      break;
  }

  QWidget *msg = QgsMessageBar::createMessage( title, text, QgsApplication::getThemeIcon( msgIcon ), this );
  pushWidget( msg, level, duration );
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
  mItemCount->setText( mList.count() > 0 ? tr( "%n more", "unread messages", mList.count() ) : QString( "" ) );
}
