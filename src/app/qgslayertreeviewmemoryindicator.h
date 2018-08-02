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

//! Adds indicators showing whether layers are memory layers.
class QgsLayerTreeViewMemoryIndicatorProvider : public QObject
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewMemoryIndicatorProvider( QgsLayerTreeView *view );

  private slots:
    //! Connects to signals of layers newly added to the tree
    void onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void onLayerLoaded();

  private:
    std::unique_ptr< QgsLayerTreeViewIndicator > newIndicator();
    void addIndicatorForMemoryLayer( QgsLayerTreeNode *node );

  private:
    QgsLayerTreeView *mLayerTreeView = nullptr;
    QIcon mIcon;
    QSet<QgsLayerTreeViewIndicator *> mIndicators;
};

#endif // QGSLAYERTREEVIEWMEMORYINDICATOR_H
