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
#include <qcolor.h>
#include "qgssymbol.h"

QgsSymbol::QgsSymbol(QColor c):m_color(c)
{

}
QgsSymbol::QgsSymbol(const QgsSymbol &sym){
	m_color = sym.color();
	m_fillColor = sym.fillColor();
	m_lineWidth = sym.lineWidth();
}
QgsSymbol::~QgsSymbol()
{
}
QColor QgsSymbol::color() const
{
	return m_color;
}

void QgsSymbol::setColor(QColor c)
{
	m_color = c;
}

QColor QgsSymbol::fillColor() const
{
	return m_fillColor;
}

void QgsSymbol::setFillColor(QColor c)
{
	m_fillColor = c;
}

int QgsSymbol::lineWidth() const
{
	return m_lineWidth;
}

void QgsSymbol::setLineWidth(int w)
{
	m_lineWidth = w;
}

QgsSymbol & QgsSymbol::operator=(const QgsSymbol &r1){
	
	if(&r1 != this){
		m_color = r1.color();
		m_fillColor = r1.fillColor();
		m_lineWidth = r1.lineWidth();
	}
	return *this;
}
