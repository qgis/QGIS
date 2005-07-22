/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton   *
 *   aps02ts@macbuntu   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef QGSLEGENDITEM_H
#define QGSLEGENDITEM_H

#include <qlistview.h>
#include <qpopupmenu.h>

class QgsLegendGroup;
class QgsLegendLayer;
class QgsLegendPropertyGroup;
class QgsLegendPropertyItem;

/**
This is an abstract base class that all qgis legen items inerit from

@author Tim Sutton
*/
class QgsLegendItem : public QListViewItem
{


public:
    QgsLegendItem(QListViewItem*, QString);
    QgsLegendItem (QListView *,QString);
    ~QgsLegendItem();
   enum LEGEND_ITEM_TYPE 
   {
     LEGEND_GROUP,
     LEGEND_LAYER,
     LEGEND_PROPERTY_GROUP,
     LEGEND_PROPERTY_ITEM,
     LEGEND_SYMBOL_GROUP,
     LEGEND_SYMBOL_ITEM,
     LEGEND_VECTOR_SYMBOL_ITEM,
     LEGEND_LAYER_FILE
   } ;
    virtual bool isLeafNode()=0;
    virtual const LEGEND_ITEM_TYPE type(){return mType;}
    virtual void addItem(QgsLegendItem*){}
    virtual bool accept(LEGEND_ITEM_TYPE type)=0;
    /**Specifies the items reaction to a double click*/
    virtual void handleDoubleClickEvent(){}
    virtual void handleRightClickEvent(const QPoint& position) {}
    void print(QgsLegendItem * theItem);
protected:
   bool mLeafNodeFlag;
   LEGEND_ITEM_TYPE mType; 
};

#endif
