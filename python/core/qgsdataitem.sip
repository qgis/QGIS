class QgsDataItem : QObject
{
%TypeHeaderCode
#include <qgsdataitem.h>
%End
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
    QgsDataItem( QgsDataItem::Type type, QgsDataItem* parent /TransferThis/, QString name, QString path );
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
    bool isPopulated();

    // Insert new child using alphabetical order based on mName, emits necessary signal to model before and after, sets parent and connects signals
    // refresh - refresh populated item, emit signals to model
    virtual void addChildItem( QgsDataItem *child /Transfer/, bool refresh = false );

    // remove and delete child item, signals to browser are emitted
    virtual void deleteChildItem( QgsDataItem * child );

    // remove child item but don't delete it, signals to browser are emitted
    // returns pointer to the removed item or null if no such item was found
    virtual QgsDataItem *removeChildItem( QgsDataItem * child ) /TransferBack/;

    virtual bool equal( const QgsDataItem *other );

    virtual QWidget *paramWidget() /Factory/;

    // list of actions provided by this item - usually used for popup menu on right-click
    virtual QList<QAction*> actions();

    // whether accepts drag&drop'd layers - e.g. for import
    virtual bool acceptDrop();

    // try to process the data dropped on this item
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

    virtual void setCapabilities( Capabilities capabilities );

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
    void setIcon( QIcon icon );
    void setIconName( const QString & iconName );

    void setToolTip( QString msg );
    QString toolTip() const;

    // deleteLater() items anc clear the vector
    static void deleteLater( QVector<QgsDataItem*> &items );

    /** Move object and all its descendants to thread */
    void moveToThread( QThread * targetThread );


  public slots:

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

    QgsLayerItem( QgsDataItem* parent, QString name, QString path, QString uri, LayerType layerType, QString providerKey );

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
    QgsDataCollectionItem( QgsDataItem* parent, QString name, QString path = QString::null );
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

    QgsDirectoryItem( QgsDataItem* parent, QString name, QString path );
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

    QgsErrorItem( QgsDataItem* parent, QString error, QString path );
    ~QgsErrorItem();

};


// ---------

class QgsDirectoryParamWidget : QTreeWidget
{
%TypeHeaderCode
#include <qgsdataitem.h>
%End
  public:
    QgsDirectoryParamWidget( QString path, QWidget* parent /TransferThis/ = NULL );

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
    QgsFavouritesItem( QgsDataItem* parent, QString name, QString path = QString() );
    ~QgsFavouritesItem();

    QVector<QgsDataItem*> createChildren();

    void addDirectory( QString favIcon );
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
    QgsZipItem( QgsDataItem* parent, QString name, QString path );
    ~QgsZipItem();

    QVector<QgsDataItem*> createChildren();
    const QStringList & getZipFileList();

    //! @note not available via python bindings
    // static QVector<dataItem_t *> mDataItemPtr;
    static QStringList mProviderNames;

    static QString vsiPrefix( QString uri );

    static QgsDataItem* itemFromPath( QgsDataItem* parent, QString path, QString name ) /Factory/;
    static QgsDataItem* itemFromPath( QgsDataItem* parent, QString filePath, QString name, QString path ) /Factory,PyName=itemFromFilePath/;

    static const QIcon &iconZip();

};
