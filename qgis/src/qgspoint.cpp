/***************************************************************************
                          qgspoint.cpp -  description
                             -------------------
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
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
#include <iostream>
#include <qstring.h>
#include <qtextstream.h>
#include "qgspoint.h"
QgsPoint::QgsPoint()
{
}

QgsPoint::QgsPoint(double x, double y):m_x(x), m_y(y)
{

}

QgsPoint::~QgsPoint()
{
}

double QgsPoint::x() const
{
  return m_x;
}

double QgsPoint::y() const
{
  return m_y;
}

int QgsPoint::xToInt()
{
  return (int) m_x;
}

int QgsPoint::yToInt()
{
  return (int) m_y;
}

QString QgsPoint::stringRep() const
{
  QString rep;
  QTextOStream ot(&rep);
  ot.precision(12);
  ot << m_x << ", " << m_y;
  return rep;
}

// operators
bool QgsPoint::operator==(const QgsPoint & other)
{
  if ((m_x == other.x()) && (m_y == other.y()))
    return true;
  else
    return false;
}

bool QgsPoint::operator!=(const QgsPoint & other)
{
  if ((m_x == other.x()) && (m_y == other.y()))
    return false;
  else
    return true;
}

QgsPoint & QgsPoint::operator=(const QgsPoint & other)
{
  if (&other != this)
    {
      m_x = other.x();
      m_y = other.y();
    }

  return *this;
}
