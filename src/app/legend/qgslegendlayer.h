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
#include <QFileInfo>

#include "qgsmapcanvas.h"

class QgsLegendLayer;
class QgsLegendPropertyGroup;
class QgsMapLayer;
class QgsRasterLayer;
class QgsVectorLayer;

class QTreeWidget;

typedef std::list< std::pair<QString, QPixmap> > SymbologyList;

/**
Container for layer, including layer file(s), symbology class breaks and properties

@author Tim Sutton
*/
class QgsLegendLayer : public QgsLegendItem
{
    Q_OBJECT

  public:
    QgsLegendLayer( QgsMapLayer* layer );
    ~QgsLegendLayer();

    bool isLeafNode();
    QgsLegendItem::DRAG_ACTION accept( LEGEND_ITEM_TYPE type );
    QgsLegendItem::DRAG_ACTION accept( const QgsLegendItem* li ) const;
    /**Returns the map layer associated the item*/
    QgsMapLayer* layer();
    QgsMapCanvasLayer& canvasLayer();
    /**Goes through all the legendlayerfiles and sets check state to checked/partially checked/unchecked*/
    //void updateCheckState();

    /**Updates symbology of the layer and copies symbology to other layer files in the group*/
    void refreshSymbology( const QString& key, double widthScale = 1.0 );

    /** called to add appropriate menu items to legend's popup menu */
    void addToPopupMenu( QMenu& theMenu, QAction* toggleEditingAction );

    /** Set layer to be visible in canvas */
    void setVisible( bool visible = TRUE );
    /** Find out whether the layer is visible */
    bool isVisible();

    void setInOverview( bool isInOverview = TRUE );
    /**Determines whether there are layers in overview*/
    bool isInOverview();

    /**Returns a label for a layer. Is static such that
     the name can be passed to the constructor of QgsLegendLayer*/
    static QString nameFromLayer( QgsMapLayer* layer );

  public slots:

    /**Toggle show in overview*/
    void showInOverview();

    /**Show layer attribute table*/
    void table();

    void saveAsShapefile();
    void saveSelectionAsShapefile();

    /**update the layer's icon to show whether is in editing mode or in overview */
    void updateIcon();

    /**Layer name has changed - set it also in legend*/
    void layerNameChanged();

  protected:

    /** Prepare and change symbology for vector layer */
    void vectorLayerSymbology( const QgsVectorLayer* mapLayer, double widthScale = 1.0 );

    /** Prepare and change symbology for raster layer */
    void rasterLayerSymbology( QgsRasterLayer* mapLayer );

    /** Removes the symbology items of a layer and adds new ones. */
    void changeSymbologySettings( const QgsMapLayer* mapLayer, const SymbologyList& newSymbologyItems );

    QPixmap getOriginalPixmap();

    /**Save as shapefile (called from saveAsShapefile and saveSelectionAsShapefile)*/
    void saveAsShapefileGeneral( bool saveOnlySelection );

  private:
    /** Helper method to make the font bold from all ctors.
     *  Not to be confused with setFont() which is inherited
     *  from the QTreeWidgetItem base class.
     */
    void setupFont();

  protected:

    /** layer identified by its layer id */
    QgsMapCanvasLayer mLyr;
};

#endif
