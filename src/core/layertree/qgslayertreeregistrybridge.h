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

class QgsLayerTreeGroup;
class QgsLayerTreeNode;

class QgsMapLayer;

/**
 * Listens to the updates in map layer registry and does changes in layer tree
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

  signals:

  protected slots:
    void layersAdded( QList<QgsMapLayer*> layers );
    void layersWillBeRemoved( QStringList layerIds );

    void groupWillRemoveChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void groupRemovedChildren();

  protected:

  protected:
    QgsLayerTreeGroup* mRoot;
    QStringList mLayerIdsForRemoval;
    bool mEnabled;

    QgsLayerTreeGroup* mInsertionPointGroup;
    int mInsertionPointIndex;
};

#endif // QGSLAYERTREEREGISTRYBRIDGE_H
