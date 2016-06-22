/***************************************************************************
               QgsBookmarks.h  - Spatial Bookmarks
                             -------------------
    begin                : 2005-04-23
    copyright            : (C) 2005 Gary Sherman
    email                : sherman at mrcc dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBOOKMARKS_H
#define QGSBOOKMARKS_H

#include <QSqlTableModel>
#include <QScopedPointer>

#include "ui_qgsbookmarksbase.h"
#include "qgscontexthelp.h"
#include "qgsdockwidget.h"

/*
 * Model for project bookmarks
 */
class QgsProjectBookmarksTableModel: public QAbstractTableModel
{
    Q_OBJECT

  public:

    QgsProjectBookmarksTableModel();

    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;

    int columnCount( const QModelIndex& parent = QModelIndex() ) const override;

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;

    bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;

    bool insertRows( int row, int count, const QModelIndex& parent = QModelIndex() ) override;

    bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() ) override;

  private slots:
    void projectRead() { emit layoutChanged(); };
};

/*
 * Model that merge the QGIS and project model
 */
class QgsMergedBookmarksTableModel: public QAbstractTableModel
{
    Q_OBJECT

  public:

    QgsMergedBookmarksTableModel( QAbstractTableModel& qgisTableModel, QAbstractTableModel& projectTableModel, QTreeView* treeView );

    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;

    int columnCount( const QModelIndex& parent = QModelIndex() ) const override;

    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;

    bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex& index ) const override;

    bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    QAbstractTableModel* qgisModel();

  private:
    QAbstractTableModel& mQgisTableModel;
    QAbstractTableModel& mProjectTableModel;
    QTreeView* mTreeView;
    bool mProjectOpen;

    void moveBookmark( QAbstractTableModel& modelFrom, QAbstractTableModel& modelTo, int row );

  private slots:
    void projectRead() { mProjectOpen = true; };
    void allLayoutChanged() { emit layoutChanged(); };
    void qgisDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
    {
      emit dataChanged( topLeft, bottomRight );
    };
    void projectDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
    {
      emit dataChanged(
        index( topLeft.row() + mQgisTableModel.rowCount(), topLeft.column() ),
        index( bottomRight.row() + mQgisTableModel.rowCount(), bottomRight.column() ) );
    };
};


class APP_EXPORT QgsBookmarks : public QgsDockWidget, private Ui::QgsBookmarksBase
{
    Q_OBJECT

  public:
    QgsBookmarks( QWidget *parent = nullptr );
    ~QgsBookmarks();

  public slots:
    void addClicked();

  private slots:
    void deleteClicked();
    void zoomToBookmark();
    void exportToXML();
    void importFromXML();

    void on_lstBookmarks_doubleClicked( const QModelIndex & );

  private:
    QSqlTableModel* mQgisModel;
    QgsProjectBookmarksTableModel* mProjectModel;
    QScopedPointer<QgsMergedBookmarksTableModel> mModel;

    void saveWindowLocation();
    void restorePosition();

};


#endif // QGSBOOKMARKS_H
