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

#include <QTreeWidget>
#include <QTreeWidgetItem>

class QgsLegend;
class QgsLegendGroup;
class QgsLegendLayer;
class QgsLegendPropertyGroup;
class QgsLegendPropertyItem;

/**
This is an abstract base class that all qgis legen items inerit from

@author Tim Sutton
*/
class QgsLegendItem : public QTreeWidgetItem, public QObject
{

  public:
    QgsLegendItem( QTreeWidgetItem*, QString );
    QgsLegendItem( QTreeWidget*, QString );
    QgsLegendItem();
    virtual ~QgsLegendItem();

    enum LEGEND_ITEM_TYPE
    {
      LEGEND_GROUP,
      LEGEND_LAYER,
      LEGEND_PROPERTY_GROUP,
      LEGEND_PROPERTY_ITEM,
      LEGEND_SYMBOL_GROUP,
      LEGEND_SYMBOL_ITEM,
      LEGEND_VECTOR_SYMBOL_ITEM,
    };

    /**Describes the action that will be done if the mouse button will be released after a drag*/
    enum DRAG_ACTION
    {
      REORDER, //change order of items (drag with left mouse button)
      INSERT, //insert an item into another one (drag with middle mouse button)
      NO_ACTION //do nothing
    };

    virtual bool isLeafNode() = 0;
    virtual LEGEND_ITEM_TYPE type() const {return mType;}
    /**Returns the type of action that will be done if a drag, originating at a certain
     item type, will be released at this item*/
    virtual DRAG_ACTION accept( LEGEND_ITEM_TYPE type ) = 0;
    /**Retrns the type of action that will be done if a legend item is dragged over this item*/
    virtual DRAG_ACTION accept( const QgsLegendItem* li ) const = 0;
    /**Subclasses which allow insertion of other items may implement this method.
       @param theItem legend item to insert into this item
       @param changesettings Some insert actions may change the state of the layers or the map canvas.
       If false, such settings don't change (e.g. during mouse drags). If true, the layers and/or map canvas
       settings are allowed to change (e.g. if the mouse button is released).
       @return true in case of success and false if theItem cannot be inserted*/
    virtual bool insert( QgsLegendItem* theItem ) {return false;}
    void print( QgsLegendItem * theItem );
    /**Returns a pointer to the first child or 0 if there is none*/
    QgsLegendItem* firstChild();
    /**Returns the older sibling or 0 if there is none*/
    QgsLegendItem* nextSibling();
    /**Returns the younger sibling or 0 if this item is the first child of its parent*/
    QgsLegendItem* findYoungerSibling();
    /**Moves this item after (as an older sibling) another item*/
    void moveItem( QgsLegendItem* after );
    /**Removes all the children of this item. This function is for qt-4.0.1 compatibility, where
     'takeChildren()' does not yet exist*/
    void removeAllChildren();
    /**Stores the current appearance settings (expanded, hidden). The purpose of this is that if an item is removed
     from its position and insert into another position, these settings can be restored after insertion. storeAppearanceSettings is
    also called for all childs*/
    void storeAppearanceSettings();
    /**Restore appearanc settings (expanded and hidden) e.g. after being inserted into a new place in the tree widget*/
    void restoreAppearanceSettings();
    /**Returns a pointer to the legend widget*/
    QgsLegend* legend() const;
    /**Returns child of the item - convenience function to distinguish between
       QTreeWidgetItem's and QObject's child() function */
    QTreeWidgetItem* child( int i ) const;
    /**Returns parent of the item - convenience function to distinguish between
       QTreeWidgetItem's and QObject's parent() function */
    QTreeWidgetItem* parent() const;
    /**Inserts child - convenience function to distinguish between
      QTreeWidgetItem's and QObject's insertChild() function */
    void insertChild( int index, QTreeWidgetItem *child );
    /**Do preparations after a new child was inserted (default empty)*/
    virtual void receive( QgsLegendItem* newChild ) {}
    /**Do cleanups after a child item leaves (default empty)*/
    virtual void release( QgsLegendItem* formerChild ) {}

  protected:
    bool mLeafNodeFlag;
    LEGEND_ITEM_TYPE mType;
    /**Stores expanded property when storeAppearanceSettings is called*/
    bool mExpanded;
    /**Stores hidden property when storeAppearanceSettings is called*/
    bool mHidden;
};

#endif
