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
#include "qgslayertreeview.h"
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
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool canDropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;

  signals:

    /**
     * Emitted when layers should be added to the profile, e.g. via a drag and drop action.
     *
     * \since QGIS 3.32
     */
    void addLayers( const QList<QgsMapLayer *> &layers );

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
class GUI_EXPORT QgsElevationProfileLayerTreeView : public QgsLayerTreeViewBase
{
    Q_OBJECT

  public:
    static const QString CUSTOM_NODE_ELEVATION_PROFILE_SOURCE;

    /**
     * Construct a new tree view with given layer tree (root node must not be NULLPTR).
     * The root node is not transferred by the view.
     */
    explicit QgsElevationProfileLayerTreeView( QgsLayerTree *rootNode, QWidget *parent = nullptr );

    /**
     * Initially populates the tree view using layers from a \a project, as well as sources from the source registry.
     */
    void populateInitialSources( QgsProject *project );

    /**
     * Returns the view's proxy model.
     *
     * \since QGIS 3.32
     */
    QgsElevationProfileLayerTreeProxyModel *proxyModel();

  public slots:
    /**
     * Adds a custom node in the layer tree corresponding to a registered profile source.
     *
     * The created node can be accessed via the source id.
     * If the source already has a corresponding node in the layer tree, a second node won't be created.
     *
     * \param sourceId    Unique identifier of the registered profile source.
     * \param sourceName  Name of the registered profile source.
     *
     * \since QGIS 4.0
     */
    void addNodeForRegisteredSource( const QString &sourceId, const QString &sourceName );

    /**
     * Removes a custom node from the layer tree corresponding to an unregistered profile source.
     *
     * \param sourceId  Unique identifier of the unregistered profile source.
     *
     * \since QGIS 4.0
     */
    void removeNodeForUnregisteredSource( const QString &sourceId );

  signals:

    /**
     * Emitted when layers should be added to the profile, e.g. via a drag and drop action.
     *
     * \since QGIS 3.32
     */
    void addLayers( const QList<QgsMapLayer *> &layers );

  protected:
    void resizeEvent( QResizeEvent *event ) override;

  private:
    /**
     * Initially populates the tree view using layers from a \a project.
     */
    void populateInitialLayers( QgsProject *project );

    QgsElevationProfileLayerTreeModel *mModel = nullptr;
    QgsElevationProfileLayerTreeProxyModel *mProxyModel = nullptr;
    QgsLayerTree *mLayerTree = nullptr;
};


#endif // QGSELEVATIONPROFILELAYERTREEVIEW_H
