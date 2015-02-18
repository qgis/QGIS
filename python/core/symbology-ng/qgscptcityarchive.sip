class QgsCptCityArchive
{
%TypeHeaderCode
#include <qgscptcityarchive.h>
%End
  public:
    QgsCptCityArchive( QString archiveName = DEFAULT_CPTCITY_ARCHIVE,
                       QString baseDir = QString() );
    ~QgsCptCityArchive();

    // basic dir info
    QString baseDir() const;
    static QString baseDir( QString archiveName );
    static QString defaultBaseDir();
    void setBaseDir( QString dirName );

    // collection + selection info
    QString copyingFileName( const QString& dirName ) const;
    QString descFileName( const QString& dirName ) const;
    static QString findFileName( const QString & target, const QString & startDir, const QString & baseDir );
    static QMap< QString, QString > copyingInfo( const QString& fileName );
    static QMap< QString, QString > description( const QString& fileName );
    //! note not available in python bindings
    // static QMap< double, QPair<QColor, QColor> > gradientColorMap( const QString& fileName );

    // archive management
    bool isEmpty();
    QString archiveName() const;
    static void initArchives( bool loadAll = false );
    static void initArchive( QString archiveName, QString archiveBaseDir );
    static void initDefaultArchive();
    static void clearArchives();
    static QgsCptCityArchive* defaultArchive();
    static QMap< QString, QgsCptCityArchive* > archiveRegistry();

    // items
    QVector< QgsCptCityDataItem* > rootItems() const;
    QVector< QgsCptCityDataItem* > selectionItems() const;
};

/** base class for all items in the model */
class QgsCptCityDataItem : QObject
{
%TypeHeaderCode
#include <qgscptcityarchive.h>
%End

  public:
    enum Type
    {
      ColorRamp,
      Collection,
      Directory,
      Selection,
      AllRamps
    };

    QgsCptCityDataItem( QgsCptCityDataItem::Type type, QgsCptCityDataItem* parent,
                        QString name, QString path );
    virtual ~QgsCptCityDataItem();

    bool hasChildren();

    int rowCount();
    // retrieve total count of "leaf" items (all children which are end nodes)
    virtual int leafCount() const;

    //

    virtual void refresh();

    // Create vector of children
    virtual QVector<QgsCptCityDataItem*> createChildren();

    // Populate children using children vector created by createChildren()
    virtual void populate();
    bool isPopulated();

    // Insert new child using alphabetical order based on mName, emits necessary signal to model before and after, sets parent and connects signals
    // refresh - refresh populated item, emit signals to model
    virtual void addChildItem( QgsCptCityDataItem * child /Transfer/, bool refresh = false );

    // remove and delete child item, signals to browser are emitted
    virtual void deleteChildItem( QgsCptCityDataItem * child );

    // remove child item but don't delete it, signals to browser are emitted
    // returns pointer to the removed item or null if no such item was found
    virtual QgsCptCityDataItem * removeChildItem( QgsCptCityDataItem * child ) /TransferBack/;

    virtual bool equal( const QgsCptCityDataItem *other );

    virtual QWidget *paramWidget() /Factory/;

    // list of actions provided by this item - usually used for popup menu on right-click
    virtual QList<QAction*> actions();

    // whether accepts drag&drop'd layers - e.g. for import
    virtual bool acceptDrop();

    // try to process the data dropped on this item
    virtual bool handleDrop( const QMimeData * /*data*/, Qt::DropAction /*action*/ );

    // static methods

    // Find child index in vector of items using '==' operator
    static int findItem( QVector<QgsCptCityDataItem*> items, QgsCptCityDataItem * item );

    // members

    Type type() const;
    QgsCptCityDataItem* parent() const;
    void setParent( QgsCptCityDataItem* parent );
    QVector<QgsCptCityDataItem*> children() const;
    virtual QIcon icon();
    virtual QIcon icon( const QSize& size );
    QString name() const;
    QString path() const;
    QString info() const;
    QString shortInfo() const;

    void setIcon( QIcon icon );

    void setToolTip( QString msg );
    QString toolTip() const;

    bool isValid();

  public slots:
    void emitBeginInsertItems( QgsCptCityDataItem* parent, int first, int last );
    void emitEndInsertItems();
    void emitBeginRemoveItems( QgsCptCityDataItem* parent, int first, int last );
    void emitEndRemoveItems();

  signals:
    void beginInsertItems( QgsCptCityDataItem* parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsCptCityDataItem* parent, int first, int last );
    void endRemoveItems();
};

/** Item that represents a layer that can be opened with one of the providers */
class QgsCptCityColorRampItem : QgsCptCityDataItem
{
%TypeHeaderCode
#include <qgscptcityarchive.h>
%End

  public:
    QgsCptCityColorRampItem( QgsCptCityDataItem* parent,
                             QString name, QString path,
                             QString variantName = QString(),
                             bool initialize = false );
    QgsCptCityColorRampItem( QgsCptCityDataItem* parent,
                             QString name, QString path,
                             QStringList variantList,
                             bool initialize = false );
    ~QgsCptCityColorRampItem();

    // --- reimplemented from QgsCptCityDataItem ---

    virtual bool equal( const QgsCptCityDataItem *other );
    virtual int leafCount() const;

    // --- New virtual methods for layer item derived classes ---
    const QgsCptCityColorRampV2& ramp() const;
    QIcon icon();
    QIcon icon( const QSize& size );
    void init();
};


/** A Collection: logical collection of subcollections and color ramps */
class QgsCptCityCollectionItem : QgsCptCityDataItem
{
%TypeHeaderCode
#include <qgscptcityarchive.h>
%End

  public:
    QgsCptCityCollectionItem( QgsCptCityDataItem* parent,
                              QString name, QString path );
    ~QgsCptCityCollectionItem();

    void setPopulated();
    void addChild( QgsCptCityDataItem *item /Transfer/ );
    QVector<QgsCptCityDataItem*> childrenRamps( bool recursive );
};

/** A directory: contains subdirectories and color ramps */
class QgsCptCityDirectoryItem : QgsCptCityCollectionItem
{
%TypeHeaderCode
#include <qgscptcityarchive.h>
%End
  public:
    QgsCptCityDirectoryItem( QgsCptCityDataItem* parent,
                             QString name, QString path );
    ~QgsCptCityDirectoryItem();

    QVector<QgsCptCityDataItem*> createChildren();

    virtual bool equal( const QgsCptCityDataItem *other );

    static QgsCptCityDataItem* dataItem( QgsCptCityDataItem* parent,
                                         QString name, QString path );

  protected:
    QMap< QString, QStringList > rampsMap();
    QStringList dirEntries() const;
};

/** A selection: contains subdirectories and color ramps */
class QgsCptCitySelectionItem : QgsCptCityCollectionItem
{
%TypeHeaderCode
#include <qgscptcityarchive.h>
%End

  public:
    QgsCptCitySelectionItem( QgsCptCityDataItem* parent, QString name, QString path );
    ~QgsCptCitySelectionItem();

    QVector<QgsCptCityDataItem*> createChildren();

    virtual bool equal( const QgsCptCityDataItem *other );

    QStringList selectionsList() const;

  protected:
    void parseXML();
};

/** An "All ramps item", which contains all items in a flat hierarchy */
class QgsCptCityAllRampsItem : QgsCptCityCollectionItem
{
%TypeHeaderCode
#include <qgscptcityarchive.h>
%End
  public:
    QgsCptCityAllRampsItem( QgsCptCityDataItem* parent, QString name,
                            QVector<QgsCptCityDataItem*> items );
    ~QgsCptCityAllRampsItem();

    QVector<QgsCptCityDataItem*> createChildren();
};


class QgsCptCityBrowserModel : QAbstractItemModel
{
%TypeHeaderCode
#include <qgscptcityarchive.h>
%End

  public:

    enum ViewType
    {
      Authors,
      Selections,
      List // not used anymore
    };

    QgsCptCityBrowserModel( QObject* parent /TransferThis/ = 0,
                            QgsCptCityArchive* archive = QgsCptCityArchive::defaultArchive(),
                            ViewType Type = Authors );
    ~QgsCptCityBrowserModel();

    // implemented methods from QAbstractItemModel for read-only access

    /** Used by other components to obtain information about each item provided by the model.
      In many models, the combination of flags should include Qt::ItemIsEnabled and Qt::ItemIsSelectable. */
    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    /** Used to supply item data to views and delegates. Generally, models only need to supply data
      for Qt::DisplayRole and any application-specific user roles, but it is also good practice
      to provide data for Qt::ToolTipRole, Qt::AccessibleTextRole, and Qt::AccessibleDescriptionRole.
      See the Qt::ItemDataRole enum documentation for information about the types associated with each role. */
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    /** Provides views with information to show in their headers. The information is only retrieved
      by views that can display header information. */
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    /** Provides the number of rows of data exposed by the model. */
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    /** Provides the number of columns of data exposed by the model. List models do not provide this function
      because it is already implemented in QAbstractListModel. */
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    /** Returns the index of the item in the model specified by the given row, column and parent index. */
    virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

    QModelIndex findItem( QgsCptCityDataItem *item, QgsCptCityDataItem *parent = 0 ) const;

    /** Returns the parent of the model item with the given index.
     * If the item has no parent, an invalid QModelIndex is returned.
     */
    virtual QModelIndex parent( const QModelIndex &index ) const;

    /** Returns a list of mime that can describe model indexes */
    /* virtual QStringList mimeTypes() const; */

    /** Returns an object that contains serialized items of data corresponding to the list of indexes specified */
    /* virtual QMimeData * mimeData( const QModelIndexList &indexes ) const; */

    /** Handles the data supplied by a drag and drop operation that ended with the given action */
    /* virtual bool dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent ); */

    QgsCptCityDataItem *dataItem( const QModelIndex &idx ) const;

    bool hasChildren( const QModelIndex &parent = QModelIndex() ) const;

    // Reload the whole model
    void reload();

    // Refresh item specified by path
    void refresh( QString path );

    // Refresh item childs
    void refresh( const QModelIndex &index = QModelIndex() );

    //! return index of a path
    QModelIndex findPath( QString path );

    void connectItem( QgsCptCityDataItem *item );

    bool canFetchMore( const QModelIndex & parent ) const;
    void fetchMore( const QModelIndex & parent );

  signals:

  public slots:
    //void removeItems( QgsCptCityDataItem * parent, QVector<QgsCptCityDataItem *>items );
    //void addItems( QgsCptCityDataItem * parent, QVector<QgsCptCityDataItem *>items );
    //void refreshItems( QgsCptCityDataItem * parent, QVector<QgsCptCityDataItem *>items );

    void beginInsertItems( QgsCptCityDataItem *parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsCptCityDataItem *parent, int first, int last );
    void endRemoveItems();

  protected:

    // populates the model
    void addRootItems();
    void removeRootItems();
};
