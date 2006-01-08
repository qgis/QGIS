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

bool QgsRangeRenderItem::writeXML( QDomNode & parent, QDomDocument & document )
{
    bool returnval=false;
    QDomElement rangerenderitem=document.createElement("rangerenderitem");
    parent.appendChild(rangerenderitem);
    QDomElement lowervalue=document.createElement("lowervalue");
    QDomText lowervaluetxt=document.createTextNode(mValue);
    lowervalue.appendChild(lowervaluetxt);
    rangerenderitem.appendChild(lowervalue);
    QDomElement uppervalue=document.createElement("uppervalue");
    QDomText uppervaluetxt=document.createTextNode(m_upper_value);
    uppervalue.appendChild(uppervaluetxt);
    rangerenderitem.appendChild(uppervalue);
    if(mSymbol)
    {
	returnval=mSymbol->writeXML(rangerenderitem,document);
    }
    QDomElement label=document.createElement("label");
    QDomText labeltxt=document.createTextNode(mLabel);
    label.appendChild(labeltxt);
    rangerenderitem.appendChild(label);
    return returnval;
}
