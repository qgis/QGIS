/***************************************************************************
  qgslayertreeviewmemoryindicator.h
  --------------------------------------
  Date                 : July 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEVIEWMEMORYINDICATOR_H
#define QGSLAYERTREEVIEWMEMORYINDICATOR_H

#include "qgslayertreeviewindicator.h"

#include <QSet>
#include <memory>

class QgsLayerTreeNode;
class QgsLayerTreeView;
class QgsVectorLayer;

//! Adds indicators showing whether layers are memory layers.
class QgsLayerTreeViewMemoryIndicatorProvider : public QObject
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewMemoryIndicatorProvider( QgsLayerTreeView *view );

  private slots:
    //! Connects to signals of layers newly added to the tree
    void onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Disconnects from layers about to be removed from the tree
    void onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void onLayerLoaded();
    //! Adds/removes indicator of a layer
    void onDataSourceChanged();

  private:
    std::unique_ptr< QgsLayerTreeViewIndicator > newIndicator();
    void addOrRemoveIndicator( QgsLayerTreeNode *node, QgsVectorLayer *vlayer );

  private:
    QgsLayerTreeView *mLayerTreeView = nullptr;
    QIcon mIcon;
    QSet<QgsLayerTreeViewIndicator *> mIndicators;
};

#endif // QGSLAYERTREEVIEWMEMORYINDICATOR_H
