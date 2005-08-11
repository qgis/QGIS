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
#include "qgslegendgroup.h"

QgsLegendGroup::QgsLegendGroup(QListViewItem * theItem ,QString theName)
    : QgsLegendItem(theItem,theName)
{
  mType=LEGEND_GROUP;
  QPixmap myPixmap(QString(PKGDATAPATH)+QString("/images/icons/group.png"));
  setPixmap(0,myPixmap);
}
QgsLegendGroup::QgsLegendGroup(QListView * theListView, QString theString)
    : QgsLegendItem(theListView,theString)
{
  mType=LEGEND_GROUP;
  QPixmap myPixmap(QString(PKGDATAPATH)+QString("/images/icons/group.png"));
  setPixmap(0,myPixmap);
}

QgsLegendGroup::~QgsLegendGroup()
{}


bool QgsLegendGroup::isLeafNode()
{
  return mLeafNodeFlag;
}

bool QgsLegendGroup::accept(DRAG_TYPE dt, LEGEND_ITEM_TYPE type)
{
    if( dt == QgsLegendItem::REORDER)
    {
	if ( type == LEGEND_GROUP )
	{
	    return true;
	}
	else
	{
	    return false;
	}
    }
    else if( dt == QgsLegendItem::INSERT)
    {
	if( type == LEGEND_LAYER )
	{
	    return true;
	}
	else
	{
	    return false;
	}
    }
}

bool QgsLegendGroup::insert(QgsLegendItem* theItem)
{
    if(theItem->type() == LEGEND_LAYER)
    {
	insertItem(theItem);
    }
}
