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
/* $Id$ */
#include "qgssymbol.h"
#include <qpainter.h>
QgsSymbol::QgsSymbol()
{

}

QgsSymbol::QgsSymbol(QColor c)
{
  mPen.setColor(c);
  mBrush.setColor(c);
}

QgsSymbol::~QgsSymbol()
{
}
QColor QgsSymbol::color() const
{
  return mPen.color();
}

void QgsSymbol::setColor(QColor c)
{
  mPen.setColor(c);
}

QColor QgsSymbol::fillColor() const
{
  return mBrush.color();
}

void QgsSymbol::setFillColor(QColor c)
{
  mBrush.setColor(c);
}

int QgsSymbol::lineWidth() const
{
  return mPen.width();
}

void QgsSymbol::setLineWidth(int w)
{
  mPen.setWidth(w);
}

QPixmap  QgsSymbol::getSymbolAsPixmap(int xDim, int yDim)
{
        QPixmap myQPixmap(xDim,yDim);
        QPainter myQPainter;
        myQPainter.begin(&myQPixmap);
        myQPainter.setBrush(mBrush);
        myQPainter.setPen(mPen);
        myQPainter.drawRect(0, 0, xDim, yDim);
        myQPainter.end();
        return myQPixmap;

}