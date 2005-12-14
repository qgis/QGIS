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
#include <QCoreApplication>
#include <QIcon>

QgsLegendGroup::QgsLegendGroup(QTreeWidgetItem * theItem ,QString theName)
    : QgsLegendItem(theItem,theName)
{
  mType=LEGEND_GROUP;
#if defined(Q_OS_MACX) || defined(WIN32)
  QString pkgDataPath(QCoreApplication::applicationDirPath()+QString("/share/qgis"));
#else
  QString pkgDataPath(PKGDATAPATH);
#endif
  setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
  QIcon myIcon(pkgDataPath+QString("/images/icons/folder.png"));
  setIcon(0, myIcon);
}
QgsLegendGroup::QgsLegendGroup(QTreeWidget* theListView, QString theString)
    : QgsLegendItem(theListView,theString)
{
  mType=LEGEND_GROUP;
#if defined(Q_OS_MACX) || defined(WIN32)
  QString pkgDataPath(QCoreApplication::applicationDirPath()+QString("/share/qgis"));
#else
  QString pkgDataPath(PKGDATAPATH);
#endif
  setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled);
  QIcon myIcon(pkgDataPath+QString("/images/icons/folder.png"));
  setIcon(0, myIcon);
}

QgsLegendGroup::~QgsLegendGroup()
{}


bool QgsLegendGroup::isLeafNode()
{
  return mLeafNodeFlag;
}

QgsLegendItem::DRAG_ACTION QgsLegendGroup::accept(LEGEND_ITEM_TYPE type)
{
    if ( type == LEGEND_GROUP )
    {
	return REORDER;
    }
    if( type == LEGEND_LAYER )
    {
	return INSERT;
    }
    else
    {
	return NO_ACTION;
    }
}

QgsLegendItem::DRAG_ACTION QgsLegendGroup::accept(const QgsLegendItem* li) const
{
  if(li)
    {
      LEGEND_ITEM_TYPE type = li->type();
      if ( type == LEGEND_GROUP )
	{
	  return REORDER;
	}
      if( type == LEGEND_LAYER )
	{
	  return INSERT;
	}
    }
  return NO_ACTION;
}

bool QgsLegendGroup::insert(QgsLegendItem* theItem, bool changesettings)
{
    if(theItem->type() == LEGEND_LAYER)
    {
	addChild(theItem);
    }
}
