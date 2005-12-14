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

//#include <qobject.h>
#include <qgslegenditem.h>

class QgsLegendLayer;
class QgsLegendLayerFile;
class QgsLegendPropertyGroup;
class QgsMapLayer;
class QTreeWidget;

/**
Container for layer, including layer file(s), symbology class breaks and properties

@author Tim Sutton
*/
class QgsLegendLayer : public QgsLegendItem, public QObject //for signal/ slot
{
public:
    QgsLegendLayer(QTreeWidgetItem * ,QString);
    QgsLegendLayer(QTreeWidget* ,QString);
    QgsLegendLayer(QString name);
    ~QgsLegendLayer();
    bool isLeafNode();
    QgsLegendItem::DRAG_ACTION accept(LEGEND_ITEM_TYPE type);
    QgsLegendItem::DRAG_ACTION accept(const QgsLegendItem* li) const;
    void handleRightClickEvent(const QPoint& position);
    /**Returns the map layer associated with the first QgsLegendLayerFile or 0 if
     there is no QgsLegendLayerFile*/
    QgsMapLayer* firstMapLayer();
    /**Returns the map layers associated with the QgsLegendLayerFiles*/
    std::list<QgsMapLayer*> mapLayers();
    /**Returns the legend layer file items associated with this legend layer*/
    std::list<QgsLegendLayerFile*> legendLayerFiles();
};

#endif
