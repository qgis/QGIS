/***************************************************************************
                         qgslegendvectorsymbologyitem.cpp  -  description
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

#include "qgslegendvectorsymbologyitem.h"

QgsLegendVectorSymbologyItem::QgsLegendVectorSymbologyItem( QTreeWidgetItem * theItem, QString col1, int pixmapWidth, int pixmapHeight )
    : QgsLegendSymbologyItem( theItem, col1, pixmapWidth, pixmapHeight )
{
  mType = LEGEND_VECTOR_SYMBOL_ITEM;
}

void QgsLegendVectorSymbologyItem::addSymbol( QgsSymbol* s )
{
  mSymbols.push_back( s );
}

void QgsLegendVectorSymbologyItem::handleDoubleClickEvent()
{
  //todo: show the dialog
  //QgsSiSyDialog d(0);
  //std::list<QgsSymbol*>::iterator iter = mSymbols.begin();
  //d.set(*iter);
  //d.exec();
}
