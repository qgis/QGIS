/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the
** GNU Lesser General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "characterwidget.h"
#include "qgsapplication.h"

#include <QFontDatabase>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QScrollArea>
#include <QScrollBar>
#include <QToolTip>

CharacterWidget::CharacterWidget( QWidget *parent )
  : QWidget( parent )
{
  setMouseTracking( true );
  setFocusPolicy( Qt::StrongFocus );
}

void CharacterWidget::setFont( const QFont &font )
{
  mDisplayFont.setFamily( font.family() );
  mSquareSize = std::max( 34, QFontMetrics( mDisplayFont ).xHeight() * 3 );
  adjustSize();
  update();
}

void CharacterWidget::setFontSize( double fontSize )
{
  mDisplayFont.setPointSizeF( fontSize );
  mSquareSize = std::max( 34, QFontMetrics( mDisplayFont ).xHeight() * 3 );
  adjustSize();
  update();
}

void CharacterWidget::setFontStyle( const QString &fontStyle )
{
  const QFontDatabase fontDatabase;
  const QFont::StyleStrategy oldStrategy = mDisplayFont.styleStrategy();
  mDisplayFont = fontDatabase.font( mDisplayFont.family(), fontStyle, mDisplayFont.pointSize() );
  mDisplayFont.setStyleStrategy( oldStrategy );
  mSquareSize = std::max( 34, QFontMetrics( mDisplayFont ).xHeight() * 3 );
  adjustSize();
  update();
}

void CharacterWidget::updateFontMerging( bool enable )
{
  if ( enable )
    mDisplayFont.setStyleStrategy( QFont::PreferDefault );
  else
    mDisplayFont.setStyleStrategy( QFont::NoFontMerging );
  adjustSize();
  update();
}

void CharacterWidget::setColumns( int columns )
{
  if ( mColumns == columns || columns < 1 )
    return;
  mColumns = columns;
  adjustSize();
  update();
}

void CharacterWidget::setCharacter( QChar character )
{
  const bool changed = character.unicode() != mLastKey;
  mLastKey = character.isNull() ? -1 : character.unicode();
  QWidget *widget = parentWidget();
  if ( widget )
  {
    QScrollArea *scrollArea = qobject_cast< QScrollArea *>( widget->parent() );
    if ( scrollArea && mLastKey < 65536 )
    {
      scrollArea->ensureVisible( 0, mLastKey / mColumns * mSquareSize );
    }
  }
  if ( changed )
    emit characterSelected( mLastKey >= 0 ? QChar( mLastKey ) : QChar() );

  update();
}

void CharacterWidget::clearCharacter()
{
  mLastKey = -1;
  update();
}

QSize CharacterWidget::sizeHint() const
{
  return QSize( mColumns * mSquareSize, ( 65536 / mColumns ) * mSquareSize );
}

void CharacterWidget::keyPressEvent( QKeyEvent *event )
{
  const QFontMetrics fm( mDisplayFont );

  if ( event->key() == Qt::Key_Right )
  {
    int next = std::min( mLastKey + 1, 0xfffc );
    while ( next < 0xfffc && !fm.inFont( QChar( next ) ) )
    {
      next++;
    }
    setCharacter( QChar( next ) );
  }
  else if ( event->key() == Qt::Key_Left )
  {
    int next = mLastKey - 1;
    while ( next > 0 && !fm.inFont( QChar( next ) ) )
    {
      next--;
    }
    setCharacter( QChar( next ) );
  }
  else if ( event->key() == Qt::Key_Down )
  {
    int next = std::min( mLastKey + mColumns, 0xfffc );
    while ( next < 0xfffc && !fm.inFont( QChar( next ) ) )
    {
      next = std::min( next + mColumns, 0xfffc );
    }
    setCharacter( QChar( next ) );
  }
  else if ( event->key() == Qt::Key_Up )
  {
    int next = std::max( 0, mLastKey - mColumns );
    while ( next > 0 && !fm.inFont( QChar( next ) ) )
    {
      next = std::max( 0, next - mColumns );
    }
    setCharacter( QChar( next ) );
  }
  else if ( event->key() == Qt::Key_Home )
  {
    int next = 0;
    while ( next < 0xfffc && !fm.inFont( QChar( next ) ) )
    {
      next++;
    }
    setCharacter( QChar( next ) );
  }
  else if ( event->key() == Qt::Key_End )
  {
    int next = 0xfffc;
    while ( next > 0 && !fm.inFont( QChar( next ) ) )
    {
      next--;
    }
    setCharacter( QChar( next ) );
  }
  else if ( !event->text().isEmpty() )
  {
    QChar chr = event->text().at( 0 );
    if ( chr.unicode() != mLastKey )
    {
      setCharacter( chr );
    }
  }
}

void CharacterWidget::mouseMoveEvent( QMouseEvent *event )
{
  const QPoint widgetPosition = mapFromGlobal( event->globalPos() );
  const uint key = ( widgetPosition.y() / mSquareSize ) * mColumns + widgetPosition.x() / mSquareSize;

  const QString text = tr( "<p>Character: <span style=\"font-size: 24pt; font-family: %1\">%2</span><p>Decimal: %3<p>Hex: 0x%4" )
                       .arg( mDisplayFont.family() )
                       .arg( QChar( key ) )
                       .arg( key )
                       .arg( QString::number( key, 16 ) );
  QToolTip::showText( event->globalPos(), text, this );
}

void CharacterWidget::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton )
  {
    mLastKey = ( event->y() / mSquareSize ) * mColumns + event->x() / mSquareSize;
    if ( QChar( mLastKey ).category() != QChar::Other_NotAssigned )
      emit characterSelected( QChar( mLastKey ) );
    update();
  }
  else
    QWidget::mousePressEvent( event );
}

void CharacterWidget::paintEvent( QPaintEvent *event )
{
  QPainter painter( this );
  painter.setFont( mDisplayFont );

  const QFontMetrics fontMetrics( mDisplayFont );

  const QRect redrawRect = event->rect();
  const int beginRow = redrawRect.top() / mSquareSize;
  const int endRow = redrawRect.bottom() / mSquareSize;
  const int beginColumn = redrawRect.left() / mSquareSize;
  const int endColumn = std::min( mColumns - 1, redrawRect.right() / mSquareSize );

  const QPalette palette = qApp->palette();
  painter.setPen( QPen( palette.color( QPalette::Mid ) ) );
  for ( int row = beginRow; row <= endRow; ++row )
  {
    for ( int column = beginColumn; column <= endColumn; ++column )
    {
      const int key = row * mColumns + column;
      painter.setBrush( fontMetrics.inFont( QChar( key ) ) ? QBrush( palette.color( QPalette::Base ) ) : Qt::NoBrush );
      painter.drawRect( column * mSquareSize, row * mSquareSize, mSquareSize, mSquareSize );
    }
  }

  for ( int row = beginRow; row <= endRow; ++row )
  {
    for ( int column = beginColumn; column <= endColumn; ++column )
    {
      const int key = row * mColumns + column;
      painter.setClipRect( column * mSquareSize, row * mSquareSize, mSquareSize, mSquareSize );
      painter.setPen( QPen( palette.color( key == mLastKey ? QPalette::HighlightedText : QPalette::WindowText ) ) );

      if ( key == mLastKey )
        painter.fillRect( column * mSquareSize + 1, row * mSquareSize + 1, mSquareSize, mSquareSize, QBrush( palette.color( QPalette::Highlight ) ) );

      if ( fontMetrics.inFont( QChar( key ) ) )
      {
        painter.drawText( column * mSquareSize + ( mSquareSize / 2 ) - fontMetrics.boundingRect( QChar( key ) ).width() / 2,
                          row * mSquareSize + 4 + fontMetrics.ascent(),
                          QString( QChar( key ) ) );
      }
    }
  }
}

void CharacterWidget::resizeEvent( QResizeEvent *event )
{
  mColumns = event->size().width() / mSquareSize;
  QWidget::resizeEvent( event );
}
