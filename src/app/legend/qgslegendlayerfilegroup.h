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

class QgsLegendLayerFile;

class QgsLegendLayerFileGroup: public QgsLegendItem
{
  public:
    QgsLegendLayerFileGroup( QTreeWidgetItem* theItem, QString theString );
    bool isLeafNode() {return false;}
    DRAG_ACTION accept( LEGEND_ITEM_TYPE type );
    QgsLegendItem::DRAG_ACTION accept( const QgsLegendItem* li ) const;
    bool insert( QgsLegendItem* theItem );
    /**Returns true if llf is a childelement*/
    bool containsLegendLayerFile( const QgsLegendLayerFile* llf ) const;
    /**Makes the parent QgsLegendLayer update the checkState and the icon after a new
     QgsLegendLayerFile was inserted*/
    void receive( QgsLegendItem* newChild );
    /**Cleanups when after a child QgsLegendLayerFile item has left. Tells the parent QgsLegendLayer to
     refresh the checkState and the icon*/
    void release( QgsLegendItem* formerChild );
};
