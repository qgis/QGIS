/***************************************************************************
                         qgsrangerenderitem.cpp  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
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
#include "qgsrangerenderitem.h"

QgsRangeRenderItem::QgsRangeRenderItem():QgsRenderItem()
{

}

QgsRangeRenderItem::QgsRangeRenderItem(QgsSymbol* symbol, QString _value, QString u_value, QString _label):QgsRenderItem(symbol, _value, _label),
m_upper_value(u_value)
{

}

void QgsRangeRenderItem::setUpperValue(QString value)
{
  m_upper_value = value;
}

const QString & QgsRangeRenderItem::upper_value() const
{
  return m_upper_value;
}
