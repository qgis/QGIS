/***************************************************************************
    qgslegendpropertyitem.cpp
    ---------------------
    begin                : January 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslegendpropertyitem.h"
#include "qgsapplication.h"
#include <QIcon>

QgsLegendPropertyItem::QgsLegendPropertyItem( QTreeWidgetItem * theItem, QString theString )
    : QgsLegendItem( theItem, theString )
{
  mType = LEGEND_PROPERTY_ITEM;
  QIcon myIcon( QgsApplication::pkgDataPath() + QString( "/images/icons/property_item.png" ) );
  setIcon( 0, myIcon );
}


QgsLegendPropertyItem::~QgsLegendPropertyItem()
{
  mType = LEGEND_PROPERTY_ITEM;
}
