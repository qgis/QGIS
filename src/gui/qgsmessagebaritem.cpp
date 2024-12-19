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
#include "qgsgui.h"
#include "qgsnative.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QDesktopServices>
#include <QFileInfo>

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
    QString msgIcon( QStringLiteral( "/mIconInfo.svg" ) );
    switch ( mLevel )
    {
      case Qgis::MessageLevel::Critical:
        msgIcon = QStringLiteral( "/mIconCritical.svg" );
        break;
      case Qgis::MessageLevel::Warning:
        msgIcon = QStringLiteral( "/mIconWarning.svg" );
        break;
      case Qgis::MessageLevel::Success:
        msgIcon = QStringLiteral( "/mIconSuccess.svg" );
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
    contentStyleSheet = QStringLiteral( "<style> a, a:visited, a:hover { color:#268300; } </style>" );
  }
  else if ( mLevel == Qgis::MessageLevel::Critical )
  {
    mStyleSheet = QStringLiteral( "QgsMessageBar { background-color: #d65253; border: 1px solid #9b3d3d; } "
                                  "QLabel,QTextEdit { color: white; } " );
    contentStyleSheet = QStringLiteral( "<style>a, a:visited, a:hover { color:#4e0001; }</style>" );
  }
  else if ( mLevel == Qgis::MessageLevel::Warning )
  {
    mStyleSheet = QStringLiteral( "QgsMessageBar { background-color: #ffc800; border: 1px solid #e0aa00; } "
                                  "QLabel,QTextEdit { color: black; } " );
    contentStyleSheet = QStringLiteral( "<style>a, a:visited, a:hover { color:#945a00; }</style>" );
  }
  else if ( mLevel == Qgis::MessageLevel::Info )
  {
    mStyleSheet = QStringLiteral( "QgsMessageBar { background-color: #e7f5fe; border: 1px solid #b9cfe4; } "
                                  "QLabel,QTextEdit { color: #2554a1; } " );
    contentStyleSheet = QStringLiteral( "<style>a, a:visited, a:hover { color:#3bb2fe; }</style>" );
  }
  mStyleSheet += QLatin1String( "QLabel#mItemCount { font-style: italic; }" );

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
      mTextBrowser->setObjectName( QStringLiteral( "textEdit" ) );
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
      if ( !content.isEmpty() && !t.endsWith( ':' ) && !t.endsWith( QLatin1String( ": " ) ) )
        t += QLatin1String( ": " );
      content.prepend( QStringLiteral( "<b>" ) + t + " </b>" );
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
