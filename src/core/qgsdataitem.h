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

#include <QIcon>
#include <QLibrary>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QVector>
#include <QTreeWidget>

#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgscoordinatereferencesystem.h"

class QgsDataProvider;
class QgsDataItem;

typedef QgsDataItem * dataItem_t( QString, QgsDataItem* );


/** base class for all items in the model */
class CORE_EXPORT QgsDataItem : public QObject
{
    Q_OBJECT
  public:
    enum Type
    {
      Collection,
      Directory,
      Layer,
      Error,
      Favourites
    };

    QgsDataItem( QgsDataItem::Type type, QgsDataItem* parent, QString name, QString path );
    virtual ~QgsDataItem();

    bool hasChildren();

    int rowCount();

    virtual void refresh();
    virtual void refresh( QVector<QgsDataItem*> children );

    // Create vector of children
    virtual QVector<QgsDataItem*> createChildren();

    // Populate children using children vector created by createChildren()
    virtual void populate();
    virtual void populate( QVector<QgsDataItem*> children );

    /** Remove children recursively and set as not populated. This is used when refreshing collapsed items. */
    virtual void depopulate();

    bool isPopulated() { return mPopulated; }

    /** Set as populated without populating. */
    void setPopulated() { mPopulated = true; }

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
    QgsDataItem* parent() const { return mParent; }
    void setParent( QgsDataItem* parent ) { mParent = parent; }
    QVector<QgsDataItem*> children() const { return mChildren; }
    virtual QIcon icon();
    QString name() const { return mName; }
    QString path() const { return mPath; }
    void setPath( const QString &path ) { mPath = path; }

    // Because QIcon (QPixmap) must not be used in outside the GUI thread, it is
    // not possible to set mIcon in constructor. Either use mIconName/setIconName()
    // or implement icon().
    void setIcon( QIcon icon ) { mIcon = icon; }
    void setIconName( const QString & iconName ) { mIconName = iconName; }

    void setToolTip( QString msg ) { mToolTip = msg; }
    QString toolTip() const { return mToolTip; }

  protected:

    Type mType;
    Capabilities mCapabilities;
    QgsDataItem* mParent;
    QVector<QgsDataItem*> mChildren; // easier to have it always
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
    void emitBeginInsertItems( QgsDataItem* parent, int first, int last );
    void emitEndInsertItems();
    void emitBeginRemoveItems( QgsDataItem* parent, int first, int last );
    void emitEndRemoveItems();

  signals:
    void beginInsertItems( QgsDataItem* parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsDataItem* parent, int first, int last );
    void endRemoveItems();
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

    virtual bool equal( const QgsDataItem *other );

    // --- New virtual methods for layer item derived classes ---

    // Returns QgsMapLayer::LayerType
    QgsMapLayer::LayerType mapLayerType();

    // Returns layer uri or empty string if layer cannot be created
    QString uri() { return mUri; }

    // Returns provider key
    QString providerKey() { return mProviderKey; }

  protected:

    QString mProviderKey;
    QString mUri;
    LayerType mLayerType;

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

    QVector<QgsDataItem*> createChildren();

    QString dirPath() const { return mDirPath; }
    virtual bool equal( const QgsDataItem *other );
    virtual QIcon icon();
    virtual QWidget *paramWidget();

    /* static QVector<QgsDataProvider*> mProviders; */
    //! @note not available via python bindings
    static QVector<QLibrary*> mLibraries;

  protected:
    void init();
    QString mDirPath;
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
    void mousePressEvent( QMouseEvent* event );

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

    QVector<QgsDataItem*> createChildren();

    void addDirectory( QString favIcon );
    void removeDirectory( QgsDirectoryItem *item );

    static const QIcon &iconFavourites();
};

/** A zip file: contains layers, using GDAL/OGR VSIFILE mechanism */
class CORE_EXPORT QgsZipItem : public QgsDataCollectionItem
{
    Q_OBJECT

  protected:
    QString mDirPath;
    QString mVsiPrefix;
    QStringList mZipFileList;

  public:
    QgsZipItem( QgsDataItem* parent, QString name, QString path );
    QgsZipItem( QgsDataItem* parent, QString name, QString dirPath, QString path );
    ~QgsZipItem();

    QVector<QgsDataItem*> createChildren();
    const QStringList & getZipFileList();

    //! @note not available via python bindings
    static QVector<dataItem_t *> mDataItemPtr;
    static QStringList mProviderNames;

    static QString vsiPrefix( QString uri ) { return qgsVsiPrefix( uri ); }

    static QgsDataItem* itemFromPath( QgsDataItem* parent, QString path, QString name );
    static QgsDataItem* itemFromPath( QgsDataItem* parent, QString dirPath, QString name, QString path );

    static const QIcon &iconZip();

  private:
    void init();
};

#endif // QGSDATAITEM_H

