/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdataitem.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsDataItem : QObject
{
%Docstring
 Base class for all items in the model.
 Parent/children hierarchy is not based on QObject.
%End

%TypeHeaderCode
#include "qgsdataitem.h"
%End

%ConvertToSubClassCode
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
%End
  public:
    enum Type
    {
      Collection,
      Directory,
      Layer,
      Error,
      Favorites,
      Project
    };


    QgsDataItem( QgsDataItem::Type type, QgsDataItem *parent /TransferThis/, const QString &name, const QString &path );
%Docstring
Create new data item.
%End
    virtual ~QgsDataItem();

    bool hasChildren();
%Docstring
 :rtype: bool
%End

    int rowCount();
%Docstring
 :rtype: int
%End

    virtual QVector<QgsDataItem *> createChildren() /Factory/;
%Docstring
 Create children. Children are not expected to have parent set.
 This method MUST BE THREAD SAFE. *
 :rtype: list of QgsDataItem
%End

    enum State
    {
      NotPopulated,
      Populating,
      Populated
    };

    State state() const;
%Docstring
.. versionadded:: 2.8
 :rtype: State
%End

    virtual void setState( State state );
%Docstring
 Set item state. It also take care about starting/stopping loading icon animation.
 \param state
.. versionadded:: 2.8
%End

    virtual void addChildItem( QgsDataItem *child /Transfer/, bool refresh = false );
%Docstring
 Inserts a new child item. The child will be inserted at a position using an alphabetical order based on mName.
 \param child child item to insert. Ownership is transferred, and item parent will be set and relevant connections made.
 \param refresh - set to true to refresh populated item, emitting relevant signals to the model
.. seealso:: deleteChildItem()
%End

    virtual void deleteChildItem( QgsDataItem *child );
%Docstring
 Removes and deletes a child item, emitting relevant signals to the model.
 \param child child to remove. Item must exist as a current child.
.. seealso:: addChildItem()
%End

    virtual QgsDataItem *removeChildItem( QgsDataItem *child ) /TransferBack/;
%Docstring
 Removes a child item and returns it without deleting it. Emits relevant signals to model as required.
 \param child child to remove
 :return: pointer to the removed item or null if no such item was found
 :rtype: QgsDataItem
%End

    virtual bool equal( const QgsDataItem *other );
%Docstring
 Returns true if this item is equal to another item (by testing item type and path).
 :rtype: bool
%End

    virtual QWidget *paramWidget() /Factory/;
%Docstring
 :rtype: QWidget
%End

    virtual QList<QAction *> actions();
%Docstring
 Returns the list of actions available for this item. This is usually used for the popup menu on right-clicking
 the item. Subclasses should override this to provide actions.
 :rtype: list of QAction
%End

    virtual bool acceptDrop();
%Docstring
 Returns whether the item accepts drag and dropped layers - e.g. for importing a dataset to a provider.
 Subclasses should override this and handleDrop() to accept dropped layers.
.. seealso:: handleDrop()
 :rtype: bool
%End

    virtual bool handleDrop( const QMimeData * /*data*/, Qt::DropAction /*action*/ );
%Docstring
 Attempts to process the mime data dropped on this item. Subclasses must override this and acceptDrop() if they
 accept dropped layers.
.. seealso:: acceptDrop()
 :rtype: bool
%End

    virtual bool hasDragEnabled() const;
%Docstring
 Returns true if the item may be dragged.
 Default implementation returns false.
 A draggable item has to implement mimeUri() that will be used to pass data.
.. seealso:: mimeUri()
.. versionadded:: 3.0
 :rtype: bool
%End

    virtual QgsMimeDataUtils::Uri mimeUri() const;
%Docstring
 Return mime URI for the data item.
 Items that return valid URI will be returned in mime data when dragging a selection from browser model.
.. seealso:: hasDragEnabled()
.. versionadded:: 3.0
 :rtype: QgsMimeDataUtils.Uri
%End

    enum Capability
    {
      NoCapabilities,
      SetCrs,
      Fertile,
      Fast,
      Collapse
    };
    typedef QFlags<QgsDataItem::Capability> Capabilities;


    virtual bool setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
 Writes the selected crs into data source. The original data source will be modified when calling this
 method.
 :rtype: bool
%End

    virtual Capabilities capabilities2() const;
%Docstring
 :rtype: Capabilities
%End

    virtual void setCapabilities( Capabilities capabilities );
%Docstring
 Sets the capabilities for the data item.
%End


    static int findItem( QVector<QgsDataItem *> items, QgsDataItem *item );
%Docstring
 :rtype: int
%End


    Type type() const;
%Docstring
 :rtype: Type
%End

    QgsDataItem *parent() const;
%Docstring
 Get item parent. QgsDataItem maintains its own items hierarchy, it does not use
  QObject hierarchy. *
 :rtype: QgsDataItem
%End

    void setParent( QgsDataItem *parent );
%Docstring
 Set item parent and connect / disconnect parent to / from item signals.
  It does not add itself to parents children (mChildren) *
%End
    QVector<QgsDataItem *> children() const;
%Docstring
 :rtype: list of QgsDataItem
%End
    virtual QIcon icon();
%Docstring
 :rtype: QIcon
%End
    QString name() const;
%Docstring
 :rtype: str
%End
    void setName( const QString &name );
    QString path() const;
%Docstring
 :rtype: str
%End
    void setPath( const QString &path );
    static QString pathComponent( const QString &component );
%Docstring
Create path component replacing path separators
 :rtype: str
%End

    void setIcon( const QIcon &icon );
    void setIconName( const QString &iconName );

    void setToolTip( const QString &msg );
    QString toolTip() const;
%Docstring
 :rtype: str
%End

    static void deleteLater( QVector<QgsDataItem *> &items );

    void moveToThread( QThread *targetThread );
%Docstring
Move object and all its descendants to thread
%End

  protected:
    virtual void populate( const QVector<QgsDataItem *> &children );

    virtual void refresh( const QVector<QgsDataItem *> &children );
%Docstring
 Refresh the items from a specified list of child items.
%End

    bool deferredDelete();
%Docstring
 The item is scheduled to be deleted. E.g. if deleteLater() is called when
 item is in Populating state (createChildren() running in another thread),
 the deferredDelete() returns true and item will be deleted once Populating finished.
 Items with slow reateChildren() (for example network or database based) may
 check during createChildren() if deferredDelete() returns true and return from
 createChildren() immediately because result will be useless. *
 :rtype: bool
%End


  public slots:

    virtual void deleteLater();
%Docstring
 Safely delete the item:
   - disconnects parent
   - unsets parent (but does not remove itself)
   - deletes all its descendants recursively
   - waits until Populating state (createChildren() in thread) finished without blocking main thread
   - calls QObject.deleteLater()
%End

    virtual void populate( bool foreground = false );

    virtual void depopulate();
%Docstring
Remove children recursively and set as not populated. This is used when refreshing collapsed items.
%End

    virtual void refresh();

    virtual void refreshConnections();
%Docstring
Refresh connections: update GUI and emit signal
%End

    virtual void childrenCreated();

  signals:
    void beginInsertItems( QgsDataItem *parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsDataItem *parent, int first, int last );
    void endRemoveItems();
    void dataChanged( QgsDataItem *item );
    void stateChanged( QgsDataItem *item, QgsDataItem::State oldState );
    void connectionsChanged();
%Docstring
open browsers
%End

  protected slots:

    void updateIcon();
%Docstring
 Will request a repaint of this icon.

.. versionadded:: 3.0
%End

};

QFlags<QgsDataItem::Capability> operator|(QgsDataItem::Capability f1, QFlags<QgsDataItem::Capability> f2);


class QgsLayerItem : QgsDataItem
{
%Docstring
 Item that represents a layer that can be opened with one of the providers
%End

%TypeHeaderCode
#include "qgsdataitem.h"
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
      Plugin
    };


    QgsLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, LayerType layerType, const QString &providerKey );


    virtual bool equal( const QgsDataItem *other );

    virtual bool hasDragEnabled() const;

    virtual QgsMimeDataUtils::Uri mimeUri() const;


    QgsMapLayer::LayerType mapLayerType() const;
%Docstring
Returns QgsMapLayer.LayerType
 :rtype: QgsMapLayer.LayerType
%End

    QString uri() const;
%Docstring
Returns layer uri or empty string if layer cannot be created
 :rtype: str
%End

    QString providerKey() const;
%Docstring
Returns provider key
 :rtype: str
%End

    QStringList supportedCrs() const;
%Docstring
 Returns the supported CRS
.. versionadded:: 2.8
 :rtype: list of str
%End

    QStringList supportedFormats() const;
%Docstring
 Returns the supported formats
.. versionadded:: 2.8
 :rtype: list of str
%End

    virtual QString comments() const;
%Docstring
 Returns comments of the layer
.. versionadded:: 2.12
 :rtype: str
%End

    static QString layerTypeAsString( const LayerType &layerType );
%Docstring
 Returns the string representatio of the given ``layerType``
.. versionadded:: 3
 :rtype: str
%End

  protected:


  public:
    static QIcon iconPoint();
%Docstring
 :rtype: QIcon
%End
    static QIcon iconLine();
%Docstring
 :rtype: QIcon
%End
    static QIcon iconPolygon();
%Docstring
 :rtype: QIcon
%End
    static QIcon iconTable();
%Docstring
 :rtype: QIcon
%End
    static QIcon iconRaster();
%Docstring
 :rtype: QIcon
%End
    static QIcon iconDefault();
%Docstring
 :rtype: QIcon
%End

    virtual QString layerName() const;
%Docstring
:return: the layer name
 :rtype: str
%End
};


class QgsDataCollectionItem : QgsDataItem
{
%Docstring
 A Collection: logical collection of layers or subcollections, e.g. GRASS location/mapset, database? wms source?
%End

%TypeHeaderCode
#include "qgsdataitem.h"
%End
  public:
    QgsDataCollectionItem( QgsDataItem *parent, const QString &name, const QString &path = QString() );
    ~QgsDataCollectionItem();

    void addChild( QgsDataItem *item /Transfer/ );

    static QIcon iconDir(); // shared icon: open/closed directory
    static QIcon iconDataCollection(); // default icon for data collection
};

class QgsDirectoryItem : QgsDataCollectionItem
{
%Docstring
 A directory: contains subdirectories and layers
%End

%TypeHeaderCode
#include "qgsdataitem.h"
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

    QgsDirectoryItem( QgsDataItem *parent, const QString &name, const QString &path );

    QgsDirectoryItem( QgsDataItem *parent, const QString &name, const QString &dirPath, const QString &path );
%Docstring
 Constructor.
 \param parent
 \param name directory name
 \param dirPath path to directory in file system
 \param path item path in the tree, it may be dirPath or dirPath with some prefix, e.g. favorites: *
%End

    virtual void setState( State state );

    virtual QVector<QgsDataItem *> createChildren();


    QString dirPath() const;
%Docstring
 :rtype: str
%End
    virtual bool equal( const QgsDataItem *other );
    virtual QIcon icon();
    virtual QWidget *paramWidget() /Factory/;

    static bool hiddenPath( const QString &path );
%Docstring
Check if the given path is hidden from the browser model
 :rtype: bool
%End


  public slots:
    virtual void childrenCreated();
    void directoryChanged();

  protected:
    void init();

};

class QgsProjectItem : QgsDataItem
{
%Docstring
Data item that can be used to represent QGIS projects.
%End

%TypeHeaderCode
#include "qgsdataitem.h"
%End
  public:

    QgsProjectItem( QgsDataItem *parent, const QString &name, const QString &path );
%Docstring
 A data item holding a reference to a QGIS project file.
 \param parent The parent data item.
 \param name The name of the of the project. Displayed to the user.
 \param path The full path to the project.
%End

    virtual bool hasDragEnabled() const;

};

class QgsErrorItem : QgsDataItem
{
%Docstring
Data item that can be used to report problems (e.g. network error)
%End

%TypeHeaderCode
#include "qgsdataitem.h"
%End
  public:

    QgsErrorItem( QgsDataItem *parent, const QString &error, const QString &path );

};



class QgsDirectoryParamWidget : QTreeWidget
{

%TypeHeaderCode
#include "qgsdataitem.h"
%End
  public:
    QgsDirectoryParamWidget( const QString &path, QWidget *parent /TransferThis/ = 0 );

  protected:
    virtual void mousePressEvent( QMouseEvent *event );


  public slots:
    void showHideColumn();
};

class QgsFavoritesItem : QgsDataCollectionItem
{
%Docstring
 Contains various Favorites directories
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsdataitem.h"
%End
  public:

    QgsFavoritesItem( QgsDataItem *parent, const QString &name, const QString &path = QString() );
%Docstring
 Constructor for QgsFavoritesItem. Accepts a path argument specifying the file path associated with
 the item.
%End

    virtual QVector<QgsDataItem *> createChildren();


    void addDirectory( const QString &directory );
%Docstring
 Adds a new directory to the favorites group.
.. seealso:: removeDirectory()
%End

    void removeDirectory( QgsDirectoryItem *item );
%Docstring
 Removes an existing directory from the favorites group.
.. seealso:: addDirectory()
%End

    static QIcon iconFavorites();
%Docstring
Icon for favorites group
 :rtype: QIcon
%End

};

class QgsZipItem : QgsDataCollectionItem
{
%Docstring
 A zip file: contains layers, using GDAL/OGR VSIFILE mechanism
%End

%TypeHeaderCode
#include "qgsdataitem.h"
%End
  protected:

  public:
    QgsZipItem( QgsDataItem *parent, const QString &name, const QString &path );
    QgsZipItem( QgsDataItem *parent, const QString &name, const QString &filePath, const QString &path );

    virtual QVector<QgsDataItem *> createChildren();

    QStringList getZipFileList();
%Docstring
 :rtype: list of str
%End

    static QStringList sProviderNames;

    static QString vsiPrefix( const QString &uri );
%Docstring
 :rtype: str
%End

    static QgsDataItem *itemFromPath( QgsDataItem *parent, const QString &path, const QString &name ) /Factory/;
%Docstring
 Creates a new data item from the specified path.
 :rtype: QgsDataItem
%End

    static QgsDataItem *itemFromPath( QgsDataItem *parent, const QString &filePath, const QString &name, const QString &path ) /Factory,PyName=itemFromFilePath/;
%Docstring
 Creates a new data item from the specified path.
.. note::

   available in Python as itemFromFilePath
 :rtype: QgsDataItem
%End

    static QIcon iconZip();
%Docstring
 :rtype: QIcon
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsdataitem.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
