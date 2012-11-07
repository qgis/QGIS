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
#include <QToolButton>
#include <QGridLayout>
#include <QMenu>


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

  mItemCount = new QLabel( this );
  mItemCount->setObjectName( "mItemCount" );
  mItemCount->setToolTip( tr( "Remaining messages" ) );
  mItemCount->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
  mLayout->addWidget( mItemCount, 0, 1, 1, 1 );

  mCloseMenu = new QMenu( this );
  mCloseMenu->setObjectName( "mCloseMenu" );
  mActionCloseAll = new QAction( tr( "Close all" ), this );
  mCloseMenu->addAction( mActionCloseAll );
  connect( mActionCloseAll, SIGNAL( triggered() ), this, SLOT( clearWidgets() ) );

  mCloseBtn = new QToolButton( this );
  mCloseBtn->setToolTip( tr( "Close" ) );
  mCloseBtn->setMinimumWidth( 36 );
  mCloseBtn->setStyleSheet(
    "QToolButton { background-color: rgba(255, 255, 255, 0); } "
    "QToolButton::menu-indicator { subcontrol-position: right bottom; subcontrol-origin: padding; bottom: 6px; }" );
  mCloseBtn->setCursor( Qt::PointingHandCursor );
  mCloseBtn->setIcon( QgsApplication::getThemeIcon( "/mIconClose.png" ) );
  mCloseBtn->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
  mCloseBtn->setMenu( mCloseMenu );
  connect( mCloseBtn, SIGNAL( clicked() ), this, SLOT( popWidget() ) );
  mLayout->addWidget( mCloseBtn, 0, 2, 1, 1 );

  connect( this, SIGNAL( widgetAdded( QWidget* ) ), this, SLOT( updateItemCount() ) );
  connect( this, SIGNAL( widgetRemoved( QWidget* ) ), this, SLOT( updateItemCount() ) );

  // start hidden
  setVisible( false );
}

QgsMessageBar::~QgsMessageBar()
{
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

  QgsMessageBarItem *item = mCurrentItem;
  popItem( item );
  delete item;

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
  mLayout->addWidget( item->widget(), 0, 0, 1, 1 );
  mCurrentItem->widget()->show();

  setStyleSheet( item->styleSheet() );
  show();

  emit widgetAdded( item->widget() );
}

void QgsMessageBar::pushWidget( QWidget *widget, int level )
{
  QString stylesheet;
  if ( level >= 2 )
  {
    stylesheet = "QgsMessageBar { background-color: #d65253; border: 1px solid #9b3d3d; } "
                 "QLabel { color: white; } ";
  }
  else if ( level == 1 )
  {
    stylesheet = "QgsMessageBar { background-color: #ffc800; border: 1px solid #e0aa00; } "
                 "QLabel { color: black; } ";
  }
  else if ( level <= 0 )
  {
    stylesheet = "QgsMessageBar { background-color: #e7f5fe; border: 1px solid #b9cfe4; } "
                 "QLabel { color: #2554a1; } ";
  }
  stylesheet += "QLabel#mMsgTitle { font-weight: bold; } "
                "QLabel#mItemCount { font-style: italic; }";
  pushWidget( widget, stylesheet );
}

void QgsMessageBar::pushWidget( QWidget *widget, const QString &styleSheet )
{
  if ( !widget )
    return;

  // avoid duplicated widget
  popWidget( widget );

  pushItem( new QgsMessageBarItem( widget, styleSheet ) );
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

  if ( !title.isEmpty() )
  {
    QLabel *lblTitle = new QLabel( title, widget );
    lblTitle->setObjectName( "mMsgTitle" );
    layout->addWidget( lblTitle );
  }

  QLabel *lblText = new QLabel( text, widget );
  lblText->setObjectName( "mMsgText" );
  lblText->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  lblText->setWordWrap( true );
  layout->addWidget( lblText );

  return widget;
}

void QgsMessageBar::updateItemCount()
{
  mItemCount->setText( mList.count() > 0 ? QString::number( mList.count() ) + QString( " " ) + tr( "more" ) : QString( "" ) );
}
