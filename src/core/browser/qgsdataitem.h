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

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgsmimedatautils.h"
#include "qgis.h"
#include <QObject>
#include <QFutureWatcher>
#include <QIcon>

class QgsDataItem;
class QMenu;
class QgsAbstractDatabaseProviderConnection;
class QgsAnimatedIcon;
class QgsCoordinateReferenceSystem;

class QAction;
class QWidget;

typedef QgsDataItem *dataItem_t( QString, QgsDataItem * ) SIP_SKIP;

/**
 * \ingroup core
 * \brief Base class for all items in the model.
 *
 * Parent/children hierarchy is not based on QObject.
*/
class CORE_EXPORT QgsDataItem : public QObject
{
#ifdef SIP_RUN
#include "qgslayeritem.h"
#include "qgsdirectoryitem.h"
#include "qgsfavoritesitem.h"
#include "qgszipitem.h"
#include "qgsdatacollectionitem.h"
#include "qgsprojectitem.h"
#endif

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
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
    SIP_END
#endif

    Q_OBJECT

  public:

    /**
     * Constructor for QgsDataItem, with the specified \a parent item.
     *
     * The \a name argument specifies the text to show in the model for the item. A translated string should
     * be used wherever appropriate.
     *
     * The \a path argument gives the item path in the browser tree. The \a path string can take any form,
     * but QgsDataItem items pointing to different logical locations should always use a different item \a path.
     *
     * The optional \a providerKey string (added in QGIS 3.12) can be used to specify the key for the QgsDataItemProvider that created this item.
     */
    QgsDataItem( Qgis::BrowserItemType type, QgsDataItem *parent SIP_TRANSFERTHIS, const QString &name, const QString &path, const QString &providerKey = QString() );

    ~QgsDataItem() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsDataItem: \"%1\" %2>" ).arg( sipCpp->name(), sipCpp->path() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    bool hasChildren();

    /**
     * Returns TRUE if the data item is a collection of layers
     * The default implementation returns FALSE, subclasses must implement this method if their children are layers.
     * \since QGIS 3.14
     */
    virtual bool layerCollection() const;

    int rowCount();

    /**
     * Create children. Children are not expected to have parent set.
     * \warning This method MUST BE THREAD SAFE.
    */
    virtual QVector<QgsDataItem *> createChildren() SIP_TRANSFERBACK;
#ifdef SIP_RUN
    SIP_VIRTUAL_CATCHER_CODE
    PyObject *sipResObj = sipCallMethod( 0, sipMethod, "" );
    // H = Convert a Python object to a mapped type instance.
    // 5 = 1 (disallows the conversion of Py_None to NULL) + 4 (returns a copy of the C/C++ instance)
    sipIsErr = !sipResObj || sipParseResult( 0, sipMethod, sipResObj, "H5", sipType_QVector_0101QgsDataItem, &sipRes ) < 0;
    if ( !sipIsErr )
    {
      for ( QgsDataItem *item : sipRes )
      {
        PyObject *pyItem = sipGetPyObject( item, sipType_QgsDataItem );
        if ( pyItem != NULL )
        {
          // pyItem is given an extra reference which is removed when the C++ instance’s destructor is called.
          sipTransferTo( pyItem, Py_None );
        }
      }
    }
    if ( sipResObj != NULL )
    {
      Py_DECREF( sipResObj );
    }
    SIP_END
#endif

    //! \since QGIS 2.8
    Qgis::BrowserItemState state() const;

    /**
     * Set item state. It also take care about starting/stopping loading icon animation.
     * \param state
     * \since QGIS 2.8
     */
    virtual void setState( Qgis::BrowserItemState state );

    /**
     * Inserts a new child item. The child will be inserted at a position using an alphabetical order based on mName.
     * \param child child item to insert. Ownership is transferred, and item parent will be set and relevant connections made.
     * \param refresh - set to TRUE to refresh populated item, emitting relevant signals to the model
     * \see deleteChildItem()
     */
    virtual void addChildItem( QgsDataItem *child SIP_TRANSFER, bool refresh = false );

    /**
     * Removes and deletes a child item, emitting relevant signals to the model.
     * \param child child to remove. Item must exist as a current child.
     * \see addChildItem()
     */
    virtual void deleteChildItem( QgsDataItem *child );

    /**
     * Removes a child item and returns it without deleting it. Emits relevant signals to model as required.
     * \param child child to remove
     * \returns pointer to the removed item or NULLPTR if no such item was found
     */
    virtual QgsDataItem *removeChildItem( QgsDataItem *child ) SIP_TRANSFERBACK;

    /**
     * Returns TRUE if this item is equal to another item (by testing item type and path).
     */
    virtual bool equal( const QgsDataItem *other );

    /**
     * Returns source widget from data item for QgsBrowserPropertiesWidget
     *
     * Use QgsDataItemGuiProvider::createParamWidget() instead
     *
     * \deprecated QGIS 3.10
     */
    Q_DECL_DEPRECATED virtual QWidget *paramWidget() SIP_FACTORY SIP_DEPRECATED { return nullptr; }

    /**
     * Returns the list of actions available for this item. This is usually used for the popup menu on right-clicking
     * the item. Subclasses should override this to provide actions.
     *
     * Subclasses should ensure that ownership of created actions is correctly handled by parenting them
     * to the specified parent widget.
     */
    virtual QList<QAction *> actions( QWidget *parent );

    /**
     * Returns the list of menus available for this item. This is usually used for the popup menu on right-clicking
     * the item. Subclasses should override this to provide actions. Subclasses should ensure that ownership of
     * created menus is correctly handled by parenting them to the specified parent widget.
     * \param parent a parent widget of the menu
     * \returns list of menus
     * \since QGIS 3.0
     */
    virtual QList<QMenu *> menus( QWidget *parent );

    /**
     * Returns whether the item accepts drag and dropped layers - e.g. for importing a dataset to a provider.
     * Subclasses should override this and handleDrop() to accept dropped layers.
     * \see handleDrop()
     * \see QgsDataItemGuiProvider::handleDrop()
     *
     * \deprecated QGIS 3.10
     */
    Q_DECL_DEPRECATED virtual bool acceptDrop() SIP_DEPRECATED { return false; }

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

    /**
     * Attempts to process the mime data dropped on this item. Subclasses must override this and acceptDrop() if they
     * accept dropped layers.
     * \see acceptDrop()
     * \see QgsDataItemGuiProvider::handleDrop()
     *
     * \deprecated QGIS 3.10
     */
    Q_DECL_DEPRECATED virtual bool handleDrop( const QMimeData * /*data*/, Qt::DropAction /*action*/ ) SIP_DEPRECATED { return false; }
#ifdef __clang__
#pragma clang diagnostic pop
#endif

    /**
     * Called when a user double clicks on the item. Subclasses should return TRUE
     * if they have implemented a double-click handler and do not want the default
     * double-click behavior for items.
     * \since QGIS 3.0
     */
    virtual bool handleDoubleClick();

    /**
     * Returns TRUE if the item may be dragged.
     * Default implementation returns FALSE.
     * A draggable item has to implement mimeUris() that will be used to pass data.
     * \see mimeUris()
     * \since QGIS 3.0
     */
    virtual bool hasDragEnabled() const { return false; }

    /**
     * Returns mime URI for the data item.
     * Items that return valid URI will be returned in mime data when dragging a selection from browser model.
     * \see hasDragEnabled()
     * \deprecated since QGIS 3.18, use mimeUris() instead
     * \since QGIS 3.0
     */
    Q_DECL_DEPRECATED virtual QgsMimeDataUtils::Uri mimeUri() const SIP_DEPRECATED;

    /**
     * Returns mime URIs for the data item, most data providers will only return a single URI
     * but some data collection items (e.g. GPKG, OGR) may report multiple URIs (e.g. for vector and
     * raster layer types).
     *
     * Items that return valid URI will be returned in mime data when dragging a selection from browser model.
     * \since QGIS 3.18
     */
    virtual QgsMimeDataUtils::UriList mimeUris() const;

    /**
     * Writes the selected crs into data source. The original data source will be modified when calling this
     * method.
     *
     * \deprecated since QGIS 3.6. This method is no longer used by QGIS and will be removed in QGIS 4.0.
     */
    Q_DECL_DEPRECATED virtual bool setCrs( const QgsCoordinateReferenceSystem &crs ) SIP_DEPRECATED;

    /**
     * Sets a new \a name for the item, and returns TRUE if the item was successfully renamed.
     *
     * Items which implement this method should return the QgsDataItem::Rename capability.
     *
     * The default implementation does nothing.
     *
     * Use QgsDataItemGuiProvider:
     *
     * \since QGIS 3.4
     * \deprecated QGIS 3.10
     */
    Q_DECL_DEPRECATED virtual bool rename( const QString &name ) SIP_DEPRECATED;

    // ### QGIS 4 - rename to capabilities()

    /**
     * Returns the capabilities for the data item.
     *
     * \see setCapabilities()
     */
    virtual Qgis::BrowserItemCapabilities capabilities2() const { return mCapabilities; }

    /**
     * Sets the capabilities for the data item.
     *
     * \see capabilities2()
     */
    virtual void setCapabilities( Qgis::BrowserItemCapabilities capabilities ) SIP_PYNAME( setCapabilitiesV2 ) { mCapabilities = capabilities; }

    /**
     * \deprecated use setCapabilitiesV2 instead.
     */
    Q_DECL_DEPRECATED void setCapabilities( int capabilities ) SIP_DEPRECATED;

    // static methods

    // Find child index in vector of items using '==' operator
    static int findItem( QVector<QgsDataItem *> items, QgsDataItem *item );

    // members

    Qgis::BrowserItemType type() const { return mType; }

    /**
     * Gets item parent. QgsDataItem maintains its own items hierarchy, it does not use
     * QObject hierarchy.
    */
    QgsDataItem *parent() const { return mParent; }

    /**
     * Set item parent and connect / disconnect parent to / from item signals.
     * It does not add itself to parents children (mChildren)
    */
    void setParent( QgsDataItem *parent );
    QVector<QgsDataItem *> children() const { return mChildren; }
    virtual QIcon icon();

    /**
     * Returns the name of the item (the displayed text for the item).
     *
     * \see setName()
     */
    QString name() const { return mName; }

    /**
     * Sets the \a name of the item (the displayed text for the item).
     *
     * \see name()
     */
    void setName( const QString &name );

    QString path() const { return mPath; }
    void setPath( const QString &path ) { mPath = path; }

    /**
     * Returns the provider key that created this item (e.g. "PostGIS")
     *
     * If key has a prefix "special:", it marks that the item was not created with a provider,
     * but manually. For example "special:Favorites", "special:Home"
     *
     * \since QGIS 3.12
     */
    QString providerKey() const;

    /**
     * Sets the provider key that created this item (e.g. "PostGIS")
     *
     * If key has a prefix "special:", it marks that the item was not created with a provider,
     * but manually. For example "special:Favorites"
     *
     * \since QGIS 3.12
     */
    void setProviderKey( const QString &value );

    //! Create path component replacing path separators
    static QString pathComponent( const QString &component );

    /**
     * Returns the sorting key for the item. By default name() is returned,
     * but setSortKey() can be used to set a custom sort key for the item.
     *
     * Alternatively subclasses can override this method to return a custom
     * sort key.
     *
     * \see setSortKey()
     * \since QGIS 3.0
     */
    virtual QVariant sortKey() const;

    /**
     * Sets a custom sorting \a key for the item.
     * \see sortKey()
     * \since QGIS 3.0
     */
    void setSortKey( const QVariant &key );


    // Because QIcon (QPixmap) must not be used in outside the GUI thread, it is
    // not possible to set mIcon in constructor. Either use mIconName/setIconName()
    // or implement icon().
    void setIcon( const QIcon &icon ) { mIcon = icon; }
    void setIconName( const QString &iconName ) { mIconName = iconName; }

    void setToolTip( const QString &msg ) { mToolTip = msg; }
    QString toolTip() const { return mToolTip; }

    // deleteLater() items and clear the vector
    static void deleteLater( QVector<QgsDataItem *> &items );

    //! Move object and all its descendants to thread
    void moveToThread( QThread *targetThread );

    /**
     * For data items that represent a DB connection or one of its children,
     * this method returns a connection.
     * All other data items will return NULL.
     *
     * Ownership of the returned objects is transferred to the caller.
     *
     * \since QGIS 3.16
     */
    virtual QgsAbstractDatabaseProviderConnection *databaseConnection() const SIP_FACTORY;

  protected:
    virtual void populate( const QVector<QgsDataItem *> &children );

    /**
     * Refresh the items from a specified list of child items.
     */
    virtual void refresh( const QVector<QgsDataItem *> &children );

    /**
     * The item is scheduled to be deleted. E.g. if deleteLater() is called when
     * item is in Populating state (createChildren() running in another thread),
     * the deferredDelete() returns TRUE and item will be deleted once Populating finished.
     * Items with slow reateChildren() (for example network or database based) may
     * check during createChildren() if deferredDelete() returns TRUE and return from
     * createChildren() immediately because result will be useless.
    */
    bool deferredDelete() { return mDeferredDelete; }

    Qgis::BrowserItemType mType;
    Qgis::BrowserItemCapabilities mCapabilities = Qgis::BrowserItemCapability::NoCapabilities;
    QgsDataItem *mParent = nullptr;
    QVector<QgsDataItem *> mChildren; // easier to have it always
    Qgis::BrowserItemState mState = Qgis::BrowserItemState::NotPopulated;
    QString mName;
    QString mProviderKey;
    // Path is slash ('/') separated chain of item identifiers which are usually item names, but may be different if it is
    // necessary to distinguish paths of two providers to the same source (e.g GRASS location and standard directory have the same
    // name but different paths). Identifiers in path must not contain '/' characters.
    // The path is used to identify item in tree.
    QString mPath;
    QString mToolTip;
    QString mIconName;
    QIcon mIcon;
    QMap<QString, QIcon> mIconMap;

    //! Custom sort key. If invalid, name() will be used for sorting instead.
    QVariant mSortKey;

  public slots:

    /**
     * Safely delete the item:
     *
     * - disconnects parent
     * - unsets parent (but does not remove itself)
     * - deletes all its descendants recursively
     * - waits until Populating state (createChildren() in thread) finished without blocking main thread
     * - calls QObject::deleteLater()
     */
    virtual void deleteLater();

    // Populate children using children vector created by createChildren()
    // \param foreground run createChildren in foreground
    virtual void populate( bool foreground = false );

    //! Remove children recursively and set as not populated. This is used when refreshing collapsed items.
    virtual void depopulate();

    virtual void refresh();

    /**
     * Causes a data item provider to refresh all registered connections.
     *
     * If \a providerKey is specified then only the matching provider will be refreshed. Otherwise,
     * all providers will be refreshed (which is potentially very expensive!).
     */
    virtual void refreshConnections( const QString &providerKey = QString() );

    virtual void childrenCreated();

  signals:
    void beginInsertItems( QgsDataItem *parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsDataItem *parent, int first, int last );
    void endRemoveItems();
    void dataChanged( QgsDataItem *item );

    /**
     * Emitted when an item's state is changed.
     */
    void stateChanged( QgsDataItem *item, Qgis::BrowserItemState oldState );

    /**
     * Emitted when the connections of the provider with the specified \a providerKey have changed.
     *
     * This signal is normally forwarded to the app in order to refresh the connection
     * item in the provider dialogs and to refresh the connection items in the other
     * open browsers.
     */
    void connectionsChanged( const QString &providerKey = QString() );

  protected slots:

    /**
     * Will request a repaint of this icon.
     *
     * \since QGIS 3.0
     */
    void updateIcon();

  private:
    static QVector<QgsDataItem *> runCreateChildren( QgsDataItem *item );

    // Set to true if object has to be deleted when possible (nothing running in threads)
    bool mDeferredDelete = false;
    QFutureWatcher< QVector <QgsDataItem *> > *mFutureWatcher = nullptr;
    // number of items currently in loading (populating) state
    static QgsAnimatedIcon *sPopulatingIcon;
};

/**
 * \ingroup core
 * \brief Data item that can be used to report problems (e.g. network error)
 */
class CORE_EXPORT QgsErrorItem : public QgsDataItem
{
    Q_OBJECT
  public:

    QgsErrorItem( QgsDataItem *parent, const QString &error, const QString &path );

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsErrorItem: \"%1\" %2>" ).arg( sipCpp->name(), sipCpp->path() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

};

#endif // QGSDATAITEM_H


