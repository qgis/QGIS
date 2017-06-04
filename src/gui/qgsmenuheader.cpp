/***************************************************************************
                             qgsmenuheader.cpp
                             -----------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmenuheader.h"
#include <QPainter>
#include <QApplication>

#define LABEL_SIZE 20 //label rect height
#define LABEL_MARGIN 4 //spacing between label box and text

QgsMenuHeader::QgsMenuHeader( const QString &text, QWidget *parent )
  : QWidget( parent )
  , mText( text )
{
  int textMinWidth = fontMetrics().width( mText );
  mMinWidth = 2 * LABEL_MARGIN + textMinWidth;
  setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  updateGeometry();
}

QSize QgsMenuHeader::minimumSizeHint() const
{
  return QSize( mMinWidth, LABEL_SIZE );
}

QSize QgsMenuHeader::sizeHint() const
{
  return QSize( mMinWidth, LABEL_SIZE );
}

void QgsMenuHeader::paintEvent( QPaintEvent * )
{
  QPainter painter( this );
  QPalette pal = QPalette( qApp->palette() );
  QColor headerBgColor = pal.color( QPalette::Mid );
  QColor headerTextColor = pal.color( QPalette::BrightText );

  //draw header background
  painter.setBrush( headerBgColor );
  painter.setPen( Qt::NoPen );
  painter.drawRect( QRect( 0, 0, width(), LABEL_SIZE ) );

  //draw header text
  painter.setPen( headerTextColor );
  painter.drawText( QRect( LABEL_MARGIN, 0, width() - 2 * LABEL_MARGIN, LABEL_SIZE ),
                    Qt::AlignLeft | Qt::AlignVCenter, mText );
  painter.end();
}

QgsMenuHeaderWidgetAction::QgsMenuHeaderWidgetAction( const QString &text, QObject *parent )
  : QWidgetAction( parent )
{
  QgsMenuHeader *w = new QgsMenuHeader( text, nullptr );
  setDefaultWidget( w ); //transfers ownership
}
