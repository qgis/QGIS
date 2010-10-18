/***************************************************************************
                         qgslegendvectorsymbologyitem.h  -  description
                             -------------------
    begin                : Juli 2005
    copyright            : (C) 2005 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLEGENDVECTORSYMBOLOGYITEM_H
#define QGSLEGENDVECTORSYMBOLOGYITEM_H

#include <QList>
#include "qgslegendsymbologyitem.h"

class QgsSymbol;

class QgsLegendVectorSymbologyItem: public QgsLegendSymbologyItem
{
  public:
    QgsLegendVectorSymbologyItem( QTreeWidgetItem * theItem, QString col1, int pixmapWidth, int pixmapHeight );
    /**Add a symbol to the list such that it will be updated if necessary*/
    void addSymbol( QgsSymbol* s );
    /**Brings up a single symbol dialog. The changes are then applied to all entries of mSymbols*/
    void handleDoubleClickEvent();
  private:
    /**Collection of pointers to the vector layer symbols associated with this item*/
    QList<QgsSymbol*> mSymbols;
};

#endif
