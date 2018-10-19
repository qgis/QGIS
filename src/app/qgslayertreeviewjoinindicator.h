/***************************************************************************
  qgslayertreeviewjoinindicator.h
  --------------------------------------
  Date                 : October 2018
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

#ifndef QGSLAYERTREEVIEWJOININDICATOR_H
#define QGSLAYERTREEVIEWJOININDICATOR_H

#include "qgslayertreeviewindicator.h"

#include <QSet>
#include <memory>

class QgsLayerTreeNode;
class QgsLayerTreeView;
class QgsVectorLayer;

//! Adds indicators showing whether layers have joined data.
class QgsLayerTreeViewJoinIndicatorProvider : public QObject
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewJoinIndicatorProvider( QgsLayerTreeView *view );

  private slots:
    //! Connects to signals of layers newly added to the tree
    void onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Disconnects from layers about to be removed from the tree
    void onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void onLayerLoaded();
    //! Adds/removes indicator of a layer
    void onUpdatedFields();

  private:
    std::unique_ptr< QgsLayerTreeViewIndicator > newIndicator( const QString &joinsNames );
    void updateIndicator( QgsLayerTreeViewIndicator *indicator, const QString &joinsNames );
    void addOrRemoveIndicator( QgsLayerTreeNode *node, QgsVectorLayer *vlayer );

  private:
    QgsLayerTreeView *mLayerTreeView = nullptr;
    QIcon mIcon;
    QSet<QgsLayerTreeViewIndicator *> mIndicators;
};

#endif // QGSLAYERTREEVIEWJOININDICATOR_H
