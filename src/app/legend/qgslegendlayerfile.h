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
#ifndef QGSLEGENDLAYERFILE_H
#define QGSLEGENDLAYERFILE_H

#include <qgslegenditem.h>
#include <QPixmap>

#include "qgsmapcanvas.h"

class QgsMapLayer;
class QgsAttributeTableDisplay;

/**
@author Tim Sutton
*/
class QgsLegendLayerFile : public QgsLegendItem
{
  Q_OBJECT;
  
public:
    QgsLegendLayerFile(QTreeWidgetItem * theLegendItem, QString theString, QgsMapLayer* theLayer);
    ~QgsLegendLayerFile();
    bool isLeafNode() {return true;}
    DRAG_ACTION accept(LEGEND_ITEM_TYPE type);
    QgsLegendItem::DRAG_ACTION accept(const QgsLegendItem* li) const;
    QgsMapLayer* layer() { return mLyr.layer(); }
    const QgsMapLayer* layer() const { return mLyr.layer(); }
    QgsMapCanvasLayer& canvasLayer() { return mLyr; }
    
    QPixmap getOriginalPixmap() const;

    /** updates item to the current state of the layer */
    void updateLegendItem();
    
    void setIconAppearance(bool inOverview, bool editable);

    /**Sets mVisibilityCheckBox to on/off*/
    void toggleCheckBox(bool state);

    /**Returns a label for a layer. Is static such that
     the name can be passed to the constructor of QgsLegendLayerFile*/
    static QString nameFromLayer(QgsMapLayer* layer);
    
    
    void setVisible(bool visible = TRUE);
    bool isVisible();
    
    void setInOverview(bool inOverview = TRUE);
    bool isInOverview();
    
  public slots:
    
    /**Open attribute table*/
    void table();
    
    /**Connected to deleted() signal of attribute table*/
    void invalidateTableDisplay();

    /**Connected to layer's selectionChanged() */
    void selectionChanged();
    
    /**Connected to layer's wasModified() */
    void closeTable(bool onlyGeometryWasChanged);
    
    /**Save as shapefile*/
    void saveAsShapefile();
    
    /**Toggle editing for layer*/
    void toggleEditing();
    
    /**Layer name has changed - set it also in legend*/
    void layerNameChanged();
    
 protected:

    /** layer identified by its layer id */
    QgsMapCanvasLayer mLyr;
   
    /** Pointer to the table display object if there is one, otherwise NULL */
    QgsAttributeTableDisplay* mTableDisplay;

};

#endif
