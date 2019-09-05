/***************************************************************************
    qgsappbrowserproviders.cpp
    ---------------------------
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

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsappbrowserproviders.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgsproject.h"
#include "qgsstyleexportimportdialog.h"
#include "qgsstyle.h"
#include "qgslayertreenode.h"
#include "qgslayertree.h"
#include "qgsstylemanagerdialog.h"
#include "qgsguiutils.h"

#include <QDesktopServices>

QIcon QgsBookmarksItem::iconBookmarks()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowBookmarks.svg" ) );
}

QVariant QgsBookmarksItem::sortKey() const
{
  return QStringLiteral( " 1" );
}

QIcon QgsBookmarkManagerItem::iconBookmarkManager()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolder.svg" ) );
}

QIcon QgsBookmarkGroupItem::iconBookmarkGroup()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolder.svg" ) );
}

QIcon QgsBookmarkItem::iconBookmark()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mItemBookmark.svg" ) );
}

bool QgsBookmarkItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::Uri QgsBookmarkItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "custom" );
  u.providerKey = QStringLiteral( "bookmark" );
  u.name = name();

  QDomDocument doc;
  doc.appendChild( mBookmark.writeXml( doc ) );
  u.uri = doc.toString();

  return u;
}

//
// QgsQlrDataItem
//

QgsQlrDataItem::QgsQlrDataItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsLayerItem( parent, name, path, path, QgsLayerItem::NoType, QStringLiteral( "qlr" ) )
{
  setState( QgsDataItem::Populated ); // no children
  setIconName( QStringLiteral( ":/images/icons/qgis-icon-16x16.png" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
}

bool QgsQlrDataItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::Uri QgsQlrDataItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "custom" );
  u.providerKey = QStringLiteral( "qlr" );
  u.name = name();
  u.uri = path();
  return u;
}

//
// QgsQlrDataItemProvider
//

QString QgsQlrDataItemProvider::name()
{
  return QStringLiteral( "QLR" );
}

int QgsQlrDataItemProvider::capabilities() const
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsQlrDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QFileInfo fileInfo( path );

  if ( fileInfo.suffix().compare( QLatin1String( "qlr" ), Qt::CaseInsensitive ) == 0 )
  {
    return new QgsQlrDataItem( parentItem, fileInfo.fileName(), path );
  }
  return nullptr;
}

QString QgsQlrDropHandler::customUriProviderKey() const
{
  return QStringLiteral( "qlr" );
}

void QgsQlrDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  QString path = uri.uri;
  QgisApp::instance()->openLayerDefinition( path );
}

//
// QgsQptDataItemProvider
//

QString QgsQptDataItemProvider::name()
{
  return QStringLiteral( "QPT" );
}

int QgsQptDataItemProvider::capabilities() const
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsQptDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QFileInfo fileInfo( path );

  if ( fileInfo.suffix().compare( QLatin1String( "qpt" ), Qt::CaseInsensitive ) == 0 )
  {
    return new QgsQptDataItem( parentItem, fileInfo.baseName(), path );
  }
  return nullptr;
}

//
// QgsQptDropHandler
//

QString QgsQptDropHandler::customUriProviderKey() const
{
  return QStringLiteral( "qpt" );
}

void QgsQptDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  QString path = uri.uri;
  QgisApp::instance()->openTemplate( path );
}

bool QgsQptDropHandler::handleFileDrop( const QString &file )
{
  QFileInfo fi( file );
  if ( fi.completeSuffix().compare( QLatin1String( "qpt" ), Qt::CaseInsensitive ) == 0 )
  {
    QgisApp::instance()->openTemplate( file );
    return true;
  }
  return false;
}

//
// QgsQptDataItem
//

QgsQptDataItem::QgsQptDataItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataItem( QgsDataItem::Custom, parent, name, path )
{
  setState( QgsDataItem::Populated ); // no children
  setIconName( QStringLiteral( "/mIconQptFile.svg" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
}

bool QgsQptDataItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::Uri QgsQptDataItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "custom" );
  u.providerKey = QStringLiteral( "qpt" );
  u.name = name();
  u.uri = path();
  return u;
}

bool QgsQptDataItem::handleDoubleClick()
{
  QgisApp::instance()->openTemplate( path() );
  return true;
}

QList<QAction *> QgsQptDataItem::actions( QWidget *parent )
{
  QAction *newLayout = new QAction( tr( "New Layout from Template" ), parent );
  connect( newLayout, &QAction::triggered, this, [ = ]
  {
    QgisApp::instance()->openTemplate( path() );
  } );
  return QList<QAction *>() << newLayout;
}

//
// QgsPyDataItem
//

QgsPyDataItem::QgsPyDataItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataItem( QgsDataItem::Custom, parent, name, path )
{
  setState( QgsDataItem::Populated ); // no children
  setIconName( QStringLiteral( "/mIconPythonFile.svg" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
}

bool QgsPyDataItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::Uri QgsPyDataItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "custom" );
  u.providerKey = QStringLiteral( "py" );
  u.name = name();
  u.uri = path();
  return u;
}

bool QgsPyDataItem::handleDoubleClick()
{
  QgisApp::instance()->runScript( path() );
  return true;
}

QList<QAction *> QgsPyDataItem::actions( QWidget *parent )
{
  QAction *runScript = new QAction( tr( "&Run Script" ), parent );
  connect( runScript, &QAction::triggered, this, [ = ]
  {
    QgisApp::instance()->runScript( path() );
  } );
  QAction *editScript = new QAction( tr( "Open in External &Editor" ), this );
  connect( editScript, &QAction::triggered, this, [ = ]
  {
    QDesktopServices::openUrl( QUrl::fromLocalFile( path() ) );
  } );
  return QList<QAction *>() << runScript << editScript;
}

//
// QgsPyDataItemProvider
//

QString QgsPyDataItemProvider::name()
{
  return QStringLiteral( "py" );
}

int QgsPyDataItemProvider::capabilities() const
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsPyDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QFileInfo fileInfo( path );

  if ( fileInfo.suffix().compare( QLatin1String( "py" ), Qt::CaseInsensitive ) == 0 )
  {
    return new QgsPyDataItem( parentItem, fileInfo.baseName(), path );
  }
  return nullptr;
}

//
// QgsPyDropHandler
//

QString QgsPyDropHandler::customUriProviderKey() const
{
  return QStringLiteral( "py" );
}

void QgsPyDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  QString path = uri.uri;
  QgisApp::instance()->runScript( path );
}

bool QgsPyDropHandler::handleFileDrop( const QString &file )
{
  QFileInfo fi( file );
  if ( fi.completeSuffix().compare( QLatin1String( "py" ), Qt::CaseInsensitive ) == 0 )
  {
    QgisApp::instance()->runScript( file );
    return true;
  }
  return false;
}




//
// QgsStyleXmlDataItem
//

QgsStyleXmlDataItem::QgsStyleXmlDataItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataItem( QgsDataItem::Custom, parent, name, path )
{
  setState( QgsDataItem::Populated ); // no children
  setIconName( QStringLiteral( "/mActionStyleManager.svg" ) );
  setToolTip( QStringLiteral( "<b>%1</b><br>%2" ).arg( tr( "QGIS style library" ), QDir::toNativeSeparators( path ) ) );
}

bool QgsStyleXmlDataItem::hasDragEnabled() const
{
  return true;
}

QgsMimeDataUtils::Uri QgsStyleXmlDataItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "custom" );
  u.providerKey = QStringLiteral( "style_xml" );
  u.name = name();
  u.uri = path();
  return u;
}

bool QgsStyleXmlDataItem::handleDoubleClick()
{
  browseStyle( mPath );
  return true;
}

QList<QAction *> QgsStyleXmlDataItem::actions( QWidget *parent )
{
  QAction *browseAction = new QAction( tr( "&Open Style…" ), parent );
  const QString path = mPath;
  connect( browseAction, &QAction::triggered, this, [path]
  {
    browseStyle( path );
  } );

  QAction *importAction = new QAction( tr( "&Import Style…" ), parent );
  connect( importAction, &QAction::triggered, this, [path]
  {
    QgsStyleExportImportDialog dlg( QgsStyle::defaultStyle(), QgisApp::instance(), QgsStyleExportImportDialog::Import );
    dlg.setImportFilePath( path );
    dlg.exec();
  } );
  return QList<QAction *>() << browseAction << importAction;
}

void QgsStyleXmlDataItem::browseStyle( const QString &xmlPath )
{
  QgsStyle s;
  s.createMemoryDatabase();

  auto cursorOverride = qgis::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
  if ( s.importXml( xmlPath ) )
  {
    cursorOverride.reset();
    QFileInfo fi( xmlPath );
    QgsStyleManagerDialog dlg( &s, QgisApp::instance(), Qt::WindowFlags(), true );
    dlg.setSmartGroupsVisible( false );
    dlg.setFavoritesGroupVisible( false );
    dlg.setBaseStyleName( fi.baseName() );
    dlg.exec();
  }
}

//
// QgsStyleXmlDataItemProvider
//

QString QgsStyleXmlDataItemProvider::name()
{
  return QStringLiteral( "style_xml" );
}

int QgsStyleXmlDataItemProvider::capabilities() const
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsStyleXmlDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( QgsStyle::isXmlStyleFile( path ) )
  {
    return new QgsStyleXmlDataItem( parentItem, QFileInfo( path ).fileName(), path );
  }
  return nullptr;
}

//
// QgsStyleXmlDropHandler
//

QString QgsStyleXmlDropHandler::customUriProviderKey() const
{
  return QStringLiteral( "style_xml" );
}

void QgsStyleXmlDropHandler::handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const
{
  QgsStyleExportImportDialog dlg( QgsStyle::defaultStyle(), QgisApp::instance(), QgsStyleExportImportDialog::Import );
  dlg.setImportFilePath( uri.uri );
  dlg.exec();
}

bool QgsStyleXmlDropHandler::handleFileDrop( const QString &file )
{
  if ( QgsStyle::isXmlStyleFile( file ) )
  {
    QgsStyleExportImportDialog dlg( QgsStyle::defaultStyle(), QgisApp::instance(), QgsStyleExportImportDialog::Import );
    dlg.setImportFilePath( file );
    dlg.exec();
    return true;
  }
  return false;
}

//
// QgsProjectRootDataItem
//

QgsProjectRootDataItem::QgsProjectRootDataItem( QgsDataItem *parent, const QString &path )
  : QgsProjectItem( parent, QFileInfo( path ).completeBaseName(), path )
{
  mCapabilities = Collapse | Fertile; // collapse by default to avoid costly population on startup
  setState( NotPopulated );
}


QVector<QgsDataItem *> QgsProjectRootDataItem::createChildren()
{
  QVector<QgsDataItem *> childItems;

  QgsProject p;
  if ( !p.read( mPath, QgsProject::FlagDontResolveLayers ) )
  {
    childItems.append( new QgsErrorItem( nullptr, p.error(), mPath + "/error" ) );
    return childItems;
  }

  // recursively create groups and layer items for project's layer tree
  std::function<void( QgsDataItem *parentItem, QgsLayerTreeGroup *group )> addNodes;
  addNodes = [this, &addNodes, &childItems]( QgsDataItem * parentItem, QgsLayerTreeGroup * group )
  {
    const QList< QgsLayerTreeNode * > children = group->children();
    for ( QgsLayerTreeNode *child : children )
    {
      switch ( child->nodeType() )
      {
        case QgsLayerTreeNode::NodeLayer:
        {
          if ( QgsLayerTreeLayer *layerNode = qobject_cast< QgsLayerTreeLayer * >( child ) )
          {
            QgsMapLayer *layer = layerNode->layer();
#if 0 // TODO
            QString style;
            if ( layer )
            {
              QString errorMsg;
              QDomDocument doc( QStringLiteral( "qgis" ) );
              QgsReadWriteContext context;
              context.setPathResolver( p.pathResolver() );
              layer->exportNamedStyle( doc, errorMsg, context );
              style = doc.toString();
            }
#endif

            QgsLayerItem *layerItem = new QgsLayerItem( nullptr, layerNode->name(),
                layer ? layer->source() : QString(),
                layer ? layer->source() : QString(),
                layer ? QgsLayerItem::typeFromMapLayer( layer ) : QgsLayerItem::NoType,
                layer ? layer->providerType() : QString() );
            layerItem->setState( Populated ); // children are not expected
            layerItem->setToolTip( layer ? layer->source() : QString() );
            if ( parentItem == this )
              childItems << layerItem;
            else
              parentItem->addChildItem( layerItem, true );
          }
          break;
        }

        case QgsLayerTreeNode::NodeGroup:
        {
          if ( QgsLayerTreeGroup *groupNode = qobject_cast< QgsLayerTreeGroup * >( child ) )
          {
            QgsProjectLayerTreeGroupItem *groupItem = new QgsProjectLayerTreeGroupItem( nullptr, groupNode->name() );
            addNodes( groupItem, groupNode );
            groupItem->setState( Populated );
            if ( parentItem == this )
              childItems << groupItem;
            else
              parentItem->addChildItem( groupItem, true );
          }
        }
        break;
      }
    }
  };

  addNodes( this, p.layerTreeRoot() );
  return childItems;
}


//
// QgsProjectLayerTreeGroupItem
//

QgsProjectLayerTreeGroupItem::QgsProjectLayerTreeGroupItem( QgsDataItem *parent, const QString &name )
  : QgsDataCollectionItem( parent, name )
{
  mIconName = QStringLiteral( "mActionFolder.svg" );
  mCapabilities = NoCapabilities;
  setToolTip( name );
}


//
// QgsProjectDataItemProvider
//

QString QgsProjectDataItemProvider::name()
{
  return QStringLiteral( "project_item" );
}

int QgsProjectDataItemProvider::capabilities() const
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsProjectDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QFileInfo fileInfo( path );
  if ( fileInfo.suffix().compare( QLatin1String( "qgs" ), Qt::CaseInsensitive ) == 0 || fileInfo.suffix().compare( QLatin1String( "qgz" ), Qt::CaseInsensitive ) == 0 )
  {
    return new QgsProjectRootDataItem( parentItem, path );
  }
  return nullptr;
}

//
// QgsBookmarksDataItemProvider
//

QString QgsBookmarksDataItemProvider::name()
{
  return QStringLiteral( "bookmarks_item" );
}

int QgsBookmarksDataItemProvider::capabilities() const
{
  return QgsDataProvider::Database;
}

QgsDataItem *QgsBookmarksDataItemProvider::createDataItem( const QString &, QgsDataItem *parentItem )
{
  return new QgsBookmarksItem( parentItem, QObject::tr( "Spatial Bookmarks" ), QgsApplication::bookmarkManager(), QgsProject::instance()->bookmarkManager() );
}

QgsBookmarksItem::QgsBookmarksItem( QgsDataItem *parent, const QString &name, QgsBookmarkManager *applicationManager, QgsBookmarkManager *projectManager )
  : QgsDataCollectionItem( parent, name, QStringLiteral( "bookmarks:" ) )
{
  mType = Custom;
  mCapabilities = Fast;
  mApplicationManager = applicationManager;
  mProjectManager = projectManager;
  mIconName = QStringLiteral( "/mActionShowBookmarks.svg" );
  populate();
}

QVector<QgsDataItem *> QgsBookmarksItem::createChildren()
{
  QVector<QgsDataItem *> children;
  if ( mApplicationManager )
    children << new QgsBookmarkManagerItem( this, tr( "User Bookmarks" ), mApplicationManager );
  if ( mProjectManager )
    children << new QgsBookmarkManagerItem( this, tr( "Project Bookmarks" ), mProjectManager );
  return children;
}

QList<QAction *> QgsBookmarksItem::actions( QWidget *parent )
{
  QAction *showBookmarksPanel = new QAction( tr( "&Show Spatial Bookmarks Panel" ), parent );
  connect( showBookmarksPanel, &QAction::triggered, this, [ = ]
  {
    QgisApp::instance()->showBookmarks( true );
  } );
  QAction *addBookmark = new QAction( tr( "&New Spatial Bookmark" ), parent );
  connect( addBookmark, &QAction::triggered, this, [ = ]
  {
    QgisApp::instance()->newBookmark();
  } );
  return QList<QAction *>() << showBookmarksPanel << addBookmark;
}

QgsBookmarkManagerItem::QgsBookmarkManagerItem( QgsDataItem *parent, const QString &name, QgsBookmarkManager *manager )
  : QgsDataCollectionItem( parent, name, QStringLiteral( "bookmarks:%1" ).arg( name.toLower() ) )
{
  mType = Custom;
  mCapabilities = Fast;
  mManager = manager;
  mIconName = QStringLiteral( "/mIconFolder.svg" );

  connect( mManager, &QgsBookmarkManager::bookmarkAdded, this, [ = ]( const QString & ) { depopulate(); refresh(); } );
  connect( mManager, &QgsBookmarkManager::bookmarkChanged, this, [ = ]( const QString & ) { depopulate(); refresh(); } );
  connect( mManager, &QgsBookmarkManager::bookmarkRemoved, this, [ = ]( const QString & ) { depopulate(); refresh(); } );

  populate();
}

QVector<QgsDataItem *> QgsBookmarkManagerItem::createChildren()
{
  QVector<QgsDataItem *> children;
  for ( const QString &group : mManager->groups() )
  {
    if ( group.isEmpty() )
    {
      for ( const QgsBookmark &bookmark : mManager->bookmarksByGroup( QString() ) )
      {
        children << new QgsBookmarkItem( this, bookmark.name(), bookmark, mManager );
      }
    }
    else
    {
      children << new QgsBookmarkGroupItem( this, group, mManager );
    }
  }
  return children;
}

QgsBookmarkGroupItem::QgsBookmarkGroupItem( QgsDataItem *parent, const QString &name, QgsBookmarkManager *manager )
  : QgsDataCollectionItem( parent, name, QStringLiteral( "bookmarks:%1" ).arg( name.toLower() ) )
  , mGroup( name )
{
  mType = Custom;
  mCapabilities = Fast;
  mManager = manager;
  mIconName = QStringLiteral( "/mIconFolder.svg" );
  populate();
}

QVector<QgsDataItem *> QgsBookmarkGroupItem::createChildren()
{
  QVector<QgsDataItem *> children;
  const QList< QgsBookmark > bookmarks = mManager->bookmarksByGroup( mName );
  children.reserve( bookmarks.size() );
  for ( const QgsBookmark &bookmark : bookmarks )
  {
    children << new QgsBookmarkItem( this, bookmark.name(), bookmark, mManager );
  }
  return children;
}

QgsBookmarkItem::QgsBookmarkItem( QgsDataItem *parent, const QString &name, const QgsBookmark &bookmark, QgsBookmarkManager *manager )
  : QgsDataItem( Custom, parent, name, QStringLiteral( "bookmarks:%1/%2" ).arg( bookmark.group().toLower(), bookmark.id() ) )
  , mBookmark( bookmark )
  , mManager( manager )
{
  mType = Custom;
  mCapabilities = NoCapabilities;
  mIconName = QStringLiteral( "/mItemBookmark.svg" );
  setState( Populated ); // no more children
}

bool QgsBookmarkItem::handleDoubleClick()
{
  QgsReferencedRectangle rect = mBookmark.extent();
  QgsRectangle canvasExtent = rect;
  if ( rect.crs() != QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs() )
  {
    QgsCoordinateTransform ct( rect.crs(),
                               QgisApp::instance()->mapCanvas()->mapSettings().destinationCrs(), QgsProject::instance() );
    try
    {
      canvasExtent = ct.transform( rect );
    }
    catch ( QgsCsException & )
    {
      QgisApp::instance()->messageBar()->pushWarning( tr( "Zoom to Bookmark" ), tr( "Could not reproject bookmark extent to project CRS." ) );
      return true;
    }
    if ( canvasExtent.isEmpty() )
    {
      QgisApp::instance()->messageBar()->pushWarning( tr( "Zoom to Bookmark" ), tr( "Bookmark extent is empty" ) );
      return  true;
    }
  }

  // set the extent to the bookmark and refresh
  QgisApp::instance()->mapCanvas()->setExtent( canvasExtent );
  QgisApp::instance()->mapCanvas()->refresh();
  return true;
}

QString QgsBookmarkDropHandler::customUriProviderKey() const
{
  return QStringLiteral( "bookmark" );
}

bool QgsBookmarkDropHandler::canHandleCustomUriCanvasDrop( const QgsMimeDataUtils::Uri &uri, QgsMapCanvas * )
{
  return uri.providerKey == customUriProviderKey();
}

bool QgsBookmarkDropHandler::handleCustomUriCanvasDrop( const QgsMimeDataUtils::Uri &uri, QgsMapCanvas *canvas ) const
{
  QDomDocument doc;
  doc.setContent( uri.uri );
  QDomElement elem = doc.documentElement();
  QgsBookmark b = QgsBookmark::fromXml( elem, doc );

  QgsReferencedRectangle rect = b.extent();
  QgsRectangle canvasExtent = rect;
  if ( rect.crs() != canvas->mapSettings().destinationCrs() )
  {
    QgsCoordinateTransform ct( rect.crs(),
                               canvas->mapSettings().destinationCrs(), QgsProject::instance() );
    try
    {
      canvasExtent = ct.transform( rect );
    }
    catch ( QgsCsException & )
    {
      QgisApp::instance()->messageBar()->pushWarning( tr( "Zoom to Bookmark" ), tr( "Could not reproject bookmark extent to canvas CRS." ) );
      return true;
    }
    if ( canvasExtent.isEmpty() )
    {
      QgisApp::instance()->messageBar()->pushWarning( tr( "Zoom to Bookmark" ), tr( "Bookmark extent is empty" ) );
      return true;
    }
  }

  // set the extent to the bookmark and refresh
  canvas->setExtent( canvasExtent );
  canvas->refresh();

  return true;
}

QString QgsBookmarksItemGuiProvider::name()
{
  return QStringLiteral( "bookmark_item" );
}

bool QgsBookmarksItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( qobject_cast< QgsBookmarkManagerItem * >( item ) )
    return true;

  if ( qobject_cast< QgsBookmarkGroupItem * >( item ) )
    return true;

  return false;
}

bool QgsBookmarksItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext, const QMimeData *data, Qt::DropAction )
{
  QgsBookmarkManagerItem *managerItem = qobject_cast< QgsBookmarkManagerItem * >( item );
  QgsBookmarkGroupItem *groupItem = qobject_cast< QgsBookmarkGroupItem * >( item );
  if ( managerItem || groupItem )
  {
    QgsBookmarkManager *target = managerItem ? managerItem->manager() : groupItem->manager();
    if ( QgsMimeDataUtils::isUriList( data ) )
    {
      const QgsMimeDataUtils::UriList list = QgsMimeDataUtils::decodeUriList( data );
      for ( const QgsMimeDataUtils::Uri &uri : list )
      {
        QDomDocument doc;
        doc.setContent( uri.uri );
        QDomElement elem = doc.documentElement();
        QgsBookmark b = QgsBookmark::fromXml( elem, doc );

        if ( !groupItem )
          b.setGroup( QString() );
        else
          b.setGroup( groupItem->group() );

        // if bookmark doesn't already exist in manager, we add it. Otherwise we update it.
        if ( target->bookmarkById( b.id() ).id().isEmpty() )
          target->addBookmark( b );
        else
          target->updateBookmark( b );
      }
      return true;
    }
  }

  return false;
}
