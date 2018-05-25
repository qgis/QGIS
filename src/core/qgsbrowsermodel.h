/***************************************************************************
    qgsbrowsermodel.h
    ---------------------
    begin                : July 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBROWSERMODEL_H
#define QGSBROWSERMODEL_H

#include "qgis_core.h"
#include <QAbstractItemModel>
#include <QIcon>
#include <QMimeData>
#include <QMovie>
#include <QFuture>
#include <QFutureWatcher>

#include "qgsdataitem.h"

/**
 * \ingroup core
 * \class QgsBrowserWatcher
 * \note not available in Python bindings
*/
#ifndef SIP_RUN
class CORE_EXPORT QgsBrowserWatcher : public QFutureWatcher<QVector <QgsDataItem *> >
{
    Q_OBJECT

  public:
    QgsBrowserWatcher( QgsDataItem *item );

    QgsDataItem *item() const { return mItem; }

  signals:
    void finished( QgsDataItem *item, const QVector <QgsDataItem *> &items );

  private:
    QgsDataItem *mItem = nullptr;
};
#endif

/**
 * \ingroup core
 * \class QgsBrowserModel
 */
class CORE_EXPORT QgsBrowserModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    explicit QgsBrowserModel( QObject *parent = nullptr );
    ~QgsBrowserModel() override;

    enum ItemDataRole
    {
      PathRole = Qt::UserRole, //!< Item path used to access path in the tree, see QgsDataItem::mPath
      CommentRole = Qt::UserRole + 1, //!< Item comment
      SortRole, //!< Custom sort role, see QgsDataItem::sortKey()
    };
    // implemented methods from QAbstractItemModel for read-only access

    /**
     * Used by other components to obtain information about each item provided by the model.
      In many models, the combination of flags should include Qt::ItemIsEnabled and Qt::ItemIsSelectable. */
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    /**
     * Used to supply item data to views and delegates. Generally, models only need to supply data
      for Qt::DisplayRole and any application-specific user roles, but it is also good practice
      to provide data for Qt::ToolTipRole, Qt::AccessibleTextRole, and Qt::AccessibleDescriptionRole.
      See the Qt::ItemDataRole enum documentation for information about the types associated with each role. */
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;

    /**
     * Provides views with information to show in their headers. The information is only retrieved
      by views that can display header information. */
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    //! Provides the number of rows of data exposed by the model.
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;

    /**
     * Provides the number of columns of data exposed by the model. List models do not provide this function
      because it is already implemented in QAbstractListModel. */
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

    //! Returns the index of the item in the model specified by the given row, column and parent index.
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;

    QModelIndex findItem( QgsDataItem *item, QgsDataItem *parent = nullptr ) const;

    /**
     * Returns the parent of the model item with the given index.
     * If the item has no parent, an invalid QModelIndex is returned.
     */
    QModelIndex parent( const QModelIndex &index ) const override;

    //! Returns a list of mime that can describe model indexes
    QStringList mimeTypes() const override;

    //! Returns an object that contains serialized items of data corresponding to the list of indexes specified
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;

    //! Handles the data supplied by a drag and drop operation that ended with the given action
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;

    QgsDataItem *dataItem( const QModelIndex &idx ) const;

    bool hasChildren( const QModelIndex &parent = QModelIndex() ) const override;

    //! Refresh item specified by path
    void refresh( const QString &path );

    //! Refresh item children
    void refresh( const QModelIndex &index = QModelIndex() );

    /**
     * Returns index of item with given path. It only searches in currently fetched
     * items, i.e. it does not fetch children.
     * \param path item path
     * \param matchFlag supported is Qt::MatchExactly and Qt::MatchStartsWith which has reverse meaning, i.e. find
     *        item with the longest match from start with path (to get as close/deep as possible to deleted item).
     * \returns model index, invalid if item not found */
    QModelIndex findPath( const QString &path, Qt::MatchFlag matchFlag = Qt::MatchExactly );

    //! \note not available in Python bindings
    static QModelIndex findPath( QAbstractItemModel *model, const QString &path, Qt::MatchFlag matchFlag = Qt::MatchExactly ) SIP_SKIP;

    void connectItem( QgsDataItem *item );

    bool canFetchMore( const QModelIndex &parent ) const override;
    void fetchMore( const QModelIndex &parent ) override;

    //! Returns true if the model has been initialized
    bool initialized( ) const { return mInitialized;  }

  signals:
    //! Emitted when item children fetch was finished
    void stateChanged( const QModelIndex &index, QgsDataItem::State oldState );

    /**
     * Connections changed in the browser, forwarded to the widget and used to
     * notify the provider dialogs of a changed connection
     */
    void connectionsChanged();

  public slots:
    //! Reload the whole model
    void reload();
    void beginInsertItems( QgsDataItem *parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsDataItem *parent, int first, int last );
    void endRemoveItems();
    void itemDataChanged( QgsDataItem *item );
    void itemStateChanged( QgsDataItem *item, QgsDataItem::State oldState );

    /**
     * Adds a \a directory to the favorites group.
     *
     * If \a name is specified, it will be used as the favorite's name. Otherwise
     * the name will be set to match \a directory.
     *
     * \since QGIS 3.0
     * \see removeFavorite()
     */
    void addFavoriteDirectory( const QString &directory, const QString &name = QString() );

    /**
     * Removes a favorite directory from its corresponding model index.
     * \since QGIS 3.0
     * \see addFavoriteDirectory()
     */
    void removeFavorite( const QModelIndex &index );

    void updateProjectHome();

    //! Hide the given path in the browser model
    void hidePath( QgsDataItem *item );

    //! Delayed initialization, needed because the provider registry must be already populated
    void initialize();

  protected:
    //! Populates the model
    void addRootItems();
    void removeRootItems();

    QVector<QgsDataItem *> mRootItems;
    QgsFavoritesItem *mFavorites = nullptr;
    QgsDirectoryItem *mProjectHome = nullptr;

  private:
    bool mInitialized = false;
};

#endif // QGSBROWSERMODEL_H
