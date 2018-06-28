/***************************************************************************
    qgsmeshdatasetgrouptreeview.h
    -----------------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHDATASETGROUPTREE_H
#define QGSMESHDATASETGROUPTREE_H

#include "qgis_app.h"

#include <QObject>
#include <QTreeView>
#include <QMap>
#include <QVector>
#include <QItemSelection>
#include <QStandardItemModel>
#include <QList>
#include <memory>

class QgsMeshLayer;

/**
 * Tree item for display of the mesh dataset groups.
 * Dataset group is set of datasets with the same name,
 * but different control variable (e.g. time)
 *
 * Support for multiple levels, because groups can have
 * subgroups, for example
 *
 * Groups:
 *   Depth
 *     - Maximum/Depth
 *   Velocity
 *   Wind speed
 *     - Maximum/Wind Speed
 */
class APP_NO_EXPORT QgsMeshDatasetGroupTreeItem
{
  public:
    QgsMeshDatasetGroupTreeItem( QgsMeshDatasetGroupTreeItem *parent = nullptr );
    QgsMeshDatasetGroupTreeItem( const QString &name, QgsMeshDatasetGroupTreeItem *parent = nullptr );
    ~QgsMeshDatasetGroupTreeItem();

    void appendChild( QgsMeshDatasetGroupTreeItem *node );
    QgsMeshDatasetGroupTreeItem *child( int row ) const;
    int columnCount() const;
    int childCount() const;
    QgsMeshDatasetGroupTreeItem *parentItem() const;
    int row() const;
    QVariant data( int column ) const;

  private:
    QgsMeshDatasetGroupTreeItem *mParent = nullptr;
    QList< QgsMeshDatasetGroupTreeItem * > mChildren;

    // Data
    QString mName;
};

/**
 * Item Model for QgsMeshDatasetGroupTreeItem
 */
class APP_NO_EXPORT QgsMeshDatasetGroupTreeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    explicit QgsMeshDatasetGroupTreeModel( QObject *parent = nullptr );
    ~QgsMeshDatasetGroupTreeModel();

    QVariant data( const QModelIndex &index, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const override;
    QModelIndex index( int row, int column,
                       const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    //! Add groups to the model
    void setupModelData( const QStringList &groups );

  private:
    std::unique_ptr<QgsMeshDatasetGroupTreeItem> mRootItem;
};

/**
 * Tree widget for display of the mesh dataset groups.
 *
 * One dataset group is selected (active)
 */
class APP_EXPORT QgsMeshDatasetGroupTreeView : public QTreeView
{
    Q_OBJECT

  public:
    QgsMeshDatasetGroupTreeView( QWidget *parent = nullptr );
    ~QgsMeshDatasetGroupTreeView() = default;

    //! Associates mesh layer with the widget
    void setLayer( QgsMeshLayer *layer );

    //! Returns all the dataset indexes in the active group
    QVector<int> datasetsInActiveGroup() const;

    //! Synchronize widgets state with associated mesh layer
    void syncToLayer();

  signals:
    //! Selected dataset group changed
    void activeGroupChanged();

  private slots:
    void onSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

  private:
    void extractGroups();
    int setActiveGroupFromActiveDataset();

    QgsMeshDatasetGroupTreeModel mModel;
    QgsMeshLayer *mMeshLayer = nullptr; // not owned
    QMap<QString, QVector<int>> mGroups; // group name -> dataset indices
    QString mActiveGroup;
};

#endif // QGSMESHDATASETGROUPTREE_H
