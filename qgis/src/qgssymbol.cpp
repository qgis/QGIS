/***************************************************************************
                          QgsSymbol.cpp  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
       Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbol.h"

QgsSymbol::QgsSymbol(QColor c)
{
  m_pen.setColor(c);
  m_brush.setColor(c);
}

QgsSymbol::~QgsSymbol()
{
}
QColor QgsSymbol::color() const const
{
  return m_pen.color();
}

void QgsSymbol::setColor(QColor c)
{
  m_pen.setColor(c);
}

QColor QgsSymbol::fillColor() const const
{
  return m_brush.color();
}

void QgsSymbol::setFillColor(QColor c)
{
  m_brush.setColor(c);
}

int QgsSymbol::lineWidth() const const
{
  return m_pen.width();
}

void QgsSymbol::setLineWidth(int w)
{
  m_pen.setWidth(w);
}
