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
#ifndef QGSLEGENDSYMBOLOGYGROUP_H
#define QGSLEGENDSYMBOLOGYGROUP_H

#include <qgslegenditem.h>

class QgsMapLayer;

/**
@author Tim Sutton
*/
class QgsLegendSymbologyGroup : public QgsLegendItem
{
public:
  QgsLegendSymbologyGroup(QListViewItem * theItem, QString theString);
  ~QgsLegendSymbologyGroup();
  bool isLeafNode() {return false;}
  DRAG_ACTION accept(LEGEND_ITEM_TYPE type);
  /** Overloads cmpare function of QListViewItem
    * @note The symbology group must always be the second in the list
    */
  int compare (QListViewItem * i,int col, bool ascending);
  /**Copies the symbology settings of the layer to all maplayers in the QgsLegendLayerFileGroup.
   This method should be called whenever a layer in this group changes it symbology settings
  (normally from QgsMapLayer::refreshLegend)*/
  void updateLayerSymbologySettings(const QgsMapLayer* thelayer);
};

#endif
