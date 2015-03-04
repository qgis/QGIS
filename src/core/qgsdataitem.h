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


/** Base class for all items in the model.
 *  Parent/children hierarchy is not based on QObject. */
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
      Favourites
    };

    /** Create new data item. */
    QgsDataItem( QgsDataItem::Type type, QgsDataItem* parent, QString name, QString path );
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

    State state() const;

    /** Set item state. It also take care about starting/stopping loading icon animation.
     * @param state */
    virtual void setState( State state );

    //! @deprecated in 2.8, use state()
    bool isPopulated() { return state() == Populated; }

    // Insert new child using alphabetical order based on mName, emits necessary signal to model before and after, sets parent and connects signals
    // refresh - refresh populated item, emit signals to model
    virtual void addChildItem( QgsDataItem *child, bool refresh = false );

    // remove and delete child item, signals to browser are emitted
    virtual void deleteChildItem( QgsDataItem * child );

    // remove child item but don't delete it, signals to browser are emitted
    // returns pointer to the removed item or null if no such item was found
    virtual QgsDataItem *removeChildItem( QgsDataItem * child );

    virtual bool equal( const QgsDataItem *other );

    virtual QWidget *paramWidget() { return 0; }

    // list of actions provided by this item - usually used for popup menu on right-click
    virtual QList<QAction*> actions() { return QList<QAction*>(); }

    // whether accepts drag&drop'd layers - e.g. for import
    virtual bool acceptDrop() { return false; }

    // try to process the data dropped on this item
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

    virtual void setCapabilities( Capabilities capabilities ) { mCapabilities = capabilities; }

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
    void setIcon( QIcon icon ) { mIcon = icon; }
    void setIconName( const QString & iconName ) { mIconName = iconName; }

    void setToolTip( QString msg ) { mToolTip = msg; }
    QString toolTip() const { return mToolTip; }

    // deleteLater() items anc clear the vector
    static void deleteLater( QVector<QgsDataItem*> &items );

    /** Move object and all its descendants to thread */
    void moveToThread( QThread * targetThread );

  protected:
    virtual void populate( QVector<QgsDataItem*> children );
    virtual void refresh( QVector<QgsDataItem*> children );
    QIcon populatingIcon() { return mPopulatingIcon; }
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
    static QMap<QString, QIcon> mIconMap;

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
    virtual void populate();

    /** Remove children recursively and set as not populated. This is used when refreshing collapsed items. */
    virtual void depopulate();

    virtual void refresh();

    void emitBeginInsertItems( QgsDataItem* parent, int first, int last );
    void emitEndInsertItems();
    void emitBeginRemoveItems( QgsDataItem* parent, int first, int last );
    void emitEndRemoveItems();
    void emitDataChanged( QgsDataItem* item );
    void emitDataChanged( );
    void emitStateChanged( QgsDataItem* item, QgsDataItem::State oldState );
    virtual void childrenCreated();
    void setPopulatingIcon();

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
    static int mPopulatingCount;
    static QMovie * mPopulatingMovie;
    static QIcon mPopulatingIcon;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsDataItem::Capabilities )

/** Item that represents a layer that can be opened with one of the providers */
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
      Table
    };

    QgsLayerItem( QgsDataItem* parent, QString name, QString path, QString uri, LayerType layerType, QString providerKey );

    // --- reimplemented from QgsDataItem ---

    virtual bool equal( const QgsDataItem *other ) override;

    // --- New virtual methods for layer item derived classes ---

    // Returns QgsMapLayer::LayerType
    QgsMapLayer::LayerType mapLayerType();

    // Returns layer uri or empty string if layer cannot be created
    QString uri() { return mUri; }

    // Returns provider key
    QString providerKey() { return mProviderKey; }

    /** Returns the supported CRS
     *  @note Added in 2.8
     */
    QStringList supportedCRS() { return mSupportedCRS; }

    /** Returns the supported formats
     *  @note Added in 2.8
     */
    QStringList supportedFormats() { return mSupportFormats; }

  protected:

    QString mProviderKey;
    QString mUri;
    LayerType mLayerType;
    QStringList mSupportedCRS;
    QStringList mSupportFormats;

  public:
    static const QIcon &iconPoint();
    static const QIcon &iconLine();
    static const QIcon &iconPolygon();
    static const QIcon &iconTable();
    static const QIcon &iconRaster();
    static const QIcon &iconDefault();

    virtual QString layerName() const { return name(); }
};


/** A Collection: logical collection of layers or subcollections, e.g. GRASS location/mapset, database? wms source? */
class CORE_EXPORT QgsDataCollectionItem : public QgsDataItem
{
    Q_OBJECT
  public:
    QgsDataCollectionItem( QgsDataItem* parent, QString name, QString path = QString::null );
    ~QgsDataCollectionItem();

    void addChild( QgsDataItem *item ) { mChildren.append( item ); }

    static const QIcon &iconDir(); // shared icon: open/closed directory
    static const QIcon &iconDataCollection(); // default icon for data collection
};

/** A directory: contains subdirectories and layers */
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

    QgsDirectoryItem( QgsDataItem* parent, QString name, QString path );

    /** Constructor.
     * @param parent
     * @param name directory name
     * @param dirPath path to directory in file system
     * @param path item path in the tree, it may be dirPath or dirPath with some prefix, e.g. favourites: */
    QgsDirectoryItem( QgsDataItem* parent, QString name, QString dirPath, QString path );
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

/**
 Data item that can be used to report problems (e.g. network error)
 */
class CORE_EXPORT QgsErrorItem : public QgsDataItem
{
    Q_OBJECT
  public:

    QgsErrorItem( QgsDataItem* parent, QString error, QString path );
    ~QgsErrorItem();

};


// ---------

class CORE_EXPORT QgsDirectoryParamWidget : public QTreeWidget
{
    Q_OBJECT

  public:
    QgsDirectoryParamWidget( QString path, QWidget* parent = NULL );

  protected:
    void mousePressEvent( QMouseEvent* event ) override;

  public slots:
    void showHideColumn();
};

/** Contains various Favourites directories */
class CORE_EXPORT QgsFavouritesItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsFavouritesItem( QgsDataItem* parent, QString name, QString path = QString() );
    ~QgsFavouritesItem();

    QVector<QgsDataItem*> createChildren() override;

    void addDirectory( QString favIcon );
    void removeDirectory( QgsDirectoryItem *item );

    static const QIcon &iconFavourites();
};

/** A zip file: contains layers, using GDAL/OGR VSIFILE mechanism */
class CORE_EXPORT QgsZipItem : public QgsDataCollectionItem
{
    Q_OBJECT

  protected:
    QString mFilePath;
    QString mVsiPrefix;
    QStringList mZipFileList;

  public:
    QgsZipItem( QgsDataItem* parent, QString name, QString path );
    QgsZipItem( QgsDataItem* parent, QString name, QString filePath, QString path );
    ~QgsZipItem();

    QVector<QgsDataItem*> createChildren() override;
    const QStringList & getZipFileList();

    //! @note not available via python bindings
    static QVector<dataItem_t *> mDataItemPtr;
    static QStringList mProviderNames;

    static QString vsiPrefix( QString uri ) { return qgsVsiPrefix( uri ); }

    static QgsDataItem* itemFromPath( QgsDataItem* parent, QString path, QString name );
    static QgsDataItem* itemFromPath( QgsDataItem* parent, QString filePath, QString name, QString path );

    static const QIcon &iconZip();

  private:
    void init();
};

#endif // QGSDATAITEM_H


