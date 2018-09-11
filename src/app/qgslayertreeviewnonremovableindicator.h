/***************************************************************************
  qgslayertreeviewnonremovableindicator.h
  --------------------------------------
  Date                 : Sep 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEVIEWNONREMOVABLEINDICATOR_H
#define QGSLAYERTREEVIEWNONREMOVABLEINDICATOR_H

#include "qgslayertreeviewindicator.h"

#include <QSet>
#include <memory>

class QgsLayerTreeNode;
class QgsLayerTreeView;
class QgsMapLayer;

class QgsLayerTreeViewNonRemovableIndicatorProvider : public QObject
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewNonRemovableIndicatorProvider( QgsLayerTreeView *view );

  private slots:
    //! Connects to signals of layers newly added to the tree
    void onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Disconnects from layers about to be removed from the tree
    void onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Starts listening to layer provider's dataChanged signal
    void onLayerLoaded();
    void onFlagsChanged();

  private:
    std::unique_ptr< QgsLayerTreeViewIndicator > newIndicator();
    void addOrRemoveIndicator( QgsLayerTreeNode *node, QgsMapLayer *layer );

  private:
    QgsLayerTreeView *mLayerTreeView = nullptr;
    QIcon mIcon;
    QSet<QgsLayerTreeViewIndicator *> mIndicators;
};


#endif // QGSLAYERTREEVIEWNONREMOVABLEINDICATOR_H
