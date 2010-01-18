/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
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

#include <QtGui>

#include "characterwidget.h"

//! [0]
CharacterWidget::CharacterWidget( QWidget *parent )
    : QWidget( parent )
{
  squareSize = 24;
  columns = 16;
  lastKey = -1;
  setMouseTracking( true );
}
//! [0]

//! [1]
void CharacterWidget::updateFont( const QFont &font )
{
  displayFont.setFamily( font.family() );
  squareSize = qMax( 24, QFontMetrics( displayFont ).xHeight() * 3 );
  adjustSize();
  update();
}
//! [1]

//! [2]
void CharacterWidget::updateSize( double fontSize )
{
  displayFont.setPointSizeF( fontSize );
  squareSize = qMax( 24, QFontMetrics( displayFont ).xHeight() * 3 );
  adjustSize();
  update();
}
//! [2]

void CharacterWidget::updateStyle( const QString &fontStyle )
{
  QFontDatabase fontDatabase;
  const QFont::StyleStrategy oldStrategy = displayFont.styleStrategy();
  displayFont = fontDatabase.font( displayFont.family(), fontStyle, displayFont.pointSize() );
  displayFont.setStyleStrategy( oldStrategy );
  squareSize = qMax( 24, QFontMetrics( displayFont ).xHeight() * 3 );
  adjustSize();
  update();
}

void CharacterWidget::updateFontMerging( bool enable )
{
  if ( enable )
    displayFont.setStyleStrategy( QFont::PreferDefault );
  else
    displayFont.setStyleStrategy( QFont::NoFontMerging );
  adjustSize();
  update();
}

//! [3]
QSize CharacterWidget::sizeHint() const
{
  return QSize( columns*squareSize, ( 65536 / columns )*squareSize );
}
//! [3]

//! [4]
void CharacterWidget::mouseMoveEvent( QMouseEvent *event )
{
  QPoint widgetPosition = mapFromGlobal( event->globalPos() );
  uint key = ( widgetPosition.y() / squareSize ) * columns + widgetPosition.x() / squareSize;

  QString text = QString::fromLatin1( "<p>Character: <span style=\"font-size: 24pt; font-family: %1\">" ).arg( displayFont.family() )
                 + QChar( key )
                 + QString::fromLatin1( "</span><p>Value: 0x" )
                 + QString::number( key, 16 );
  QToolTip::showText( event->globalPos(), text, this );
}
//! [4]

//! [5]
void CharacterWidget::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::LeftButton )
  {
    lastKey = ( event->y() / squareSize ) * columns + event->x() / squareSize;
    if ( QChar( lastKey ).category() != QChar::NoCategory )
      emit characterSelected( QChar( lastKey ) );
    update();
  }
  else
    QWidget::mousePressEvent( event );
}
//! [5]

//! [6]
void CharacterWidget::paintEvent( QPaintEvent *event )
{
  QPainter painter( this );
  painter.fillRect( event->rect(), QBrush( Qt::white ) );
  painter.setFont( displayFont );
//! [6]

//! [7]
  QRect redrawRect = event->rect();
  int beginRow = redrawRect.top() / squareSize;
  int endRow = redrawRect.bottom() / squareSize;
  int beginColumn = redrawRect.left() / squareSize;
  int endColumn = redrawRect.right() / squareSize;
//! [7]

//! [8]
  painter.setPen( QPen( Qt::gray ) );
  for ( int row = beginRow; row <= endRow; ++row )
  {
    for ( int column = beginColumn; column <= endColumn; ++column )
    {
      painter.drawRect( column*squareSize, row*squareSize, squareSize, squareSize );
    }
//! [8] //! [9]
  }
//! [9]

//! [10]
  QFontMetrics fontMetrics( displayFont );
  painter.setPen( QPen( Qt::black ) );
  for ( int row = beginRow; row <= endRow; ++row )
  {

    for ( int column = beginColumn; column <= endColumn; ++column )
    {

      int key = row * columns + column;
      painter.setClipRect( column*squareSize, row*squareSize, squareSize, squareSize );

      if ( key == lastKey )
        painter.fillRect( column*squareSize + 1, row*squareSize + 1, squareSize, squareSize, QBrush( Qt::red ) );

      painter.drawText( column*squareSize + ( squareSize / 2 ) - fontMetrics.width( QChar( key ) ) / 2,
                        row*squareSize + 4 + fontMetrics.ascent(),
                        QString( QChar( key ) ) );
    }
  }
}
//! [10]
