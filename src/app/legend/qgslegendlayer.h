/***************************************************************************
    qgslegendlayer.h
    ---------------------
    begin                : January 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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
class QgsSymbol;
class QgsVectorLayer;

class QTreeWidget;

typedef QList< QPair<QString, QPixmap> > SymbologyList;

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

    /**Returns the map layer associated the item*/
    QgsMapLayer* layer();
    QgsMapCanvasLayer& canvasLayer();
    /**Goes through all the legendlayerfiles and sets check state to checked/partially checked/unchecked*/
    //void updateCheckState();

    /**Updates symbology of the layer and copies symbology to other layer files in the group*/
    void refreshSymbology( const QString& key, double widthScale = 1.0 );

    /** Helper method to set font characteristics.
     *  Not to be confused with setFont() which is inherited
     *  from the QTreeWidgetItem base class.
     */
    void setupFont();

    /** called to add appropriate menu items to legend's popup menu */
    void addToPopupMenu( QMenu& theMenu );

    /** Set layer to be visible in canvas */
    void setVisible( bool visible = true );
    /** Find out whether the layer is visible */
    bool isVisible();

    void setInOverview( bool isInOverview = true );
    /**Determines whether there are layers in overview*/
    bool isInOverview();

    /**Returns a label for a layer. Is static such that
     the name can be passed to the constructor of QgsLegendLayer*/
    static QString nameFromLayer( QgsMapLayer* layer );

    /** set check state, but only if user checkable */
    void setCheckState( int column, Qt::CheckState state );

    void setDrawingOrder( int order );
    int drawingOrder() const { return mDrawingOrder; }

    /** Get layer name currently set in legend */
    QString layerName();

    /**Called before edit*/
    void beforeEdit();

    /**Called after edit*/
    void afterEdit();

  public slots:

    /**Toggle show in overview*/
    void showInOverview();

    /**update the layer's icon to show whether is in editing mode or in overview */
    void updateIcon();

    /**Layer name has changed - set it also in legend*/
    void layerNameChanged();

    /**Update symbology (e.g. to update feature count in the legend after editing operations)*/
    void updateAfterLayerModification();

    void setShowFeatureCount( bool show, bool update = true );
    bool showFeatureCount() const { return mShowFeatureCount; }

  protected:

    /** Prepare and change symbology for vector layer */
    void vectorLayerSymbology( QgsVectorLayer* mapLayer, double widthScale = 1.0 );

    void vectorLayerSymbologyV2( QgsVectorLayer* vlayer );

    /** Prepare and change symbology for raster layer */
    void rasterLayerSymbology( QgsRasterLayer* mapLayer );

    /** Removes the symbology items of a layer and adds new ones. */
    void changeSymbologySettings( const QgsMapLayer* mapLayer, const SymbologyList& newSymbologyItems );

    /**Adds feature counts to the symbology items (for symbology v2)*/
    void updateItemListCountV2( SymbologyList& itemList, QgsVectorLayer* layer );
    /**Calculates feature count for the individual symbols (old symbology)*/
    void updateItemListCount( QgsVectorLayer* layer, const QList<QgsSymbol*>& sym, QMap< QgsSymbol*, int >& featureCountMap );

    QPixmap getOriginalPixmap();

  private:

    /** Label, may be layer name or layer name + [feature count] */
    QString label();

  protected:

    /** layer identified by its layer id */
    QgsMapCanvasLayer mLyr;

    /** drawing order */
    int mDrawingOrder;

    /**True if number of features per legend class should is shown in the legend items*/
    bool mShowFeatureCount;
};

#endif
