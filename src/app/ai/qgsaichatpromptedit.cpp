/***************************************************************************
    qgsaichatpromptedit.cpp
    ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaichatpromptedit.h"

#include "qgsaivisualcontextutils.h"

#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QImageReader>
#include <QMimeData>
#include <QString>
#include <QUrl>

#include "moc_qgsaichatpromptedit.cpp"

using namespace Qt::StringLiterals;

QgsAiChatPromptEdit::QgsAiChatPromptEdit( QWidget *parent )
  : QTextEdit( parent )
{
  setAcceptDrops( true );
}

QStringList QgsAiChatPromptEdit::extractLocalFilePaths( const QMimeData *source ) const
{
  QStringList paths;
  if ( !source )
    return paths;

  if ( source->hasUrls() )
  {
    const QList<QUrl> urls = source->urls();
    paths.reserve( urls.size() );
    for ( const QUrl &url : urls )
    {
      if ( !url.isLocalFile() )
        continue;

      const QString localPath = QDir::cleanPath( url.toLocalFile() );
      if ( !localPath.isEmpty() && QFileInfo( localPath ).isFile() && !paths.contains( localPath ) )
        paths << localPath;
    }
  }

  if ( source->hasText() )
  {
    const QString text = source->text().trimmed();
    if ( text.startsWith( u"file://"_s, Qt::CaseInsensitive ) )
    {
      const QString localPath = QDir::cleanPath( QUrl( text ).toLocalFile() );
      if ( !localPath.isEmpty() && QFileInfo( localPath ).isFile() && !paths.contains( localPath ) )
        paths << localPath;
    }
  }

  if ( source->hasImage() )
  {
    const QImage image = qvariant_cast<QImage>( source->imageData() );
    if ( !image.isNull() )
    {
      const QString savedPath = QgsAiVisualContextUtils::saveDroppedImage( image );
      if ( !savedPath.isEmpty() && !paths.contains( savedPath ) )
        paths << savedPath;
    }
  }

  return paths;
}

void QgsAiChatPromptEdit::handleFilePaths( const QStringList &paths )
{
  if ( paths.isEmpty() )
    return;

  emit filesDropped( paths );
}

void QgsAiChatPromptEdit::dragEnterEvent( QDragEnterEvent *event )
{
  if ( !extractLocalFilePaths( event->mimeData() ).isEmpty() )
  {
    event->acceptProposedAction();
    return;
  }

  QTextEdit::dragEnterEvent( event );
}

void QgsAiChatPromptEdit::dropEvent( QDropEvent *event )
{
  const QStringList paths = extractLocalFilePaths( event->mimeData() );
  if ( !paths.isEmpty() )
  {
    handleFilePaths( paths );
    event->acceptProposedAction();
    return;
  }

  QTextEdit::dropEvent( event );
}

bool QgsAiChatPromptEdit::canInsertFromMimeData( const QMimeData *source ) const
{
  if ( !extractLocalFilePaths( source ).isEmpty() )
    return false;

  return QTextEdit::canInsertFromMimeData( source );
}

void QgsAiChatPromptEdit::insertFromMimeData( const QMimeData *source )
{
  const QStringList paths = extractLocalFilePaths( source );
  if ( !paths.isEmpty() )
  {
    handleFilePaths( paths );
    return;
  }

  QTextEdit::insertFromMimeData( source );
}
