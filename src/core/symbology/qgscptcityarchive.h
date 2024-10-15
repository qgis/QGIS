/***************************************************************************
    qgscptcityarchive.h
    ---------------------
    begin                : August 2012
    copyright            : (C) 2009 by Martin Dobias
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny.dev at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCPTCITYARCHIVE_H
#define QGSCPTCITYARCHIVE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgscolorrampimpl.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <QMimeData>

class QgsCptCityColorRamp;
class QgsCptCityDataItem;
class QgsCptCitySelectionItem;

#define DEFAULT_CPTCITY_ARCHIVE "cpt-city-qgis-min"

/**
 * \class QgsCptCityArchive
 * \ingroup core
 */
class CORE_EXPORT QgsCptCityArchive
{
  public:
    QgsCptCityArchive( const QString &archiveName = DEFAULT_CPTCITY_ARCHIVE,
                       const QString &baseDir = QString() );
    ~QgsCptCityArchive();

    //! QgsCptCityArchive cannot be copied
    QgsCptCityArchive( const QgsCptCityArchive &rh ) = delete;
    //! QgsCptCityArchive cannot be copied
    QgsCptCityArchive &operator=( const QgsCptCityArchive &rh ) = delete;

    // basic dir info
    QString baseDir() const;
    static QString baseDir( QString archiveName );
    static QString defaultBaseDir();
    void setBaseDir( const QString &dirName ) { mBaseDir = dirName; }

    // collection + selection info
    QString copyingFileName( const QString &dirName ) const;
    QString descFileName( const QString &dirName ) const;
    static QString findFileName( const QString &target, const QString &startDir, const QString &baseDir );
    static QMap< QString, QString > copyingInfo( const QString &fileName );
    static QMap< QString, QString > description( const QString &fileName );
    //! \note not available in Python bindings
    static QMap< double, QPair<QColor, QColor> > gradientColorMap( const QString &fileName ) SIP_SKIP;

    // archive management
    bool isEmpty() const;
    QString archiveName() const { return mArchiveName; }
    static void initArchives( bool loadAll = false );
    static void initArchive( const QString &archiveName, const QString &archiveBaseDir );
    static void initDefaultArchive();
    static void clearArchives();
    static QgsCptCityArchive *defaultArchive();
    static QMap< QString, QgsCptCityArchive * > archiveRegistry();

    // items
    QVector< QgsCptCityDataItem * > rootItems() const { return mRootItems; }
    QVector< QgsCptCityDataItem * > selectionItems() const { return mSelectionItems; }

  private:

    QString mArchiveName;
    QString mBaseDir;
    // root items, namely directories at root of archive
    QVector< QgsCptCityDataItem * > mRootItems;
    QVector<QgsCptCityDataItem *> mSelectionItems;

  private:
#ifdef SIP_RUN
    QgsCptCityArchive( const QgsCptCityArchive &rh );
#endif

};

/**
 * Base class for all items in the model
 * \ingroup core
*/
class CORE_EXPORT QgsCptCityDataItem : public QObject
{
    Q_OBJECT
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
    // retrieve total count of "leaf" items (all children which are end nodes)
    virtual int leafCount() const;

    //

    virtual void refresh();

    // Create vector of children
    virtual QVector<QgsCptCityDataItem *> createChildren();

    // Populate children using children vector created by createChildren()
    virtual void populate();
    bool isPopulated() { return mPopulated; }

    // Insert new child using alphabetical order based on mName, emits necessary signal to model before and after, sets parent and connects signals
    // refresh - refresh populated item, emit signals to model
    virtual void addChildItem( QgsCptCityDataItem *child SIP_TRANSFER, bool refresh = false );

    // remove and delete child item, signals to browser are emitted
    virtual void deleteChildItem( QgsCptCityDataItem *child );

    // remove child item but don't delete it, signals to browser are emitted
    // returns pointer to the removed item or null if no such item was found
    virtual QgsCptCityDataItem *removeChildItem( QgsCptCityDataItem *child ) SIP_TRANSFERBACK;

    virtual bool equal( const QgsCptCityDataItem *other );

    virtual QWidget *paramWidget() SIP_FACTORY { return nullptr; }

    // whether accepts drag&drop'd layers - e.g. for import
    virtual bool acceptDrop() { return false; }

    // try to process the data dropped on this item
    virtual bool handleDrop( const QMimeData * /*data*/, Qt::DropAction /*action*/ ) { return false; }

    // static methods

    // Find child index in vector of items using '==' operator
    static int findItem( QVector<QgsCptCityDataItem *> items, QgsCptCityDataItem *item );

    // members

    Type type() const { return mType; }
    QgsCptCityDataItem *parent() const { return mParent; }
    void setParent( QgsCptCityDataItem *parent ) { mParent = parent; }
    QVector<QgsCptCityDataItem *> children() const { return mChildren; }
    virtual QIcon icon() { return mIcon; }
    virtual QIcon icon( QSize size ) { Q_UNUSED( size ) return icon(); }
    QString name() const { return mName; }
    QString path() const { return mPath; }
    QString info() const { return mInfo; }
    QString shortInfo() const { return mShortInfo; }

    void setIcon( const QIcon &icon ) { mIcon = icon; }

    void setToolTip( const QString &msg ) { mToolTip = msg; }
    QString toolTip() const { return mToolTip; }

    bool isValid() { return mValid; }

  protected:

    Type mType;
    QgsCptCityDataItem *mParent = nullptr;
    QVector<QgsCptCityDataItem *> mChildren; // easier to have it always
    bool mPopulated;
    QString mName;
    QString mPath; // it is also used to identify item in tree
    QString mInfo;
    QString mShortInfo;
    QString mToolTip;
    QIcon mIcon;
    bool mValid;

  signals:
    void beginInsertItems( QgsCptCityDataItem *parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsCptCityDataItem *parent, int first, int last );
    void endRemoveItems();
};

/**
 * Item that represents a layer that can be opened with one of the providers
 * \ingroup core
*/
class CORE_EXPORT QgsCptCityColorRampItem : public QgsCptCityDataItem
{
    Q_OBJECT
  public:
    QgsCptCityColorRampItem( QgsCptCityDataItem *parent,
                             const QString &name, const QString &path,
                             const QString &variantName = QString(),
                             bool initialize = false );
    QgsCptCityColorRampItem( QgsCptCityDataItem *parent,
                             const QString &name, const QString &path,
                             const QStringList &variantList,
                             bool initialize = false );

    // --- reimplemented from QgsCptCityDataItem ---

    bool equal( const QgsCptCityDataItem *other ) override;
    int leafCount() const override { return 1; }

    // --- New virtual methods for layer item derived classes ---
    const QgsCptCityColorRamp &ramp() const { return mRamp; }
    QIcon icon() override;
    QIcon icon( QSize size ) override;
    void init();

  protected:

    bool mInitialized;
    QgsCptCityColorRamp mRamp;
    QList< QIcon > mIcons;
};


/**
 * A Collection: logical collection of subcollections and color ramps
 * \ingroup core
*/
class CORE_EXPORT QgsCptCityCollectionItem : public QgsCptCityDataItem
{
    Q_OBJECT
  public:
    QgsCptCityCollectionItem( QgsCptCityDataItem *parent,
                              const QString &name, const QString &path );
    ~QgsCptCityCollectionItem() override;

    void setPopulated() { mPopulated = true; }
    void addChild( QgsCptCityDataItem *item SIP_TRANSFER ) { mChildren.append( item ); }
    QVector<QgsCptCityDataItem *> childrenRamps( bool recursive );

  protected:
    bool mPopulatedRamps;
};

/**
 * A directory: contains subdirectories and color ramps
 * \ingroup core
*/
class CORE_EXPORT QgsCptCityDirectoryItem : public QgsCptCityCollectionItem
{
    Q_OBJECT
  public:
    QgsCptCityDirectoryItem( QgsCptCityDataItem *parent,
                             const QString &name, const QString &path );

    QVector<QgsCptCityDataItem *> createChildren() override;

    bool equal( const QgsCptCityDataItem *other ) override;

    static QgsCptCityDataItem *dataItem( QgsCptCityDataItem *parent,
                                         const QString &name, const QString &path );

  protected:
    QMap< QString, QStringList > rampsMap();
    QStringList dirEntries() const;
    QMap< QString, QStringList > mRampsMap;
};

/**
 * \ingroup core
 * \class QgsCptCitySelectionItem
 * \brief A selection: contains subdirectories and color ramps
*/
class CORE_EXPORT QgsCptCitySelectionItem : public QgsCptCityCollectionItem
{
    Q_OBJECT
  public:
    QgsCptCitySelectionItem( QgsCptCityDataItem *parent, const QString &name, const QString &path );

    QVector<QgsCptCityDataItem *> createChildren() override;

    bool equal( const QgsCptCityDataItem *other ) override;

    QStringList selectionsList() const { return mSelectionsList; }

  protected:
    void parseXml();
    QStringList mSelectionsList;
};

/**
 * \ingroup core
 * \brief An "All ramps item", which contains all items in a flat hierarchy
*/
class CORE_EXPORT QgsCptCityAllRampsItem : public QgsCptCityCollectionItem
{
    Q_OBJECT
  public:
    QgsCptCityAllRampsItem( QgsCptCityDataItem *parent, const QString &name,
                            const QVector<QgsCptCityDataItem *> &items );

    QVector<QgsCptCityDataItem *> createChildren() override;

  protected:
    QVector<QgsCptCityDataItem *> mItems;
};

/**
 * \ingroup core
 * \class QgsCptCityBrowserModel
 */
class CORE_EXPORT QgsCptCityBrowserModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    enum ViewType
    {
      Authors = 0,
      Selections = 1,
      List = 2 // not used anymore
    };

    QgsCptCityBrowserModel( QObject *parent SIP_TRANSFERTHIS = nullptr,
                            QgsCptCityArchive *archive = QgsCptCityArchive::defaultArchive(),
                            ViewType Type = Authors );
    ~QgsCptCityBrowserModel() override;

    // implemented methods from QAbstractItemModel for read-only access
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;

    QModelIndex findItem( QgsCptCityDataItem *item, QgsCptCityDataItem *parent = nullptr ) const;

    QModelIndex parent( const QModelIndex &index ) const override;

    //! Returns a list of mime that can describe model indexes
    /* virtual QStringList mimeTypes() const; */

    //! Returns an object that contains serialized items of data corresponding to the list of indexes specified
    /* virtual QMimeData * mimeData( const QModelIndexList &indexes ) const; */

    //! Handles the data supplied by a drag and drop operation that ended with the given action
    /* virtual bool dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent ); */

    QgsCptCityDataItem *dataItem( const QModelIndex &idx ) const;

    bool hasChildren( const QModelIndex &parent = QModelIndex() ) const override;

    // Reload the whole model
    void reload();

    // Refresh item specified by path
    void refresh( const QString &path );

    // Refresh item children
    void refresh( const QModelIndex &index = QModelIndex() );

    //! Returns index of a path
    QModelIndex findPath( const QString &path );

    void connectItem( QgsCptCityDataItem *item );

    bool canFetchMore( const QModelIndex &parent ) const override;
    void fetchMore( const QModelIndex &parent ) override;

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

    QVector<QgsCptCityDataItem *> mRootItems;
    QgsCptCityArchive *mArchive = nullptr;
    ViewType mViewType;
    QSize mIconSize;
};

// clazy:excludeall=qstring-allocations

#endif
