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
#include <QStyledItemDelegate>
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
 *     - Maximum
 *   Velocity
 *   Wind speed
 *     - Maximum
 */
class APP_NO_EXPORT QgsMeshDatasetGroupTreeItem
{
  public:
    QgsMeshDatasetGroupTreeItem( QgsMeshDatasetGroupTreeItem *parent = nullptr );
    QgsMeshDatasetGroupTreeItem( const QString &name,
                                 bool isVector,
                                 int index,
                                 QgsMeshDatasetGroupTreeItem *parent = nullptr );
    ~QgsMeshDatasetGroupTreeItem();

    void appendChild( QgsMeshDatasetGroupTreeItem *node );
    QgsMeshDatasetGroupTreeItem *child( int row ) const;
    int childCount() const;
    QgsMeshDatasetGroupTreeItem *parentItem() const;
    int row() const;
    QString name() const;
    bool isVector() const;
    int datasetGroupIndex() const;

  private:
    QgsMeshDatasetGroupTreeItem *mParent = nullptr;
    QList< QgsMeshDatasetGroupTreeItem * > mChildren;

    // Data
    QString mName;
    bool mIsVector = false;
    int mDatasetGroupIndex = -1;
};

/**
 * Item Model for QgsMeshDatasetGroupTreeItem
 */
class APP_NO_EXPORT QgsMeshDatasetGroupTreeModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    enum Roles
    {
      Name = Qt::UserRole,
      IsVector,
      IsActiveScalarDatasetGroup,
      IsActiveVectorDatasetGroup,
      DatasetGroupIndex
    };

    explicit QgsMeshDatasetGroupTreeModel( QObject *parent = nullptr );

    QVariant data( const QModelIndex &index, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const override;
    QModelIndex index( int row, int column,
                       const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    //! Returns index of active group for contours
    int activeScalarGroup() const;

    //! Sets active group for contours
    void setActiveScalarGroup( int group );

    //! Returns index of active group for vectors
    int activeVectorGroup() const;

    //! Sets active vector group
    void setActiveVectorGroup( int group );

    //! Add groups to the model from mesh layer
    void syncToLayer( QgsMeshLayer *layer );

  private:
    void addTreeItem( const QString &groupName, bool isVector, int groupIndex, QgsMeshDatasetGroupTreeItem *parent );
    QModelIndex groupIndexToModelIndex( int groupIndex );

    std::unique_ptr<QgsMeshDatasetGroupTreeItem> mRootItem;
    QMap<QString, QgsMeshDatasetGroupTreeItem *> mNameToItem;
    QMap<int, QgsMeshDatasetGroupTreeItem *> mDatasetGroupIndexToItem;
    int mActiveScalarGroupIndex = -1;
    int mActiveVectorGroupIndex = -1;
};

/**
 * Delegate to display tree item with a contours and vector selector
 */
class APP_EXPORT QgsMeshDatasetGroupTreeItemDelagate: public QStyledItemDelegate
{
  public:
    QgsMeshDatasetGroupTreeItemDelagate( QObject *parent = Q_NULLPTR );

    void paint( QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index ) const override;

    //! Icon rectangle for given item rectangle
    QRect iconRect( const QRect rect, bool isVector ) const;

    QSize sizeHint( const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const override;
  private:
    const QPixmap mScalarSelectedPixmap;
    const QPixmap mScalarUnselectedPixmap;
    const QPixmap mVectorSelectedPixmap;
    const QPixmap mVectorUnselectedPixmap;
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

    //! Associates mesh layer with the widget
    void setLayer( QgsMeshLayer *layer );

    //! Returns index of active group for contours
    int activeScalarGroup() const;

    //! Sets active group for contours
    void setActiveScalarGroup( int group );

    //! Returns index of active group for vectors
    int activeVectorGroup() const;

    //! Sets active vector group
    void setActiveVectorGroup( int group );

    //! Synchronize widgets state with associated mesh layer
    void syncToLayer();

    void mousePressEvent( QMouseEvent *event ) override;

  signals:
    //! Selected dataset group for contours changed. -1 for invalid group
    void activeScalarGroupChanged( int groupIndex );

    //! Selected dataset group for vectors changed. -1 for invalid group
    void activeVectorGroupChanged( int groupIndex );

  private slots:
    // void onSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );

  private:
    void setActiveGroupFromActiveDataset();

    QgsMeshDatasetGroupTreeModel mModel;
    QgsMeshDatasetGroupTreeItemDelagate mDelegate;
    QgsMeshLayer *mMeshLayer = nullptr; // not owned
};

#endif // QGSMESHDATASETGROUPTREE_H
