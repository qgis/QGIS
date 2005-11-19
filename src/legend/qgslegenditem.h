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

#include <q3listview.h>
#include <q3popupmenu.h>

class QgsLegendGroup;
class QgsLegendLayer;
class QgsLegendPropertyGroup;
class QgsLegendPropertyItem;

/**
This is an abstract base class that all qgis legen items inerit from

@author Tim Sutton
*/
class QgsLegendItem : public Q3ListViewItem
{
public:
    QgsLegendItem(Q3ListViewItem*, QString);
    QgsLegendItem (Q3ListView *,QString);
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
     LEGEND_LAYER_FILE_GROUP,
     LEGEND_LAYER_FILE
   } ;

   /**Describes the action that will be done if the mouse button will be released after a drag*/
   enum DRAG_ACTION
       {
	   REORDER, //change order of items (drag with left mouse button)
	   INSERT, //insert an item into another one (drag with middle mouse button)
	   NO_ACTION //do nothing
       };

    virtual bool isLeafNode()=0;
    virtual LEGEND_ITEM_TYPE type() const {return mType;}
    virtual void addItem(QgsLegendItem*){}
    /**Returns the type of action that will be done if a drag, originating at a certain
     item type, will be released at this item*/
    virtual DRAG_ACTION accept(LEGEND_ITEM_TYPE type)=0;
    /**Retrns the type of action that will be done if a legend item is dragged over this item*/
    virtual DRAG_ACTION accept(const QgsLegendItem* li) const = 0;
    /**Subclasses which allow insertion of other items may implement this method. 
       @param theItem legend item to insert into this item
       @param changesettings Some insert actions may change the state of the layers or the map canvas. 
       If false, such settings don't change (e.g. during mouse drags). If true, the layers and/or map canvas
       settings are allowed to change (e.g. if the mouse button is released).
       @return true in case of success and false if theItem cannot be inserted*/
    virtual bool insert(QgsLegendItem* theItem, bool changesettings = true) {return false;}
    void print(QgsLegendItem * theItem);
    /**Returns the younger sibling or 0 if this item is the first child of its parent*/
    QgsLegendItem* findYoungerSibling();
protected:
   bool mLeafNodeFlag;
   LEGEND_ITEM_TYPE mType; 
};

#endif
