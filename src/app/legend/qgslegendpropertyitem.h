/***************************************************************************
    qgslegendpropertyitem.h
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
#ifndef QGSLEGENDPROPERTYITEM_H
#define QGSLEGENDPROPERTYITEM_H

#include <qgslegenditem.h>

/**
A legend property item is a leaf node (it can have no children) that is used to represent a layer property e.g. projection, labels,metadata etc.

@author Tim Sutton
*/
class QgsLegendPropertyItem : public QgsLegendItem
{
  public:
    QgsLegendPropertyItem( QTreeWidgetItem * theItem, QString theString );
    ~QgsLegendPropertyItem();
};

#endif
