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

#include "qgis_gui.h"

#include <QObject>
#include <QTreeView>
#include <QMap>
#include <QMenu>
#include <QVector>
#include <QItemSelection>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QList>
#include <QSortFilterProxyModel>
#include <memory>
#include "qgsmeshdataset.h"

SIP_NO_FILE

class QgsMeshLayer;

/**
 * \ingroup gui
 * \class QgsMeshDatasetGroupSaveMenu
 */
class QgsMeshDatasetGroupSaveMenu: public QObject
{
    Q_OBJECT
  public:
    QgsMeshDatasetGroupSaveMenu( QObject *parent = nullptr ): QObject( parent ) {}
    QMenu *createSaveMenu( int groupIndex, QMenu *parentMenu = nullptr );

    void setMeshLayer( QgsMeshLayer *meshLayer );

  signals:
    void datasetGroupSaved( const QString &uri );

  private:
    QgsMeshLayer *mMeshLayer = nullptr;

    void saveDatasetGroup( int datasetGroup, const QString &driver, const QString &fileSuffix );
};

/**
 * \ingroup gui
 * \class QgsMeshDatasetGroupTreeModel
 *
 * \brief Item Model for QgsMeshDatasetGroupTreeItem
 */
class QgsMeshDatasetGroupTreeModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    enum Roles
    {
      Name = Qt::UserRole,
      IsVector,
      IsActiveScalarDatasetGroup,
      IsActiveVectorDatasetGroup,
      DatasetGroupIndex,
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

    //! Synchronizes groups to the model from mesh layer
    void syncToLayer( QgsMeshLayer *layer );

    //! Returns the dataset group root tree item, keeps ownership
    QgsMeshDatasetGroupTreeItem *datasetGroupTreeRootItem();

    //! Returns the dataset group tree item with \a index, keeps ownership
    QgsMeshDatasetGroupTreeItem *datasetGroupTreeItem( int groupIndex );

    //! Returns the dataset group tree item corresponding to \a index, keeps ownership
    QgsMeshDatasetGroupTreeItem *datasetGroupTreeItem( QModelIndex index );

    //! Returns whether the dataset groups related to the QModelIndex is set as enabled
    bool isEnabled( const QModelIndex &index ) const;

    //! Resets all groups with default state from the mesh layer
    void resetDefault( QgsMeshLayer *meshLayer );

    //! Sets all groups as enabled
    void setAllGroupsAsEnabled( bool isEnabled );

    //! Removes an item from the tree
    void removeItem( const QModelIndex &index );

    //! Sets the dataset group as persistent with specified uri and for specified index
    void setPersistentDatasetGroup( const QModelIndex &index, const QString &uri );

  private:
    std::unique_ptr<QgsMeshDatasetGroupTreeItem> mRootItem;

};

/**
 * \ingroup gui
 * \class QgsMeshAvailableDatasetGroupTreeModel
 */
class QgsMeshAvailableDatasetGroupTreeModel: public QgsMeshDatasetGroupTreeModel
{
    Q_OBJECT
  public:
    QgsMeshAvailableDatasetGroupTreeModel( QObject *parent = nullptr );

    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

  private:
    QString textDisplayed( const QModelIndex &index ) const;
    QColor backGroundColor( const QModelIndex &index ) const;
};

/**
 * \ingroup gui
 * \class QgsMeshDatasetGroupProxyModel
 */
class  QgsMeshDatasetGroupProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    QgsMeshDatasetGroupProxyModel( QAbstractItemModel *sourceModel );

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;

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

  protected:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:
    int mActiveScalarGroupIndex = -1;
    int mActiveVectorGroupIndex = -1;
};

/**
 * \ingroup gui
 * \class QgsMeshDatasetGroupTreeItemDelagate
 *
 * \brief Delegate to display tree item with a contours and vector selector
 */
class QgsMeshDatasetGroupTreeItemDelagate: public QStyledItemDelegate
{
    Q_OBJECT
  public:
    QgsMeshDatasetGroupTreeItemDelagate( QObject *parent = nullptr );

    void paint( QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index ) const override;

    //! Icon rectangle for given item rectangle
    QRect iconRect( const QRect &rect, bool isVector ) const;

    QSize sizeHint( const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const override;
  private:
    const QPixmap mScalarSelectedPixmap;
    const QPixmap mScalarDeselectedPixmap;
    const QPixmap mVectorSelectedPixmap;
    const QPixmap mVectorDeselectedPixmap;

    QRect iconRect( const QRect &rect, int pos ) const;
};

/**
 * \ingroup gui
 * \class QgsMeshDatasetGroupTreeView
 *
 * \brief Tree widget for display of the mesh dataset groups.
 */
class GUI_EXPORT QgsMeshDatasetGroupTreeView: public QTreeView
{
    Q_OBJECT
  public:
    QgsMeshDatasetGroupTreeView( QWidget *parent = nullptr );

    void syncToLayer( QgsMeshLayer *layer );
    void resetDefault( QgsMeshLayer *meshLayer );

    QgsMeshDatasetGroupTreeItem *datasetGroupTreeRootItem();

  signals:
    void apply();

  public slots:
    void selectAllGroups();
    void deselectAllGroups();

  protected:
    void contextMenuEvent( QContextMenuEvent *event ) override;

  private slots:
    void removeCurrentItem();
    void onDatasetGroupSaved( const QString &uri );

  private:
    QgsMeshAvailableDatasetGroupTreeModel *mModel;
    QgsMeshDatasetGroupSaveMenu *mSaveMenu;

    void selectAllItem( bool isChecked );
    QMenu *createContextMenu();
};

/**
 * \ingroup gui
 * \class QgsMeshActiveDatasetGroupTreeView
 *
 * \brief Tree widget for display of the mesh dataset groups.
 *
 * One dataset group is selected (active)
 */
class GUI_EXPORT QgsMeshActiveDatasetGroupTreeView : public QTreeView
{
    Q_OBJECT

  public:
    QgsMeshActiveDatasetGroupTreeView( QWidget *parent = nullptr );

    //! Associates mesh layer with the widget
    void setLayer( QgsMeshLayer *layer );

    //! Returns index of active group for contours
    int activeScalarGroup() const;

    //! Returns index of active group for vectors
    int activeVectorGroup() const;

    //! Synchronize widgets state with associated mesh layer
    void syncToLayer();

    void mousePressEvent( QMouseEvent *event ) override;

  public slots:
    //! Sets active group for contours
    void setActiveScalarGroup( int group );

    //! Sets active vector group
    void setActiveVectorGroup( int group );

  signals:
    //! Selected dataset group for contours changed. -1 for invalid group
    void activeScalarGroupChanged( int groupIndex );

    //! Selected dataset group for vectors changed. -1 for invalid group
    void activeVectorGroupChanged( int groupIndex );

  private:
    void setActiveGroup();

    QgsMeshDatasetGroupProxyModel *mProxyModel;
    QgsMeshDatasetGroupTreeItemDelagate mDelegate;
    QgsMeshLayer *mMeshLayer = nullptr; // not owned
};

/**
 * \ingroup gui
 * \class QgsMeshDatasetGroupListModel
 */
class GUI_EXPORT QgsMeshDatasetGroupListModel: public QAbstractListModel
{
    Q_OBJECT
  public:
    explicit QgsMeshDatasetGroupListModel( QObject *parent );

    //! Add groups to the model from mesh layer
    void syncToLayer( QgsMeshLayer *layer );

    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;

    void setDisplayProviderName( bool displayProviderName );

    QStringList variableNames() const;

  private:
    QgsMeshDatasetGroupTreeItem *mRootItem = nullptr;
    bool mDisplayProviderName = false;
};

#endif // QGSMESHDATASETGROUPTREE_H
