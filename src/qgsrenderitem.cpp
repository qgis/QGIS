/***************************************************************************
                         qgsrenderer.cpp  -  description
                             -------------------
    begin                : Sat Jan 4 2003
    copyright            : (C) 2003 by Gary E.Sherman
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
#include <qstring.h>
#include "qgsrenderitem.h"

QgsRenderItem::QgsRenderItem()
{
};

QgsRenderItem::QgsRenderItem(QgsSymbol symbol, QString _value, QString _label):
sym(symbol), m_value(_value), m_label(_label)
{

}

void QgsRenderItem::setLabel(QString label)
{
  m_label = label;
}

void QgsRenderItem::setSymbol(QgsSymbol s)
{
  sym = s;
}

const QString & QgsRenderItem::label() const const
{
  return m_label;
}
