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

QString QgsPoint::stringRep() const
{
  QString rep;
  QTextOStream ot(&rep);
  ot.precision(12);
  ot << m_x << ", " << m_y;
  return rep;
}

QString QgsPoint::stringRep(int thePrecision) const
{
  QString rep = QString::number(m_x,'f',thePrecision) + QString(",") + 
  QString::number( m_y,'f',thePrecision);
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
