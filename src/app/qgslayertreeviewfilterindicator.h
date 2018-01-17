/***************************************************************************
  qgslayertreeviewfilterindicator.h
  --------------------------------------
  Date                 : January 2018
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

#ifndef QGSLAYERTREEVIEWFILTERINDICATOR_H
#define QGSLAYERTREEVIEWFILTERINDICATOR_H

#include "qgslayertreeviewindicator.h"

#include <QSet>

class QgsLayerTreeNode;
class QgsLayerTreeView;
class QgsVectorDataProvider;


//! Adds indicators showing whether vector layers have a filter applied.
class QgsLayerTreeViewFilterIndicatorProvider : public QObject
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewFilterIndicatorProvider( QgsLayerTreeView *view );

  private slots:
    //! Connects to signals of layers newly added to the tree
    void onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Disconnects from layers about to be removed from the tree
    void onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Starts listening to layer provider's dataChanged signal
    void onLayerLoaded();
    //! Adds/removes indicator of a layer
    void onProviderDataChanged();

    void onIndicatorClicked( const QModelIndex &index );

  private:
    QgsLayerTreeViewIndicator *newIndicator( const QString &filter );
    void updateIndicator( QgsLayerTreeViewIndicator *indicator, const QString &filter );
    void addOrRemoveIndicator( QgsLayerTreeNode *node, QgsVectorDataProvider *provider );

  private:
    QgsLayerTreeView *mLayerTreeView;
    QIcon mIcon;
    QSet<QgsLayerTreeViewIndicator *> mIndicators;
};

#endif // QGSLAYERTREEVIEWFILTERINDICATOR_H
