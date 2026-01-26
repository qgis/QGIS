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

#include "qgsmessagebaritem.h"

#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgsmessagebar.h"
#include "qgsnative.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextBrowser>

#include "moc_qgsmessagebaritem.cpp"

QgsMessageBarItem::QgsMessageBarItem( const QString &text, Qgis::MessageLevel level, int duration, QWidget *parent )
  : QWidget( parent )
  , mText( text )
  , mLevel( level )
  , mDuration( duration )
{
  writeContent();
}

QgsMessageBarItem::QgsMessageBarItem( const QString &title, const QString &text, Qgis::MessageLevel level, int duration, QWidget *parent )
  : QWidget( parent )
  , mTitle( title )
  , mText( text )
  , mLevel( level )
  , mDuration( duration )
{
  writeContent();
}

QgsMessageBarItem::QgsMessageBarItem( const QString &title, const QString &text, QWidget *widget, Qgis::MessageLevel level, int duration, QWidget *parent )
  : QWidget( parent )
  , mTitle( title )
  , mText( text )
  , mLevel( level )
  , mDuration( duration )
  , mWidget( widget )
  , mUserIcon( QIcon() )

{
  writeContent();
}

QgsMessageBarItem::QgsMessageBarItem( QWidget *widget, Qgis::MessageLevel level, int duration, QWidget *parent )
  : QWidget( parent )
  , mLevel( level )
  , mDuration( duration )
  , mWidget( widget )
  , mUserIcon( QIcon() )

{
  writeContent();
}

void QgsMessageBarItem::writeContent()
{
  if ( mDuration < 0 )
    mDuration = QgsMessageBar::defaultMessageTimeout( mLevel );

  if ( !mLayout )
  {
    mLayout = new QHBoxLayout( this );
    mLayout->setContentsMargins( 0, 0, 0, 0 );
    mTextBrowser = nullptr;
    mLblIcon = nullptr;
  }

  // ICON
  if ( !mLblIcon )
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
    QString msgIcon( u"/mIconInfo.svg"_s );
    switch ( mLevel )
    {
      case Qgis::MessageLevel::Critical:
        msgIcon = u"/mIconCritical.svg"_s;
        break;
      case Qgis::MessageLevel::Warning:
        msgIcon = u"/mIconWarning.svg"_s;
        break;
      case Qgis::MessageLevel::Success:
        msgIcon = u"/mIconSuccess.svg"_s;
        break;
      default:
        break;
    }
    icon = QgsApplication::getThemeIcon( msgIcon );
  }
  const int iconSize = std::max( 24.0, fontMetrics().height() * 1.2 );
  mLblIcon->setPixmap( icon.pixmap( iconSize ) );


  // STYLESHEETS
  QString contentStyleSheet;
  if ( mLevel == Qgis::MessageLevel::Success )
  {
    mStyleSheet = QStringLiteral( "QgsMessageBar { background-color: #dff0d8; border: 1px solid #8e998a; } "
                                  "QLabel,QTextEdit { color: black; } " );
    contentStyleSheet = u"<style> a, a:visited, a:hover { color:#268300; } </style>"_s;
  }
  else if ( mLevel == Qgis::MessageLevel::Critical )
  {
    mStyleSheet = QStringLiteral( "QgsMessageBar { background-color: #d65253; border: 1px solid #9b3d3d; } "
                                  "QLabel,QTextEdit { color: white; } " );
    contentStyleSheet = u"<style>a, a:visited, a:hover { color:#4e0001; }</style>"_s;
  }
  else if ( mLevel == Qgis::MessageLevel::Warning )
  {
    mStyleSheet = QStringLiteral( "QgsMessageBar { background-color: #ffc800; border: 1px solid #e0aa00; } "
                                  "QLabel,QTextEdit { color: black; } " );
    contentStyleSheet = u"<style>a, a:visited, a:hover { color:#945a00; }</style>"_s;
  }
  else if ( mLevel == Qgis::MessageLevel::Info )
  {
    mStyleSheet = QStringLiteral( "QgsMessageBar { background-color: #e7f5fe; border: 1px solid #b9cfe4; } "
                                  "QLabel,QTextEdit { color: #2554a1; } " );
    contentStyleSheet = u"<style>a, a:visited, a:hover { color:#3bb2fe; }</style>"_s;
  }
  mStyleSheet += "QLabel#mItemCount { font-style: italic; }"_L1;

  // TITLE AND TEXT
  if ( mTitle.isEmpty() && mText.isEmpty() )
  {
    if ( mTextBrowser )
    {
      delete mTextBrowser;
      mTextBrowser = nullptr;
    }
  }
  else
  {
    if ( !mTextBrowser )
    {
      mTextBrowser = new QTextBrowser( this );
      mTextBrowser->setObjectName( u"textEdit"_s );
      mTextBrowser->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Maximum );
      mTextBrowser->setReadOnly( true );
      mTextBrowser->setOpenLinks( false );
      connect( mTextBrowser, &QTextBrowser::anchorClicked, this, &QgsMessageBarItem::urlClicked );

      mTextBrowser->setFrameShape( QFrame::NoFrame );
      // stylesheet set here so Qt-style substituted scrollbar arrows can show within limited height
      // adjusts to height of font set in app options
      mTextBrowser->setStyleSheet( "QTextEdit { background-color: rgba(0,0,0,0); margin-top: 0.25em; max-height: 1.75em; min-height: 1.75em; } "
                                   "QScrollBar { background-color: rgba(0,0,0,0); } "
                                   "QScrollBar::add-page,QScrollBar::sub-page,QScrollBar::handle { background-color: rgba(0,0,0,0); color: rgba(0,0,0,0); } "
                                   "QScrollBar::up-arrow,QScrollBar::down-arrow { color: rgb(0,0,0); } " );
      mLayout->addWidget( mTextBrowser );
    }
    QString content = mText;
    if ( !mTitle.isEmpty() )
    {
      // add ':' to end of title
      QString t = mTitle.trimmed();
      if ( !content.isEmpty() && !t.endsWith( ':' ) && !t.endsWith( ": "_L1 ) )
        t += ": "_L1;
      content.prepend( u"<b>"_s + t + " </b>" );
    }
    content.prepend( contentStyleSheet );
    mTextBrowser->setText( content );
  }

  // WIDGET
  if ( mWidget )
  {
    QLayoutItem *item = mLayout->itemAt( 2 );
    if ( !item || item->widget() != mWidget )
    {
      mLayout->addWidget( mWidget );
    }
  }
}

QgsMessageBarItem *QgsMessageBarItem::setText( const QString &text )
{
  mText = text;
  writeContent();
  return this;
}

QString QgsMessageBarItem::text() const
{
  return mText;
}

QgsMessageBarItem *QgsMessageBarItem::setTitle( const QString &title )
{
  mTitle = title;
  writeContent();
  return this;
}

QString QgsMessageBarItem::title() const
{
  return mTitle;
}

QgsMessageBarItem *QgsMessageBarItem::setLevel( Qgis::MessageLevel level )
{
  if ( level != mLevel )
  {
    mLevel = level;
    writeContent();
    emit styleChanged( mStyleSheet );
  }

  return this;
}

Qgis::MessageLevel QgsMessageBarItem::level() const
{
  return mLevel;
}

QgsMessageBarItem *QgsMessageBarItem::setWidget( QWidget *widget )
{
  if ( mWidget )
  {
    QLayoutItem *item = nullptr;
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

QWidget *QgsMessageBarItem::widget() const
{
  return mWidget;
}

QgsMessageBarItem *QgsMessageBarItem::setIcon( const QIcon &icon )
{
  mUserIcon = icon;
  return this;
}

QIcon QgsMessageBarItem::icon() const
{
  return mUserIcon;
}


QgsMessageBarItem *QgsMessageBarItem::setDuration( int duration )
{
  mDuration = duration;
  return this;
}

void QgsMessageBarItem::dismiss()
{
  if ( !mMessageBar )
    return;

  mMessageBar->popWidget( this );
}

void QgsMessageBarItem::urlClicked( const QUrl &url )
{
  const QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
  dismiss();
}
