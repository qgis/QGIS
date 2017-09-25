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
%Docstring
 :rtype: str
%End
    static QString baseDir( QString archiveName );
%Docstring
 :rtype: str
%End
    static QString defaultBaseDir();
%Docstring
 :rtype: str
%End
    void setBaseDir( const QString &dirName );

    QString copyingFileName( const QString &dirName ) const;
%Docstring
 :rtype: str
%End
    QString descFileName( const QString &dirName ) const;
%Docstring
 :rtype: str
%End
    static QString findFileName( const QString &target, const QString &startDir, const QString &baseDir );
%Docstring
 :rtype: str
%End
    static QMap< QString, QString > copyingInfo( const QString &fileName );
%Docstring
 :rtype: QMap< str, QString >
%End
    static QMap< QString, QString > description( const QString &fileName );
%Docstring
 :rtype: QMap< str, QString >
%End

    bool isEmpty();
%Docstring
 :rtype: bool
%End
    QString archiveName() const;
%Docstring
 :rtype: str
%End
    static void initArchives( bool loadAll = false );
    static void initArchive( const QString &archiveName, const QString &archiveBaseDir );
    static void initDefaultArchive();
    static void clearArchives();
    static QgsCptCityArchive *defaultArchive();
%Docstring
 :rtype: QgsCptCityArchive
%End
    static QMap< QString, QgsCptCityArchive * > archiveRegistry();
%Docstring
 :rtype: QMap< str, QgsCptCityArchive * >
%End

    QVector< QgsCptCityDataItem * > rootItems() const;
%Docstring
 :rtype: list of QgsCptCityDataItem
%End
    QVector< QgsCptCityDataItem * > selectionItems() const;
%Docstring
 :rtype: list of QgsCptCityDataItem
%End

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
%Docstring
 :rtype: bool
%End

    int rowCount();
%Docstring
 :rtype: int
%End
    virtual int leafCount() const;
%Docstring
 :rtype: int
%End


    virtual void refresh();

    virtual QVector<QgsCptCityDataItem *> createChildren();
%Docstring
 :rtype: list of QgsCptCityDataItem
%End

    virtual void populate();
    bool isPopulated();
%Docstring
 :rtype: bool
%End

    virtual void addChildItem( QgsCptCityDataItem *child /Transfer/, bool refresh = false );

    virtual void deleteChildItem( QgsCptCityDataItem *child );

    virtual QgsCptCityDataItem *removeChildItem( QgsCptCityDataItem *child ) /TransferBack/;
%Docstring
 :rtype: QgsCptCityDataItem
%End

    virtual bool equal( const QgsCptCityDataItem *other );
%Docstring
 :rtype: bool
%End

    virtual QWidget *paramWidget() /Factory/;
%Docstring
 :rtype: QWidget
%End

    virtual QList<QAction *> actions();
%Docstring
 :rtype: list of QAction
%End

    virtual bool acceptDrop();
%Docstring
 :rtype: bool
%End

    virtual bool handleDrop( const QMimeData * /*data*/, Qt::DropAction /*action*/ );
%Docstring
 :rtype: bool
%End


    static int findItem( QVector<QgsCptCityDataItem *> items, QgsCptCityDataItem *item );
%Docstring
 :rtype: int
%End


    Type type() const;
%Docstring
 :rtype: Type
%End
    QgsCptCityDataItem *parent() const;
%Docstring
 :rtype: QgsCptCityDataItem
%End
    void setParent( QgsCptCityDataItem *parent );
    QVector<QgsCptCityDataItem *> children() const;
%Docstring
 :rtype: list of QgsCptCityDataItem
%End
    virtual QIcon icon();
%Docstring
 :rtype: QIcon
%End
    virtual QIcon icon( QSize size );
%Docstring
 :rtype: QIcon
%End
    QString name() const;
%Docstring
 :rtype: str
%End
    QString path() const;
%Docstring
 :rtype: str
%End
    QString info() const;
%Docstring
 :rtype: str
%End
    QString shortInfo() const;
%Docstring
 :rtype: str
%End

    void setIcon( const QIcon &icon );

    void setToolTip( const QString &msg );
    QString toolTip() const;
%Docstring
 :rtype: str
%End

    bool isValid();
%Docstring
 :rtype: bool
%End

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
%Docstring
 :rtype: QgsCptCityColorRamp
%End
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
%Docstring
 :rtype: list of QgsCptCityDataItem
%End

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
%Docstring
 :rtype: QgsCptCityDataItem
%End

  protected:
    QMap< QString, QStringList > rampsMap();
%Docstring
 :rtype: QMap< str, list of str >
%End
    QStringList dirEntries() const;
%Docstring
 :rtype: list of str
%End
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
%Docstring
 :rtype: list of str
%End

  protected:
    void parseXml();
};

class QgsCptCityAllRampsItem : QgsCptCityCollectionItem
{
%Docstring
 An "All ramps item", which contains all items in a flat hierarchy *
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
%Docstring
 :rtype: QModelIndex
%End

    virtual QModelIndex parent( const QModelIndex &index ) const;




    QgsCptCityDataItem *dataItem( const QModelIndex &idx ) const;
%Docstring
 virtual bool dropMimeData( const QMimeData * data, Qt.DropAction action, int row, int column, const QModelIndex & parent ); */
 :rtype: QgsCptCityDataItem
%End

    virtual bool hasChildren( const QModelIndex &parent = QModelIndex() ) const;


    void reload();

    void refresh( const QString &path );

    void refresh( const QModelIndex &index = QModelIndex() );

    QModelIndex findPath( const QString &path );
%Docstring
return index of a path
 :rtype: QModelIndex
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
