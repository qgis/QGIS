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
#include "qgslegendlayer.h"
#include <qlistview.h>
#include <qpixmap.h>
#include <iostream>

QgsLegendLayer::QgsLegendLayer(QListViewItem * parent,QString name)
    : QgsLegendItem(parent, name)
{
  mType=LEGEND_LAYER;
  QPixmap myPixmap("/home/marco/sourcen/treeview/layer.png");
  setPixmap(0,myPixmap);
}

QgsLegendLayer::QgsLegendLayer(QListView * parent,QString name)
    : QgsLegendItem(parent, name)
{
  mType=LEGEND_LAYER;
  QPixmap myPixmap("/home/marco/sourcen/treeview/layer.png");
  setPixmap(0,myPixmap);
}
QgsLegendLayer::~QgsLegendLayer()
{
  mType=LEGEND_LAYER;
}

bool QgsLegendLayer::isLeafNode()
{
  return false;
}

bool QgsLegendLayer::accept(LEGEND_ITEM_TYPE type)
{
  if(type==LEGEND_PROPERTY_GROUP || type == LEGEND_SYMBOL_GROUP || type==LEGEND_LAYER_FILE)
  {
    QListViewItem * myItem = this->firstChild();
    if (myItem==0)
    {
      return true;
    }
    do
    {
      LEGEND_ITEM_TYPE curtype = dynamic_cast<QgsLegendItem *>(myItem)->type();
      std::cout << myItem->text(0) << " - " << curtype << " = " << this->text(0) << " - " << type << "\n";
      if (curtype == type)
      {
        return false;
      }
    }
    while ((myItem = myItem->nextSibling()) != 0);


    return true;
  }
  else
  {
    return false;
  }
}
