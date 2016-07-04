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

#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QIcon>
#include <QLibrary>
#include <QMovie>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QTreeWidget>
#include <QVector>

#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgscoordinatereferencesystem.h"

class QgsDataProvider;
class QgsDataItem;

typedef QgsDataItem * dataItem_t( QString, QgsDataItem* );

/** \ingroup core
 * Animated icon is keeping an animation running if there are listeners connected to frameChanged
*/
class CORE_EXPORT QgsAnimatedIcon : public QObject
{
    Q_OBJECT
  public:

    /** Constructor
     * @param iconPath path to a movie, e.g. animated GIF */
    QgsAnimatedIcon( const QString & iconPath = QString::null );

    QString iconPath() const;
    void setIconPath( const QString & iconPath );
    QIcon icon() const { return mIcon; }

    /** Connect listener to frameChanged() signal */
    void connectFrameChanged( const QObject * receiver, const char * method );
    /** Disconnect listener from frameChanged() signal */
    void disconnectFrameChanged( const QObject * receiver, const char * method );

  public slots:
    void onFrameChanged();

  signals:
    /** Emitted when icon changed */
    void frameChanged();

  private:
    void resetMovie();
    int mCount; // number of listeners
    QMovie * mMovie;
    QIcon mIcon;
};

/** \ingroup core
 * Base class for all items in the model.
 * Parent/children hierarchy is not based on QObject.
*/
class CORE_EXPORT QgsDataItem : public QObject
{
    Q_OBJECT
    Q_ENUMS( Type )
    Q_ENUMS( State )
  public:
    enum Type
    {
      Collection,
      Directory,
      Layer,
      Error,
      Favourites,
      Project //! Represents a QGIS project
    };

    /** Create new data item. */
    QgsDataItem( QgsDataItem::Type type, QgsDataItem* parent, const QString& name, const QString& path );
    virtual ~QgsDataItem();

    bool hasChildren();

    int rowCount();

    /** Create children. Children are not expected to have parent set.
     * This method MUST BE THREAD SAFE. */
    virtual QVector<QgsDataItem*> createChildren();

    enum State
    {
      NotPopulated, //!< Children not yet created
      Populating,   //!< Creating children in separate thread (populating or refreshing)
      Populated     //!< children created
    };

    //! @note added in 2.8
    State state() const;

    /** Set item state. It also take care about starting/stopping loading icon animation.
     * @param state
     * @note added in 2.8
     */
    virtual void setState( State state );

    //! @deprecated in 2.8, use state()
    Q_DECL_DEPRECATED bool isPopulated() { return state() == Populated; }

    /** Inserts a new child item. The child will be inserted at a position using an alphabetical order based on mName.
     * @param child child item to insert. Ownership is transferred, and item parent will be set and relevant connections made.
     * @param refresh - set to true to refresh populated item, emitting relevant signals to the model
     * @see deleteChildItem()
     */
    virtual void addChildItem( QgsDataItem *child, bool refresh = false );

    /** Removes and deletes a child item, emitting relevant signals to the model.
     * @param child child to remove. Item must exist as a current child.
     * @see addChildItem()
     */
    virtual void deleteChildItem( QgsDataItem * child );

    /** Removes a child item and returns it without deleting it. Emits relevant signals to model as required.
     * @param child child to remove
     * @returns pointer to the removed item or null if no such item was found
     */
    virtual QgsDataItem *removeChildItem( QgsDataItem * child );

    /** Returns true if this item is equal to another item (by testing item type and path).
     */
    virtual bool equal( const QgsDataItem *other );

    virtual QWidget *paramWidget() { return nullptr; }

    /** Returns the list of actions available for this item. This is usually used for the popup menu on right-clicking
     * the item. Subclasses should override this to provide actions.
     */
    virtual QList<QAction*> actions() { return QList<QAction*>(); }

    /** Returns whether the item accepts drag and dropped layers - e.g. for importing a dataset to a provider.
     * Subclasses should override this and handleDrop() to accept dropped layers.
     * @see handleDrop()
     */
    virtual bool acceptDrop() { return false; }

    /** Attempts to process the mime data dropped on this item. Subclasses must override this and acceptDrop() if they
     * accept dropped layers.
     * @see acceptDrop()
     */
    virtual bool handleDrop( const QMimeData * /*data*/, Qt::DropAction /*action*/ ) { return false; }

    enum Capability
    {
      NoCapabilities = 0,
      SetCrs         = 1 << 0, //!< Can set CRS on layer or group of layers
      Fertile        = 1 << 1, //!< Can create children. Even items without this capability may have children, but cannot create them, it means that children are created by item ancestors.
      Fast           = 1 << 2  //!< createChildren() is fast enough to be run in main thread when refreshing items, most root items (wms,wfs,wcs,postgres...) are considered fast because they are reading data only from QSettings
    };
    Q_DECLARE_FLAGS( Capabilities, Capability )

    // This will _write_ selected crs in data source
    virtual bool setCrs( QgsCoordinateReferenceSystem crs )
    { Q_UNUSED( crs ); return false; }

    //! @deprecated since 2.8, returned type this will changed to Capabilities
    Q_DECL_DEPRECATED virtual Capability capabilities() { return NoCapabilities; }

    virtual Capabilities capabilities2() const { return mCapabilities; }

    virtual void setCapabilities( const Capabilities& capabilities ) { mCapabilities = capabilities; }

    // static methods

    // Find child index in vector of items using '==' operator
    static int findItem( QVector<QgsDataItem*> items, QgsDataItem * item );

    // members

    Type type() const { return mType; }

    /** Get item parent. QgsDataItem maintains its own items hierarchy, it does not use
     *  QObject hierarchy. */
    QgsDataItem* parent() const { return mParent; }
    /** Set item parent and connect / disconnect parent to / from item signals.
     *  It does not add itself to parents children (mChildren) */
    void setParent( QgsDataItem* parent );
    QVector<QgsDataItem*> children() const { return mChildren; }
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
    void setIcon( const QIcon& icon ) { mIcon = icon; }
    void setIconName( const QString & iconName ) { mIconName = iconName; }

    void setToolTip( const QString& msg ) { mToolTip = msg; }
    QString toolTip() const { return mToolTip; }

    // deleteLater() items anc clear the vector
    static void deleteLater( QVector<QgsDataItem*> &items );

    /** Move object and all its descendants to thread */
    void moveToThread( QThread * targetThread );

  protected:
    virtual void populate( const QVector<QgsDataItem*>& children );
    virtual void refresh( QVector<QgsDataItem*> children );
    /** The item is scheduled to be deleted. E.g. if deleteLater() is called when
     * item is in Populating state (createChildren() running in another thread),
     * the deferredDelete() returns true and item will be deleted once Populating finished.
     * Items with slow reateChildren() (for example network or database based) may
     * check during createChildren() if deferredDelete() returns true and return from
     * createChildren() immediately because result will be useless. */
    bool deferredDelete() { return mDeferredDelete; }

    Type mType;
    Capabilities mCapabilities;
    QgsDataItem* mParent;
    QVector<QgsDataItem*> mChildren; // easier to have it always
    State mState;
    //! @deprecated since 2.8, use mState
    bool mPopulated;
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
    // @param foreground run createChildren in foreground
    virtual void populate( bool foreground = false );

    /** Remove children recursively and set as not populated. This is used when refreshing collapsed items. */
    virtual void depopulate();

    virtual void refresh();

    void emitBeginInsertItems( QgsDataItem* parent, int first, int last );
    void emitEndInsertItems();
    void emitBeginRemoveItems( QgsDataItem* parent, int first, int last );
    void emitEndRemoveItems();
    void emitDataChanged( QgsDataItem* item );
    void emitDataChanged();
    void emitStateChanged( QgsDataItem* item, QgsDataItem::State oldState );
    virtual void childrenCreated();

  signals:
    void beginInsertItems( QgsDataItem* parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsDataItem* parent, int first, int last );
    void endRemoveItems();
    void dataChanged( QgsDataItem * item );
    void stateChanged( QgsDataItem * item, QgsDataItem::State oldState );

  private:
    static QVector<QgsDataItem*> runCreateChildren( QgsDataItem* item );

    // Set to true if object has to be deleted when possible (nothing running in threads)
    bool mDeferredDelete;
    QFutureWatcher< QVector <QgsDataItem*> > *mFutureWatcher;
    // number of items currently in loading (populating) state
    static QgsAnimatedIcon * mPopulatingIcon;
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
      Plugin     //!< added in 2.10
    };

    QgsLayerItem( QgsDataItem* parent, const QString& name, const QString& path, const QString& uri, LayerType layerType, const QString& providerKey );

    // --- reimplemented from QgsDataItem ---

    virtual bool equal( const QgsDataItem *other ) override;

    // --- New virtual methods for layer item derived classes ---

    /** Returns QgsMapLayer::LayerType */
    QgsMapLayer::LayerType mapLayerType();

    /** Returns layer uri or empty string if layer cannot be created */
    QString uri() { return mUri; }

    /** Returns provider key */
    QString providerKey() { return mProviderKey; }

    /** Returns the supported CRS
     *  @note Added in 2.8
     */
    QStringList supportedCRS() { return mSupportedCRS; }

    /** Returns the supported formats
     *  @note Added in 2.8
     */
    QStringList supportedFormats() { return mSupportFormats; }

    /** Returns comments of the layer
     * @note added in 2.12
     */
    virtual QString comments() const { return QString(); }

  protected:

    /** The provider key */
    QString mProviderKey;
    /** The URI */
    QString mUri;
    /** The layer type */
    LayerType mLayerType;
    /** The list of supported CRS */
    QStringList mSupportedCRS;
    /** The list of supported formats */
    QStringList mSupportFormats;

  public:
    static const QIcon &iconPoint();
    static const QIcon &iconLine();
    static const QIcon &iconPolygon();
    static const QIcon &iconTable();
    static const QIcon &iconRaster();
    static const QIcon &iconDefault();

    /** @return the layer name */
    virtual QString layerName() const { return name(); }
};


/** \ingroup core
 * A Collection: logical collection of layers or subcollections, e.g. GRASS location/mapset, database? wms source?
*/
class CORE_EXPORT QgsDataCollectionItem : public QgsDataItem
{
    Q_OBJECT
  public:
    QgsDataCollectionItem( QgsDataItem* parent, const QString& name, const QString& path = QString::null );
    ~QgsDataCollectionItem();

    void addChild( QgsDataItem *item ) { mChildren.append( item ); }

    static const QIcon &iconDir(); // shared icon: open/closed directory
    static const QIcon &iconDataCollection(); // default icon for data collection
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

    QgsDirectoryItem( QgsDataItem* parent, const QString& name, const QString& path );

    /** Constructor.
     * @param parent
     * @param name directory name
     * @param dirPath path to directory in file system
     * @param path item path in the tree, it may be dirPath or dirPath with some prefix, e.g. favourites: */
    QgsDirectoryItem( QgsDataItem* parent, const QString& name, const QString& dirPath, const QString& path );
    ~QgsDirectoryItem();

    virtual void setState( State state ) override;

    QVector<QgsDataItem*> createChildren() override;

    QString dirPath() const { return mDirPath; }
    virtual bool equal( const QgsDataItem *other ) override;
    virtual QIcon icon() override;
    virtual QWidget *paramWidget() override;

    /* static QVector<QgsDataProvider*> mProviders; */
    //! @note not available via python bindings
    //! @note deprecated since 2.10 - use QgsDataItemProviderRegistry
    Q_DECL_DEPRECATED static QVector<QLibrary*> mLibraries;

    /** Check if the given path is hidden from the browser model */
    static bool hiddenPath( QString path );

  public slots:
    virtual void childrenCreated() override;
    void directoryChanged();

  protected:
    void init();
    QString mDirPath;

  private:
    QFileSystemWatcher * mFileSystemWatcher;
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
     * @brief A data item holding a reference to a QGIS project file.
     * @param parent The parent data item.
     * @param name The name of the of the project. Displayed to the user.
     * @param path The full path to the project.
     */
    QgsProjectItem( QgsDataItem* parent, const QString& name, const QString& path );
    ~QgsProjectItem();

};

/** \ingroup core
 Data item that can be used to report problems (e.g. network error)
 */
class CORE_EXPORT QgsErrorItem : public QgsDataItem
{
    Q_OBJECT
  public:

    QgsErrorItem( QgsDataItem* parent, const QString& error, const QString& path );
    ~QgsErrorItem();

};


// ---------
/** \ingroup core
 * \class QgsDirectoryParamWidget
 */
class CORE_EXPORT QgsDirectoryParamWidget : public QTreeWidget
{
    Q_OBJECT

  public:
    QgsDirectoryParamWidget( const QString& path, QWidget* parent = nullptr );

  protected:
    void mousePressEvent( QMouseEvent* event ) override;

  public slots:
    void showHideColumn();
};

/** \ingroup core
 * Contains various Favourites directories
*/
class CORE_EXPORT QgsFavouritesItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsFavouritesItem( QgsDataItem* parent, const QString& name, const QString& path = QString() );
    ~QgsFavouritesItem();

    QVector<QgsDataItem*> createChildren() override;

    void addDirectory( const QString& favIcon );
    void removeDirectory( QgsDirectoryItem *item );

    static const QIcon &iconFavourites();

  private:
    QVector<QgsDataItem*> createChildren( const QString& favDir );
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
    QgsZipItem( QgsDataItem* parent, const QString& name, const QString& path );
    QgsZipItem( QgsDataItem* parent, const QString& name, const QString& filePath, const QString& path );
    ~QgsZipItem();

    QVector<QgsDataItem*> createChildren() override;
    const QStringList & getZipFileList();

    //! @note not available via python bindings
    static QVector<dataItem_t *> mDataItemPtr;
    static QStringList mProviderNames;

    static QString vsiPrefix( const QString& uri ) { return qgsVsiPrefix( uri ); }

    static QgsDataItem* itemFromPath( QgsDataItem* parent, QString path, QString name );
    //! @note available in python as itemFromFilePath
    static QgsDataItem* itemFromPath( QgsDataItem* parent, const QString& filePath, const QString& name, const QString& path );

    static const QIcon &iconZip();

  private:
    void init();
};

#endif // QGSDATAITEM_H


