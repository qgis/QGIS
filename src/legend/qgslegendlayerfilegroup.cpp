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

QgsLegendLayerFileGroup::QgsLegendLayerFileGroup(QListViewItem* theItem, QString theString): QgsLegendItem(theItem, theString)
{

}

bool QgsLegendLayerFileGroup::accept(LEGEND_ITEM_TYPE type)
{
    return false;
}
