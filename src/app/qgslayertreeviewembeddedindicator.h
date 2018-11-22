/***************************************************************************
  qgslayertreeviewembeddedindicator.h
  --------------------------------------
  Date                 : June 2018
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

#ifndef QGSLAYERTREEVIEWEMBEDDEDINDICATOR_H
#define QGSLAYERTREEVIEWEMBEDDEDINDICATOR_H

#include "qgslayertreeviewindicator.h"

#include <QSet>
#include <memory>

class QgsLayerTreeNode;
class QgsLayerTreeView;

//! Adds indicators showing whether layers are embedded.
class QgsLayerTreeViewEmbeddedIndicatorProvider : public QObject
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewEmbeddedIndicatorProvider( QgsLayerTreeView *view );

  private slots:
    //! Connects to signals of layers newly added to the tree
    void onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );

  private:
    std::unique_ptr< QgsLayerTreeViewIndicator > newIndicator( const QString &project );
    void addIndicatorForEmbeddedLayer( QgsLayerTreeNode *node );

  private:
    QgsLayerTreeView *mLayerTreeView = nullptr;
    QIcon mIcon;
    QSet<QgsLayerTreeViewIndicator *> mIndicators;
};

#endif // QGSLAYERTREEVIEWEMBEDDEDINDICATOR_H
