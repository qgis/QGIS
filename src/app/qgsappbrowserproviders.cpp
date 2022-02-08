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
#include "qgsbookmarkeditordialog.h"
#include "qgsmapcanvas.h"
#include "qgsmessagebar.h"
#include "qgsproject.h"
#include "qgsstyleexportimportdialog.h"
#include "qgsstyle.h"
#include "qgslayertreenode.h"
#include "qgslayertree.h"
#include "qgsstylemanagerdialog.h"
#include "qgsguiutils.h"
#include "qgsfileutils.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QFileDialog>
#include <QUrl>

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
  : QgsLayerItem( parent, name, path, path, Qgis::BrowserLayerType::NoType, QStringLiteral( "qlr" ) )
{
  setState( Qgis::BrowserItemState::Populated ); // no children
  setIconName( QStringLiteral( ":/images/icons/qgis-icon-16x16.png" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
  mCapabilities |= Qgis::BrowserItemCapability::ItemRepresentsFile;
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
  u.filePath = path();
  return u;
}

QgsMimeDataUtils::UriList QgsQlrDataItem::mimeUris() const
{
  return QgsMimeDataUtils::UriList() << QgsQlrDataItem::mimeUri();
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
  const QFileInfo fileInfo( path );

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
  const QString path = uri.uri;
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
  const QFileInfo fileInfo( path );

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
  const QString path = uri.uri;
  QgisApp::instance()->openTemplate( path );
}

bool QgsQptDropHandler::handleFileDrop( const QString &file )
{
  const QFileInfo fi( file );
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
  : QgsDataItem( Qgis::BrowserItemType::Custom, parent, name, path )
{
  setState( Qgis::BrowserItemState::Populated ); // no children
  setIconName( QStringLiteral( "/mIconQptFile.svg" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
  mCapabilities |= Qgis::BrowserItemCapability::ItemRepresentsFile;
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
  u.filePath = path();
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
  : QgsDataItem( Qgis::BrowserItemType::Custom, parent, name, path )
{
  setState( Qgis::BrowserItemState::Populated ); // no children
  setIconName( QStringLiteral( "/mIconPythonFile.svg" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
  mCapabilities |= Qgis::BrowserItemCapability::ItemRepresentsFile;
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
  u.filePath = path();
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
  const QFileInfo fileInfo( path );

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
  const QString path = uri.uri;
  QgisApp::instance()->runScript( path );
}

bool QgsPyDropHandler::handleFileDrop( const QString &file )
{
  const QFileInfo fi( file );
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
  : QgsDataItem( Qgis::BrowserItemType::Custom, parent, name, path )
{
  setState( Qgis::BrowserItemState::Populated ); // no children
  setIconName( QStringLiteral( "/mActionStyleManager.svg" ) );
  setToolTip( QStringLiteral( "<b>%1</b><br>%2" ).arg( tr( "QGIS style library" ), QDir::toNativeSeparators( path ) ) );
  mCapabilities |= Qgis::BrowserItemCapability::ItemRepresentsFile;
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
  u.filePath = path();
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

  auto cursorOverride = std::make_unique< QgsTemporaryCursorOverride >( Qt::WaitCursor );
  if ( s.importXml( xmlPath ) )
  {
    cursorOverride.reset();
    const QFileInfo fi( xmlPath );
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
  mCapabilities = Qgis::BrowserItemCapability::Collapse | Qgis::BrowserItemCapability::Fertile; // collapse by default to avoid costly population on startup
  mCapabilities |= Qgis::BrowserItemCapability::ItemRepresentsFile;
  setState( Qgis::BrowserItemState::NotPopulated );
}


QVector<QgsDataItem *> QgsProjectRootDataItem::createChildren()
{
  QVector<QgsDataItem *> childItems;

  QgsProject p;
  if ( !p.read( mPath, QgsProject::ReadFlag::FlagDontResolveLayers | QgsProject::ReadFlag::FlagDontLoadLayouts | QgsProject::ReadFlag::FlagDontStoreOriginalStyles ) )
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
                layer ? QgsLayerItem::typeFromMapLayer( layer ) : Qgis::BrowserLayerType::NoType,
                layer ? layer->providerType() : QString() );
            layerItem->setState( Qgis::BrowserItemState::Populated ); // children are not expected
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
            groupItem->setState( Qgis::BrowserItemState::Populated );
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
  mCapabilities = Qgis::BrowserItemCapability::NoCapabilities;
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
  const QFileInfo fileInfo( path );
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
  mType = Qgis::BrowserItemType::Custom;
  mCapabilities = Qgis::BrowserItemCapability::Fast;
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

QgsBookmarkManagerItem::QgsBookmarkManagerItem( QgsDataItem *parent, const QString &name, QgsBookmarkManager *manager )
  : QgsDataCollectionItem( parent, name, QStringLiteral( "bookmarks:%1" ).arg( name.toLower() ) )
{
  mType = Qgis::BrowserItemType::Custom;
  mCapabilities = Qgis::BrowserItemCapability::Fast;
  mManager = manager;
  mIconName = QStringLiteral( "/mIconFolder.svg" );

  connect( mManager, &QgsBookmarkManager::bookmarkAdded, this, [ = ]( const QString & id )
  {
    const QgsBookmark newDetails = mManager->bookmarkById( id );
    if ( newDetails.group().isEmpty() )
      addChildItem( new QgsBookmarkItem( this, newDetails.name(), newDetails, mManager ), true );
    else
    {
      if ( QgsBookmarkGroupItem *newGroup = groupItem( newDetails.group() ) )
      {
        // existing group, add this bookmark to it
        newGroup->addBookmark( newDetails );
      }
      else
      {
        // need to create a new group for this (will automatically add the new bookmark)
        addChildItem( new QgsBookmarkGroupItem( this, newDetails.group(), mManager ), true );
      }
    }
  } );
  connect( mManager, &QgsBookmarkManager::bookmarkChanged, this, [ = ]( const QString & id )
  {
    const QgsBookmark newDetails = mManager->bookmarkById( id );

    // have to do a deep dive to find the old item...!
    const QVector<QgsDataItem *> c = children();
    for ( QgsDataItem *i : c )
    {
      if ( QgsBookmarkItem *bookmarkItem = qobject_cast< QgsBookmarkItem * >( i ) )
      {
        if ( bookmarkItem->bookmark().id() == id )
        {
          // found the target! now, what's changed?
          if ( bookmarkItem->bookmark().group() == newDetails.group() )
          {
            // good, not the group. Just update the existing bookmark then.
            bookmarkItem->setBookmark( newDetails );
            return;
          }
          else
          {
            // group has changed, ouch!
            // first remove from existing group
            deleteChildItem( bookmarkItem );
            // and add a child to the new group
            if ( QgsBookmarkGroupItem *newGroup = groupItem( newDetails.group() ) )
            {
              newGroup->addBookmark( newDetails );
            }
            else
            {
              // need to create a new group for this (will automatically add the new bookmark)
              addChildItem( new QgsBookmarkGroupItem( this, newDetails.group(), mManager ), true );
            }
          }
          break;
        }
      }
      else if ( QgsBookmarkGroupItem *group = qobject_cast< QgsBookmarkGroupItem * >( i ) )
      {
        if ( QgsBookmarkItem *bookmarkItem = group->childItemById( id ) )
        {
          // ok, found old group, now compare
          if ( bookmarkItem->bookmark().group() == newDetails.group() )
          {
            // good, not the group. Just update the existing bookmark then.
            bookmarkItem->setBookmark( newDetails );
          }
          else
          {
            // group has changed!
            // first remove from existing group
            group->deleteChildItem( bookmarkItem );
            if ( group->children().empty() )
              deleteChildItem( group );

            // and add a child to the new group
            if ( !newDetails.group().isEmpty() )
            {
              if ( QgsBookmarkGroupItem *newGroup = groupItem( newDetails.group() ) )
              {
                newGroup->addBookmark( newDetails );
              }
              else
              {
                // need to create a new group for this (will automatically add the new bookmark)
                addChildItem( new QgsBookmarkGroupItem( this, newDetails.group(), mManager ), true );
              }
            }
            else
            {
              addChildItem( new QgsBookmarkItem( this, newDetails.name(), newDetails, mManager ), true );
            }
          }
          break;
        }
      }
    }
  } );
  connect( mManager, &QgsBookmarkManager::bookmarkAboutToBeRemoved, this, [ = ]( const QString & id )
  {
    const QgsBookmark b = mManager->bookmarkById( id );
    if ( !b.group().isEmpty() )
    {
      if ( QgsBookmarkGroupItem *group = groupItem( b.group() ) )
      {
        group->removeBookmarkChildById( id );
        if ( group->children().empty() )
          deleteChildItem( group );
      }
    }
    else if ( QgsBookmarkItem *bookmarkItem = childItemById( id ) )
    {
      deleteChildItem( bookmarkItem );
    }
  } );

  populate();
}

QVector<QgsDataItem *> QgsBookmarkManagerItem::createChildren()
{
  QVector<QgsDataItem *> children;
  const QStringList groupNames = mManager->groups();
  for ( const QString &group : groupNames )
  {
    if ( group.isEmpty() )
    {
      const QList<QgsBookmark> matching = mManager->bookmarksByGroup( QString() );
      for ( const QgsBookmark &bookmark : matching )
      {
        children << new QgsBookmarkItem( this, bookmark.name(), bookmark, mManager );
      }
    }
    else
    {
      QgsBookmarkGroupItem *item = new QgsBookmarkGroupItem( this, group, mManager );
      children << item;
    }
  }
  return children;
}

QgsBookmarkGroupItem *QgsBookmarkManagerItem::groupItem( const QString &group )
{
  const QVector<QgsDataItem *> c = children();
  for ( QgsDataItem *i : c )
  {
    if ( QgsBookmarkGroupItem *groupItem = qobject_cast< QgsBookmarkGroupItem * >( i ) )
    {
      if ( groupItem->group() == group )
      {
        return groupItem;
      }
    }
  }
  return nullptr;
}

QgsBookmarkItem *QgsBookmarkManagerItem::childItemById( const QString &id )
{
  const QVector<QgsDataItem *> c = children();
  for ( QgsDataItem *i : c )
  {
    if ( QgsBookmarkItem *bookmarkItem = qobject_cast< QgsBookmarkItem * >( i ) )
    {
      if ( bookmarkItem->bookmark().id() == id )
      {
        return bookmarkItem;
      }
    }
  }
  return nullptr;
}

QgsBookmarkGroupItem::QgsBookmarkGroupItem( QgsDataItem *parent, const QString &name, QgsBookmarkManager *manager )
  : QgsDataCollectionItem( parent, name, QStringLiteral( "bookmarks:%1" ).arg( name.toLower() ) )
  , mGroup( name )
{
  mType = Qgis::BrowserItemType::Custom;
  mCapabilities = Qgis::BrowserItemCapability::Fast | Qgis::BrowserItemCapability::Rename;
  mManager = manager;
  mIconName = QStringLiteral( "/mIconFolder.svg" );
  setToolTip( name );

  setSortKey( QStringLiteral( "  %1" ).arg( name ) );

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

void QgsBookmarkGroupItem::addBookmark( const QgsBookmark &bookmark )
{
  addChildItem( new QgsBookmarkItem( this, bookmark.name(), bookmark, mManager ), true );
}

QgsBookmarkItem *QgsBookmarkGroupItem::childItemById( const QString &id )
{
  const QVector<QgsDataItem *> c = children();
  for ( QgsDataItem *i : c )
  {
    if ( QgsBookmarkItem *bookmarkItem = qobject_cast< QgsBookmarkItem * >( i ) )
    {
      if ( bookmarkItem->bookmark().id() == id )
      {
        return bookmarkItem;
      }
    }
  }
  return nullptr;
}

void QgsBookmarkGroupItem::removeBookmarkChildById( const QString &id )
{
  if ( QgsBookmarkItem *bookmarkItem = childItemById( id ) )
    deleteChildItem( bookmarkItem );
}

QgsBookmarkItem::QgsBookmarkItem( QgsDataItem *parent, const QString &name, const QgsBookmark &bookmark, QgsBookmarkManager *manager )
  : QgsDataItem( Qgis::BrowserItemType::Custom, parent, name, QStringLiteral( "bookmarks:%1/%2" ).arg( bookmark.group().toLower(), bookmark.id() ) )
  , mManager( manager )
  , mBookmark( bookmark )
{
  mType = Qgis::BrowserItemType::Custom;
  mCapabilities = Qgis::BrowserItemCapability::Rename;
  mIconName = QStringLiteral( "/mItemBookmark.svg" );
  setToolTip( name );
  setState( Qgis::BrowserItemState::Populated ); // no more children
}

void QgsBookmarkItem::setBookmark( const QgsBookmark &bookmark )
{
  setName( bookmark.name() );
  setToolTip( bookmark.name() );
  mBookmark = bookmark;
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
  const QDomElement elem = doc.documentElement();
  const QgsBookmark b = QgsBookmark::fromXml( elem, doc );

  try
  {
    if ( ! canvas->setReferencedExtent( b.extent() ) )
    {
      QgisApp::instance()->messageBar()->pushWarning( tr( "Zoom to Bookmark" ), tr( "Bookmark extent is empty" ) );
    }
    else
    {
      canvas->refresh();
    }
  }
  catch ( QgsCsException & )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Zoom to Bookmark" ), tr( "Could not reproject bookmark extent to canvas CRS." ) );
  }
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
        const QDomElement elem = doc.documentElement();
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

void QgsBookmarksItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context )
{
  if ( qobject_cast< QgsBookmarksItem * >( item ) )
  {
    QAction *addBookmark = new QAction( tr( "New Spatial Bookmark…" ), menu );
    connect( addBookmark, &QAction::triggered, this, [ = ]
    {
      QgisApp::instance()->newBookmark();
    } );
    menu->addAction( addBookmark );
    QAction *showBookmarksPanel = new QAction( tr( "Show Spatial Bookmarks Manager" ), menu );
    connect( showBookmarksPanel, &QAction::triggered, this, [ = ]
    {
      QgisApp::instance()->showBookmarkManager( true );
    } );
    menu->addAction( showBookmarksPanel );
    menu->addSeparator();
    QAction *importBookmarks = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSharingImport.svg" ) ), tr( "Import Spatial Bookmarks…" ), menu );
    connect( importBookmarks, &QAction::triggered, this, [ = ]
    {
      importBookmarksToManager( QgsApplication::bookmarkManager(), context.messageBar() );
    } );
    menu->addAction( importBookmarks );
    QAction *exportBookmarks = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSharingExport.svg" ) ), tr( "Export Spatial Bookmarks…" ), menu );
    connect( exportBookmarks, &QAction::triggered, this, [ = ]
    {
      exportBookmarksFromManagers( QList< const QgsBookmarkManager * >() << QgsApplication::bookmarkManager() << QgsProject::instance()->bookmarkManager(), context.messageBar() );
    } );
    menu->addAction( exportBookmarks );
  }
  else if ( QgsBookmarkManagerItem *managerItem = qobject_cast< QgsBookmarkManagerItem * >( item ) )
  {
    QAction *addBookmark = new QAction( tr( "New Spatial Bookmark…" ), menu );
    const bool inProject = managerItem->manager() != QgsApplication::bookmarkManager();
    connect( addBookmark, &QAction::triggered, this, [ = ]
    {
      QgisApp::instance()->newBookmark( inProject );
    } );
    menu->addAction( addBookmark );
    menu->addSeparator();
    QAction *importBookmarks = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSharingImport.svg" ) ), tr( "Import Spatial Bookmarks…" ), menu );
    connect( importBookmarks, &QAction::triggered, this, [ = ]
    {
      importBookmarksToManager( managerItem->manager(), context.messageBar() );
    } );
    menu->addAction( importBookmarks );

    QAction *exportBookmarks = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSharingExport.svg" ) ), tr( "Export Spatial Bookmarks…" ), menu );
    connect( exportBookmarks, &QAction::triggered, this, [ = ]
    {
      exportBookmarksFromManagers( QList< const QgsBookmarkManager * >() << managerItem->manager(), context.messageBar() );
    } );
    menu->addAction( exportBookmarks );
  }
  else if ( QgsBookmarkItem *bookmarkItem = qobject_cast< QgsBookmarkItem * >( item ) )
  {
    QAction *actionZoom = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToLayer.svg" ) ), tr( "Zoom to Bookmark" ), menu );
    connect( actionZoom, &QAction::triggered, this, [bookmarkItem, context]
    {
      try
      {
        if ( !QgisApp::instance()->mapCanvas()->setReferencedExtent( bookmarkItem->bookmark().extent() ) )
        {
          context.messageBar()->pushWarning( tr( "Zoom to Bookmark" ), tr( "Bookmark extent is empty" ) );
        }
        else
        {
          QgisApp::instance()->mapCanvas()->refresh();
        }
      }
      catch ( QgsCsException & )
      {
        context.messageBar()->pushWarning( tr( "Zoom to Bookmark" ), tr( "Could not reproject bookmark extent to project CRS." ) );
      }
    } );
    menu->addAction( actionZoom );
    menu->addSeparator();

    QAction *actionEdit = new QAction( tr( "Edit Spatial Bookmark…" ), menu );
    connect( actionEdit, &QAction::triggered, this, [bookmarkItem]
    {
      QgsBookmarkEditorDialog *dlg = new QgsBookmarkEditorDialog( bookmarkItem->bookmark(), bookmarkItem->manager() == QgsProject::instance()->bookmarkManager(), QgisApp::instance(), QgisApp::instance()->mapCanvas() );
      dlg->setAttribute( Qt::WA_DeleteOnClose );
      dlg->show();
    } );
    menu->addAction( actionEdit );

    QStringList ids;
    for ( QgsDataItem *i : selectedItems )
    {
      if ( QgsBookmarkItem *b = qobject_cast< QgsBookmarkItem * >( i ) )
      {
        if ( b->manager() == bookmarkItem->manager() )
          ids << b->bookmark().id();
      }
    }

    QAction *actionDelete = new QAction( selectedItems.count() == 1 ? tr( "Delete Spatial Bookmark" ) : tr( "Delete Spatial Bookmarks" ), menu );
    connect( actionDelete, &QAction::triggered, this, [bookmarkItem, ids]
    {
      if ( ids.count() == 1 )
      {
        if ( QMessageBox::question( nullptr, QObject::tr( "Delete Spatial Bookmark" ),
                                    QObject::tr( "Are you sure you want to delete the %1 bookmark?" ).arg( bookmarkItem->name() ),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
          return;

        bookmarkItem->manager()->removeBookmark( bookmarkItem->bookmark().id() );
      }
      else
      {
        if ( QMessageBox::question( nullptr, QObject::tr( "Delete Spatial Bookmarks" ),
                                    QObject::tr( "Are you sure you want to delete the %n selected bookmark(s)?", nullptr, ids.count() ),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
          return;

        for ( const QString &id : ids )
          bookmarkItem->manager()->removeBookmark( id );
      }
    } );
    menu->addAction( actionDelete );
  }
  else if ( QgsBookmarkGroupItem *groupItem = qobject_cast< QgsBookmarkGroupItem * >( item ) )
  {
    QStringList groups;
    QgsBookmarkManager *manager = groupItem->manager();
    for ( QgsDataItem *i : selectedItems )
    {
      if ( QgsBookmarkGroupItem *g = qobject_cast< QgsBookmarkGroupItem * >( i ) )
      {
        if ( g->manager() == manager )
          groups << g->group();
      }
    }

    QAction *exportBookmarks = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSharingExport.svg" ) ), tr( "Export Spatial Bookmarks…" ), menu );
    connect( exportBookmarks, &QAction::triggered, this, [ = ]
    {
      exportBookmarksFromManagers( QList< const QgsBookmarkManager * >() << groupItem->manager(), context.messageBar(), groupItem->group() );
    } );
    menu->addAction( exportBookmarks );

    // Add spatial bookmark
    QAction *addBookmarkToGroup = new QAction( tr( "New Spatial Bookmark…" ), menu );
    const bool inProject = manager != QgsApplication::bookmarkManager();
    connect( addBookmarkToGroup, &QAction::triggered, this, [ = ]
    {
      QgisApp::instance()->newBookmark( inProject, groupItem->group() );
    } );
    menu->addAction( addBookmarkToGroup );
    menu->addSeparator();

    QAction *actionDelete = new QAction( selectedItems.count() == 1 ? tr( "Delete Bookmark Group" ) : tr( "Delete Bookmark Groups" ), menu );
    connect( actionDelete, &QAction::triggered, this, [selectedItems, groups, manager]
    {
      if ( groups.count() == 1 )
      {
        if ( QMessageBox::question( nullptr, QObject::tr( "Delete Bookmark Group" ),
                                    QObject::tr( "Are you sure you want to delete the %1 bookmark group? This will delete all bookmarks in this group." ).arg( groups.at( 0 ) ),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
          return;

        const QList<QgsBookmark> matching = manager->bookmarksByGroup( groups.at( 0 ) );
        for ( const QgsBookmark &bookmark : matching )
        {
          manager->removeBookmark( bookmark.id() );
        }
      }
      else
      {
        if ( QMessageBox::question( nullptr, QObject::tr( "Delete Bookmark Groups" ),
                                    QObject::tr( "Are you sure you want to delete the %n selected bookmark group(s)? This will delete all bookmarks in these groups.", nullptr, groups.count() ),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
          return;

        int i = 0;
        for ( const QString &g : groups )
        {
          const QList<QgsBookmark> matching = manager->bookmarksByGroup( g );
          for ( const QgsBookmark &bookmark : matching )
          {
            manager->removeBookmark( bookmark.id() );
          }
          i++;
        }
      }
    } );
    menu->addAction( actionDelete );
  }
}

bool QgsBookmarksItemGuiProvider::handleDoubleClick( QgsDataItem *item, QgsDataItemGuiContext context )
{
  if ( QgsBookmarkItem *bookmarkItem = qobject_cast< QgsBookmarkItem * >( item ) )
  {
    try
    {
      if ( !QgisApp::instance()->mapCanvas()->setReferencedExtent( bookmarkItem->bookmark().extent() ) )
      {
        context.messageBar()->pushWarning( tr( "Zoom to Bookmark" ), tr( "Bookmark extent is empty" ) );
      }
      else
      {
        QgisApp::instance()->mapCanvas()->refresh();
      }
    }
    catch ( QgsCsException & )
    {
      context.messageBar()->pushWarning( tr( "Zoom to Bookmark" ), tr( "Could not reproject bookmark extent to project CRS." ) );
    }
    return true;
  }
  return false;
}

bool QgsBookmarksItemGuiProvider::rename( QgsDataItem *item, const QString &name, QgsDataItemGuiContext context )
{
  if ( QgsBookmarkItem *bookmarkItem = qobject_cast< QgsBookmarkItem * >( item ) )
  {
    QgsBookmark bookmark = bookmarkItem->bookmark();
    bookmark.setName( name );
    if ( !bookmarkItem->manager()->updateBookmark( bookmark ) )
    {
      context.messageBar()->pushWarning( tr( "Rename Bookmark" ), tr( "Could not rename bookmark" ) );
      return true;
    }
    return true;
  }
  else if ( QgsBookmarkGroupItem *groupItem = qobject_cast< QgsBookmarkGroupItem * >( item ) )
  {
    groupItem->manager()->renameGroup( groupItem->group(), name );
    return true;
  }
  return false;
}

void QgsBookmarksItemGuiProvider::exportBookmarksFromManagers( const QList<const QgsBookmarkManager *> &managers, QgsMessageBar *messageBar, const QString &group )
{
  QgsSettings settings;

  const QString lastUsedDir = settings.value( QStringLiteral( "Windows/Bookmarks/LastUsedDirectory" ), QDir::homePath() ).toString();
  QString fileName = QFileDialog::getSaveFileName( QgisApp::instance(), tr( "Export Bookmarks" ), lastUsedDir,
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  // ensure the user never omitted the extension from the file name
  fileName = QgsFileUtils::ensureFileNameHasExtension( fileName, QStringList() << QStringLiteral( "xml" ) );

  if ( !QgsBookmarkManager::exportToFile( fileName, managers, group ) )
  {
    messageBar->pushWarning( tr( "Export Bookmarks" ), tr( "Error exporting bookmark file" ) );
  }
  else
  {
    messageBar->pushSuccess( tr( "Export Bookmarks" ), tr( "Successfully exported bookmarks to <a href=\"%1\">%2</a>" )
                             .arg( QUrl::fromLocalFile( fileName ).toString(), QDir::toNativeSeparators( fileName ) ) );
  }

  settings.setValue( QStringLiteral( "Windows/Bookmarks/LastUsedDirectory" ), QFileInfo( fileName ).path() );
}

void QgsBookmarksItemGuiProvider::importBookmarksToManager( QgsBookmarkManager *manager, QgsMessageBar *messageBar )
{
  QgsSettings settings;

  const QString lastUsedDir = settings.value( QStringLiteral( "Windows/Bookmarks/LastUsedDirectory" ), QDir::homePath() ).toString();
  const QString fileName = QFileDialog::getOpenFileName( QgisApp::instance(), tr( "Import Bookmarks" ), lastUsedDir,
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  if ( !manager->importFromFile( fileName ) )
  {
    messageBar->pushWarning( tr( "Import Bookmarks" ), tr( "Error importing bookmark file" ) );
  }
  else
  {
    messageBar->pushSuccess( tr( "Import Bookmarks" ), tr( "Bookmarks imported successfully" ) );
  }
  settings.setValue( QStringLiteral( "Windows/Bookmarks/LastUsedDirectory" ), QFileInfo( fileName ).path() );
}

//
// QgsHtmlDataItemProvider
//

QString QgsHtmlDataItemProvider::name()
{
  return QStringLiteral( "html" );
}

int QgsHtmlDataItemProvider::capabilities() const
{
  return QgsDataProvider::File;
}

QgsDataItem *QgsHtmlDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  const QFileInfo fileInfo( path );

  if ( fileInfo.suffix().compare( QLatin1String( "htm" ), Qt::CaseInsensitive ) == 0
       || fileInfo.suffix().compare( QLatin1String( "html" ), Qt::CaseInsensitive ) == 0 )
  {
    return new QgsHtmlDataItem( parentItem, fileInfo.fileName(), path );
  }
  return nullptr;
}

//
// QgsHtmlDataItemProvider
//

QgsHtmlDataItem::QgsHtmlDataItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataItem( Qgis::BrowserItemType::Custom, parent, name, path )
{
  setState( Qgis::BrowserItemState::Populated ); // no children
  setIconName( QStringLiteral( "/mIconHtml.svg" ) );
  setToolTip( QDir::toNativeSeparators( path ) );
  mCapabilities |= Qgis::BrowserItemCapability::ItemRepresentsFile;
}

bool QgsHtmlDataItem::handleDoubleClick()
{
  QDesktopServices::openUrl( QUrl::fromLocalFile( path() ) );
  return true;
}

QList<QAction *> QgsHtmlDataItem::actions( QWidget *parent )
{
  QAction *openAction = new QAction( tr( "&Open File…" ), parent );
  connect( openAction, &QAction::triggered, this, [ = ]
  {
    QDesktopServices::openUrl( QUrl::fromLocalFile( path() ) );
  } );
  return QList<QAction *>() << openAction;
}
