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

#include <QFontDatabase>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QToolTip>

#include "characterwidget.h"

CharacterWidget::CharacterWidget( QWidget *parent )
  : QWidget( parent )
{
  setMouseTracking( true );
}

void CharacterWidget::setFont( const QFont &font )
{
  mDisplayFont.setFamily( font.family() );
  mSquareSize = std::max( 24, QFontMetrics( mDisplayFont ).xHeight() * 3 );
  adjustSize();
  update();
}

void CharacterWidget::setFontSize( double fontSize )
{
  mDisplayFont.setPointSizeF( fontSize );
  mSquareSize = std::max( 24, QFontMetrics( mDisplayFont ).xHeight() * 3 );
  adjustSize();
  update();
}

void CharacterWidget::setFontStyle( const QString &fontStyle )
{
  QFontDatabase fontDatabase;
  const QFont::StyleStrategy oldStrategy = mDisplayFont.styleStrategy();
  mDisplayFont = fontDatabase.font( mDisplayFont.family(), fontStyle, mDisplayFont.pointSize() );
  mDisplayFont.setStyleStrategy( oldStrategy );
  mSquareSize = std::max( 24, QFontMetrics( mDisplayFont ).xHeight() * 3 );
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
  mLastKey = character.unicode();
  update();
}

QSize CharacterWidget::sizeHint() const
{
  return QSize( mColumns * mSquareSize, ( 65536 / mColumns ) * mSquareSize );
}

void CharacterWidget::mouseMoveEvent( QMouseEvent *event )
{
  QPoint widgetPosition = mapFromGlobal( event->globalPos() );
  uint key = ( widgetPosition.y() / mSquareSize ) * mColumns + widgetPosition.x() / mSquareSize;

  QString text = tr( "<p>Character: <span style=\"font-size: 24pt; font-family: %1\">%2</span><p>Value: 0x%3" )
                 .arg( mDisplayFont.family() )
                 .arg( QChar( key ) )
                 .arg( key, 16 );
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
  painter.fillRect( event->rect(), QBrush( Qt::white ) );
  painter.setFont( mDisplayFont );

  QRect redrawRect = event->rect();
  int beginRow = redrawRect.top() / mSquareSize;
  int endRow = redrawRect.bottom() / mSquareSize;
  int beginColumn = redrawRect.left() / mSquareSize;
  int endColumn = redrawRect.right() / mSquareSize;

  painter.setPen( QPen( Qt::gray ) );
  for ( int row = beginRow; row <= endRow; ++row )
  {
    for ( int column = beginColumn; column <= endColumn; ++column )
    {
      painter.drawRect( column * mSquareSize, row * mSquareSize, mSquareSize, mSquareSize );
    }
  }

  QFontMetrics fontMetrics( mDisplayFont );
  painter.setPen( QPen( Qt::black ) );
  for ( int row = beginRow; row <= endRow; ++row )
  {

    for ( int column = beginColumn; column <= endColumn; ++column )
    {

      int key = row * mColumns + column;
      painter.setClipRect( column * mSquareSize, row * mSquareSize, mSquareSize, mSquareSize );

      if ( key == mLastKey )
        painter.fillRect( column * mSquareSize + 1, row * mSquareSize + 1, mSquareSize, mSquareSize, QBrush( Qt::red ) );

      painter.drawText( column * mSquareSize + ( mSquareSize / 2 ) - fontMetrics.width( QChar( key ) ) / 2,
                        row * mSquareSize + 4 + fontMetrics.ascent(),
                        QString( QChar( key ) ) );
    }
  }
}
