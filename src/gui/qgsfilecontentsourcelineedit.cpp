/***************************************************************************
 qgsfilecontentsourcelineedit.cpp
 -----------------------
 begin                : July 2018
 copyright            : (C) 2018 by Nyall Dawson
 email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfilecontentsourcelineedit.h"
#include "qgssettings.h"
#include "qgsmessagebar.h"
#include "qgsfilterlineedit.h"
#include "qgspropertyoverridebutton.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QImageReader>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QToolButton>
#include <QUrl>
#include <QMovie>

//
// QgsAbstractFileContentSourceLineEdit
//

QgsAbstractFileContentSourceLineEdit::QgsAbstractFileContentSourceLineEdit( QWidget *parent )
  : QWidget( parent )
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  mFileLineEdit = new QgsFilterLineEdit( this );
  mFileLineEdit->setShowClearButton( true );
  mFileToolButton = new QToolButton( this );
  mFileToolButton->setText( QString( QChar( 0x2026 ) ) );
  mPropertyOverrideButton = new QgsPropertyOverrideButton( this );
  layout->addWidget( mFileLineEdit, 1 );
  layout->addWidget( mFileToolButton );
  layout->addWidget( mPropertyOverrideButton );
  setLayout( layout );

  QMenu *sourceMenu = new QMenu( mFileToolButton );

  QAction *selectFileAction = new QAction( tr( "Select File…" ), sourceMenu );
  connect( selectFileAction, &QAction::triggered, this, &QgsAbstractFileContentSourceLineEdit::selectFile );
  sourceMenu->addAction( selectFileAction );

  QAction *embedFileAction = new QAction( tr( "Embed File…" ), sourceMenu );
  connect( embedFileAction, &QAction::triggered, this, &QgsAbstractFileContentSourceLineEdit::embedFile );
  sourceMenu->addAction( embedFileAction );

  QAction *extractFileAction = new QAction( tr( "Extract Embedded File…" ), sourceMenu );
  connect( extractFileAction, &QAction::triggered, this, &QgsAbstractFileContentSourceLineEdit::extractFile );
  sourceMenu->addAction( extractFileAction );

  connect( sourceMenu, &QMenu::aboutToShow, this, [this, extractFileAction]
  {
    extractFileAction->setEnabled( mMode == ModeBase64 );
  } );

  QAction *enterUrlAction = new QAction( tr( "From URL…" ), sourceMenu );
  connect( enterUrlAction, &QAction::triggered, this, &QgsAbstractFileContentSourceLineEdit::selectUrl );
  sourceMenu->addAction( enterUrlAction );

  mFileToolButton->setMenu( sourceMenu );
  mFileToolButton->setPopupMode( QToolButton::MenuButtonPopup );
  connect( mFileToolButton, &QToolButton::clicked, this, &QgsAbstractFileContentSourceLineEdit::selectFile );

  connect( mFileLineEdit, &QLineEdit::textEdited, this, &QgsAbstractFileContentSourceLineEdit::mFileLineEdit_textEdited );
  connect( mFileLineEdit, &QgsFilterLineEdit::cleared, this, [ = ]
  {
    mMode = ModeFile;
    mFileLineEdit->setPlaceholderText( QString() );
    mBase64.clear();
    emit sourceChanged( QString() );
  } );

  mPropertyOverrideButton->setVisible( mPropertyOverrideButtonVisible );

}

QString QgsAbstractFileContentSourceLineEdit::source() const
{
  switch ( mMode )
  {
    case ModeFile:
      return mFileLineEdit->text();

    case ModeBase64:
      return mBase64;
  }

  return QString();
}

void QgsAbstractFileContentSourceLineEdit::setLastPathSettingsKey( const QString &key )
{
  mLastPathKey = key;
}

void QgsAbstractFileContentSourceLineEdit::setPropertyOverrideToolButtonVisible( bool visible )
{
  mPropertyOverrideButtonVisible = visible;
  mPropertyOverrideButton->setVisible( visible );
}

void QgsAbstractFileContentSourceLineEdit::setSource( const QString &source )
{
  const bool isBase64 = source.startsWith( QLatin1String( "base64:" ), Qt::CaseInsensitive );

  if ( ( !isBase64 && source == mFileLineEdit->text() && mBase64.isEmpty() ) || ( isBase64 && source == mBase64 ) )
    return;

  if ( isBase64 )
  {
    mMode = ModeBase64;
    mBase64 = source;
    mFileLineEdit->clear();
    mFileLineEdit->setPlaceholderText( tr( "Embedded file" ) );
  }
  else
  {
    mMode = ModeFile;
    mBase64.clear();
    mFileLineEdit->setText( source );
    mFileLineEdit->setPlaceholderText( QString() );
  }

  emit sourceChanged( source );
}

void QgsAbstractFileContentSourceLineEdit::selectFile()
{
  QgsSettings s;
  const QString file = QFileDialog::getOpenFileName( nullptr,
                       selectFileTitle(),
                       defaultPath(),
                       fileFilter() );
  const QFileInfo fi( file );
  if ( file.isEmpty() || !fi.exists() || file == source() )
  {
    return;
  }
  mMode = ModeFile;
  mBase64.clear();
  mFileLineEdit->setText( file );
  mFileLineEdit->setPlaceholderText( QString() );
  s.setValue( settingsKey(), fi.absolutePath() );
  emit sourceChanged( mFileLineEdit->text() );
}

void QgsAbstractFileContentSourceLineEdit::selectUrl()
{
  bool ok = false;
  const QString path = QInputDialog::getText( this, fileFromUrlTitle(), fileFromUrlText(), QLineEdit::Normal, mFileLineEdit->text(), &ok );
  if ( ok && path != source() )
  {
    mMode = ModeFile;
    mBase64.clear();
    mFileLineEdit->setText( path );
    mFileLineEdit->setPlaceholderText( QString() );
    emit sourceChanged( mFileLineEdit->text() );
  }
}

void QgsAbstractFileContentSourceLineEdit::embedFile()
{
  QgsSettings s;
  const QString file = QFileDialog::getOpenFileName( nullptr,
                       embedFileTitle(),
                       defaultPath(),
                       fileFilter() );
  const QFileInfo fi( file );
  if ( file.isEmpty() || !fi.exists() )
  {
    return;
  }

  s.setValue( settingsKey(), fi.absolutePath() );

  // encode file as base64
  QFile fileSource( file );
  if ( !fileSource.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  const QByteArray blob = fileSource.readAll();
  const QByteArray encoded = blob.toBase64();

  QString path( encoded );
  path.prepend( QLatin1String( "base64:" ) );
  if ( path == source() )
    return;

  mBase64 = path;
  mMode = ModeBase64;

  mFileLineEdit->clear();
  mFileLineEdit->setPlaceholderText( tr( "Embedded file" ) );

  emit sourceChanged( path );
}

void QgsAbstractFileContentSourceLineEdit::extractFile()
{
  QgsSettings s;
  const QString file = QFileDialog::getSaveFileName( nullptr,
                       extractFileTitle(),
                       defaultPath(),
                       fileFilter() );
  if ( file.isEmpty() )
  {
    return;
  }

  const QFileInfo fi( file );
  s.setValue( settingsKey(), fi.absolutePath() );

  // decode current base64 embedded file
  const QByteArray base64 = mBase64.mid( 7 ).toLocal8Bit(); // strip 'base64:' prefix
  const QByteArray decoded = QByteArray::fromBase64( base64, QByteArray::OmitTrailingEquals );

  QFile fileOut( file );
  fileOut.open( QIODevice::WriteOnly );
  fileOut.write( decoded );
  fileOut.close();

  if ( mMessageBar )
  {
    mMessageBar->pushMessage( extractFileTitle(),
                              tr( "Successfully extracted file to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( file ).toString(), QDir::toNativeSeparators( file ) ),
                              Qgis::MessageLevel::Success, 0 );
  }
}

void QgsAbstractFileContentSourceLineEdit::mFileLineEdit_textEdited( const QString &text )
{
  mFileLineEdit->setPlaceholderText( QString() );
  mBase64.clear();
  mMode = ModeFile;
  if ( !text.isEmpty() && !QFileInfo::exists( text ) )
  {
    const QUrl url( text );
    if ( !url.isValid() )
    {
      return;
    }
  }
  emit sourceChanged( text );
}

QString QgsAbstractFileContentSourceLineEdit::defaultPath() const
{
  if ( QFileInfo::exists( source() ) )
    return source();

  return QgsSettings().value( settingsKey(), QDir::homePath() ).toString();
}

QString QgsAbstractFileContentSourceLineEdit::settingsKey() const
{
  return mLastPathKey.isEmpty() ? defaultSettingsKey() : mLastPathKey;
}

void QgsAbstractFileContentSourceLineEdit::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

QgsMessageBar *QgsAbstractFileContentSourceLineEdit::messageBar() const
{
  return mMessageBar;
}



//
// QgsPictureSourceLineEditBase
//

///@cond PRIVATE


QString QgsPictureSourceLineEditBase::fileFilter() const
{
  switch ( mFormat )
  {
    case Svg:
      return tr( "SVG files" ) + " (*.svg)";
    case Image:
    {
      QStringList formatsFilter;
      const QByteArrayList supportedFormats = QImageReader::supportedImageFormats();
      for ( const auto &format : supportedFormats )
      {
        formatsFilter.append( QString( QStringLiteral( "*.%1" ) ).arg( QString( format ) ) );
      }
      return QString( "%1 (%2);;%3 (*.*)" ).arg( tr( "Images" ), formatsFilter.join( QLatin1Char( ' ' ) ), tr( "All files" ) );
    }

    case AnimatedImage:
    {
      QStringList formatsFilter;
      const QByteArrayList supportedFormats = QMovie::supportedFormats();
      for ( const auto &format : supportedFormats )
      {
        formatsFilter.append( QString( QStringLiteral( "*.%1" ) ).arg( QString( format ) ) );
      }
      return QString( "%1 (%2);;%3 (*.*)" ).arg( tr( "Animated Images" ), formatsFilter.join( QLatin1Char( ' ' ) ), tr( "All files" ) );
    }
  }
  BUILTIN_UNREACHABLE
}

QString QgsPictureSourceLineEditBase::selectFileTitle() const
{
  switch ( mFormat )
  {
    case Svg:
      return tr( "Select SVG File" );
    case Image:
      return tr( "Select Image File" );
    case AnimatedImage:
      return tr( "Select Animated Image File" );
  }
  BUILTIN_UNREACHABLE
}

QString QgsPictureSourceLineEditBase::fileFromUrlTitle() const
{
  switch ( mFormat )
  {
    case Svg:
      return tr( "SVG From URL" );
    case Image:
      return tr( "Image From URL" );
    case AnimatedImage:
      return tr( "Animated Image From URL" );
  }
  BUILTIN_UNREACHABLE
}

QString QgsPictureSourceLineEditBase::fileFromUrlText() const
{
  switch ( mFormat )
  {
    case Svg:
      return tr( "Enter SVG URL" );
    case Image:
      return tr( "Enter image URL" );
    case AnimatedImage:
      return tr( "Enter animated image URL" );
  }
  BUILTIN_UNREACHABLE
}

QString QgsPictureSourceLineEditBase::embedFileTitle() const
{
  switch ( mFormat )
  {
    case Svg:
      return tr( "Embed SVG File" );
    case Image:
      return tr( "Embed Image File" );
    case AnimatedImage:
      return tr( "Embed Animated Image File" );
  }
  BUILTIN_UNREACHABLE
}

QString QgsPictureSourceLineEditBase::extractFileTitle() const
{
  switch ( mFormat )
  {
    case Svg:
      return tr( "Extract SVG File" );
    case Image:
      return tr( "Extract Image File" );
    case AnimatedImage:
      return tr( "Extract Animated Image File" );
  }
  BUILTIN_UNREACHABLE
}

QString QgsPictureSourceLineEditBase::defaultSettingsKey() const
{
  switch ( mFormat )
  {
    case Svg:
      return QStringLiteral( "/UI/lastSVGDir" );
    case Image:
      return QStringLiteral( "/UI/lastImageDir" );
    case AnimatedImage:
      return QStringLiteral( "/UI/lastAnimatedImageDir" );
  }
  BUILTIN_UNREACHABLE
}

///@endcond


