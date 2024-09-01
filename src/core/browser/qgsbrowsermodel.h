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
#include "qgis.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <QMimeData>
#include <QMovie>

class QgsDataItem;
class QgsDataItemProvider;
class QgsDirectoryItem;
class QgsFavoriteItem;
class QgsFavoritesItem;

/**
 * \ingroup core
 * \class QgsBrowserModel
 *
 * \brief A model for showing available data sources and other items in a structured
 * tree.
 *
 * QgsBrowserModel is the foundation for the QGIS browser panel, and includes
 * items for the different data providers and folders accessible to users.
 *
 * QgsBrowserModel models are not initially populated and use a deferred initialization
 * approach. After constructing a QgsBrowserModel, a call must be made
 * to initialize() in order to populate the model.
 *
 * \note Since QGIS 3.10 it is recommended to use QgsBrowserGuiModel from GUI library.
 * Implementation of data items used from QgsBrowserModel should not trigger any GUI
 * operations such as opening of widgets/dialogs or showing message boxes. Such actions
 * should be implemented in a new QgsDataItemGuiProvider subclass which is used
 * by QgsBrowserGuiModel (but not by QgsBrowserModel).
 */
class CORE_EXPORT QgsBrowserModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsBrowserModel, with the specified \a parent object.
     *
     * \note QgsBrowserModel models are not initially populated and use a deferred initialization
     * approach. After constructing a QgsBrowserModel, a call must be made
     * to initialize() in order to populate the model.
     */
    explicit QgsBrowserModel( QObject *parent = nullptr );

    ~QgsBrowserModel() override;

    // *INDENT-OFF*

    /**
     * Custom model roles.
     *
     * \note Prior to QGIS 3.36 this was available as QgsBrowserModel::ItemDataRole
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsBrowserModel, ItemDataRole ): int
      {
      Path SIP_MONKEYPATCH_COMPAT_NAME( PathRole ) = Qt::UserRole, //!< Item path used to access path in the tree, see QgsDataItem::mPath
      Comment SIP_MONKEYPATCH_COMPAT_NAME( CommentRole ) = Qt::UserRole + 1, //!< Item comment
      Sort SIP_MONKEYPATCH_COMPAT_NAME( SortRole ), //!< Custom sort role, see QgsDataItem::sortKey()
      ProviderKey SIP_MONKEYPATCH_COMPAT_NAME( ProviderKeyRole ), //!< Data item provider key that created the item, see QgsDataItem::providerKey() \since QGIS 3.12
      LayerMetadata SIP_MONKEYPATCH_COMPAT_NAME( LayerMetadataRole ), //! Data item layer metadata for layer items
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    // implemented methods from QAbstractItemModel for read-only access

    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;
    bool hasChildren( const QModelIndex &parent = QModelIndex() ) const override;
    bool canFetchMore( const QModelIndex &parent ) const override;
    void fetchMore( const QModelIndex &parent ) override;

    /**
     * Returns the model index corresponding to the specified data \a item.
     * If the item was not found, an invalid QModelIndex is returned.
     *
     * If the \a parent item is argument is specified, then only items which are children
     * of \a parent are searched. If no \a parent is specified, then all items within
     * the model are searched.
     */
    QModelIndex findItem( QgsDataItem *item, QgsDataItem *parent = nullptr ) const;

    /**
     * Returns the data item at the specified index, or NULLPTR if no item
     * exists at the index.
     */
    QgsDataItem *dataItem( const QModelIndex &idx ) const;

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
     * \returns model index, invalid if item not found
    */
    QModelIndex findPath( const QString &path, Qt::MatchFlag matchFlag = Qt::MatchExactly );

    //! \note not available in Python bindings
    static QModelIndex findPath( QAbstractItemModel *model, const QString &path, Qt::MatchFlag matchFlag = Qt::MatchExactly ) SIP_SKIP;

    /**
     * Returns index of layer item with given uri. It only searches in currently fetched
     * items, i.e. it does not fetch children.
     * \param uri item uri
     * \param index the current index of the parent (to search for children)
     * \returns model index, invalid if item not found
     *
     * \since QGIS 3.6
     */
    QModelIndex findUri( const QString &uri, QModelIndex index = QModelIndex() );

    /**
     * \deprecated QGIS 3.4. This method has no effect, and is dangerous to call in earlier QGIS versions. Any usage should be removed (and will have no harmful side-effects!).
     */
    Q_DECL_DEPRECATED void connectItem( QgsDataItem *item ) SIP_DEPRECATED;

    /**
     * Returns TRUE if the model has been initialized.
     *
     * \see initialize()
     */
    bool initialized() const { return mInitialized;  }

    /**
     * Returns a map of the root drive items shown in the browser.
     *
     * These correspond to the top-level directory items shown, e.g. on Windows the C:\, D:\, etc,
     * and on Linux the "/" root directory.
     *
     * \since QGIS 3.6
     */
    QMap<QString, QgsDirectoryItem *> driveItems() const;

    /**
     * Returns the root items for the model.
     *
     * \since QGIS 3.28
     */
    QVector<QgsDataItem *> rootItems() const { return mRootItems; }

  signals:

    //! Emitted when item children fetch was finished
    void stateChanged( const QModelIndex &index, Qgis::BrowserItemState oldState );

    /**
     * Emitted when connections for the specified \a providerKey have changed in the browser.
     *
     * Forwarded to the widget and used to notify the provider dialogs of a changed connection.
     */
    void connectionsChanged( const QString &providerKey );

  public slots:
    //! Reload the whole model
    void reload();

    /**
     * Refreshes the list of drive items, removing any corresponding to removed
     * drives and adding newly added drives.
     *
     * \since QGIS 3.4
     */
    void refreshDrives();

    void beginInsertItems( QgsDataItem *parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsDataItem *parent, int first, int last );
    void endRemoveItems();
    void itemDataChanged( QgsDataItem *item );

    /**
     * Emitted when an \a item's state is changed.
     */
    void itemStateChanged( QgsDataItem *item, Qgis::BrowserItemState oldState );

    /**
     * Adds a \a directory to the favorites group.
     *
     * If \a name is specified, it will be used as the favorite's name. Otherwise
     * the name will be set to match \a directory.
     *
     * \see removeFavorite()
     */
    void addFavoriteDirectory( const QString &directory, const QString &name = QString() );

    /**
     * Removes a favorite directory from its corresponding model index.
     * \see addFavoriteDirectory()
     */
    void removeFavorite( const QModelIndex &index );

    /**
     * Removes a \a favorite item.
     * \see addFavoriteDirectory()
     * \note Not available in Python bindings
     * \since QGIS 3.6
     */
    void removeFavorite( QgsFavoriteItem *favorite ) SIP_SKIP;

    void updateProjectHome();

    //! Hide the given path in the browser model
    void hidePath( QgsDataItem *item );

    /**
     * Delayed initialization, needed because the provider registry must be already populated.
     * \see initialized()
     */
    void initialize();

  protected:
    //! Populates the model
    void addRootItems();
    void removeRootItems();

    QVector<QgsDataItem *> mRootItems;
    QgsFavoritesItem *mFavorites = nullptr;
    QgsDirectoryItem *mProjectHome = nullptr;

  private slots:
    void dataItemProviderAdded( QgsDataItemProvider *provider );
    void dataItemProviderWillBeRemoved( QgsDataItemProvider *provider );
    void onConnectionsChanged( const QString &providerKey );

  private:
    bool mInitialized = false;
    QMap< QString, QgsDirectoryItem * > mDriveItems;

    void setupItemConnections( QgsDataItem *item );

    void removeRootItem( QgsDataItem *item );

    QgsDataItem *addProviderRootItem( QgsDataItemProvider *provider );

    friend class TestQgsBrowserModel;
    friend class TestQgsBrowserProxyModel;
};

#endif // QGSBROWSERMODEL_H
