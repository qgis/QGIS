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
#ifndef QGSLEGENDLAYER_H
#define QGSLEGENDLAYER_H

#include <qgslegenditem.h>

class QgsLegendLayer;
class QgsLegendPropertyGroup;
class QListView;

/**
Container for layer, including layer file(s), symbology class breaks and properties

@author Tim Sutton
*/
class QgsLegendLayer : public QgsLegendItem
{
public:
    QgsLegendLayer(QListViewItem * ,QString);
    QgsLegendLayer(QListView * ,QString);
    ~QgsLegendLayer();

    bool isLeafNode();
    bool accept(LEGEND_ITEM_TYPE type);

};

#endif
