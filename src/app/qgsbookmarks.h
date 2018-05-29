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
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

#include "ui_qgsbookmarksbase.h"
#include "qgsdockwidget.h"
#include "qgis_app.h"

/*
 * Model for project bookmarks
 */
class QgsProjectBookmarksTableModel: public QAbstractTableModel
{
    Q_OBJECT

  public:

    QgsProjectBookmarksTableModel( QObject *parent = nullptr );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;

    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

  private slots:
    void projectRead();
};


class QgsBookmarksProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    QgsBookmarksProxyModel( QObject *parent = nullptr );

    //! This override is required because the merge model only defines headers for the SQL model
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

  public slots:

    void _resetModel()
    {
      reset();
    }
};


/**
 * \brief QgsDoubleSpinBoxBookmarksDelegate class shows 6 digits when value is a double
 */
class QgsDoubleSpinBoxBookmarksDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:

    explicit QgsDoubleSpinBoxBookmarksDelegate( QObject *parent = nullptr );

    QString displayText( const QVariant &value, const QLocale &locale ) const override;

    QWidget *createEditor( QWidget *parent,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index ) const override;
  private:

    static const int  DECIMAL_PLACES;

};

/*
 * Model that merge the QGIS and project model
 */
class QgsMergedBookmarksTableModel: public QAbstractTableModel
{
    Q_OBJECT

  public:

    QgsMergedBookmarksTableModel( QAbstractTableModel &qgisTableModel, QAbstractTableModel &projectTableModel, QTreeView *treeView, QObject *parent = nullptr );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;

    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    QAbstractTableModel *qgisModel();

  private:
    QAbstractTableModel &mQgisTableModel;
    QAbstractTableModel &mProjectTableModel;
    QTreeView *mTreeView = nullptr;
    bool projectAvailable() const;
    void moveBookmark( QAbstractTableModel &modelFrom, QAbstractTableModel &modelTo, int row );

  signals:

    void selectItem( const QModelIndex &index );

  private slots:
    void allLayoutChanged()
    {
      emit layoutChanged();
    }
};


class APP_EXPORT QgsBookmarks : public QgsDockWidget, private Ui::QgsBookmarksBase
{
    Q_OBJECT

  public:
    QgsBookmarks( QWidget *parent = nullptr );
    ~QgsBookmarks() override;
    QMap<QString, QModelIndex> getIndexMap();
    void zoomToBookmarkIndex( const QModelIndex & );

  public slots:
    void addClicked();

  private slots:
    void deleteClicked();
    void zoomToBookmark();
    void exportToXml();
    void importFromXml();

    void lstBookmarks_doubleClicked( const QModelIndex & );

  private:
    QSqlTableModel *mQgisModel = nullptr;
    QgsProjectBookmarksTableModel *mProjectModel = nullptr;
    QgsMergedBookmarksTableModel *mMergedModel = nullptr;
    QgsBookmarksProxyModel *mProxyModel = nullptr;

    void saveWindowLocation();

};


#endif // QGSBOOKMARKS_H
