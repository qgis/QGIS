/***************************************************************************
                          qgselevationprofilelayertreeview.h
                          ---------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSELEVATIONPROFILELAYERTREEVIEW_H
#define QGSELEVATIONPROFILELAYERTREEVIEW_H

#include "qgsconfig.h"

#include <QTreeView>

class QgsLayerTree;
class QgsElevationProfileLayerTreeModel;
class QgsElevationProfileLayerTreeProxyModel;
class QgsMapLayer;

/**
 * \ingroup app
 * \brief A layer tree view for elevation profiles.
 *
 * \since QGIS 3.26
 */
class QgsElevationProfileLayerTreeView : public QTreeView
{

    Q_OBJECT

  public:

    /**
     * Construct a new tree view with given layer tree (root node must not be NULLPTR).
     * The root node is not transferred by the view.
     */
    explicit QgsElevationProfileLayerTreeView( QgsLayerTree *rootNode, QWidget *parent = nullptr );

    /**
     * Converts a view \a index to a map layer.
     */
    QgsMapLayer *indexToLayer( const QModelIndex &index );

  protected:

    void contextMenuEvent( QContextMenuEvent *event ) override;
    void resizeEvent( QResizeEvent *event ) override;

  private:

    QgsElevationProfileLayerTreeModel *mModel = nullptr;
    QgsElevationProfileLayerTreeProxyModel *mProxyModel = nullptr;
    QgsLayerTree *mLayerTree = nullptr;

};


#endif // QGSELEVATIONPROFILELAYERTREEVIEW_H
