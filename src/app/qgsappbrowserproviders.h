/***************************************************************************
    qgsappbrowserproviders.h
    -------------------------
    begin                : September 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPBROWSERPROVIDERS_H
#define QGSAPPBROWSERPROVIDERS_H

#include "qgis_app.h"
#include "qgsbookmarkmanager.h"
#include "qgsdataitemprovider.h"
#include "qgscustomdrophandler.h"
#include "qgsdataitemguiprovider.h"
#include "qgslayeritem.h"
#include "qgsprojectitem.h"
#include "qgsdatacollectionitem.h"

/**
 * Custom data item for QLR files.
 */
class QgsQlrDataItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsQlrDataItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::Uri mimeUri() const override;
    QgsMimeDataUtils::UriList mimeUris() const override;
};

/**
 * Data item provider for showing QLR layer files in the browser.
 */
class QgsQlrDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    Qgis::DataItemProviderCapabilities capabilities() const override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

/**
 * Handles drag and drop of QLR files to app.
 */
class QgsQlrDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT

  public:
    QString customUriProviderKey() const override;
    void handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const override;
};

/**
 * Custom data item for QPT print template files.
 */
class QgsQptDataItem : public QgsDataItem
{
    Q_OBJECT

  public:
    QgsQptDataItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::Uri mimeUri() const override;
    bool handleDoubleClick() override;
    QList<QAction *> actions( QWidget *parent ) override;
};

/**
 * Data item provider for showing QPT print templates in the browser.
 */
class QgsQptDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    Qgis::DataItemProviderCapabilities capabilities() const override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

/**
 * Handles drag and drop of QPT print templates to app.
 */
class QgsQptDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT

  public:
    QString customUriProviderKey() const override;
    void handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const override;
    bool handleFileDrop( const QString &file ) override;
};


/**
 * Custom data item for py Python scripts.
 */
class QgsPyDataItem : public QgsDataItem
{
    Q_OBJECT

  public:
    QgsPyDataItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::Uri mimeUri() const override;
    bool handleDoubleClick() override;
    QList<QAction *> actions( QWidget *parent ) override;
};

/**
 * Data item provider for showing Python py scripts in the browser.
 */
class QgsPyDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    Qgis::DataItemProviderCapabilities capabilities() const override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

/**
 * Handles drag and drop of Python py scripts to app.
 */
class QgsPyDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT

  public:
    QString customUriProviderKey() const override;
    void handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const override;
    bool handleFileDrop( const QString &file ) override;
};


/**
 * Custom data item for XML style libraries.
 */
class QgsStyleXmlDataItem : public QgsDataItem
{
    Q_OBJECT

  public:
    QgsStyleXmlDataItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::Uri mimeUri() const override;
    bool handleDoubleClick() override;
    QList<QAction *> actions( QWidget *parent ) override;

    static void browseStyle( const QString &xmlPath );
};

/**
 * Data item provider for showing style XML libraries in the browser.
 */
class QgsStyleXmlDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    Qgis::DataItemProviderCapabilities capabilities() const override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

/**
 * Handles drag and drop of style XML libraries to app.
 */
class QgsStyleXmlDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT

  public:
    QString customUriProviderKey() const override;
    void handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const override;
    bool handleFileDrop( const QString &file ) override;
};

/**
 * Custom data item for qgs/qgz QGIS project files, with more functionality than default browser project
 * file handling. Specifically allows browsing of the project's layer structure within the browser
 */
class APP_EXPORT QgsProjectRootDataItem : public QgsProjectItem
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsProjectRootDataItem, with the specified
     * project \a path.
     */
    QgsProjectRootDataItem( QgsDataItem *parent, const QString &path );
    QVector<QgsDataItem *> createChildren() override;
};

/**
 * Represents a layer tree group node within a QGIS project file.
 */
class APP_EXPORT QgsProjectLayerTreeGroupItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsProjectLayerTreeGroupItem, with the specified group \a name.
     */
    QgsProjectLayerTreeGroupItem( QgsDataItem *parent, const QString &name );
};

/**
 * Custom data item provider for showing qgs/qgz QGIS project files within the browser,
 * including the ability to browser the whole project's layer tree structure directly
 * within the browser.
 */
class APP_EXPORT QgsProjectDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    Qgis::DataItemProviderCapabilities capabilities() const override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

/**
 * Custom data item provider for showing bookmarks within the browser.
 */
class APP_EXPORT QgsBookmarksDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    Qgis::DataItemProviderCapabilities capabilities() const override;
    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;
};

class QgsBookmarksItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT

  public:
    QgsBookmarksItemGuiProvider() = default;

    QString name() override;
    bool acceptDrop( QgsDataItem *item, QgsDataItemGuiContext context ) override;
    bool handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction action ) override;
    void populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;
    bool handleDoubleClick( QgsDataItem *item, QgsDataItemGuiContext context ) override;
    bool rename( QgsDataItem *item, const QString &name, QgsDataItemGuiContext context ) override;

  private:
    void exportBookmarksFromManagers( const QList<const QgsBookmarkManager *> &managers, QgsMessageBar *messageBar, const QString &group = QString() );
    void importBookmarksToManager( QgsBookmarkManager *manager, QgsMessageBar *messageBar );
};


/**
 * Contains content of user and project bookmark managers
*/
class APP_EXPORT QgsBookmarksItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsBookmarksItem.
     */
    QgsBookmarksItem( QgsDataItem *parent, const QString &name, QgsBookmarkManager *applicationManager = nullptr, QgsBookmarkManager *projectManager = nullptr );

    QVector<QgsDataItem *> createChildren() override;

    //! Icon for bookmark manager container
    static QIcon iconBookmarks();

    QVariant sortKey() const override;

  private:
    QgsBookmarkManager *mApplicationManager = nullptr;
    QgsBookmarkManager *mProjectManager = nullptr;
};

class QgsBookmarkGroupItem;
class QgsBookmarkItem;

/**
 * Contains bookmarks content
*/
class APP_EXPORT QgsBookmarkManagerItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsBookmarkManagerItem.
     */
    QgsBookmarkManagerItem( QgsDataItem *parent, const QString &name, QgsBookmarkManager *manager );

    QVector<QgsDataItem *> createChildren() override;

    QgsBookmarkManager *manager() { return mManager; }
    QgsBookmarkGroupItem *groupItem( const QString &group );
    QgsBookmarkItem *childItemById( const QString &id );

    //! Icon for bookmark manager
    static QIcon iconBookmarkManager();

  private:
    QgsBookmarkManager *mManager = nullptr;
};


/**
 * Contains bookmarks
*/
class APP_EXPORT QgsBookmarkGroupItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsBookmarkGroupItem.
     */
    QgsBookmarkGroupItem( QgsDataItem *parent, const QString &name, QgsBookmarkManager *manager );

    QVector<QgsDataItem *> createChildren() override;

    void addBookmark( const QgsBookmark &bookmark );

    QString group() const { return mGroup; }
    QgsBookmarkManager *manager() { return mManager; }

    QgsBookmarkItem *childItemById( const QString &id );
    void removeBookmarkChildById( const QString &id );

    //! Icon for bookmark group
    static QIcon iconBookmarkGroup();

  private:
    QgsBookmarkManager *mManager = nullptr;
    QString mGroup;
};

/**
 * Bookmark data item
*/
class APP_EXPORT QgsBookmarkItem : public QgsDataItem
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsBookmarkGroupItem.
     */
    QgsBookmarkItem( QgsDataItem *parent, const QString &name, const QgsBookmark &bookmark, QgsBookmarkManager *manager );
    QgsBookmarkManager *manager() { return mManager; }
    QgsBookmark bookmark() const { return mBookmark; }
    void setBookmark( const QgsBookmark &bookmark );

    //! Icon for bookmark item
    static QIcon iconBookmark();
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::UriList mimeUris() const override;

  private:
    QgsBookmarkManager *mManager = nullptr;

    QgsBookmark mBookmark;
};

/**
 * Handles drag and drop of bookmarks files to canvases.
 */
class QgsBookmarkDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT

  public:
    QString customUriProviderKey() const override;
    bool canHandleCustomUriCanvasDrop( const QgsMimeDataUtils::Uri &uri, QgsMapCanvas *canvas ) override;
    bool handleCustomUriCanvasDrop( const QgsMimeDataUtils::Uri &uri, QgsMapCanvas *canvas ) const override;
};


/**
 * Data item provider for showing html files in the browser.
 */
class QgsHtmlDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    Qgis::DataItemProviderCapabilities capabilities() const override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

/**
 * Custom data item for html files.
 */
class QgsHtmlDataItem : public QgsDataItem
{
    Q_OBJECT

  public:
    QgsHtmlDataItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool handleDoubleClick() override;
    QList<QAction *> actions( QWidget *parent ) override;
};

#endif // QGSAPPBROWSERPROVIDERS_H
