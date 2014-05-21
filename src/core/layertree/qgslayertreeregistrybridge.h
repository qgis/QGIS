/***************************************************************************
  qgslayertreeregistrybridge.h
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEREGISTRYBRIDGE_H
#define QGSLAYERTREEREGISTRYBRIDGE_H

#include <QObject>
#include <QStringList>

class QAction;

class QgsLayerTreeGroup;
class QgsLayerTreeNode;

#include "qgsmaplayer.h"

struct LegendLayerAction
{
  LegendLayerAction( QAction* a, QString m, QString i, bool all )
      : action( a ), menu( m ), id( i ), allLayers( all ) {}
  QAction* action;
  QString menu;
  QString id;
  bool allLayers;
  QList<QgsMapLayer*> layers;
};

/**
 * Listens to the updates in map layer registry and does changes in layer tree.
 *
 * When connected to a layer tree, any layers added to the map layer registry
 * will be also added to the layer tree. Similarly, map layers that are removed
 * from registry will be removed from the layer tree.
 *
 * If a layer is completely removed from the layer tree, it will be also removed
 * from the map layer registry.
 *
 * @note added in 2.4
 */
class CORE_EXPORT QgsLayerTreeRegistryBridge : public QObject
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeRegistryBridge( QgsLayerTreeGroup* root, QObject *parent = 0 );

    void setEnabled( bool enabled ) { mEnabled = enabled; }
    bool isEnabled() const { return mEnabled; }

    //! Set where the new layers should be inserted - can be used to follow current selection.
    //! By default it is root group with zero index.
    void setLayerInsertionPoint( QgsLayerTreeGroup* parentGroup, int index );

	void addLegendLayerAction( QAction* action, QString menu, QString id,
                               QgsMapLayer::LayerType type, bool allLayers );
    bool removeLegendLayerAction( QAction* action );
    void addLegendLayerActionForLayer( QAction* action, QgsMapLayer* layer );
    void removeLegendLayerActionsForLayer( QgsMapLayer* layer );
    QList< LegendLayerAction > legendLayerActions( QgsMapLayer::LayerType type ) const;


  signals:

  protected slots:
    void layersAdded( QList<QgsMapLayer*> layers );
    void layersWillBeRemoved( QStringList layerIds );

    void groupWillRemoveChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void groupRemovedChildren();

  protected:
    QgsLayerTreeGroup* mRoot;
    QStringList mLayerIdsForRemoval;
    bool mEnabled;

    QgsLayerTreeGroup* mInsertionPointGroup;
    int mInsertionPointIndex;    

    QMap< QgsMapLayer::LayerType, QList< LegendLayerAction > > mLegendLayerActionMap;
};

#endif // QGSLAYERTREEREGISTRYBRIDGE_H
