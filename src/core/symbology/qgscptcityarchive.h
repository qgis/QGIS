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
 * \brief Represents a CPT City color scheme.
 */
class CORE_EXPORT QgsCptCityArchive
{
  public:
    QgsCptCityArchive( const QString &archiveName = DEFAULT_CPTCITY_ARCHIVE,
                       const QString &baseDir = QString() );
    ~QgsCptCityArchive();

    QgsCptCityArchive( const QgsCptCityArchive &rh ) = delete;
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
    //! Returns TRUE if archive is empty
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

    /**
     * Returns the total count of "leaf" items (all children which are end nodes).
     */
    virtual int leafCount() const;

    //

    virtual void refresh();

    /**
     * Returns a vector of children items.
     */
    virtual QVector<QgsCptCityDataItem *> createChildren();

    /**
     * Populates children using children vector created by createChildren().
     */
    virtual void populate();

    /**
     * Returns TRUE if the item is already populated.
     */
    bool isPopulated() { return mPopulated; }

    /**
     * Inserts a new \a child using alphabetical order based on mName, emits necessary signal to model before and after, sets parent and connects signals.
     *
     * The \a refresh argument will refresh the populated item by emitting signals to the model.
     */
    virtual void addChildItem( QgsCptCityDataItem *child SIP_TRANSFER, bool refresh = false );

    /**
     * Removes and deletes a child \a item, signals to browser are emitted.
     */
    virtual void deleteChildItem( QgsCptCityDataItem *child );

    /**
     * Removes a \a child item but doesn't delete it, signals to browser are emitted.
     *
     * \returns pointer to the removed item or NULLPTR if no such item was found
     */
    virtual QgsCptCityDataItem *removeChildItem( QgsCptCityDataItem *child ) SIP_TRANSFERBACK;

    /**
     * Returns TRUE if this item is equal to an \a other item.
     */
    virtual bool equal( const QgsCptCityDataItem *other );

    /**
     * \deprecated QGIS 3.40. Is unused and will be removed in QGIS 4.0.
     */
    Q_DECL_DEPRECATED virtual QWidget *paramWidget() SIP_DEPRECATED { return nullptr; }

    /**
     * Returns TRUE if the item accepts drag & dropped layers - e.g. for import.
     *
     * \deprecated QGIS 3.40. Is unused and will be removed in QGIS 4.0.
     */
    Q_DECL_DEPRECATED virtual bool acceptDrop() SIP_DEPRECATED { return false; }

    /**
     * Tries to process the \a data dropped on this item.
     *
     * \deprecated QGIS 3.40. Is unused and will be removed in QGIS 4.0.
     */
    Q_DECL_DEPRECATED virtual bool handleDrop( const QMimeData *data, Qt::DropAction action ) SIP_DEPRECATED { Q_UNUSED( data ); Q_UNUSED( action ); return false; }

    // static methods

    /**
     * Finds a child index in vector of items using '==' operator.
     */
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

    /**
     * Emitted before child items are added to this item.
     *
     * This signal *must* be followed by endInsertItems().
     *
     * \param parent the parent item having children added
     * \param first index of first child item to be added
     * \param last index last child item, after the addition has occurred
     *
     * \see endInsertItems()
     */
    void beginInsertItems( QgsCptCityDataItem *parent, int first, int last );

    /**
     * Emitted after child items have been added to this data item.
     *
     * This signal will always be preceded by beginInsertItems().
     *
     * \see beginInsertItems()
     */
    void endInsertItems();

    /**
     * Emitted before child items are removed from this data item.
     *
     * This signal *must* be followed by endRemoveItems().
     *
     * \param parent the parent item having children removed
     * \param first index of first child item to be removed
     * \param last index of the last child item to be removed
     *
     * \see endRemoveItems()
     */
    void beginRemoveItems( QgsCptCityDataItem *parent, int first, int last );

    /**
     * Emitted after child items have been removed from this data item.
     *
     * This signal will always be preceded by beginRemoveItems().
     *
     * \see beginRemoveItems()
     */
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
 * \brief A custom item model for display of CPT City color palettes.
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

    /**
     * Returns the data item corresponding to the given index.
     */
    QgsCptCityDataItem *dataItem( const QModelIndex &idx ) const;

    bool hasChildren( const QModelIndex &parent = QModelIndex() ) const override;

    //! Reload the whole model
    void reload();

    //! Refresh the item specified by \a path
    void refresh( const QString &path );

    //! Refresh item children
    void refresh( const QModelIndex &index = QModelIndex() );

    //! Returns index of a path
    QModelIndex findPath( const QString &path );

    void connectItem( QgsCptCityDataItem *item );

    bool canFetchMore( const QModelIndex &parent ) const override;
    void fetchMore( const QModelIndex &parent ) override;

  public slots:

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
