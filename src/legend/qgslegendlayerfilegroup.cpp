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

QgsLegendLayerFileGroup::QgsLegendLayerFileGroup(QListViewItem* theItem, QString theString): QgsLegendItem(theItem, theString)
{
    mType = LEGEND_LAYER_FILE_GROUP;
}

bool QgsLegendLayerFileGroup::accept(DRAG_TYPE dt, LEGEND_ITEM_TYPE type)
{
#ifdef QGISDEBUG
    qWarning("in QgsLegendLayerFileGroup::accept");
#endif
    if ( dt == QgsLegendItem::INSERT)
    {
#ifdef QGISDEBUG
	qWarning("QgsLegendLayerFileGroup: insert");
#endif
	if ( type == LEGEND_LAYER_FILE )
	{
#ifdef QGISDEBUG
	    qWarning("QgsLegendLayerFileGroup: accept QgsLegendLayerFile");
#endif
	    return true; //there should be a way to already test, if the layers are symbology compatible
	}
    }
    else if ( dt == QgsLegendItem::REORDER)
    {
#ifdef QGISDEBUG
	qWarning("reorder, returning false");
#endif
	return false;
    }
    return false;
}

bool QgsLegendLayerFileGroup::insert(QgsLegendItem* theItem)
{
#ifdef QGISDEBUG
qWarning("insert");
#endif
if ( theItem->type() == LEGEND_LAYER_FILE )
{
    QListViewItem* theitem = firstChild();
    if( !theitem )//this item is the first child
    {
	insertItem(theItem);
	QgsMapLayer* newlayer = (dynamic_cast<QgsLegendLayerFile*>(theItem))->layer();
	QListViewItem* nexts = nextSibling();
	if(nexts)
	{
	    QgsLegendSymbologyGroup* sg = dynamic_cast<QgsLegendSymbologyGroup*>(nexts);
	    if(sg)
	    {
		newlayer->setLegendSymbologyGroupParent(sg);
		newlayer->refreshLegend();
	    }
	}
	return true;
    }
    //if there are already legend layer files, copy the symbology settings if the two layers are
    //symbology compatible (the same number and type of attributes)
    QgsLegendLayerFile* thefile = dynamic_cast<QgsLegendLayerFile*>(theitem);
    if(!thefile)
    {
	return false;
    }
    QgsMapLayer* thelayer = thefile->layer();
    if(!thelayer)
    {
	return false;
    }
    QgsMapLayer* newlayer = (dynamic_cast<QgsLegendLayerFile*>(theItem))->layer();
    if(newlayer->isSymbologyCompatible(*thelayer) && newlayer->copySymbologySettings(*thelayer))
    {	
	insertItem(theItem);
	QListViewItem* nexts = nextSibling();
	if(nexts)
	{
	    QgsLegendSymbologyGroup* sg = dynamic_cast<QgsLegendSymbologyGroup*>(nexts);
	    if(sg)
	    {
		newlayer->setLegendSymbologyGroupParent(sg);
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

