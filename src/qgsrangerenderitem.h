/***************************************************************************
                         qgsrangerenderitem.h  -  description
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
/* $Id */

#ifndef QGSRANGERENDERITEM_H
#define QGSRANGERENDERITEM_H

#include "qgsrenderitem.h"

/**A Renderer for ranges of values (e.g. used for graduated colors)*/
class QgsRangeRenderItem: public QgsRenderItem
{
 public:
    QgsRangeRenderItem();
    QgsRangeRenderItem(QgsSymbol symbol, QString _value, QString u_value, QString _label);
    void setUpperValue(QString value);
    const QString& upper_value() const;
 protected:
    /**Upper value for a range (the lower value is QgsRenderItem::value)*/
    QString m_upper_value;
};

#endif
