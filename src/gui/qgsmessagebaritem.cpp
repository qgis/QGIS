/***************************************************************************
                          qgsmessagebaritem.h  -  description
                             -------------------
    begin                : August 2013
    copyright            : (C) 2013 by Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsmessagebaritem.h"
#include "qgsmessagebar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>

QgsMessageBarItem::QgsMessageBarItem( const QString &text, QgsMessageBar::MessageLevel level, int duration, QWidget *parent ) :
    QWidget( parent )
    , mTitle( "" )
    , mText( text )
    , mLevel( level )
    , mDuration( duration )
    , mWidget( 0 )
    , mUserIcon( QIcon() )
    , mLayout( 0 )
{
  writeContent();
}

QgsMessageBarItem::QgsMessageBarItem( const QString &title, const QString &text, QgsMessageBar::MessageLevel level, int duration , QWidget *parent ) :
    QWidget( parent )
    , mTitle( title )
    , mText( text )
    , mLevel( level )
    , mDuration( duration )
    , mWidget( 0 )
    , mUserIcon( QIcon() )
    , mLayout( 0 )
{
  writeContent();
}

QgsMessageBarItem::QgsMessageBarItem( const QString &title, const QString &text, QWidget *widget, QgsMessageBar::MessageLevel level, int duration, QWidget *parent ) :
    QWidget( parent )
    , mTitle( title )
    , mText( text )
    , mLevel( level )
    , mDuration( duration )
    , mWidget( widget )
    , mUserIcon( QIcon() )
    , mLayout( 0 )
{
  writeContent();
}

QgsMessageBarItem::QgsMessageBarItem( QWidget *widget, QgsMessageBar::MessageLevel level, int duration, QWidget *parent ) :
    QWidget( parent )
    , mTitle( "" )
    , mText( "" )
    , mLevel( level )
    , mDuration( duration )
    , mWidget( widget )
    , mUserIcon( QIcon() )
    , mLayout( 0 )
{
  writeContent();
}

QgsMessageBarItem::~QgsMessageBarItem()
{
}

void QgsMessageBarItem::writeContent()
{
  if ( mLayout == 0 )
  {
    mLayout = new QHBoxLayout( this );
    mLayout->setContentsMargins( 0, 0, 0, 0 );
    mTextEdit = 0;
    mLblIcon = 0;
  }

  // ICON
  if ( mLblIcon == 0 )
  {
    mLblIcon = new QLabel( this );
    mLayout->addWidget( mLblIcon );
  }
  QIcon icon;
  if ( !mUserIcon.isNull() )
  {
    icon = mUserIcon;
  }
  else
  {
    QString msgIcon( "/mIconInfo.png" );
    switch ( mLevel )
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
    icon = QgsApplication::getThemeIcon( msgIcon );
  }
  mLblIcon->setPixmap( icon.pixmap( 24 ) );

  // TITLE AND TEXT
  if ( mTitle.isEmpty() && mText.isEmpty() )
  {
    if ( mTextEdit != 0 )
    {
      delete mTextEdit;
      mTextEdit = 0;
    }
  }
  else
  {
    if ( mTextEdit == 0 )
    {
      mTextEdit = new QTextEdit( this );
      mTextEdit->setObjectName( "textEdit" );
      mTextEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
      mTextEdit->setReadOnly( true );
      mTextEdit->setFrameShape( QFrame::NoFrame );
      // stylesheet set here so Qt-style substitued scrollbar arrows can show within limited height
      // adjusts to height of font set in app options
      mTextEdit->setStyleSheet( "QTextEdit { background-color: rgba(0,0,0,0); margin-top: 0.25em; max-height: 1.75em; min-height: 1.75em; } "
                                "QScrollBar { background-color: rgba(0,0,0,0); } "
                                "QScrollBar::add-page,QScrollBar::sub-page,QScrollBar::handle { background-color: rgba(0,0,0,0); color: rgba(0,0,0,0); } "
                                "QScrollBar::up-arrow,QScrollBar::down-arrow { color: rgb(0,0,0); } " );
      mLayout->addWidget( mTextEdit );
    }
    QString content = mText;
    if ( !mTitle.isEmpty() )
    {
      // add ':' to end of title
      QString t = mTitle.trimmed();
      if ( !content.isEmpty() && !t.endsWith( ":" ) && !t.endsWith( ": " ) )
        t += ": ";
      content.prepend( QString( "<b>" ) + t + " </b>" );
    }
    mTextEdit->setText( content );
  }

  // WIDGET
  if ( mWidget != 0 )
  {
    QLayoutItem *item = mLayout->itemAt( 2 );
    if ( !item || item->widget() != mWidget )
    {
      mLayout->addWidget( mWidget );
    }
  }

  // STYLESHEET
  if ( mLevel >= QgsMessageBar::CRITICAL )
  {
    mStyleSheet = "QgsMessageBar { background-color: #d65253; border: 1px solid #9b3d3d; } "
                  "QLabel,QTextEdit { color: white; } ";
  }
  else if ( mLevel == QgsMessageBar::WARNING )
  {
    mStyleSheet = "QgsMessageBar { background-color: #ffc800; border: 1px solid #e0aa00; } "
                  "QLabel,QTextEdit { color: black; } ";
  }
  else if ( mLevel <= QgsMessageBar::INFO )
  {
    mStyleSheet = "QgsMessageBar { background-color: #e7f5fe; border: 1px solid #b9cfe4; } "
                  "QLabel,QTextEdit { color: #2554a1; } ";
  }
  mStyleSheet += "QLabel#mItemCount { font-style: italic; }";
}

QgsMessageBarItem* QgsMessageBarItem::setText( QString text )
{
  mText = text;
  writeContent();
  return this;
}

QgsMessageBarItem *QgsMessageBarItem::setTitle( QString title )
{
  mTitle = title;
  writeContent();
  return this;
}

QgsMessageBarItem *QgsMessageBarItem::setLevel( QgsMessageBar::MessageLevel level )
{
  mLevel = level;
  writeContent();
  emit styleChanged( mStyleSheet );
  return this;
}

QgsMessageBarItem *QgsMessageBarItem::setWidget( QWidget *widget )
{
  if ( mWidget != 0 )
  {
    QLayoutItem *item;
    item = mLayout->itemAt( 2 );
    if ( item->widget() == mWidget )
    {
      delete item->widget();
    }
  }
  mWidget = widget;
  writeContent();
  return this;
}

QgsMessageBarItem* QgsMessageBarItem::setIcon( const QIcon &icon )
{
  mUserIcon = icon;
  return this;
}


QgsMessageBarItem* QgsMessageBarItem::setDuration( int duration )
{
  mDuration = duration;
  return this;
}

