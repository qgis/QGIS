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
#include <iostream>
#include <QCoreApplication>
#include <QIcon>

QgsLegendLayer::QgsLegendLayer(QTreeWidgetItem* parent,QString name)
    : QObject(), QgsLegendItem(parent, name)
{
    mType=LEGEND_LAYER;
#if defined(Q_OS_MACX) || defined(WIN32)
    QString pkgDataPath(QCoreApplication::applicationDirPath()+QString("/share/qgis"));
#else
    QString pkgDataPath(PKGDATAPATH);
#endif
    setFlags(Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    QIcon myIcon(pkgDataPath+QString("/images/icons/layer.png"));
    setCheckState (0, Qt::Checked);
    setText(0, name);
    setIcon(0, myIcon);
}

QgsLegendLayer::QgsLegendLayer(QTreeWidget* parent, QString name): QObject(), QgsLegendItem(parent, name)
{
    mType=LEGEND_LAYER;
#if defined(Q_OS_MACX) || defined(WIN32)
    QString pkgDataPath(QCoreApplication::applicationDirPath()+QString("/share/qgis"));
#else
    QString pkgDataPath(PKGDATAPATH);
#endif
    setFlags(Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    setCheckState (0, Qt::Checked);
    QIcon myIcon(pkgDataPath+QString("/images/icons/layer.png"));
    setText(0, name);
    setIcon(0, myIcon);
}

QgsLegendLayer::QgsLegendLayer(QString name): QObject(), QgsLegendItem()
{
  mType=LEGEND_LAYER;
#if defined(Q_OS_MACX) || defined(WIN32)
    QString pkgDataPath(QCoreApplication::applicationDirPath()+QString("/share/qgis"));
#else
    QString pkgDataPath(PKGDATAPATH);
#endif
    setFlags(Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    setCheckState (0, Qt::Checked);
    QIcon myIcon(pkgDataPath+QString("/images/icons/layer.png"));
    setText(0, name);
    setIcon(0, myIcon);
}

QgsLegendLayer::~QgsLegendLayer()
{
  mType=LEGEND_LAYER;
}

void QgsLegendLayer::setLayerTypeIcon()
{
  QgsMapLayer* firstLayer = firstMapLayer();
  if(firstLayer)
    {
      QFileInfo file(firstLayer->layerTypeIconPath());
      if(file.exists())
	{
	  QIcon myIcon(file.absoluteFilePath());
	  setIcon(0, myIcon);
	}
    }
}

bool QgsLegendLayer::isLeafNode()
{
  return false;
}

QgsLegendItem::DRAG_ACTION QgsLegendLayer::accept(LEGEND_ITEM_TYPE type)
{
    if ( type == LEGEND_LAYER || type == LEGEND_GROUP)
    {
	return REORDER;
    }
    else
    {
	return NO_ACTION;
    }
}

QgsLegendItem::DRAG_ACTION QgsLegendLayer::accept(const QgsLegendItem* li) const
{
#ifdef QGISDEBUG
  qWarning("in QgsLegendLayer::accept");
#endif
  if(li && li != this)
    {
      LEGEND_ITEM_TYPE type = li->type();
      if ( type == LEGEND_LAYER || type == LEGEND_GROUP)
	{
	  return REORDER;
	}
    }
  return NO_ACTION;
}

QgsMapLayer* QgsLegendLayer::firstMapLayer()
{
  QTreeWidgetItem* llfgroup = QTreeWidgetItem::child(0); //the legend layer file group
  if(!llfgroup)
    {
      return 0;
    }
  QTreeWidgetItem* llf = llfgroup->child(0);
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
    std::list<QgsLegendLayerFile*> llist = legendLayerFiles();
    for(std::list<QgsLegendLayerFile*>::iterator it = llist.begin(); it != llist.end(); ++it)
      {
	list.push_back((*it)->layer());
      }
    return list;

#if 0
    QTreeWidgetItem* llfgroup = QTreeWidgetItem::child(0); //the legend layer file group
    if(!llfgroup)
    {
	return list;
    }
    QgsLegendItem* llf = dynamic_cast<QgsLegendItem*>(llfgroup->child(0));
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
#endif
}

std::list<QgsLegendLayerFile*> QgsLegendLayer::legendLayerFiles()
{
  std::list<QgsLegendLayerFile*> list;
  QTreeWidgetItem* llfgroup = QTreeWidgetItem::child(0); //the legend layer file group
  if(!llfgroup)
    {
      return list;
    }
  QgsLegendItem* llf = dynamic_cast<QgsLegendItem*>(llfgroup->child(0));
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
	  list.push_back(legendlayerfile);
	}
    }
  while(llf = llf->nextSibling());
  return list;
}
