/***************************************************************************
                         qgslegendlayerfilegroup.cpp  -  description
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

#include "qgslegendlayerfilegroup.h"
#include "qgslegendlayerfile.h"
#include "qgslegendsymbologygroup.h"
#include "qgsmaplayer.h"

QgsLegendLayerFileGroup::QgsLegendLayerFileGroup(QTreeWidgetItem* theItem, QString theString): QgsLegendItem(theItem, theString)
{
    mType = LEGEND_LAYER_FILE_GROUP;
    setText(0, theString);
}

QgsLegendItem::DRAG_ACTION QgsLegendLayerFileGroup::accept(LEGEND_ITEM_TYPE type)
{
    if ( type == LEGEND_LAYER_FILE )
      {
	return INSERT; //there should be a way to already test, if the layers are symbology compatible
      }
    else
      {
	return NO_ACTION;
      }
}

QgsLegendItem::DRAG_ACTION QgsLegendLayerFileGroup::accept(const QgsLegendItem* li) const
{
#ifdef QGISDEBUG
  qWarning("in QgsLegendLayerFileGroup::accept");
#endif
  if(li)
    {
      LEGEND_ITEM_TYPE type = li->type();
      if ( type == LEGEND_LAYER_FILE && this != li->parent())
      {
	if(child(0) == 0)
	  {
	    return INSERT;
	  }
	else
	  {
	    QgsLegendLayerFile* llf = dynamic_cast<QgsLegendLayerFile*>(child(0));
	    if(llf)
	      {
		QgsMapLayer* childlayer = llf->layer();
		QgsMapLayer* newlayer = (dynamic_cast<const QgsLegendLayerFile*>(li))->layer();
		if(newlayer->isSymbologyCompatible(*childlayer))
		  {
		    return INSERT;
		  }
	      }
	  }
      }
    }
  return NO_ACTION;
}

bool QgsLegendLayerFileGroup::insert(QgsLegendItem* newItem, bool changesettings)
{
  if ( newItem->type() == LEGEND_LAYER_FILE )
    {
      QgsLegendItem* oldItem = firstChild();
      if(!oldItem)//this item is the first child
	{
	  insertChild(0, newItem);
	  if(!changesettings)
	    {
	      return true;
	    }
	  QgsMapLayer* newLayer = (dynamic_cast<QgsLegendLayerFile*>(newItem))->layer();
	  QTreeWidgetItem* nexts = nextSibling();
	  if(nexts)
	    {
	      QgsLegendSymbologyGroup* sg = dynamic_cast<QgsLegendSymbologyGroup*>(nexts);
	      if(sg)
		{
		  newLayer->setLegendSymbologyGroupParent(sg);
		  newLayer->refreshLegend();
		}
	    }
	  return true;
	}
      //if there are already legend layer files, copy the symbology settings if the two layers are
      //symbology compatible (the same number and type of attributes)
      
      //find the lowest sibling
      while(oldItem->nextSibling() != 0)
	{
	  oldItem = oldItem->nextSibling();
	}
      QgsLegendLayerFile* thefile = dynamic_cast<QgsLegendLayerFile*>(oldItem);
      
      if(!thefile)
	{
	  return false;
	}
      QgsMapLayer* thelayer = thefile->layer();
      if(!thelayer)
	{
	  return false;
	}
      QgsMapLayer* newLayer = (dynamic_cast<QgsLegendLayerFile*>(newItem))->layer();
      if(newLayer->isSymbologyCompatible(*thelayer))
	{	
	  if(changesettings)
	    {
	      if(!newLayer->copySymbologySettings(*thelayer))
		{
		  return false;
		}
	    }
	  insertChild(childCount(), newItem);
	  if(!changesettings)
	    {
	      return true;
	    }
	  QTreeWidgetItem* nexts = nextSibling();
	  if(nexts)
	    {
	      QgsLegendSymbologyGroup* sg = dynamic_cast<QgsLegendSymbologyGroup*>(nexts);
	      if(sg)
		{
		  newLayer->setLegendSymbologyGroupParent(sg);
		}
	    }
	  return true;
	}
      else
	{
	  return false;
	}  
    }
  else
    {
      return false;
    }
}

