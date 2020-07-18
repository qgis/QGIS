/***************************************************************************
                 qgsdataitem.h  - Items representing data
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATAITEM_H
#define QGSDATAITEM_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QIcon>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QTreeWidget>
#include <QVector>
#include <QDateTime>

#include "qgsmaplayer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmimedatautils.h"
#include "qgswkbtypes.h"

class QgsDataProvider;
class QgsDataItem;
class QgsAnimatedIcon;
class QgsBookmarkManager;

typedef QgsDataItem *dataItem_t( QString, QgsDataItem * ) SIP_SKIP;

/**
 * \ingroup core
 * Base class for all items in the model.
 * Parent/children hierarchy is not based on QObject.
*/
class CORE_EXPORT QgsDataItem : public QObject
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsLayerItem *>( sipCpp ) )
      sipType = sipType_QgsLayerItem;
    else if ( qobject_cast<QgsErrorItem *>( sipCpp ) )
      sipType = sipType_QgsErrorItem;
    else if ( qobject_cast<QgsDirectoryItem *>( sipCpp ) )
      sipType = sipType_QgsDirectoryItem;
    else if ( qobject_cast<QgsFavoritesItem *>( sipCpp ) )
      sipType = sipType_QgsFavoritesItem;
    else if ( qobject_cast<QgsZipItem *>( sipCpp ) )
      sipType = sipType_QgsZipItem;
    else if ( qobject_cast<QgsDataCollectionItem *>( sipCpp ) )
      sipType = sipType_QgsDataCollectionItem;
    else if ( qobject_cast<QgsProjectItem *>( sipCpp ) )
      sipType = sipType_QgsProjectItem;
    else
      sipType = 0;
    SIP_END
#endif

    Q_OBJECT

  public:
    enum Type
    {
      Collection,
      Directory,
      Layer,
      Error,
      Favorites, //!< Represents a favorite item
      Project, //!< Represents a QGIS project
      Custom, //!< Custom item type
      Fields, //!< Collection of fields
      Field, //!< Vector layer field
    };

    Q_ENUM( Type )

    /**
     * Constructor for QgsDataItem, with the specified \a parent item.
     *
     * The \a name argument specifies the text to show in the model for the item. A translated string should
     * be used wherever appropriate.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form,
     * but QgsDataItem items pointing to different logical locations should always use a different item \a path.
     *
     * The optional \a providerKey string (added in QGIS 3.12) can be used to specify the key for the QgsDataItemProvider that created this item.
     */
    QgsDataItem( QgsDataItem::Type type, QgsDataItem *parent SIP_TRANSFERTHIS, const QString &name, const QString &path, const QString &providerKey = QString() );

    ~QgsDataItem() override;

    bool hasChildren();

    /**
     * Returns TRUE if the data item is a collection of layers
     * The default implementation returns FALSE, subclasses must implement this method if their children are layers.
     * \since QGIS 3.14
     */
    virtual bool layerCollection() const;

    int rowCount();

    /**
     * Create children. Children are not expected to have parent set.
     * This method MUST BE THREAD SAFE. */
    virtual QVector<QgsDataItem *> createChildren() SIP_FACTORY;

    enum State
    {
      NotPopulated, //!< Children not yet created
      Populating,   //!< Creating children in separate thread (populating or refreshing)
      Populated     //!< Children created
    };
    Q_ENUM( State )

    //! \since QGIS 2.8
    State state() const;

    /**
     * Set item state. It also take care about starting/stopping loading icon animation.
     * \param state
     * \since QGIS 2.8
     */
    virtual void setState( State state );

    /**
     * Inserts a new child item. The child will be inserted at a position using an alphabetical order based on mName.
     * \param child child item to insert. Ownership is transferred, and item parent will be set and relevant connections made.
     * \param refresh - set to TRUE to refresh populated item, emitting relevant signals to the model
     * \see deleteChildItem()
     */
    virtual void addChildItem( QgsDataItem *child SIP_TRANSFER, bool refresh = false );

    /**
     * Removes and deletes a child item, emitting relevant signals to the model.
     * \param child child to remove. Item must exist as a current child.
     * \see addChildItem()
     */
    virtual void deleteChildItem( QgsDataItem *child );

    /**
     * Removes a child item and returns it without deleting it. Emits relevant signals to model as required.
     * \param child child to remove
     * \returns pointer to the removed item or NULLPTR if no such item was found
     */
    virtual QgsDataItem *removeChildItem( QgsDataItem *child ) SIP_TRANSFERBACK;

    /**
     * Returns TRUE if this item is equal to another item (by testing item type and path).
     */
    virtual bool equal( const QgsDataItem *other );

    /**
     * Returns source widget from data item for QgsBrowserPropertiesWidget
     *
     * Use QgsDataItemGuiProvider::createParamWidget() instead
     *
     * \deprecated QGIS 3.10
     */
    Q_DECL_DEPRECATED virtual QWidget *paramWidget() SIP_FACTORY SIP_DEPRECATED { return nullptr; }

    /**
     * Returns the list of actions available for this item. This is usually used for the popup menu on right-clicking
     * the item. Subclasses should override this to provide actions.
     *
     * Subclasses should ensure that ownership of created actions is correctly handled by parenting them
     * to the specified parent widget.
     */
    virtual QList<QAction *> actions( QWidget *parent );

    /**
     * Returns the list of menus available for this item. This is usually used for the popup menu on right-clicking
     * the item. Subclasses should override this to provide actions. Subclasses should ensure that ownership of
     * created menus is correctly handled by parenting them to the specified parent widget.
     * \param parent a parent widget of the menu
     * \returns list of menus
     * \since QGIS 3.0
     */
    virtual QList<QMenu *> menus( QWidget *parent );

    /**
     * Returns whether the item accepts drag and dropped layers - e.g. for importing a dataset to a provider.
     * Subclasses should override this and handleDrop() to accept dropped layers.
     * \see handleDrop()
     * \see QgsDataItemGuiProvider::handleDrop()
     *
     * \deprecated QGIS 3.10
     */
    Q_DECL_DEPRECATED virtual bool acceptDrop() SIP_DEPRECATED { return false; }

    /**
     * Attempts to process the mime data dropped on this item. Subclasses must override this and acceptDrop() if they
     * accept dropped layers.
     * \see acceptDrop()
     * \see QgsDataItemGuiProvider::handleDrop()
     *
     * \deprecated QGIS 3.10
     */
    Q_DECL_DEPRECATED virtual bool handleDrop( const QMimeData * /*data*/, Qt::DropAction /*action*/ ) SIP_DEPRECATED { return false; }

    /**
     * Called when a user double clicks on the item. Subclasses should return TRUE
     * if they have implemented a double-click handler and do not want the default
     * double-click behavior for items.
     * \since QGIS 3.0
     */
    virtual bool handleDoubleClick();

    /**
     * Returns TRUE if the item may be dragged.
     * Default implementation returns FALSE.
     * A draggable item has to implement mimeUri() that will be used to pass data.
     * \see mimeUri()
     * \since QGIS 3.0
     */
    virtual bool hasDragEnabled() const { return false; }

    /**
     * Returns mime URI for the data item.
     * Items that return valid URI will be returned in mime data when dragging a selection from browser model.
     * \see hasDragEnabled()
     * \since QGIS 3.0
     */
    virtual QgsMimeDataUtils::Uri mimeUri() const { return QgsMimeDataUtils::Uri(); }

    enum Capability
    {
      NoCapabilities    = 0,
      SetCrs            = 1 << 0, //!< Can set CRS on layer or group of layers. \deprecated since QGIS 3.6 -- no longer used by QGIS and will be removed in QGIS 4.0
      Fertile           = 1 << 1, //!< Can create children. Even items without this capability may have children, but cannot create them, it means that children are created by item ancestors.
      Fast              = 1 << 2, //!< CreateChildren() is fast enough to be run in main thread when refreshing items, most root items (wms,wfs,wcs,postgres...) are considered fast because they are reading data only from QgsSettings
      Collapse          = 1 << 3, //!< The collapse/expand status for this items children should be ignored in order to avoid undesired network connections (wms etc.)
      Rename            = 1 << 4, //!< Item can be renamed
      Delete            = 1 << 5, //!< Item can be deleted
    };
    Q_DECLARE_FLAGS( Capabilities, Capability )

    /**
     * Writes the selected crs into data source. The original data source will be modified when calling this
     * method.
     *
     * \deprecated since QGIS 3.6. This method is no longer used by QGIS and will be removed in QGIS 4.0.
     */
    Q_DECL_DEPRECATED virtual bool setCrs( const QgsCoordinateReferenceSystem &crs ) SIP_DEPRECATED
    {
      Q_UNUSED( crs )
      return false;
    }

    /**
     * Sets a new \a name for the item, and returns TRUE if the item was successfully renamed.
     *
     * Items which implement this method should return the QgsDataItem::Rename capability.
     *
     * The default implementation does nothing.
     *
     * Use QgsDataItemGuiProvider:
     *
     * \since QGIS 3.4
     * \deprecated QGIS 3.10
     */
    Q_DECL_DEPRECATED virtual bool rename( const QString &name ) SIP_DEPRECATED;

    // ### QGIS 4 - rename to capabilities()

    /**
     * Returns the capabilities for the data item.
     *
     * \see setCapabilities()
     */
    virtual Capabilities capabilities2() const { return mCapabilities; }

    /**
     * Sets the capabilities for the data item.
     *
     * \see capabilities2()
     */
    virtual void setCapabilities( Capabilities capabilities ) { mCapabilities = capabilities; }

    // static methods

    // Find child index in vector of items using '==' operator
    static int findItem( QVector<QgsDataItem *> items, QgsDataItem *item );

    // members

    Type type() const { return mType; }

    /**
     * Gets item parent. QgsDataItem maintains its own items hierarchy, it does not use
     *  QObject hierarchy. */
    QgsDataItem *parent() const { return mParent; }

    /**
     * Set item parent and connect / disconnect parent to / from item signals.
     *  It does not add itself to parents children (mChildren) */
    void setParent( QgsDataItem *parent );
    QVector<QgsDataItem *> children() const { return mChildren; }
    virtual QIcon icon();

    /**
     * Returns the name of the item (the displayed text for the item).
     *
     * \see setName()
     */
    QString name() const { return mName; }

    /**
     * Sets the \a name of the item (the displayed text for the item).
     *
     * \see name()
     */
    void setName( const QString &name );

    QString path() const { return mPath; }
    void setPath( const QString &path ) { mPath = path; }

    /**
     * Returns the provider key that created this item (e.g. "PostGIS")
     *
     * If key has a prefix "special:", it marks that the item was not created with a provider,
     * but manually. For example "special:Favorites", "special:Home"
     *
     * \since QGIS 3.12
     */
    QString providerKey() const;

    /**
     * Sets the provider key that created this item (e.g. "PostGIS")
     *
     * If key has a prefix "special:", it marks that the item was not created with a provider,
     * but manually. For example "special:Favorites"
     *
     * \since QGIS 3.12
     */
    void setProviderKey( const QString &value );

    //! Create path component replacing path separators
    static QString pathComponent( const QString &component );

    /**
     * Returns the sorting key for the item. By default name() is returned,
     * but setSortKey() can be used to set a custom sort key for the item.
     *
     * Alternatively subclasses can override this method to return a custom
     * sort key.
     *
     * \see setSortKey()
     * \since QGIS 3.0
     */
    virtual QVariant sortKey() const;

    /**
     * Sets a custom sorting \a key for the item.
     * \see sortKey()
     * \since QGIS 3.0
     */
    void setSortKey( const QVariant &key );


    // Because QIcon (QPixmap) must not be used in outside the GUI thread, it is
    // not possible to set mIcon in constructor. Either use mIconName/setIconName()
    // or implement icon().
    void setIcon( const QIcon &icon ) { mIcon = icon; }
    void setIconName( const QString &iconName ) { mIconName = iconName; }

    void setToolTip( const QString &msg ) { mToolTip = msg; }
    QString toolTip() const { return mToolTip; }

    // deleteLater() items and clear the vector
    static void deleteLater( QVector<QgsDataItem *> &items );

    //! Move object and all its descendants to thread
    void moveToThread( QThread *targetThread );

  protected:
    virtual void populate( const QVector<QgsDataItem *> &children );

    /**
     * Refresh the items from a specified list of child items.
     */
    virtual void refresh( const QVector<QgsDataItem *> &children );

    /**
     * The item is scheduled to be deleted. E.g. if deleteLater() is called when
     * item is in Populating state (createChildren() running in another thread),
     * the deferredDelete() returns TRUE and item will be deleted once Populating finished.
     * Items with slow reateChildren() (for example network or database based) may
     * check during createChildren() if deferredDelete() returns TRUE and return from
     * createChildren() immediately because result will be useless. */
    bool deferredDelete() { return mDeferredDelete; }

    Type mType;
    Capabilities mCapabilities;
    QgsDataItem *mParent = nullptr;
    QVector<QgsDataItem *> mChildren; // easier to have it always
    State mState;
    QString mName;
    QString mProviderKey;
    // Path is slash ('/') separated chain of item identifiers which are usually item names, but may be different if it is
    // necessary to distinguish paths of two providers to the same source (e.g GRASS location and standard directory have the same
    // name but different paths). Identifiers in path must not contain '/' characters.
    // The path is used to identify item in tree.
    QString mPath;
    QString mToolTip;
    QString mIconName;
    QIcon mIcon;
    QMap<QString, QIcon> mIconMap;

    //! Custom sort key. If invalid, name() will be used for sorting instead.
    QVariant mSortKey;

  public slots:

    /**
     * Safely delete the item:
     *
     * - disconnects parent
     * - unsets parent (but does not remove itself)
     * - deletes all its descendants recursively
     * - waits until Populating state (createChildren() in thread) finished without blocking main thread
     * - calls QObject::deleteLater()
     */
    virtual void deleteLater();

    // Populate children using children vector created by createChildren()
    // \param foreground run createChildren in foreground
    virtual void populate( bool foreground = false );

    //! Remove children recursively and set as not populated. This is used when refreshing collapsed items.
    virtual void depopulate();

    virtual void refresh();

    /**
     * Causes a data item provider to refresh all registered connections.
     *
     * If \a providerKey is specified then only the matching provider will be refreshed. Otherwise,
     * all providers will be refreshed (which is potentially very expensive!).
     */
    virtual void refreshConnections( const QString &providerKey = QString() );

    virtual void childrenCreated();

  signals:
    void beginInsertItems( QgsDataItem *parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsDataItem *parent, int first, int last );
    void endRemoveItems();
    void dataChanged( QgsDataItem *item );
    void stateChanged( QgsDataItem *item, QgsDataItem::State oldState );

    /**
     * Emitted when the connections of the provider with the specified \a providerKey have changed.
     *
     * This signal is normally forwarded to the app in order to refresh the connection
     * item in the provider dialogs and to refresh the connection items in the other
     * open browsers.
     */
    void connectionsChanged( const QString &providerKey = QString() );

  protected slots:

    /**
     * Will request a repaint of this icon.
     *
     * \since QGIS 3.0
     */
    void updateIcon();

  private:
    static QVector<QgsDataItem *> runCreateChildren( QgsDataItem *item );

    // Set to true if object has to be deleted when possible (nothing running in threads)
    bool mDeferredDelete;
    QFutureWatcher< QVector <QgsDataItem *> > *mFutureWatcher;
    // number of items currently in loading (populating) state
    static QgsAnimatedIcon *sPopulatingIcon;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsDataItem::Capabilities )

/**
 * \ingroup core
 * Item that represents a layer that can be opened with one of the providers
*/
class CORE_EXPORT QgsLayerItem : public QgsDataItem
{
    Q_OBJECT

  public:
    enum LayerType
    {
      NoType,
      Vector,
      Raster,
      Point,
      Line,
      Polygon,
      TableLayer,
      Database,
      Table,
      Plugin,    //!< Added in 2.10
      Mesh,      //!< Added in 3.2
      VectorTile //!< Added in 3.14
    };

    Q_ENUM( LayerType )

    QgsLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, LayerType layerType, const QString &providerKey );

    // --- reimplemented from QgsDataItem ---

    bool equal( const QgsDataItem *other ) override;

    bool hasDragEnabled() const override { return true; }

    QgsMimeDataUtils::Uri mimeUri() const override;

    // --- New virtual methods for layer item derived classes ---

    //! Returns QgsMapLayerType
    QgsMapLayerType mapLayerType() const;

    /**
     * Returns the layer item type corresponding to a QgsMapLayer \a layer.
     * \since QGIS 3.6
     */
    static LayerType typeFromMapLayer( QgsMapLayer *layer );

    //! Returns layer uri or empty string if layer cannot be created
    QString uri() const { return mUri; }

    //! Returns provider key
    QString providerKey() const { return mProviderKey; }

    /**
     * Returns the supported CRS
     *  \since QGIS 2.8
     */
    QStringList supportedCrs() const { return mSupportedCRS; }

    /**
     * Returns the supported formats
     *  \since QGIS 2.8
     */
    QStringList supportedFormats() const { return mSupportFormats; }

    /**
     * Returns comments of the layer
     * \since QGIS 2.12
     */
    virtual QString comments() const { return QString(); }

    /**
     * Returns the string representation of the given \a layerType
     * \since QGIS 3
     */
    static QString layerTypeAsString( LayerType layerType );

    /**
     * Returns the icon name of the given \a layerType
     * \since QGIS 3
     */
    static QString iconName( LayerType layerType );

    /**
     * Delete this layer item
     * Use QgsDataItemGuiProvider::deleteLayer instead
     *
     * \deprecated QGIS 3.10
     */
    Q_DECL_DEPRECATED virtual bool deleteLayer() SIP_DEPRECATED;

  protected:
    //! The URI
    QString mUri;
    //! The layer type
    LayerType mLayerType;
    //! The list of supported CRS
    QStringList mSupportedCRS;
    //! The list of supported formats
    QStringList mSupportFormats;

  public:
    static QIcon iconPoint();
    static QIcon iconLine();
    static QIcon iconPolygon();
    static QIcon iconTable();
    static QIcon iconRaster();
    static QIcon iconDefault();
    //! Returns icon for mesh layer type
    static QIcon iconMesh();
    //! Returns icon for vector tile layer
    static QIcon iconVectorTile();

    //! \returns the layer name
    virtual QString layerName() const { return name(); }
};


/**
 * \ingroup core
 * A Collection: logical collection of layers or subcollections, e.g. GRASS location/mapset, database? wms source?
*/
class CORE_EXPORT QgsDataCollectionItem : public QgsDataItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsDataCollectionItem, with the specified \a parent item.
     *
     * The \a name argument specifies the text to show in the model for the item. A translated string should
     * be used wherever appropriate.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form,
     * but QgsDataCollectionItem items pointing to different logical locations should always use a different item \a path.
     *
     * The optional \a providerKey string can be used to specify the key for the QgsDataItemProvider that created this item.
     */
    QgsDataCollectionItem( QgsDataItem *parent SIP_TRANSFERTHIS, const QString &name, const QString &path = QString(), const QString &providerKey = QString() );

    ~QgsDataCollectionItem() override;

    void addChild( QgsDataItem *item SIP_TRANSFER ) { mChildren.append( item ); }

    /**
     * Returns the standard browser directory icon.
     * \see iconDataCollection()
     */
    static QIcon iconDir();

    /**
     * Returns the standard browser data collection icon.
     * \see iconDir()
     */
    static QIcon iconDataCollection();

  protected:

    /**
     * Shared open directory icon.
     * \since QGIS 3.4
     */
    static QIcon openDirIcon();

    /**
     * Shared home directory icon.
     * \since QGIS 3.4
     */
    static QIcon homeDirIcon();
};


/**
 * \ingroup core
 * A Collection that represents a database schema item
 * \since QGIS 3.16
*/
class CORE_EXPORT QgsDatabaseSchemaItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsDatabaseSchemaItem, with the specified \a parent item.
     *
     * The \a name argument specifies the text to show in the model for the item. A translated string should
     * be used wherever appropriate.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form,
     * but QgsSchemaItem items pointing to different logical locations should always use a different item \a path.
     *
     * The optional \a providerKey string can be used to specify the key for the QgsDataItemProvider that created this item.
     */
    QgsDatabaseSchemaItem( QgsDataItem *parent SIP_TRANSFERTHIS, const QString &name, const QString &path = QString(), const QString &providerKey = QString() );

    ~QgsDatabaseSchemaItem() override;


    /**
     * Returns the standard browser data collection icon.
     * \see iconDir()
     */
    static QIcon iconDataCollection();

};



/**
 * \ingroup core
 * A Collection that represents a root group of connections from a single data provider
 * \since QGIS 3.16
*/
class CORE_EXPORT QgsConnectionsRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsConnectionsRootItem, with the specified \a parent item.
     *
     * The \a name argument specifies the text to show in the model for the item. A translated string should
     * be used wherever appropriate.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form,
     * but QgsSchemaItem items pointing to different logical locations should always use a different item \a path.
     *
     * The optional \a providerKey string can be used to specify the key for the QgsDataItemProvider that created this item.
     */
    QgsConnectionsRootItem( QgsDataItem *parent SIP_TRANSFERTHIS, const QString &name, const QString &path = QString(), const QString &providerKey = QString() );

    ~QgsConnectionsRootItem() override = default;
};


/**
 * \ingroup core
 * A directory: contains subdirectories and layers
*/
class CORE_EXPORT QgsDirectoryItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsDirectoryItem, with the specified \a parent item.
     *
     * The \a name argument specifies the text to show in the model for the item. This is usually
     * the directory name, but in certain cases may differ for special directories (e.g. "Home").
     * If a non-directory-name text is used, it should be a translated string when appropriate.
     *
     * The \a path argument specifies the directory path in the file system (e.g. "/home/gsherman/stuff"). A valid
     * directory path must be specified.
     */
    QgsDirectoryItem( QgsDataItem *parent SIP_TRANSFERTHIS, const QString &name, const QString &path );


    // TODO QGIS 4.0 -- rename "name" to "title" or "text" or something more descriptive, and "path" to something
    // else to clarify the role of dirPath vs path

    /**
     * Constructor for QgsDirectoryItem, with the specified \a parent item.
     *
     * The \a name argument specifies the text to show in the model for the item. This is usually
     * the directory name, but in certain cases may differ for special directories (e.g. "Home").
     * If a non-directory-name text is used, it should be a translated string when appropriate.
     *
     * The \a dirPath argument specifies the directory path in the file system (e.g. "/home/gsherman/stuff"). A valid
     * directory path must be specified.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form, but is usually
     * the same as \a dirPath or \a dirPath with a prefix, e.g. "favorites:/home/gsherman/Downloads"). QgsDirectoryItem
     * items pointing to different \a dirPaths should always use a different item \a path.
     *
     * The optional \a providerKey string can be used to specify the key for the QgsDataItemProvider that created this item.
     */
    QgsDirectoryItem( QgsDataItem *parent SIP_TRANSFERTHIS, const QString &name, const QString &dirPath, const QString &path, const QString &providerKey = QString() );

    void setState( State state ) override;

    QVector<QgsDataItem *> createChildren() override;

    /**
     * Returns the full path to the directory the item represents.
     */
    QString dirPath() const { return mDirPath; }

    bool equal( const QgsDataItem *other ) override;
    QIcon icon() override;
    Q_DECL_DEPRECATED QWidget *paramWidget() override SIP_FACTORY SIP_DEPRECATED;
    bool hasDragEnabled() const override { return true; }
    QgsMimeDataUtils::Uri mimeUri() const override;

    //! Check if the given path is hidden from the browser model
    static bool hiddenPath( const QString &path );

  public slots:
    void childrenCreated() override;
    void directoryChanged();

  protected:
    void init();
    QString mDirPath;

  private:
    QFileSystemWatcher *mFileSystemWatcher = nullptr;
    bool mRefreshLater;
    QDateTime mLastScan;
};

/**
 * \ingroup core
 Data item that can be used to represent QGIS projects.
 */
class CORE_EXPORT QgsProjectItem : public QgsDataItem
{
    Q_OBJECT
  public:

    /**
     * \brief A data item holding a reference to a QGIS project file.
     * \param parent The parent data item.
     * \param name The name of the of the project. Displayed to the user.
     * \param path The full path to the project.
     * \param providerKey key of the provider that created this item
     */
    QgsProjectItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &providerKey = QString() );

    bool hasDragEnabled() const override { return true; }

    QgsMimeDataUtils::Uri mimeUri() const override;

};

/**
 * \ingroup core
 Data item that can be used to report problems (e.g. network error)
 */
class CORE_EXPORT QgsErrorItem : public QgsDataItem
{
    Q_OBJECT
  public:

    QgsErrorItem( QgsDataItem *parent, const QString &error, const QString &path );

};

// ---------

// TODO: move to qgis_gui for QGIS 4

/**
 * \ingroup core
 * \class QgsDirectoryParamWidget
 *
 * Browser parameter widget implementation for directory items.
 */
class CORE_EXPORT QgsDirectoryParamWidget : public QTreeWidget
{
    Q_OBJECT

  public:
    QgsDirectoryParamWidget( const QString &path, QWidget *parent SIP_TRANSFERTHIS = nullptr );

  protected:
    void mousePressEvent( QMouseEvent *event ) override;

  public slots:
    void showHideColumn();
};

/**
 * \ingroup core
 * Contains various Favorites directories
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsFavoritesItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsFavoritesItem. Accepts a path argument specifying the file path associated with
     * the item.
     */
    QgsFavoritesItem( QgsDataItem *parent, const QString &name, const QString &path = QString() );

    QVector<QgsDataItem *> createChildren() override;

    /**
     * Adds a new \a directory to the favorites group.
     *
     * If \a name is specified, it will be used as the favorite's name. Otherwise
     * the name will be set to match \a directory.
     *
     * \see removeDirectory()
     */
    void addDirectory( const QString &directory, const QString &name = QString() );

    /**
     * Removes an existing directory from the favorites group.
     * \see addDirectory()
     */
    void removeDirectory( QgsDirectoryItem *item );

    /**
     * Renames the stored favorite with corresponding \a path a new \a name.
     */
    void renameFavorite( const QString &path, const QString &name );

    //! Icon for favorites group
    static QIcon iconFavorites();

    QVariant sortKey() const override;

  private:
    QVector<QgsDataItem *> createChildren( const QString &favDir, const QString &name );
};

/**
 * \ingroup core
 * A zip file: contains layers, using GDAL/OGR VSIFILE mechanism
*/
class CORE_EXPORT QgsZipItem : public QgsDataCollectionItem
{
    Q_OBJECT

  protected:
    QString mFilePath;
    QString mVsiPrefix;
    QStringList mZipFileList;

  public:
    //! Constructor
    QgsZipItem( QgsDataItem *parent, const QString &name, const QString &path );

    //! Constructor
    QgsZipItem( QgsDataItem *parent, const QString &name, const QString &filePath, const QString &path, const QString &providerKey = QString() );

    QVector<QgsDataItem *> createChildren() override;
    QStringList getZipFileList();

    //! \note not available via Python bindings
    static QVector<dataItem_t *> sDataItemPtr SIP_SKIP;
    static QStringList sProviderNames;

    static QString vsiPrefix( const QString &uri ) { return qgsVsiPrefix( uri ); }

    /**
     * Creates a new data item from the specified path.
     */
    static QgsDataItem *itemFromPath( QgsDataItem *parent, const QString &path, const QString &name ) SIP_FACTORY;

    /**
    * Creates a new data item from the specified path.
    * \note available in Python as itemFromFilePath
    */
    static QgsDataItem *itemFromPath( QgsDataItem *parent, const QString &filePath, const QString &name, const QString &path ) SIP_FACTORY SIP_PYNAME( itemFromFilePath );

    static QIcon iconZip();

  private:
    void init();
};


/**
 * \ingroup core
 * A collection of field items with some internal logic to retrieve
 * the fields and a the vector layer instance from a connection URI,
 * the schema and the table name.
 * \since QGIS 3.16
*/
class CORE_EXPORT QgsFieldsItem : public QgsDataItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsFieldsItem, with the specified \a parent item.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form,
     * but QgsDataItem items pointing to different logical locations should always use a different item \a path.
     * The \connectionUri argument is the connection part of the layer URI that it is used internally to create
     * a connection and retrieve fields information.
     * The \a providerKey string can be used to specify the key for the QgsDataItemProvider that created this item.
     * The \a schema and \a tableName are used to retrieve the layer and field information from the \a connectionUri.
     */
    QgsFieldsItem( QgsDataItem *parent SIP_TRANSFERTHIS,
                   const QString &path,
                   const QString &connectionUri,
                   const QString &providerKey,
                   const QString &schema,
                   const QString &tableName );

    ~QgsFieldsItem() override;

    QVector<QgsDataItem *> createChildren() override;

    QIcon icon() override;

    /**
     * Returns the schema name
     */
    QString schema() const;

    /**
     * Returns the table name
     */
    QString tableName() const;

    /**
     * Returns the connection URI
     */
    QString connectionUri() const;

    /**
     * Creates and returns a (possibly NULL) layer from the connection URI and schema/table information
     */
    QgsVectorLayer *layer() SIP_FACTORY;


  private:

    QString mSchema;
    QString mTableName;
    QString mConnectionUri;
};


/**
 * \ingroup core
 * A layer field item, information about the connection URI, the schema and the
 * table as well as the layer instance the field belongs to can be retrieved
 * from the parent QgsFieldsItem object.
 * \since QGIS 3.16
*/
class CORE_EXPORT QgsFieldItem : public QgsDataItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsFieldItem, with the specified \a parent item and \a field.
     * \note parent item must be a QgsFieldsItem
     */
    QgsFieldItem( QgsDataItem *parent SIP_TRANSFERTHIS,
                  const QgsField &field );

    ~QgsFieldItem() override;

    QIcon icon() override;

  private:

    const QgsField mField;

};



///@cond PRIVATE
#ifndef SIP_RUN

/**
 * \ingroup core
 * A directory item showing the current project directory.
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsProjectHomeItem : public QgsDirectoryItem
{
    Q_OBJECT

  public:

    QgsProjectHomeItem( QgsDataItem *parent, const QString &name, const QString &dirPath, const QString &path );

    QIcon icon() override;
    QVariant sortKey() const override;

};

/**
 * \ingroup core
 * A directory item showing the a single favorite directory.
 * \since QGIS 3.0
*/
Q_NOWARN_DEPRECATED_PUSH  // rename is deprecated
class CORE_EXPORT QgsFavoriteItem : public QgsDirectoryItem
{
    Q_OBJECT

  public:

    QgsFavoriteItem( QgsFavoritesItem *parent, const QString &name, const QString &dirPath, const QString &path );

    bool rename( const QString &name ) override;

  private:

    QgsFavoritesItem *mFavorites = nullptr;
};
Q_NOWARN_DEPRECATED_POP

#endif
///@endcond

#endif // QGSDATAITEM_H


