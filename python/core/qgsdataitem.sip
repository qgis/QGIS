/** Animated icon is keeping an animation running if there are listeners connected to frameChanged */
class QgsAnimatedIcon : QObject
{
%TypeHeaderCode
#include <qgsdataitem.h>
%End
  public:

    /** Constructor
     * @param iconPath path to a movie, e.g. animated GIF */
    QgsAnimatedIcon( const QString & iconPath = QString::null );

    QString iconPath() const;
    void setIconPath( const QString & iconPath );
    QIcon icon() const;

    /** Connect listener to frameChanged() signal */
    void connectFrameChanged( const QObject * receiver, const char * method );
    /** Disconnect listener from frameChanged() signal */
    void disconnectFrameChanged( const QObject * receiver, const char * method );

  public slots:
    void onFrameChanged();

  signals:
    /** Emitted when icon changed */
    void frameChanged();

};

/** Base class for all items in the model.
 *  Parent/children hierarchy is not based on QObject. */
class QgsDataItem : QObject
{
%TypeHeaderCode
#include <qgsdataitem.h>
%End

%ConvertToSubClassCode
  if (qobject_cast<QgsLayerItem*>(sipCpp))
    sipType = sipType_QgsLayerItem;
  else if (qobject_cast<QgsErrorItem*>(sipCpp))
    sipType = sipType_QgsErrorItem;
  else if (qobject_cast<QgsDirectoryItem*>(sipCpp))
    sipType = sipType_QgsDirectoryItem;
  else if (qobject_cast<QgsFavouritesItem*>(sipCpp))
    sipType = sipType_QgsFavouritesItem;
  else if (qobject_cast<QgsZipItem*>(sipCpp))
    sipType = sipType_QgsZipItem;
  else if (qobject_cast<QgsDataCollectionItem*>(sipCpp))
    sipType = sipType_QgsDataCollectionItem;
  else if (qobject_cast<QgsProjectItem*>(sipCpp))
    sipType = sipType_QgsProjectItem;
  else
    sipType = 0;
%End
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
    QgsDataItem( QgsDataItem::Type type, QgsDataItem* parent /TransferThis/, const QString& name, const QString& path );
    virtual ~QgsDataItem();

    bool hasChildren();

    int rowCount();

    /** Create children. Children are not expected to have parent set.
     * This method MUST BE THREAD SAFE. */
    virtual QVector<QgsDataItem*> createChildren() /Factory/;

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
    bool isPopulated() /Deprecated/;

    /** Inserts a new child item. The child will be inserted at a position using an alphabetical order based on mName.
     * @param child child item to insert. Ownership is transferred, and item parent will be set and relevant connections made.
     * @param refresh - set to true to refresh populated item, emitting relevant signals to the model
     * @see deleteChildItem()
     */
    virtual void addChildItem( QgsDataItem *child /Transfer/, bool refresh = false );

    /** Removes and deletes a child item, emitting relevant signals to the model.
     * @param child child to remove. Item must exist as a current child.
     * @see addChildItem()
     */
    virtual void deleteChildItem( QgsDataItem * child );

    /** Removes a child item and returns it without deleting it. Emits relevant signals to model as required.
     * @param child child to remove
     * @returns pointer to the removed item or null if no such item was found
     */
    virtual QgsDataItem *removeChildItem( QgsDataItem * child ) /TransferBack/;

    /** Returns true if this item is equal to another item (by testing item type and path).
     */
    virtual bool equal( const QgsDataItem *other );

    virtual QWidget *paramWidget() /Factory/;

    /** Returns the list of actions available for this item. This is usually used for the popup menu on right-clicking
     * the item. Subclasses should override this to provide actions.
     */
    virtual QList<QAction*> actions();

    /** Returns whether the item accepts drag and dropped layers - e.g. for importing a dataset to a provider.
     * Subclasses should override this and handleDrop() to accept dropped layers.
     * @see handleDrop()
     */
    virtual bool acceptDrop();

    /** Attempts to process the mime data dropped on this item. Subclasses must override this and acceptDrop() if they
     * accept dropped layers.
     * @see acceptDrop()
     */
    virtual bool handleDrop( const QMimeData * /*data*/, Qt::DropAction /*action*/ );

    enum Capability
    {
      NoCapabilities,
      SetCrs,  //!< Can set CRS on layer or group of layers
      Fertile, //!< Can create children. Even items without this capability may have children, but cannot create them, it means that children are created by item ancestors.
      Fast     //!< createChildren() is fast enough to be run in main thread when refreshing items, most root items (wms,wfs,wcs,postgres...) are considered fast because they are reading data only from QSettings
    };
    typedef QFlags<QgsDataItem::Capability> Capabilities;

    // This will _write_ selected crs in data source
    virtual bool setCrs( QgsCoordinateReferenceSystem crs );

    //! @deprecated since 2.8, returned type this will changed to Capabilities
    virtual Capability capabilities() /Deprecated/;

    virtual Capabilities capabilities2() const;

    virtual void setCapabilities( const Capabilities& capabilities );

    // static methods

    // Find child index in vector of items using '==' operator
    static int findItem( QVector<QgsDataItem*> items, QgsDataItem * item );

    // members

    Type type() const;

    /** Get item parent. QgsDataItem maintains its own items hierarchy, it does not use
     *  QObject hierarchy. */
    QgsDataItem* parent() const;
    /** Set item parent and connect / disconnect parent to / from item signals.
     *  It does not add itself to parents children (mChildren) */
    void setParent( QgsDataItem* parent );
    QVector<QgsDataItem*> children() const;
    QIcon icon();
    QString name() const;
    void setName( const QString &name );
    QString path() const;
    void setPath( const QString );
    //! Create path component replacing path separators
    static QString pathComponent( const QString &component );

    // Because QIcon (QPixmap) must not be used in outside the GUI thread, it is
    // not possible to set mIcon in constructor. Either use mIconName/setIconName()
    // or implement icon().
    void setIcon( const QIcon& icon );
    void setIconName( const QString & iconName );

    void setToolTip( const QString& msg );
    QString toolTip() const;

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
    bool deferredDelete();

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
};

QFlags<QgsDataItem::Capability> operator|(QgsDataItem::Capability f1, QFlags<QgsDataItem::Capability> f2);

/** Item that represents a layer that can be opened with one of the providers */
class QgsLayerItem : QgsDataItem
{
%TypeHeaderCode
#include <qgsdataitem.h>
%End
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

    virtual bool equal( const QgsDataItem *other );

    // --- New virtual methods for layer item derived classes ---

    // Returns QgsMapLayer::LayerType
    QgsMapLayer::LayerType mapLayerType();

    // Returns layer uri or empty string if layer cannot be created
    QString uri();

    // Returns provider key
    QString providerKey();

    /** Returns the supported CRS
     *  @note Added in 2.8
     */
    QStringList supportedCRS();

    /** Returns the supported formats
     *  @note Added in 2.8
     */
    QStringList supportedFormats();

    /** Returns comments of the layer
     * @note added in 2.12
     */
    virtual QString comments() const;

  public:
    static const QIcon &iconPoint();
    static const QIcon &iconLine();
    static const QIcon &iconPolygon();
    static const QIcon &iconTable();
    static const QIcon &iconRaster();
    static const QIcon &iconDefault();

    virtual QString layerName() const;
};


/** A Collection: logical collection of layers or subcollections, e.g. GRASS location/mapset, database? wms source? */
class QgsDataCollectionItem : QgsDataItem
{
%TypeHeaderCode
#include <qgsdataitem.h>
%End
  public:
    QgsDataCollectionItem( QgsDataItem* parent, const QString& name, const QString& path = QString::null );
    ~QgsDataCollectionItem();

    void addChild( QgsDataItem *item /Transfer/ );

    static const QIcon &iconDir(); // shared icon: open/closed directory
    static const QIcon &iconDataCollection(); // default icon for data collection
};

/** A directory: contains subdirectories and layers */
class QgsDirectoryItem : QgsDataCollectionItem
{
%TypeHeaderCode
#include <qgsdataitem.h>
%End
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

    virtual void setState( State state );

    QVector<QgsDataItem*> createChildren();

    QString dirPath() const;
    virtual bool equal( const QgsDataItem *other );
    virtual QIcon icon();
    virtual QWidget *paramWidget() /Factory/;

    /* static QVector<QgsDataProvider*> mProviders; */
    //! @note not available via python bindings
    // static QVector<QLibrary*> mLibraries;

    /** Check if the given path is hidden from the browser model */
    static bool hiddenPath( QString path );

  public slots:
    virtual void childrenCreated();
    void directoryChanged();

  protected:
    void init();

};

/**
 Data item that can be used to represent QGIS projects.
 */
class QgsProjectItem : QgsDataItem
{
%TypeHeaderCode
#include <qgsdataitem.h>
%End
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

/**
 Data item that can be used to report problems (e.g. network error)
 */
class QgsErrorItem : QgsDataItem
{
%TypeHeaderCode
#include <qgsdataitem.h>
%End
  public:

    QgsErrorItem( QgsDataItem* parent, const QString& error, const QString& path );
    ~QgsErrorItem();

};


// ---------

class QgsDirectoryParamWidget : QTreeWidget
{
%TypeHeaderCode
#include <qgsdataitem.h>
%End
  public:
    QgsDirectoryParamWidget( const QString& path, QWidget* parent /TransferThis/ = NULL );

  protected:
    void mousePressEvent( QMouseEvent* event );

  public slots:
    void showHideColumn();
};

/** Contains various Favourites directories */
class QgsFavouritesItem : QgsDataCollectionItem
{
%TypeHeaderCode
#include <qgsdataitem.h>
%End
  public:
    QgsFavouritesItem( QgsDataItem* parent, const QString& name, const QString& path = QString() );
    ~QgsFavouritesItem();

    QVector<QgsDataItem*> createChildren();

    void addDirectory( const QString& favIcon );
    void removeDirectory( QgsDirectoryItem *item );

    static const QIcon &iconFavourites();
};

/** A zip file: contains layers, using GDAL/OGR VSIFILE mechanism */
class QgsZipItem : QgsDataCollectionItem
{
%TypeHeaderCode
#include <qgsdataitem.h>
%End

  public:
    QgsZipItem( QgsDataItem* parent, const QString& name, const QString& path );
    ~QgsZipItem();

    QVector<QgsDataItem*> createChildren();
    const QStringList & getZipFileList();

    //! @note not available via python bindings
    // static QVector<dataItem_t *> mDataItemPtr;
    static QStringList mProviderNames;

    static QString vsiPrefix( const QString& uri );

    static QgsDataItem* itemFromPath( QgsDataItem* parent, QString path, QString name ) /Factory/;
    //! @note available in python as itemFromFilePath
    static QgsDataItem* itemFromPath( QgsDataItem* parent, const QString& filePath, const QString& name, const QString& path ) /Factory,PyName=itemFromFilePath/;

    static const QIcon &iconZip();

};
