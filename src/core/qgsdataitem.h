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

#include "qgis.h"
#include "qgis_core.h"
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QIcon>
#include <QLibrary>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QTreeWidget>
#include <QVector>

#include "qgsmaplayer.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmimedatautils.h"


class QgsDataProvider;
class QgsDataItem;
class QgsAnimatedIcon;

typedef QgsDataItem *dataItem_t( QString, QgsDataItem * ) SIP_SKIP;

/** \ingroup core
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
      Project //!< Represents a QGIS project
    };

    Q_ENUM( Type );

    //! Create new data item.
    QgsDataItem( QgsDataItem::Type type, QgsDataItem *parent SIP_TRANSFERTHIS, const QString &name, const QString &path );
    virtual ~QgsDataItem();

    bool hasChildren();

    int rowCount();

    /** Create children. Children are not expected to have parent set.
     * This method MUST BE THREAD SAFE. */
    virtual QVector<QgsDataItem *> createChildren() SIP_FACTORY;

    enum State
    {
      NotPopulated, //!< Children not yet created
      Populating,   //!< Creating children in separate thread (populating or refreshing)
      Populated     //!< Children created
    };
    Q_ENUM( State );

    //! \since QGIS 2.8
    State state() const;

    /** Set item state. It also take care about starting/stopping loading icon animation.
     * \param state
     * \since QGIS 2.8
     */
    virtual void setState( State state );

    /** Inserts a new child item. The child will be inserted at a position using an alphabetical order based on mName.
     * \param child child item to insert. Ownership is transferred, and item parent will be set and relevant connections made.
     * \param refresh - set to true to refresh populated item, emitting relevant signals to the model
     * \see deleteChildItem()
     */
    virtual void addChildItem( QgsDataItem *child SIP_TRANSFER, bool refresh = false );

    /** Removes and deletes a child item, emitting relevant signals to the model.
     * \param child child to remove. Item must exist as a current child.
     * \see addChildItem()
     */
    virtual void deleteChildItem( QgsDataItem *child );

    /** Removes a child item and returns it without deleting it. Emits relevant signals to model as required.
     * \param child child to remove
     * \returns pointer to the removed item or null if no such item was found
     */
    virtual QgsDataItem *removeChildItem( QgsDataItem *child ) SIP_TRANSFERBACK;

    /** Returns true if this item is equal to another item (by testing item type and path).
     */
    virtual bool equal( const QgsDataItem *other );

    virtual QWidget *paramWidget() SIP_FACTORY { return nullptr; }

    /** Returns the list of actions available for this item. This is usually used for the popup menu on right-clicking
     * the item. Subclasses should override this to provide actions.
     */
    virtual QList<QAction *> actions() { return QList<QAction *>(); }

    /** Returns whether the item accepts drag and dropped layers - e.g. for importing a dataset to a provider.
     * Subclasses should override this and handleDrop() to accept dropped layers.
     * \see handleDrop()
     */
    virtual bool acceptDrop() { return false; }

    /** Attempts to process the mime data dropped on this item. Subclasses must override this and acceptDrop() if they
     * accept dropped layers.
     * \see acceptDrop()
     */
    virtual bool handleDrop( const QMimeData * /*data*/, Qt::DropAction /*action*/ ) { return false; }

    /** Returns true if the item may be dragged.
     * Default implementation returns false.
     * A draggable item has to implement mimeUri() that will be used to pass data.
     * \see mimeUri()
     * \since QGIS 3.0
     */
    virtual bool hasDragEnabled() const { return false; }

    /** Return mime URI for the data item.
     * Items that return valid URI will be returned in mime data when dragging a selection from browser model.
     * \see hasDragEnabled()
     * \since QGIS 3.0
     */
    virtual QgsMimeDataUtils::Uri mimeUri() const { return QgsMimeDataUtils::Uri(); }

    enum Capability
    {
      NoCapabilities    = 0,
      SetCrs            = 1 << 0, //!< Can set CRS on layer or group of layers
      Fertile           = 1 << 1, //!< Can create children. Even items without this capability may have children, but cannot create them, it means that children are created by item ancestors.
      Fast              = 1 << 2, //!< CreateChildren() is fast enough to be run in main thread when refreshing items, most root items (wms,wfs,wcs,postgres...) are considered fast because they are reading data only from QgsSettings
      Collapse          = 1 << 3  //!< The collapse/expand status for this items children should be ignored in order to avoid undesired network connections (wms etc.)
    };
    Q_DECLARE_FLAGS( Capabilities, Capability )

    /**
     * Writes the selected crs into data source. The original data source will be modified when calling this
     * method.
     */
    virtual bool setCrs( const QgsCoordinateReferenceSystem &crs ) { Q_UNUSED( crs ); return false; }

    // ### QGIS 3 - rename to capabilities()
    virtual Capabilities capabilities2() const { return mCapabilities; }

    /**
     * Sets the capabilities for the data item.
     */
    virtual void setCapabilities( Capabilities capabilities ) { mCapabilities = capabilities; }

    // static methods

    // Find child index in vector of items using '==' operator
    static int findItem( QVector<QgsDataItem *> items, QgsDataItem *item );

    // members

    Type type() const { return mType; }

    /** Get item parent. QgsDataItem maintains its own items hierarchy, it does not use
     *  QObject hierarchy. */
    QgsDataItem *parent() const { return mParent; }

    /** Set item parent and connect / disconnect parent to / from item signals.
     *  It does not add itself to parents children (mChildren) */
    void setParent( QgsDataItem *parent );
    QVector<QgsDataItem *> children() const { return mChildren; }
    virtual QIcon icon();
    QString name() const { return mName; }
    void setName( const QString &name ) { mName = name; }
    QString path() const { return mPath; }
    void setPath( const QString &path ) { mPath = path; }
    //! Create path component replacing path separators
    static QString pathComponent( const QString &component );

    // Because QIcon (QPixmap) must not be used in outside the GUI thread, it is
    // not possible to set mIcon in constructor. Either use mIconName/setIconName()
    // or implement icon().
    void setIcon( const QIcon &icon ) { mIcon = icon; }
    void setIconName( const QString &iconName ) { mIconName = iconName; }

    void setToolTip( const QString &msg ) { mToolTip = msg; }
    QString toolTip() const { return mToolTip; }

    // deleteLater() items anc clear the vector
    static void deleteLater( QVector<QgsDataItem *> &items );

    //! Move object and all its descendants to thread
    void moveToThread( QThread *targetThread );

  protected:
    virtual void populate( const QVector<QgsDataItem *> &children );

    /**
     * Refresh the items from a specified list of child items.
     */
    virtual void refresh( const QVector<QgsDataItem *> &children );

    /** The item is scheduled to be deleted. E.g. if deleteLater() is called when
     * item is in Populating state (createChildren() running in another thread),
     * the deferredDelete() returns true and item will be deleted once Populating finished.
     * Items with slow reateChildren() (for example network or database based) may
     * check during createChildren() if deferredDelete() returns true and return from
     * createChildren() immediately because result will be useless. */
    bool deferredDelete() { return mDeferredDelete; }

    Type mType;
    Capabilities mCapabilities;
    QgsDataItem *mParent = nullptr;
    QVector<QgsDataItem *> mChildren; // easier to have it always
    State mState;
    QString mName;
    // Path is slash ('/') separated chain of item identifiers which are usually item names, but may be differen if it is
    // necessary to distinguish paths of two providers to the same source (e.g GRASS location and standard directory have the same
    // name but different paths). Identifiers in path must not contain '/' characters.
    // The path is used to identify item in tree.
    QString mPath;
    QString mToolTip;
    QString mIconName;
    QIcon mIcon;
    QMap<QString, QIcon> mIconMap;

  public slots:

    /** Safely delete the item:
     *   - disconnects parent
     *   - unsets parent (but does not remove itself)
     *   - deletes all its descendants recursively
     *   - waits until Populating state (createChildren() in thread) finished without blocking main thread
     *   - calls QObject::deleteLater()
     */
    virtual void deleteLater();

    // Populate children using children vector created by createChildren()
    // \param foreground run createChildren in foreground
    virtual void populate( bool foreground = false );

    //! Remove children recursively and set as not populated. This is used when refreshing collapsed items.
    virtual void depopulate();

    virtual void refresh();

    //! Refresh connections: update GUI and emit signal
    virtual void refreshConnections();

    virtual void childrenCreated();

  signals:
    void beginInsertItems( QgsDataItem *parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsDataItem *parent, int first, int last );
    void endRemoveItems();
    void dataChanged( QgsDataItem *item );
    void stateChanged( QgsDataItem *item, QgsDataItem::State oldState );
    //! Emitted when the provider's connections of the child items have changed
    //! This signal is normally forwarded to the app in order to refresh the connection
    //! item in the provider dialogs and to refresh the connection items in the other
    //! open browsers
    void connectionsChanged();

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

/** \ingroup core
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
      Plugin     //!< Added in 2.10
    };

    Q_ENUM( LayerType );

    QgsLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, LayerType layerType, const QString &providerKey );

    // --- reimplemented from QgsDataItem ---

    virtual bool equal( const QgsDataItem *other ) override;

    virtual bool hasDragEnabled() const override { return true; }

    virtual QgsMimeDataUtils::Uri mimeUri() const override;

    // --- New virtual methods for layer item derived classes ---

    //! Returns QgsMapLayer::LayerType
    QgsMapLayer::LayerType mapLayerType() const;

    //! Returns layer uri or empty string if layer cannot be created
    QString uri() const { return mUri; }

    //! Returns provider key
    QString providerKey() const { return mProviderKey; }

    /** Returns the supported CRS
     *  \since QGIS 2.8
     */
    QStringList supportedCrs() const { return mSupportedCRS; }

    /** Returns the supported formats
     *  \since QGIS 2.8
     */
    QStringList supportedFormats() const { return mSupportFormats; }

    /** Returns comments of the layer
     * \since QGIS 2.12
     */
    virtual QString comments() const { return QString(); }

    /** Returns the string representatio of the given \a layerType
     * \since QGIS 3
     */
    static QString layerTypeAsString( const LayerType &layerType );

  protected:

    //! The provider key
    QString mProviderKey;
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

    //! \returns the layer name
    virtual QString layerName() const { return name(); }
};


/** \ingroup core
 * A Collection: logical collection of layers or subcollections, e.g. GRASS location/mapset, database? wms source?
*/
class CORE_EXPORT QgsDataCollectionItem : public QgsDataItem
{
    Q_OBJECT
  public:
    QgsDataCollectionItem( QgsDataItem *parent, const QString &name, const QString &path = QString() );
    ~QgsDataCollectionItem();

    void addChild( QgsDataItem *item SIP_TRANSFER ) { mChildren.append( item ); }

    static QIcon iconDir(); // shared icon: open/closed directory
    static QIcon iconDataCollection(); // default icon for data collection
};

/** \ingroup core
 * A directory: contains subdirectories and layers
*/
class CORE_EXPORT QgsDirectoryItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    enum Column
    {
      Name,
      Size,
      Date,
      Permissions,
      Owner,
      Group,
      Type
    };

    QgsDirectoryItem( QgsDataItem *parent, const QString &name, const QString &path );

    /** Constructor.
     * \param parent
     * \param name directory name
     * \param dirPath path to directory in file system
     * \param path item path in the tree, it may be dirPath or dirPath with some prefix, e.g. favorites: */
    QgsDirectoryItem( QgsDataItem *parent, const QString &name, const QString &dirPath, const QString &path );

    virtual void setState( State state ) override;

    QVector<QgsDataItem *> createChildren() override;

    QString dirPath() const { return mDirPath; }
    virtual bool equal( const QgsDataItem *other ) override;
    virtual QIcon icon() override;
    virtual QWidget *paramWidget() override SIP_FACTORY;

    //! Check if the given path is hidden from the browser model
    static bool hiddenPath( const QString &path );


  public slots:
    virtual void childrenCreated() override;
    void directoryChanged();

  protected:
    void init();
    QString mDirPath;

  private:
    QFileSystemWatcher *mFileSystemWatcher = nullptr;
    bool mRefreshLater;
};

/** \ingroup core
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
     */
    QgsProjectItem( QgsDataItem *parent, const QString &name, const QString &path );

    virtual bool hasDragEnabled() const override { return true; }

};

/** \ingroup core
 Data item that can be used to report problems (e.g. network error)
 */
class CORE_EXPORT QgsErrorItem : public QgsDataItem
{
    Q_OBJECT
  public:

    QgsErrorItem( QgsDataItem *parent, const QString &error, const QString &path );

};


// ---------

/** \ingroup core
 * \class QgsDirectoryParamWidget
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

/** \ingroup core
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
     * Adds a new directory to the favorites group.
     * \see removeDirectory()
     */
    void addDirectory( const QString &directory );

    /**
     * Removes an existing directory from the favorites group.
     * \see addDirectory()
     */
    void removeDirectory( QgsDirectoryItem *item );

    //! Icon for favorites group
    static QIcon iconFavorites();

  private:
    QVector<QgsDataItem *> createChildren( const QString &favDir );
};

/** \ingroup core
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
    QgsZipItem( QgsDataItem *parent, const QString &name, const QString &path );
    QgsZipItem( QgsDataItem *parent, const QString &name, const QString &filePath, const QString &path );

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

#endif // QGSDATAITEM_H


