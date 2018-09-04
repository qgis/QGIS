/***************************************************************************
 qgssvgsourcelineedit.cpp
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

#include "qgssvgsourcelineedit.h"
#include "qgssettings.h"
#include <QMenu>
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QInputDialog>

QgsSvgSourceLineEdit::QgsSvgSourceLineEdit( QWidget *parent )
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
  connect( selectFileAction, &QAction::triggered, this, &QgsSvgSourceLineEdit::selectFile );
  sourceMenu->addAction( selectFileAction );

  QAction *embedFileAction = new QAction( tr( "Embed File…" ), sourceMenu );
  connect( embedFileAction, &QAction::triggered, this, &QgsSvgSourceLineEdit::embedFile );
  sourceMenu->addAction( embedFileAction );

  QAction *extractFileAction = new QAction( tr( "Extract Embedded File…" ), sourceMenu );
  connect( extractFileAction, &QAction::triggered, this, &QgsSvgSourceLineEdit::extractFile );
  sourceMenu->addAction( extractFileAction );

  connect( sourceMenu, &QMenu::aboutToShow, this, [this, extractFileAction]
  {
    extractFileAction->setEnabled( mFileLineEdit->text().startsWith( QLatin1String( "base64:" ), Qt::CaseInsensitive ) );
  } );

  QAction *enterUrlAction = new QAction( tr( "From URL…" ), sourceMenu );
  connect( enterUrlAction, &QAction::triggered, this, &QgsSvgSourceLineEdit::selectUrl );
  sourceMenu->addAction( enterUrlAction );

  mFileToolButton->setMenu( sourceMenu );
  mFileToolButton->setPopupMode( QToolButton::MenuButtonPopup );
  connect( mFileToolButton, &QToolButton::clicked, this, &QgsSvgSourceLineEdit::selectFile );

  connect( mFileLineEdit, &QLineEdit::textEdited, this, &QgsSvgSourceLineEdit::mFileLineEdit_textEdited );
  connect( mFileLineEdit, &QLineEdit::editingFinished, this, &QgsSvgSourceLineEdit::mFileLineEdit_editingFinished );
}

QString QgsSvgSourceLineEdit::source() const
{
  return mFileLineEdit->text();
}

void QgsSvgSourceLineEdit::setLastPathSettingsKey( const QString &key )
{
  mLastPathKey = key;
}

void QgsSvgSourceLineEdit::setSource( const QString &source )
{
  if ( source == mFileLineEdit->text() )
    return;

  mFileLineEdit->setText( source );
  emit sourceChanged( source );
}

void QgsSvgSourceLineEdit::selectFile()
{
  QgsSettings s;
  QString file = QFileDialog::getOpenFileName( nullptr,
                 tr( "Select SVG file" ),
                 defaultPath(),
                 tr( "SVG files" ) + " (*.svg)" );
  QFileInfo fi( file );
  if ( file.isEmpty() || !fi.exists() || file == source() )
  {
    return;
  }
  mFileLineEdit->setText( file );
  s.setValue( settingsKey(), fi.absolutePath() );
  emit sourceChanged( mFileLineEdit->text() );
}

void QgsSvgSourceLineEdit::selectUrl()
{
  bool ok = false;
  const QString path = QInputDialog::getText( this, tr( "SVG From URL" ), tr( "Enter SVG URL" ), QLineEdit::Normal, mFileLineEdit->text(), &ok );
  if ( ok && path != source() )
  {
    mFileLineEdit->setText( path );
    emit sourceChanged( mFileLineEdit->text() );
  }
}

void QgsSvgSourceLineEdit::embedFile()
{
  QgsSettings s;
  QString file = QFileDialog::getOpenFileName( nullptr,
                 tr( "Embed SVG File" ),
                 defaultPath(),
                 tr( "SVG files" ) + " (*.svg)" );
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

  mFileLineEdit->setText( path );
  emit sourceChanged( mFileLineEdit->text() );
}

void QgsSvgSourceLineEdit::extractFile()
{
  QgsSettings s;
  QString file = QFileDialog::getSaveFileName( nullptr,
                 tr( "Extract SVG File" ),
                 defaultPath(),
                 tr( "SVG files" ) + " (*.svg)" );
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
    QByteArray base64 = path.mid( 7 ).toLocal8Bit(); // strip 'base64:' prefix
    QByteArray decoded = QByteArray::fromBase64( base64, QByteArray::OmitTrailingEquals );

    QFile fileOut( file );
    fileOut.open( QIODevice::WriteOnly );
    fileOut.write( decoded );
    fileOut.close();
  }
}

void QgsSvgSourceLineEdit::mFileLineEdit_textEdited( const QString &text )
{
  if ( !QFileInfo::exists( text ) )
  {
    return;
  }
  emit sourceChanged( text );
}

void QgsSvgSourceLineEdit::mFileLineEdit_editingFinished()
{
  if ( !QFileInfo::exists( mFileLineEdit->text() ) )
  {
    QUrl url( mFileLineEdit->text() );
    if ( !url.isValid() )
    {
      return;
    }
  }

  emit sourceChanged( mFileLineEdit->text() );
}

QString QgsSvgSourceLineEdit::defaultPath() const
{
  if ( QFileInfo::exists( source() ) )
    return source();

  return QgsSettings().value( settingsKey(), QDir::homePath() ).toString();
}

QString QgsSvgSourceLineEdit::settingsKey() const
{
  return mLastPathKey.isEmpty() ? QStringLiteral( "/UI/lastSVGDir" ) : mLastPathKey;
}

