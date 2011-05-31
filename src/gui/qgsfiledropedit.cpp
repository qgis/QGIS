/***************************************************************************
    qgsfiledropedit.cpp - File Dropable LineEdit
     --------------------------------------
    Date                 : 31-Jan-2007
    Copyright            : (C) 2007 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfiledropedit.h"
#include <QDropEvent>
#include <QFileInfo>
#include <QPainter>
#include <QUrl>

/*!
  \class QgsFileDropEdit

  \brief The QgsDropNameEdit class provides a line edit widget which
  accepts file drops.

  Dropping can be limited to files only, files with a specific extension
  or directories only. By default, dropping is limited to files only.
*/

QgsFileDropEdit::QgsFileDropEdit( QWidget *parent )
    : QLineEdit( parent )
{
  mDirOnly = false;
  mFileOnly = true;
  mDragActive = false;
  setAcceptDrops( true );
}

QgsFileDropEdit::~QgsFileDropEdit()
{}

/*!
  Limit drops to directories.
*/
void QgsFileDropEdit::setDirOnly( bool isDirOnly )
{
  mDirOnly = isDirOnly;
  if ( mDirOnly )
  {
    mFileOnly = false;
  }
}

/*!
  Limit drops to files.
*/
void QgsFileDropEdit::setFileOnly( bool isFileOnly )
{
  mFileOnly = isFileOnly;
  if ( mFileOnly )
  {
    mDirOnly = false;
  }
}

/*!
  Limit drops to files with specified extension.
*/
void QgsFileDropEdit::setSuffixFilter( const QString& suffix )
{
  mSuffix = suffix;
}

/*!
  Return file name if object meets drop criteria.
*/
QString QgsFileDropEdit::acceptableFilePath( QDropEvent *event ) const
{
  QString path;
  if ( event->mimeData()->hasUrls() )
  {
    QFileInfo file( event->mimeData()->urls().first().toLocalFile() );
    if ( !(( mFileOnly && !file.isFile() ) ||
           ( mDirOnly && !file.isDir() ) ||
           ( !mSuffix.isEmpty() && mSuffix.compare( file.suffix(), Qt::CaseInsensitive ) ) ) )
      path = file.filePath();
  }
  return path;
}

/*!
  Check if dragged object is acceptible. Called when a drag is in progress
  and the mouse enters this widget.
*/
void QgsFileDropEdit::dragEnterEvent( QDragEnterEvent *event )
{
  QString filePath = acceptableFilePath( event );
  if ( !filePath.isEmpty() )
  {
    event->acceptProposedAction();
    mDragActive = true;
    update();
  }
  else
  {
    QLineEdit::dragEnterEvent( event );
  }
}

/*!
  Called when a drag is in progress and the mouse leaves this widget.
*/
void QgsFileDropEdit::dragLeaveEvent( QDragLeaveEvent *event )
{
  QLineEdit::dragLeaveEvent( event );
  event->accept();
  mDragActive = false;
  update();
}

/*!
  Receive the dragged object. Called when the drag is dropped on this widget.
*/
void QgsFileDropEdit::dropEvent( QDropEvent *event )
{
  QString filePath = acceptableFilePath( event );
  if ( !filePath.isEmpty() )
  {
    setText( filePath );
    selectAll();
    setFocus( Qt::MouseFocusReason );
    event->acceptProposedAction();
    mDragActive = false;
    update();
  }
  else
  {
    QLineEdit::dropEvent( event );
  }
}

/*!
  Paints line edit with drag highlight in response to a paint event.
*/
void QgsFileDropEdit::paintEvent( QPaintEvent *e )
{
  QLineEdit::paintEvent( e );
  if ( mDragActive )
  {
    QPainter p( this );
    int width = 2;  // width of highlight rectangle inside frame
    p.setPen( QPen( palette().highlight(), width ) );
    QRect r = rect().adjusted( width, width, -width, -width );
    p.drawRect( r );
  }
}
