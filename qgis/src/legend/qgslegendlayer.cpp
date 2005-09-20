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
#include "qgslegendlayerfile.h"
#include "qgsmaplayer.h"
#include <qapplication.h>
#include <qlistview.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <iostream>

QgsLegendLayer::QgsLegendLayer(QListViewItem * parent,QString name)
    : QObject(), QgsLegendItem(parent, name)
{
    mType=LEGEND_LAYER;
#if defined(Q_OS_MACX) || defined(WIN32)
    QString pkgDataPath(qApp->applicationDirPath()+QString("/share/qgis"));
#else
    QString pkgDataPath(PKGDATAPATH);
#endif
    QPixmap myPixmap(pkgDataPath+QString("/images/icons/layer.png"));
    setPixmap(0,myPixmap);
}

QgsLegendLayer::QgsLegendLayer(QListView* parent, QString name): QObject(), QgsLegendItem(parent, name)
{
    mType=LEGEND_LAYER;
#if defined(Q_OS_MACX) || defined(WIN32)
    QString pkgDataPath(qApp->applicationDirPath()+QString("/share/qgis"));
#else
    QString pkgDataPath(PKGDATAPATH);
#endif
    QPixmap myPixmap(pkgDataPath+QString("/images/icons/layer.png"));
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

QgsLegendItem::DRAG_ACTION QgsLegendLayer::accept(LEGEND_ITEM_TYPE type)
{
    if ( type == LEGEND_LAYER)
    {
	return REORDER;
    }
    else
    {
	return NO_ACTION;
    }
}

void QgsLegendLayer::handleRightClickEvent(const QPoint& position)
{
    /*QgsMapLayer* ml = firstMapLayer();
    if(ml)
    {
	QPopupMenu *mPopupMenu = ml->contextMenu();
	if (mPopupMenu)
	{
	    mPopupMenu->exec(position);
	}
	}*/
}

QgsMapLayer* QgsLegendLayer::firstMapLayer()
{
    QListViewItem* llfgroup = firstChild(); //the legend layer file group
    if(!llfgroup)
    {
	return 0;
    }
    QListViewItem* llf = llfgroup->firstChild();
    if(!llf)
    {
	return 0;
    }
    QgsLegendLayerFile* legendlayerfile = dynamic_cast<QgsLegendLayerFile*>(llf);
    if (legendlayerfile)
    {
	return legendlayerfile->layer();
    }
    else
    {
	return 0;
    }
}

std::list<QgsMapLayer*> QgsLegendLayer::mapLayers()
{
    std::list<QgsMapLayer*> list;
    QListViewItem* llfgroup = firstChild(); //the legend layer file group
    if(!llfgroup)
    {
	return list;
    }
    QListViewItem* llf = llfgroup->firstChild();
    if(!llf)
    {
	return list;
    }
    QgsLegendLayerFile* legendlayerfile = 0;
    do
    {
	legendlayerfile = dynamic_cast<QgsLegendLayerFile*>(llf);
	if(legendlayerfile)
	{
	    list.push_back(legendlayerfile->layer());
	}
    }
    while(llf = llf->nextSibling());

    return list;
}
