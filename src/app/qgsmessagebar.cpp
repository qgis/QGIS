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


QgsMessageBar::QgsMessageBar( QWidget *parent )
    : QFrame( parent ), mCurrentItem ( NULL )
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

  mLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding ), 0, 1, 1, 1 );

  mCloseBtn = new QToolButton( this );
  mCloseBtn->setText( tr( "" ) );
  mCloseBtn->setStyleSheet( "background-color: rgba(255, 255, 255, 0);" );
  mCloseBtn->setCursor( Qt::PointingHandCursor );
  mCloseBtn->setIcon( QgsApplication::getThemeIcon( "/mIconClose.png" ) );
  connect( mCloseBtn, SIGNAL( clicked() ), this, SLOT( popWidget() ) );
  mLayout->addWidget( mCloseBtn, 0, 2, 1, 1 );

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

  foreach( QgsMessageBarItem *item, mList )
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
}

void QgsMessageBar::pushWidget( QWidget *widget, int level )
{
  QString stylesheet;
  if ( level >= 2 )
  {
    stylesheet = "QgsMessageBar { background-color: #d65253; border: 1px solid #9b3d3d; } QLabel { color: white; }";
  }
  else if ( level == 1 )
  {
    stylesheet = "QgsMessageBar { background-color: #ffc800; border: 1px solid #e0aa00; } QLabel { color: black; }";
  }
  else if ( level == 0 )
  {
    stylesheet = "QgsMessageBar { background-color: #e7f5fe; border: 1px solid #b9cfe4; } QLabel { color: #2554a1; }";
  }
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
    QFont font = lblTitle->font();
    font.setBold( true );
    lblTitle->setFont( font );
    layout->addWidget( lblTitle );
  }

  QLabel *lblText = new QLabel( text, widget );
  layout->addWidget( lblText );

  return widget;
}
