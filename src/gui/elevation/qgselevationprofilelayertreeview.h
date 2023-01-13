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
#include "qgslayertreemodel.h"
#include "qgis_gui.h"

#include <QSortFilterProxyModel>
#include <QTreeView>

#define SIP_NO_FILE


class QgsLayerTree;
class QgsElevationProfileLayerTreeModel;
class QgsElevationProfileLayerTreeProxyModel;
class QgsMapLayer;


/**
 * \ingroup gui
 * \brief A layer tree model subclass for elevation profiles.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsElevationProfileLayerTreeModel : public QgsLayerTreeModel
{

    Q_OBJECT

  public:

    /**
     * Construct a new tree model with given layer tree (root node must not be NULLPTR).
     * The root node is not transferred by the model.
     */
    explicit QgsElevationProfileLayerTreeModel( QgsLayerTree *rootNode, QObject *parent = nullptr );

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;

  private:

#ifdef SIP_RUN
    QgsElevationProfileLayerTreeModel( const QgsElevationProfileLayerTreeModel &other );
#endif
};

/**
 * \ingroup gui
 * \brief A proxy model for elevation profiles.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsElevationProfileLayerTreeProxyModel : public QSortFilterProxyModel
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsElevationProfileLayerTreeProxyModel.
     */
    explicit QgsElevationProfileLayerTreeProxyModel( QgsElevationProfileLayerTreeModel *model, QObject *parent = nullptr );

  protected:

    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

  private:

    QgsElevationProfileLayerTreeModel *mModel = nullptr;

};

/**
 * \ingroup gui
 * \brief A layer tree view for elevation profiles.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsElevationProfileLayerTreeView : public QTreeView
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

    void resizeEvent( QResizeEvent *event ) override;

  private:

    QgsElevationProfileLayerTreeModel *mModel = nullptr;
    QgsElevationProfileLayerTreeProxyModel *mProxyModel = nullptr;
    QgsLayerTree *mLayerTree = nullptr;

};


#endif // QGSELEVATIONPROFILELAYERTREEVIEW_H
