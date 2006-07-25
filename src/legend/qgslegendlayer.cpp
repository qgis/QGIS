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

#include "qgsapplication.h"
#include "qgslegendlayer.h"
#include "qgslegendlayerfile.h"
#include "qgslegendlayerfilegroup.h"
#include "qgsmaplayer.h"
#include <iostream>
#include <QCoreApplication>
#include <QIcon>
#include <QPainter>

QgsLegendLayer::QgsLegendLayer(QTreeWidgetItem* parent,QString name)
    : QgsLegendItem(parent, name)
{
  mType=LEGEND_LAYER;
  setFlags(Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  setCheckState (0, Qt::Checked);
  setText(0, name);
}

QgsLegendLayer::QgsLegendLayer(QTreeWidget* parent, QString name): QgsLegendItem(parent, name)
{
  mType=LEGEND_LAYER;
  setFlags(Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  setCheckState (0, Qt::Checked);
  setText(0, name);
}

QgsLegendLayer::QgsLegendLayer(QString name): QgsLegendItem()
{
  mType=LEGEND_LAYER;
  setFlags(Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  setCheckState (0, Qt::Checked);
  setText(0, name);
}

QgsLegendLayer::~QgsLegendLayer()
{
  mType=LEGEND_LAYER;
}

void QgsLegendLayer::setLayerTypeIcon()
{
  /*QgsMapLayer* firstLayer = firstMapLayer();
  if(firstLayer)
    {
      QFileInfo file(firstLayer->layerTypeIconPath());
      if(file.exists())
	{
	  QIcon myIcon(file.absoluteFilePath());
	  setIcon(0, myIcon);
	}
	}*/
  QIcon myIcon(getOriginalPixmap());
  setIcon(0, myIcon);
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
  if(li && li != this)
    {
      LEGEND_ITEM_TYPE type = li->type();
      if ( type == LEGEND_LAYER)
	{
	  //if(parent() == li->parent())
	  //{
	      return REORDER;
	      //}
	}
      else if(type == LEGEND_GROUP)
	{
	  //only parent legend layers can change positions with groups
	  if(parent() == 0)
	    {
	      return REORDER;
	    }
	}
    }
  return NO_ACTION;
}

QgsMapLayer* QgsLegendLayer::firstMapLayer() const
{
  //first find the legend layer file group
  QgsLegendLayerFileGroup* llfg = 0;
  for(int i = 0; i < childCount(); ++i)
    {
      llfg = dynamic_cast<QgsLegendLayerFileGroup*>(child(i));
      if(llfg)
	{
	  break;
	}
    }

  if(!llfg)
    {
      return 0;
    }

  QTreeWidgetItem* llf = llfg->child(0);
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
}

std::list<QgsLegendLayerFile*> QgsLegendLayer::legendLayerFiles()
{
  std::list<QgsLegendLayerFile*> list;
  
  //find the layer file group
  QgsLegendLayerFileGroup* theLayerGroup = 0;
  for(int i = 0; i < childCount(); ++i)
    {
      theLayerGroup = dynamic_cast<QgsLegendLayerFileGroup*>(child(i));
      if(theLayerGroup)
	{
	  break;
	}
    }

  //add all the legend layer files in the group
  if(theLayerGroup)
    {
      QgsLegendLayerFile* theFile = 0;
      for(int i = 0; i < theLayerGroup->childCount(); ++i)
	{
	  theFile = dynamic_cast<QgsLegendLayerFile*>(theLayerGroup->child(i));
	  if(theFile)
	    {
	      list.push_back(theFile);
	    }
	}
    }
  return list;
}

void QgsLegendLayer::updateLayerSymbologySettings(const QgsMapLayer* mapLayer)
{
  if(mapLayer)
    {
      //find all layers
      std::list<QgsMapLayer*> theMapLayers = mapLayers();
      for(std::list<QgsMapLayer*>::iterator it = theMapLayers.begin(); it != theMapLayers.end(); ++it)
	{
	  if((*it) != mapLayer)
	    {
	      (*it)->copySymbologySettings(*mapLayer);
	    }
	}
    }
}

void QgsLegendLayer::updateCheckState()
{
  std::list<QgsLegendLayerFile*> llfiles = legendLayerFiles();
  if(llfiles.size() < 1)
    {
      return;
    }

  std::list<QgsLegendLayerFile*>::iterator iter = llfiles.begin();
  Qt::CheckState theState = (*iter)->checkState(0);
  for(; iter != llfiles.end(); ++iter)
    {
      if(theState != (*iter)->checkState(0))
	{
	  theState = Qt::PartiallyChecked;
	  break;
	}
    }

  if(theState != checkState(0))
    {
      treeWidget()->blockSignals(true);
      setCheckState(0, theState);
      treeWidget()->blockSignals(false);
    }
}

void QgsLegendLayer::updateIcon()
{
  QPixmap newIcon(getOriginalPixmap());

  QgsMapLayer* theLayer = firstMapLayer();

  if(mapLayers().size() == 1)
    {
  
      //overview
      if(theLayer->showInOverviewStatus())
	{
	  // Overlay the overview icon on the default icon
	  QPixmap myPixmap(QgsApplication::themePath()+"mIconOverview.png");
	  QPainter p(&newIcon);
	  p.drawPixmap(0,0,myPixmap);
	  p.end();
	}
      
      //editable
      if(theLayer->isEditable())
	{
	  // Overlay the editable icon on the default icon
	  QPixmap myPixmap(QgsApplication::themePath()+"mIconEditable.png");
	  QPainter p(&newIcon);
	  p.drawPixmap(0,0,myPixmap);
	  p.end();
	}
    }

  QIcon theIcon(newIcon);
  setIcon(0, theIcon);
}

QPixmap QgsLegendLayer::getOriginalPixmap() const
{
  QgsMapLayer* firstLayer = firstMapLayer();
  if(firstLayer)
    {
      QFileInfo file(firstLayer->layerTypeIconPath());
      if(file.exists())
	{
	  return QPixmap(file.absoluteFilePath());
	}
    }
  QPixmap emptyPixmap;
  return emptyPixmap;
}
