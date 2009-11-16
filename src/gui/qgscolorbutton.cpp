/***************************************************************************
    qgscolorbutton.cpp - Button which displays a color
     --------------------------------------
    Date                 : 12-Dec-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id: qgscolorbutton.cpp 6251 2006-12-13 23:23:50Z telwertowski $ */

#include "qgscolorbutton.h"
#include <QPainter>

/*!
  \class QgsColorButton

  \brief The QgsColorButton class provides a tool button widget displaying
  a color which can be altered by calling QColorDialog::getColor.

  A subclass of QToolButton is needed to draw the button content because
  some platforms such as Mac OS X and Windows XP enforce a consistent
  GUI look by always using the button color of the current style and
  not allowing button backgrounds to be changed on a button by button basis.

  This class is a simplified version of QtColorButton, an internal class used
  by Qt Designer to do the same thing.
*/

QgsColorButton::QgsColorButton( QWidget *parent )
    : QToolButton( parent )
{
  setToolButtonStyle( Qt::ToolButtonTextOnly ); // decrease default button height
}

QgsColorButton::~QgsColorButton()
{}

/*!
  Paints button in response to a paint event.
*/
void QgsColorButton::paintEvent( QPaintEvent *e )
{
  QToolButton::paintEvent( e );
  if (
#ifdef Q_WS_MAC
    // Mac shows color only a when a window is active
    isActiveWindow() &&
#endif
    isEnabled() )
  {
    QPainter p( this );
    int margin = 2;  // Leave some space for highlighting
    QRect r = rect().adjusted( margin, margin, -margin, -margin );
    p.fillRect( r, mColor );
  }
}

void QgsColorButton::setColor( const QColor &color )
{
  mColor = color;
  update();
}


//////////////////

QgsColorButtonV2::QgsColorButtonV2( QWidget* parent )
 : QPushButton(parent)
{
}

void QgsColorButtonV2::setColor( const QColor &color )
{
  mColor = color;

  QPixmap pixmap(iconSize());
  pixmap.fill(QColor(0,0,0,0));

  QRect rect(1,1, iconSize().width() - 2, iconSize().height() - 2);

  // draw a slightly rounded rectangle
  QPainter p;
  p.begin(&pixmap);
  p.setPen(Qt::NoPen);
  p.setRenderHint(QPainter::Antialiasing);
  p.setBrush(color);
  p.drawRoundedRect(rect, 4, 4);
  p.end();

  // set this pixmap as icon
  setIcon(QIcon(pixmap));
}
