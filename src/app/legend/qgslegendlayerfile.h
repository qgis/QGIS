/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton                                      *
 *   aps02ts@macbuntu                                                      *
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
#ifndef QGSLEGENDLAYERFILE_H
#define QGSLEGENDLAYERFILE_H

#include <QPixmap>

#include "qgslegenditem.h"
#include "qgsmapcanvas.h"

class QgsMapLayer;

/**
@author Tim Sutton
*/
class QgsLegendLayerFile : public QgsLegendItem
{
  Q_OBJECT
  
public:
    QgsLegendLayerFile(QTreeWidgetItem * theLegendItem, QString theString, QgsMapLayer* theLayer);

    bool isLeafNode() {return true;}
    DRAG_ACTION accept(LEGEND_ITEM_TYPE type);
    QgsLegendItem::DRAG_ACTION accept(const QgsLegendItem* li) const;
    QgsMapLayer* layer() { return mLyr.layer(); }
    const QgsMapLayer* layer() const { return mLyr.layer(); }
    QgsMapCanvasLayer& canvasLayer() { return mLyr; }
    
    QPixmap getOriginalPixmap() const;

    void setIconAppearance(bool inOverview, bool editable);

    /**Returns a label for a layer. Is static such that
     the name can be passed to the constructor of QgsLegendLayerFile*/
    static QString nameFromLayer(QgsMapLayer* layer);
    
    void setVisible(bool visible = TRUE);
    bool isVisible();
    
    void setInOverview(bool inOverview = TRUE);
    bool isInOverview();
    
    /** called to add appropriate menu items to legend's popup menu */
    void addToPopupMenu(QMenu& theMenu, QAction* toggleEditingAction);
    
  public slots:
    /** updates item to the current state of the layer */
    void updateLegendItem();
    
    /**Open attribute table*/
    void table();
    
    /**Save as shapefile*/
    void saveAsShapefile();
    
    /**Save selection as shapefile*/
    void saveSelectionAsShapefile();
    
    /**Return editing status for layer*/
    bool isEditing();

    /**Toggle show in overview*/
    void showInOverview();
    
    /**Layer name has changed - set it also in legend*/
    void layerNameChanged();

 protected:

    /**Save as shapefile (called from saveAsShapefile and saveSelectionAsShapefile)*/
    void saveAsShapefileGeneral(bool saveOnlySelection);

    /** layer identified by its layer id */
    QgsMapCanvasLayer mLyr;
};

#endif
