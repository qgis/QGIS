/***************************************************************************
                         qgslegendlayerfilegroup.h  -  description
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

#include "qgslegenditem.h"

class QgsLegendLayerFileGroup: public QgsLegendItem
{
 public:
    QgsLegendLayerFileGroup(QListViewItem* theItem, QString theString);
    bool isLeafNode() {return false;}
    DRAG_ACTION accept(LEGEND_ITEM_TYPE type);
    bool insert(QgsLegendItem* theItem);
};
