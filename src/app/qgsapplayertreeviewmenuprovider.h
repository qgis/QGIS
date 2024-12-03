/***************************************************************************
    qgsapplayertreeviewmenuprovider.h
    ---------------------
    begin                : May 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPLAYERTREEVIEWMENUPROVIDER_H
#define QGSAPPLAYERTREEVIEWMENUPROVIDER_H

#include <QObject>

#include "qgslayertreeview.h"
#include "qgis.h"

class QAction;
class QgsCoordinateReferenceSystem;

struct LegendLayerAction
{
    LegendLayerAction( QAction *a, const QString &m, bool all )
      : action( a )
      , menu( m )
      , allLayers( all )
    {}
    QAction *action = nullptr;
    QString menu;
    bool allLayers;
    QList<QgsMapLayer *> layers;
};

class QgsMapCanvas;

class QgsAppLayerTreeViewMenuProvider : public QObject, public QgsLayerTreeViewMenuProvider
{
    Q_OBJECT
  public:
    QgsAppLayerTreeViewMenuProvider( QgsLayerTreeView *view, QgsMapCanvas *canvas );

    QMenu *createContextMenu() override;

    void addLegendLayerAction( QAction *action, const QString &menu, Qgis::LayerType type, bool allLayers );
    bool removeLegendLayerAction( QAction *action );
    void addLegendLayerActionForLayer( QAction *action, QgsMapLayer *layer );
    void removeLegendLayerActionsForLayer( QgsMapLayer *layer );
    QList<LegendLayerAction> legendLayerActions( Qgis::LayerType type ) const;

  protected:
    void addCustomLayerActions( QMenu *menu, QgsMapLayer *layer );

    QgsLayerTreeView *mView = nullptr;
    QgsMapCanvas *mCanvas = nullptr;

    QMap<Qgis::LayerType, QList<LegendLayerAction>> mLegendLayerActionMap;

  private slots:

    void editVectorSymbol( const QString &layerId );
    void copyVectorSymbol( const QString &layerId );
    void pasteVectorSymbol( const QString &layerId );
    void setVectorSymbolColor( const QColor &color );
    void editSymbolLegendNodeSymbol( const QString &layerId, const QString &ruleKey );
    void copySymbolLegendNodeSymbol( const QString &layerId, const QString &ruleKey );
    void pasteSymbolLegendNodeSymbol( const QString &layerId, const QString &ruleKey );
    void setSymbolLegendNodeColor( const QColor &color );
    void setLayerCrs( const QgsCoordinateReferenceSystem &crs );
    void toggleLabels( bool enabled );

  private:
    bool removeActionEnabled();
};

#endif // QGSAPPLAYERTREEVIEWMENUPROVIDER_H
