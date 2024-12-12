/****************************************************************************
**
** Copyright (C) 2013 Jiří Procházka (Hobrasoft)
** Contact: http://www.hobrasoft.cz/
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file is under the terms of the GNU Lesser General Public License
** version 2.1 as published by the Free Software Foundation and appearing
** in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the
** GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qgsimagedroptextedit.h"
#include "moc_qgsimagedroptextedit.cpp"
#include "qgsguiutils.h"

#include <QMimeData>
#include <QBuffer>
#include <QFileInfo>
#include <QImageReader>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopServices>

///@cond PRIVATE
QgsImageDropTextEdit::QgsImageDropTextEdit( QWidget *parent )
  : QTextEdit( parent )
{
  setTextInteractionFlags( Qt::TextEditorInteraction | Qt::LinksAccessibleByMouse );
}

QgsImageDropTextEdit::~QgsImageDropTextEdit() = default;

bool QgsImageDropTextEdit::canInsertFromMimeData( const QMimeData *source ) const
{
  if ( source->hasImage() || QTextEdit::canInsertFromMimeData( source ) )
    return true;

  const QList<QUrl> urls = source->urls();
  QStringList files;
  files.reserve( urls.size() );
  for ( const QUrl &url : urls )
  {
    QString fileName = url.toLocalFile();
    // seems that some drag and drop operations include an empty url
    // so we test for length to make sure we have something
    if ( !fileName.isEmpty() )
    {
      files << fileName;
    }
  }

  bool matched = false;
  for ( const QString &file : std::as_const( files ) )
  {
    QFileInfo fi( file );
    const QList<QByteArray> formats = QImageReader::supportedImageFormats();
    for ( const QByteArray &format : formats )
    {
      if ( fi.suffix().compare( format, Qt::CaseInsensitive ) == 0 )
      {
        matched = true;
        break;
      }
    }
  }

  return matched;
}

void QgsImageDropTextEdit::insertFromMimeData( const QMimeData *source )
{
  if ( source->hasImage() )
  {
    const QStringList formats = source->formats();
    QString format;
    for ( const QString &string : formats )
    {
      if ( string == QLatin1String( "image/bmp" ) )
      {
        format = QStringLiteral( "BMP" );
        break;
      }
      if ( string == QLatin1String( "image/jpeg" ) )
      {
        format = QStringLiteral( "JPG" );
        break;
      }
      if ( string == QLatin1String( "image/jpg" ) )
      {
        format = QStringLiteral( "JPG" );
        break;
      }
      if ( string == QLatin1String( "image/gif" ) )
      {
        format = QStringLiteral( "GIF" );
        break;
      }
      if ( string == QLatin1String( "image/png" ) )
      {
        format = QStringLiteral( "PNG" );
        break;
      }
      if ( string == QLatin1String( "image/pbm" ) )
      {
        format = QStringLiteral( "PBM" );
        break;
      }
      if ( string == QLatin1String( "image/pgm" ) )
      {
        format = QStringLiteral( "PGM" );
        break;
      }
      if ( string == QLatin1String( "image/ppm" ) )
      {
        format = QStringLiteral( "PPM" );
        break;
      }
      if ( string == QLatin1String( "image/tiff" ) )
      {
        format = QStringLiteral( "TIFF" );
        break;
      }
      if ( string == QLatin1String( "image/xbm" ) )
      {
        format = QStringLiteral( "XBM" );
        break;
      }
      if ( string == QLatin1String( "image/xpm" ) )
      {
        format = QStringLiteral( "XPM" );
        break;
      }
    }
    if ( !format.isEmpty() )
    {
      dropImage( qvariant_cast<QImage>( source->imageData() ), format );
      return;
    }
  }
  else
  {
    const QList<QUrl> urls = source->urls();
    QStringList files;
    files.reserve( urls.size() );
    for ( const QUrl &url : urls )
    {
      if ( url.isLocalFile() )
      {
        QString fileName = url.toLocalFile();
        // seems that some drag and drop operations include an empty url
        // so we test for length to make sure we have something
        if ( !fileName.isEmpty() )
        {
          files << fileName;
        }
      }
      else
      {
        dropLink( url );
      }
    }

    for ( const QString &file : std::as_const( files ) )
    {
      const QFileInfo fi( file );
      const QList<QByteArray> formats = QImageReader::supportedImageFormats();
      bool isImage = false;
      for ( const QByteArray &format : formats )
      {
        if ( fi.suffix().compare( format, Qt::CaseInsensitive ) == 0 )
        {
          const QImage image( file );
          dropImage( image, format );
          isImage = true;
          break;
        }
      }
      if ( !isImage )
      {
        dropLink( QUrl::fromLocalFile( file ) );
      }
    }
    if ( !files.empty() )
      return;
  }

  QTextEdit::insertFromMimeData( source );
}

void QgsImageDropTextEdit::mouseMoveEvent( QMouseEvent *e )
{
  QTextEdit::mouseMoveEvent( e );
  mActiveAnchor = anchorAt( e->pos() );
  if ( !mActiveAnchor.isEmpty() && !mCursorOverride )
    mCursorOverride = std::make_unique<QgsTemporaryCursorOverride>( Qt::PointingHandCursor );
  else if ( mActiveAnchor.isEmpty() && mCursorOverride )
    mCursorOverride.reset();
}

void QgsImageDropTextEdit::mouseReleaseEvent( QMouseEvent *e )
{
  if ( e->button() == Qt::LeftButton && !mActiveAnchor.isEmpty() )
  {
    QDesktopServices::openUrl( QUrl( mActiveAnchor ) );
    if ( mCursorOverride )
      mCursorOverride.reset();
    mActiveAnchor.clear();
  }
  else
  {
    QTextEdit::mouseReleaseEvent( e );
  }
}

void QgsImageDropTextEdit::dropImage( const QImage &image, const QString &format )
{
  QByteArray bytes;
  QBuffer buffer( &bytes );
  buffer.open( QIODevice::WriteOnly );
  image.save( &buffer, format.toLocal8Bit().data() );
  buffer.close();
  QByteArray base64 = bytes.toBase64();
  QByteArray base64l;
  for ( int i = 0; i < base64.size(); i++ )
  {
    base64l.append( base64[i] );
    if ( i % 80 == 0 )
    {
      base64l.append( "\n" );
    }
  }

  QTextCursor cursor = textCursor();
  QTextImageFormat imageFormat;
  imageFormat.setWidth( image.width() );
  imageFormat.setHeight( image.height() );
  imageFormat.setName( QStringLiteral( "data:image/%1;base64,%2" )
                         .arg( QStringLiteral( "%1.%2" ).arg( rand() ).arg( format ), base64l.data() )
  );
  cursor.insertImage( imageFormat );
}

void QgsImageDropTextEdit::dropLink( const QUrl &url )
{
  QTextCursor cursor = textCursor();
  cursor.insertHtml( QStringLiteral( "<a href=\"%1\">%1</a>" ).arg( url.toString() ) );
}

///@endcond
