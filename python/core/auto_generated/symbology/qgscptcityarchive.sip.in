/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgscptcityarchive.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsCptCityArchive
{

%TypeHeaderCode
#include "qgscptcityarchive.h"
%End
  public:
    QgsCptCityArchive( const QString &archiveName = DEFAULT_CPTCITY_ARCHIVE,
                       const QString &baseDir = QString() );
    ~QgsCptCityArchive();


    QString baseDir() const;
    static QString baseDir( QString archiveName );
    static QString defaultBaseDir();
    void setBaseDir( const QString &dirName );

    QString copyingFileName( const QString &dirName ) const;
    QString descFileName( const QString &dirName ) const;
    static QString findFileName( const QString &target, const QString &startDir, const QString &baseDir );
    static QMap< QString, QString > copyingInfo( const QString &fileName );
    static QMap< QString, QString > description( const QString &fileName );

    bool isEmpty();
    QString archiveName() const;
    static void initArchives( bool loadAll = false );
    static void initArchive( const QString &archiveName, const QString &archiveBaseDir );
    static void initDefaultArchive();
    static void clearArchives();
    static QgsCptCityArchive *defaultArchive();
    static QMap< QString, QgsCptCityArchive * > archiveRegistry();

    QVector< QgsCptCityDataItem * > rootItems() const;
    QVector< QgsCptCityDataItem * > selectionItems() const;

  protected:


  private:
    QgsCptCityArchive( const QgsCptCityArchive &rh );
};

class QgsCptCityDataItem : QObject
{
%Docstring
Base class for all items in the model
%End

%TypeHeaderCode
#include "qgscptcityarchive.h"
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

    QgsCptCityDataItem( QgsCptCityDataItem::Type type, QgsCptCityDataItem *parent,
                        const QString &name, const QString &path );

    bool hasChildren();

    int rowCount();
    virtual int leafCount() const;


    virtual void refresh();

    virtual QVector<QgsCptCityDataItem *> createChildren();

    virtual void populate();
    bool isPopulated();

    virtual void addChildItem( QgsCptCityDataItem *child /Transfer/, bool refresh = false );

    virtual void deleteChildItem( QgsCptCityDataItem *child );

    virtual QgsCptCityDataItem *removeChildItem( QgsCptCityDataItem *child ) /TransferBack/;

    virtual bool equal( const QgsCptCityDataItem *other );

    virtual QWidget *paramWidget() /Factory/;

    virtual QList<QAction *> actions();

    virtual bool acceptDrop();

    virtual bool handleDrop( const QMimeData * /*data*/, Qt::DropAction /*action*/ );


    static int findItem( QVector<QgsCptCityDataItem *> items, QgsCptCityDataItem *item );


    Type type() const;
    QgsCptCityDataItem *parent() const;
    void setParent( QgsCptCityDataItem *parent );
    QVector<QgsCptCityDataItem *> children() const;
    virtual QIcon icon();
    virtual QIcon icon( QSize size );
    QString name() const;
    QString path() const;
    QString info() const;
    QString shortInfo() const;

    void setIcon( const QIcon &icon );

    void setToolTip( const QString &msg );
    QString toolTip() const;

    bool isValid();

  protected:


  signals:
    void beginInsertItems( QgsCptCityDataItem *parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsCptCityDataItem *parent, int first, int last );
    void endRemoveItems();
};

class QgsCptCityColorRampItem : QgsCptCityDataItem
{
%Docstring
Item that represents a layer that can be opened with one of the providers
%End

%TypeHeaderCode
#include "qgscptcityarchive.h"
%End
  public:
    QgsCptCityColorRampItem( QgsCptCityDataItem *parent,
                             const QString &name, const QString &path,
                             const QString &variantName = QString(),
                             bool initialize = false );
    QgsCptCityColorRampItem( QgsCptCityDataItem *parent,
                             const QString &name, const QString &path,
                             const QStringList &variantList,
                             bool initialize = false );


    virtual bool equal( const QgsCptCityDataItem *other );

    virtual int leafCount() const;

    const QgsCptCityColorRamp &ramp() const;
    virtual QIcon icon();

    virtual QIcon icon( QSize size );

    void init();

  protected:

};


class QgsCptCityCollectionItem : QgsCptCityDataItem
{
%Docstring
A Collection: logical collection of subcollections and color ramps
%End

%TypeHeaderCode
#include "qgscptcityarchive.h"
%End
  public:
    QgsCptCityCollectionItem( QgsCptCityDataItem *parent,
                              const QString &name, const QString &path );
    ~QgsCptCityCollectionItem();

    void setPopulated();
    void addChild( QgsCptCityDataItem *item /Transfer/ );
    QVector<QgsCptCityDataItem *> childrenRamps( bool recursive );

  protected:
};

class QgsCptCityDirectoryItem : QgsCptCityCollectionItem
{
%Docstring
A directory: contains subdirectories and color ramps
%End

%TypeHeaderCode
#include "qgscptcityarchive.h"
%End
  public:
    QgsCptCityDirectoryItem( QgsCptCityDataItem *parent,
                             const QString &name, const QString &path );

    virtual QVector<QgsCptCityDataItem *> createChildren();


    virtual bool equal( const QgsCptCityDataItem *other );


    static QgsCptCityDataItem *dataItem( QgsCptCityDataItem *parent,
                                         const QString &name, const QString &path );

  protected:
    QMap< QString, QStringList > rampsMap();
    QStringList dirEntries() const;
};

class QgsCptCitySelectionItem : QgsCptCityCollectionItem
{
%Docstring
A selection: contains subdirectories and color ramps
%End

%TypeHeaderCode
#include "qgscptcityarchive.h"
%End
  public:
    QgsCptCitySelectionItem( QgsCptCityDataItem *parent, const QString &name, const QString &path );

    virtual QVector<QgsCptCityDataItem *> createChildren();


    virtual bool equal( const QgsCptCityDataItem *other );


    QStringList selectionsList() const;

  protected:
    void parseXml();
};

class QgsCptCityAllRampsItem : QgsCptCityCollectionItem
{
%Docstring
An "All ramps item", which contains all items in a flat hierarchy
%End

%TypeHeaderCode
#include "qgscptcityarchive.h"
%End
  public:
    QgsCptCityAllRampsItem( QgsCptCityDataItem *parent, const QString &name,
                            const QVector<QgsCptCityDataItem *> &items );

    virtual QVector<QgsCptCityDataItem *> createChildren();


  protected:
};

class QgsCptCityBrowserModel : QAbstractItemModel
{

%TypeHeaderCode
#include "qgscptcityarchive.h"
%End
  public:

    enum ViewType
    {
      Authors,
      Selections,
      List
    };

    QgsCptCityBrowserModel( QObject *parent /TransferThis/ = 0,
                            QgsCptCityArchive *archive = QgsCptCityArchive::defaultArchive(),
                            ViewType Type = Authors );
    ~QgsCptCityBrowserModel();

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;


    QModelIndex findItem( QgsCptCityDataItem *item, QgsCptCityDataItem *parent = 0 ) const;

    virtual QModelIndex parent( const QModelIndex &index ) const;





    QgsCptCityDataItem *dataItem( const QModelIndex &idx ) const;
%Docstring
virtual bool dropMimeData( const QMimeData * data, Qt.DropAction action, int row, int column, const QModelIndex & parent ); */
%End

    virtual bool hasChildren( const QModelIndex &parent = QModelIndex() ) const;


    void reload();

    void refresh( const QString &path );

    void refresh( const QModelIndex &index = QModelIndex() );

    QModelIndex findPath( const QString &path );
%Docstring
Returns index of a path
%End

    void connectItem( QgsCptCityDataItem *item );

    virtual bool canFetchMore( const QModelIndex &parent ) const;

    virtual void fetchMore( const QModelIndex &parent );


  signals:

  public slots:

    void beginInsertItems( QgsCptCityDataItem *parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsCptCityDataItem *parent, int first, int last );
    void endRemoveItems();

  protected:

    void addRootItems();
    void removeRootItems();

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgscptcityarchive.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
