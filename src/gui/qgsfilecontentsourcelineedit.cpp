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
#include <QMenu>
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QInputDialog>

//
// QgsAbstractFileContentSourceLineEdit
//

QgsAbstractFileContentSourceLineEdit::QgsAbstractFileContentSourceLineEdit( QWidget *parent )
  : QWidget( parent )
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  mFileLineEdit = new QLineEdit( this );
  mFileToolButton = new QToolButton( this );
  mFileToolButton->setText( tr( "…" ) );
  layout->addWidget( mFileLineEdit, 1 );
  layout->addWidget( mFileToolButton );
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
    extractFileAction->setEnabled( mFileLineEdit->text().startsWith( QLatin1String( "base64:" ), Qt::CaseInsensitive ) );
  } );

  QAction *enterUrlAction = new QAction( tr( "From URL…" ), sourceMenu );
  connect( enterUrlAction, &QAction::triggered, this, &QgsAbstractFileContentSourceLineEdit::selectUrl );
  sourceMenu->addAction( enterUrlAction );

  mFileToolButton->setMenu( sourceMenu );
  mFileToolButton->setPopupMode( QToolButton::MenuButtonPopup );
  connect( mFileToolButton, &QToolButton::clicked, this, &QgsAbstractFileContentSourceLineEdit::selectFile );

  connect( mFileLineEdit, &QLineEdit::textEdited, this, &QgsAbstractFileContentSourceLineEdit::mFileLineEdit_textEdited );
  connect( mFileLineEdit, &QLineEdit::editingFinished, this, &QgsAbstractFileContentSourceLineEdit::mFileLineEdit_editingFinished );
}

QString QgsAbstractFileContentSourceLineEdit::source() const
{
  return mBase64.isEmpty() ? mFileLineEdit->text() : mBase64;
}

void QgsAbstractFileContentSourceLineEdit::setLastPathSettingsKey( const QString &key )
{
  mLastPathKey = key;
}

void QgsAbstractFileContentSourceLineEdit::setSource( const QString &source )
{
  const bool isBase64 = source.startsWith( QLatin1String( "base64:" ), Qt::CaseInsensitive );

  if ( ( !isBase64 && source == mFileLineEdit->text() ) || ( isBase64 && source == mBase64 ) )
    return;

  if ( isBase64 )
    mBase64 = source;
  else
    mBase64.clear();

  mFileLineEdit->setText( source );
  emit sourceChanged( source );
}

void QgsAbstractFileContentSourceLineEdit::selectFile()
{
  QgsSettings s;
  QString file = QFileDialog::getOpenFileName( nullptr,
                 selectFileTitle(),
                 defaultPath(),
                 fileFilter() );
  QFileInfo fi( file );
  if ( file.isEmpty() || !fi.exists() || file == source() )
  {
    return;
  }
  mBase64.clear();
  mFileLineEdit->setText( file );
  s.setValue( settingsKey(), fi.absolutePath() );
  emit sourceChanged( mFileLineEdit->text() );
}

void QgsAbstractFileContentSourceLineEdit::selectUrl()
{
  bool ok = false;
  const QString path = QInputDialog::getText( this, fileFromUrlTitle(), fileFromUrlText(), QLineEdit::Normal, mFileLineEdit->text(), &ok );
  if ( ok && path != source() )
  {
    mBase64.clear();
    mFileLineEdit->setText( path );
    emit sourceChanged( mFileLineEdit->text() );
  }
}

void QgsAbstractFileContentSourceLineEdit::embedFile()
{
  QgsSettings s;
  QString file = QFileDialog::getOpenFileName( nullptr,
                 embedFileTitle(),
                 defaultPath(),
                 fileFilter() );
  QFileInfo fi( file );
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

  QByteArray blob = fileSource.readAll();
  QByteArray encoded = blob.toBase64();

  QString path( encoded );
  path.prepend( QLatin1String( "base64:" ) );
  if ( path == source() )
    return;

  mBase64 = path;
  mFileLineEdit->setText( path );
  emit sourceChanged( path );
}

void QgsAbstractFileContentSourceLineEdit::extractFile()
{
  QgsSettings s;
  QString file = QFileDialog::getSaveFileName( nullptr,
                 extractFileTitle(),
                 defaultPath(),
                 fileFilter() );
  if ( file.isEmpty() )
  {
    return;
  }

  QFileInfo fi( file );
  s.setValue( settingsKey(), fi.absolutePath() );

  // decode current base64 embedded file
  QString path = mFileLineEdit->text().trimmed();
  if ( path.startsWith( QLatin1String( "base64:" ), Qt::CaseInsensitive ) )
  {
    QByteArray base64 = mBase64.mid( 7 ).toLocal8Bit(); // strip 'base64:' prefix
    QByteArray decoded = QByteArray::fromBase64( base64, QByteArray::OmitTrailingEquals );

    QFile fileOut( file );
    fileOut.open( QIODevice::WriteOnly );
    fileOut.write( decoded );
    fileOut.close();
  }
}

void QgsAbstractFileContentSourceLineEdit::mFileLineEdit_textEdited( const QString &text )
{
  if ( !QFileInfo::exists( text ) )
  {
    return;
  }
  emit sourceChanged( text );
}

void QgsAbstractFileContentSourceLineEdit::mFileLineEdit_editingFinished()
{
  if ( !mFileLineEdit->text().isEmpty() && !QFileInfo::exists( mFileLineEdit->text() ) )
  {
    QUrl url( mFileLineEdit->text() );
    if ( !url.isValid() )
    {
      return;
    }
  }

  emit sourceChanged( mFileLineEdit->text() );
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

//
// QgsSvgSourceLineEdit
//

///@cond PRIVATE

QString QgsSvgSourceLineEdit::fileFilter() const
{
  return tr( "SVG files" ) + " (*.svg)";
}

QString QgsSvgSourceLineEdit::selectFileTitle() const
{
  return tr( "Select SVG File" );
}

QString QgsSvgSourceLineEdit::fileFromUrlTitle() const
{
  return tr( "SVG From URL" );
}

QString QgsSvgSourceLineEdit::fileFromUrlText() const
{
  return tr( "Enter SVG URL" );
}

QString QgsSvgSourceLineEdit::embedFileTitle() const
{
  return tr( "Embed SVG File" );
}

QString QgsSvgSourceLineEdit::extractFileTitle() const
{
  return tr( "Extract SVG File" );
}

QString QgsSvgSourceLineEdit::defaultSettingsKey() const
{
  return QStringLiteral( "/UI/lastSVGDir" );
}
///@endcond

//
// QgsImageSourceLineEdit
//

///@cond PRIVATE

QString QgsImageSourceLineEdit::fileFilter() const
{
  return tr( "All files" ) + " (*.*)";
}

QString QgsImageSourceLineEdit::selectFileTitle() const
{
  return tr( "Select Image File" );
}

QString QgsImageSourceLineEdit::fileFromUrlTitle() const
{
  return tr( "Image From URL" );
}

QString QgsImageSourceLineEdit::fileFromUrlText() const
{
  return tr( "Enter image URL" );
}

QString QgsImageSourceLineEdit::embedFileTitle() const
{
  return tr( "Embed Image File" );
}

QString QgsImageSourceLineEdit::extractFileTitle() const
{
  return tr( "Extract Image File" );
}

QString QgsImageSourceLineEdit::defaultSettingsKey() const
{
  return QStringLiteral( "/UI/lastImageDir" );
}

///@endcond
