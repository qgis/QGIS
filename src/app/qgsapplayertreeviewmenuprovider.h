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
#include "qgsmaplayer.h"

class QAction;

struct LegendLayerAction
{
  LegendLayerAction( QAction* a, const QString& m, const QString& i, bool all )
      : action( a )
      , menu( m )
      , id( i )
      , allLayers( all )
  {}
  QAction* action;
  QString menu;
  QString id;
  bool allLayers;
  QList<QgsMapLayer*> layers;
};

class QgsMapCanvas;

class QgsAppLayerTreeViewMenuProvider : public QObject, public QgsLayerTreeViewMenuProvider
{
    Q_OBJECT
  public:
    QgsAppLayerTreeViewMenuProvider( QgsLayerTreeView* view, QgsMapCanvas* canvas );

    QMenu* createContextMenu() override;

    void addLegendLayerAction( QAction* action, const QString& menu, const QString& id,
                               QgsMapLayer::LayerType type, bool allLayers );
    bool removeLegendLayerAction( QAction* action );
    void addLegendLayerActionForLayer( QAction* action, QgsMapLayer* layer );
    void removeLegendLayerActionsForLayer( QgsMapLayer* layer );
    QList< LegendLayerAction > legendLayerActions( QgsMapLayer::LayerType type ) const;

  protected:

    void addCustomLayerActions( QMenu* menu, QgsMapLayer* layer );

    QgsLayerTreeView* mView;
    QgsMapCanvas* mCanvas;

    QMap< QgsMapLayer::LayerType, QList< LegendLayerAction > > mLegendLayerActionMap;

  private slots:

    void editVectorSymbol();
    void setVectorSymbolColor( const QColor &color );
    void editSymbolLegendNodeSymbol();
    void setSymbolLegendNodeColor( const QColor &color );
};

#endif // QGSAPPLAYERTREEVIEWMENUPROVIDER_H
