/***************************************************************************
                        qgsinbuiltdataitemproviders.cpp
                        ----------------------------
   begin                : October 2018
   copyright            : (C) 2018 by Nyall Dawson
   email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsinbuiltdataitemproviders.h"
#include "qgsdataitem.h"
#include "qgsdataitemguiproviderregistry.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsnative.h"
#include "qgisapp.h"
#include "qgsmessagebar.h"
#include "qgsmessagelog.h"
#include "qgsnewnamedialog.h"
#include "qgsbrowserguimodel.h"
#include "qgsbrowserdockwidget_p.h"
#include "qgsrasterlayer.h"
#include "qgsnewvectorlayerdialog.h"
#include "qgsfileutils.h"
#include "qgsapplication.h"
#include "processing/qgsprojectstylealgorithms.h"
#include "qgsstylemanagerdialog.h"
#include "qgsproviderregistry.h"
#include "qgsaddattrdialog.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsprovidermetadata.h"
#include "qgssourceselectproviderregistry.h"
#include "qgssourceselectprovider.h"
#include "qgsnewvectortabledialog.h"
#include "qgscolordialog.h"
#include "qgsdirectoryitem.h"
#include "qgsdatacollectionitem.h"
#include "qgsdatabaseschemaitem.h"
#include "qgsfavoritesitem.h"
#include "qgslayeritem.h"
#include "qgsprojectitem.h"
#include "qgsfieldsitem.h"
#include "qgsfielddomainsitem.h"
#include "qgsfielddomain.h"
#include "qgsconnectionsitem.h"
#include "qgsqueryresultwidget.h"
#include "qgsprojectutils.h"
#include "qgsvariantutils.h"
#include "qgsfielddomainwidget.h"
#include "qgsgeopackagedataitems.h"
#include "qgsfilebaseddataitemprovider.h"
#include "qgsvectorlayerexporter.h"
#include "qgsmessageoutput.h"
#include "qgsrelationshipsitem.h"
#include "qgsprovidersqlquerybuilder.h"
#include "qgsdbrelationshipwidget.h"
#include "qgsdbqueryhistoryprovider.h"
#include "qgshistoryproviderregistry.h"

#include <QFileInfo>
#include <QMenu>
#include <QInputDialog>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QUrl>
#include <memory>

QString QgsAppDirectoryItemGuiProvider::name()
{
  return QStringLiteral( "directory_items" );
}

void QgsAppDirectoryItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext context )
{
  if ( item->type() != Qgis::BrowserItemType::Directory )
    return;

  QgsDirectoryItem *directoryItem = qobject_cast< QgsDirectoryItem * >( item );

  QgsSettings settings;

  QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
  connect( actionRefresh, &QAction::triggered, this, [ = ] { directoryItem->refresh(); } );
  menu->addAction( actionRefresh );

  menu->addSeparator();

  QMenu *newMenu = new QMenu( tr( "New" ), menu );

  QAction *createFolder = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionNewFolder.svg" ) ), tr( "Directory…" ), menu );
  connect( createFolder, &QAction::triggered, this, [ = ]
  {
    bool ok = false;

    const QString name = QInputDialog::getText( QgisApp::instance(), tr( "Create Directory" ), tr( "Directory name" ), QLineEdit::Normal, QString(), &ok );
    if ( ok && !name.isEmpty() )
    {
      QDir dir( directoryItem->dirPath() );
      if ( QFileInfo::exists( dir.absoluteFilePath( name ) ) )
      {
        notify( tr( "Create Directory" ), tr( "The path “%1” already exists." ).arg( QDir::toNativeSeparators( dir.absoluteFilePath( name ) ) ), context, Qgis::MessageLevel::Warning );
      }
      else if ( !dir.mkdir( name ) )
      {
        notify( tr( "Create Directory" ), tr( "Could not create directory “%1”." ).arg( QDir::toNativeSeparators( dir.absoluteFilePath( name ) ) ), context, Qgis::MessageLevel::Critical );
      }
      else
      {
        directoryItem->refresh();
      }
    }
  } );
  newMenu->addAction( createFolder );

  QAction *createGpkg = new QAction( tr( "GeoPackage…" ), newMenu );
  createGpkg->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionNewGeoPackageLayer.svg" ) ) );
  connect( createGpkg, &QAction::triggered, this, [ = ]
  {
    QDir dir( directoryItem->dirPath() );
    QString newName = tr( "New GeoPackage.gpkg" );
    int i = 1;
    while ( QFileInfo::exists( dir.absoluteFilePath( newName ) ) )
    {
      i += 1;
      newName = tr( "New GeoPackage (%1).gpkg" ).arg( i );
    }

    const QString fileName = dir.absoluteFilePath( newName );
    if ( QgsProviderMetadata *ogrMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) )
    {
      QString error;
      if ( ! ogrMetadata->createDatabase( fileName, error ) )
      {
        context.messageBar()->pushCritical( tr( "New GeoPackage" ), tr( "GeoPackage creation failed: %1" ).arg( error ) );
      }
      else
      {
        QObject *contextObject = new QObject();
        connect( directoryItem, &QgsDataItem::stateChanged, contextObject, [contextObject, fileName, context]( QgsDataItem * item, Qgis::BrowserItemState )
        {
          if ( item->state() == Qgis::BrowserItemState::Populated )
          {
            // find the new item and select it
            const QVector<QgsDataItem *> children = item->children();
            for ( QgsDataItem *child : children )
            {
              if ( child->path() == fileName )
              {
                if ( QgsBrowserTreeView *view = context.view() )
                {
                  if ( view->setSelectedItem( child ) )
                    view->edit( view->currentIndex() );
                }
                break;
              }
            }
            contextObject->deleteLater();
          }
        } );
        directoryItem->refresh();
      }
    }
  } );
  newMenu->addAction( createGpkg );

  QAction *createShp = new QAction( tr( "ShapeFile…" ), newMenu );
  createShp->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionNewVectorLayer.svg" ) ) );
  connect( createShp, &QAction::triggered, this, [ = ]
  {
    QString enc;
    QDir dir( directoryItem->dirPath() );
    QString error;
    const QString newFile = QgsNewVectorLayerDialog::execAndCreateLayer( error, QgisApp::instance(), dir.filePath( QStringLiteral( "new_layer.shp" ) ), &enc, QgsProject::instance()->defaultCrsForNewLayers() );
    if ( !newFile.isEmpty() )
    {
      context.messageBar()->pushSuccess( tr( "New ShapeFile" ), tr( "Created <a href=\"%1\">%2</a>" ).arg(
                                           QUrl::fromLocalFile( newFile ).toString(), QDir::toNativeSeparators( newFile ) ) );
      item->refresh();
    }
    else if ( !error.isEmpty() )
    {
      context.messageBar()->pushCritical( tr( "New ShapeFile" ), tr( "Layer creation failed: %1" ).arg( error ) );
    }
  } );
  newMenu->addAction( createShp );

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,6,0)
  QAction *createFgdb = new QAction( tr( "ESRI FileGeodatabase…" ), newMenu );
  createFgdb->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionNewFileGeodatabase.svg" ) ) );
  connect( createFgdb, &QAction::triggered, this, [ = ]
  {
    QDir dir( directoryItem->dirPath() );
    QString newName = tr( "New File Geodatabase.gdb" );
    int i = 1;
    while ( QFileInfo::exists( dir.absoluteFilePath( newName ) ) )
    {
      i += 1;
      newName = tr( "New File Geodatabase (%1).gdb" ).arg( i );
    }

    const QString fileName = dir.absoluteFilePath( newName );
    if ( QgsProviderMetadata *ogrMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "ogr" ) ) )
    {
      QString error;
      if ( ! ogrMetadata->createDatabase( fileName, error ) )
      {
        context.messageBar()->pushCritical( tr( "New ESRI File Geodatabase" ), tr( "Database creation failed: %1" ).arg( error ) );
      }
      else
      {
        QObject *contextObject = new QObject();
        connect( directoryItem, &QgsDataItem::stateChanged, contextObject, [contextObject, fileName, context]( QgsDataItem * item, Qgis::BrowserItemState )
        {
          if ( item->state() == Qgis::BrowserItemState::Populated )
          {
            // find the new item and select it
            const QVector<QgsDataItem *> children = item->children();
            for ( QgsDataItem *child : children )
            {
              if ( child->path() == fileName )
              {
                if ( QgsBrowserTreeView *view = context.view() )
                {
                  if ( view->setSelectedItem( child ) )
                    view->edit( view->currentIndex() );
                }
                break;
              }
            }
            contextObject->deleteLater();
          }
        } );
        directoryItem->refresh();
      }
    }
  } );
  newMenu->addAction( createFgdb );
#endif

  menu->addMenu( newMenu );

  menu->addSeparator();

  bool inFavDirs = item->parent() && item->parent()->type() == Qgis::BrowserItemType::Favorites;
  if ( item->parent() && !inFavDirs )
  {
    // only non-root directories can be added as favorites
    QAction *addAsFavorite = new QAction( tr( "Add as a Favorite" ), menu );
    addAsFavorite->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFavorites.svg" ) ) );
    menu->addAction( addAsFavorite );
    connect( addAsFavorite, &QAction::triggered, this, [ = ]
    {
      addFavorite( directoryItem );
    } );
  }
  else if ( inFavDirs )
  {
    if ( QgsFavoriteItem *favoriteItem = qobject_cast< QgsFavoriteItem * >( item ) )
    {
      QAction *actionRename = new QAction( tr( "Rename Favorite…" ), menu );
      connect( actionRename, &QAction::triggered, this, [ = ]
      {
        renameFavorite( favoriteItem );
      } );
      menu->addAction( actionRename );

      QAction *removeFavoriteAction = new QAction( tr( "Remove Favorite" ), menu );
      connect( removeFavoriteAction, &QAction::triggered, this, [ = ]
      {
        removeFavorite( favoriteItem );
      } );
      menu->addAction( removeFavoriteAction );
      menu->addSeparator();
    }
  }
  QAction *hideAction = new QAction( tr( "Hide from Browser" ), menu );
  connect( hideAction, &QAction::triggered, this, [ = ]
  {
    hideDirectory( directoryItem );
  } );
  menu->addAction( hideAction );

  QMenu *hiddenMenu = new QMenu( tr( "Hidden Items" ), menu );
  int count = 0;
  const QStringList hiddenPathList = settings.value( QStringLiteral( "/browser/hiddenPaths" ) ).toStringList();
  static int MAX_HIDDEN_ENTRIES = 5;
  for ( const QString &path : hiddenPathList )
  {
    QAction *action = new QAction( QDir::toNativeSeparators( path ), hiddenMenu );
    connect( action, &QAction::triggered, this, [ = ]
    {
      QgsSettings s;
      QStringList pathsList = s.value( QStringLiteral( "/browser/hiddenPaths" ) ).toStringList();
      pathsList.removeAll( path );
      s.setValue( QStringLiteral( "/browser/hiddenPaths" ), pathsList );

      // get parent path and refresh corresponding node
      int idx = path.lastIndexOf( QLatin1Char( '/' ) );
      if ( idx != -1 && path.count( QStringLiteral( "/" ) ) > 1 )
      {
        QString parentPath = path.left( idx );
        QgisApp::instance()->browserModel()->refresh( parentPath );
      }
      else
      {
        // top-level (drive or root) node
        QgisApp::instance()->browserModel()->refreshDrives();
      }
    } );
    hiddenMenu->addAction( action );
    count += 1;
    if ( count == MAX_HIDDEN_ENTRIES )
    {
      break;
    }
  }

  if ( hiddenPathList.size() > MAX_HIDDEN_ENTRIES )
  {
    hiddenMenu->addSeparator();

    QAction *moreAction = new QAction( tr( "Show More…" ), hiddenMenu );
    connect( moreAction, &QAction::triggered, this, [ = ]
    {
      QgisApp::instance()->showOptionsDialog( QgisApp::instance(), QStringLiteral( "mOptionsPageDataSources" ) );
    } );
    hiddenMenu->addAction( moreAction );
  }
  if ( count > 0 )
  {
    menu->addMenu( hiddenMenu );
  }

  QAction *actionSetIconColor = new QAction( tr( "Set Color…" ), menu );
  if ( directoryItem->iconColor().isValid() )
  {
    const QPixmap icon = QgsColorButton::createMenuIcon( directoryItem->iconColor(), true );
    actionSetIconColor->setIcon( icon );
  }
  connect( actionSetIconColor, &QAction::triggered, this, [ = ]
  {
    changeDirectoryColor( directoryItem );
  } );
  menu->addAction( actionSetIconColor );
  if ( directoryItem->iconColor().isValid() )
  {
    QAction *actionClearIconColor = new QAction( tr( "Clear Custom Color" ), menu );
    connect( actionClearIconColor, &QAction::triggered, this, [ = ]
    {
      clearDirectoryColor( directoryItem );
    } );
    menu->addAction( actionClearIconColor );
  }

  QMenu *scanningMenu = new QMenu( tr( "Scanning" ), menu );

  QAction *monitorAction = new QAction( tr( "Monitor for Changes" ), scanningMenu );
  connect( monitorAction, &QAction::triggered, this, [ = ]
  {
    toggleMonitor( directoryItem );
  } );
  monitorAction->setCheckable( true );
  monitorAction->setChecked( directoryItem->isMonitored() );
  scanningMenu->addAction( monitorAction );

  QAction *fastScanAction = new QAction( tr( "Fast Scan this Directory" ), scanningMenu );
  connect( fastScanAction, &QAction::triggered, this, [ = ]
  {
    toggleFastScan( directoryItem );
  } );
  fastScanAction->setCheckable( true );
  fastScanAction->setChecked( settings.value( QStringLiteral( "qgis/scanItemsFastScanUris" ),
                              QStringList() ).toStringList().contains( item->path() ) );

  scanningMenu->addAction( fastScanAction );
  menu->addMenu( scanningMenu );

  menu->addSeparator();

  QAction *openFolder = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mIconFolder.svg" ) ), tr( "Open Directory…" ), menu );
  connect( openFolder, &QAction::triggered, this, [ = ]
  {
    QDesktopServices::openUrl( QUrl::fromLocalFile( directoryItem->dirPath() ) );
  } );
  menu->addAction( openFolder );

  if ( QgsGui::nativePlatformInterface()->capabilities() & QgsNative::NativeOpenTerminalAtPath )
  {
    QAction *openTerminal = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionTerminal.svg" ) ), tr( "Open in Terminal…" ), menu );
    connect( openTerminal, &QAction::triggered, this, [ = ]
    {
      QgsGui::nativePlatformInterface()->openTerminalAtPath( directoryItem->dirPath() );
    } );
    menu->addAction( openTerminal );
    menu->addSeparator();
  }

  QAction *propertiesAction = new QAction( tr( "Properties…" ), menu );
  connect( propertiesAction, &QAction::triggered, this, [ = ]
  {
    showProperties( directoryItem, context );
  } );
  menu->addAction( propertiesAction );

  if ( QgsGui::nativePlatformInterface()->capabilities() & QgsNative::NativeFilePropertiesDialog )
  {
    if ( QgsDirectoryItem *dirItem = qobject_cast< QgsDirectoryItem * >( item ) )
    {
      QAction *action = menu->addAction( tr( "Directory Properties…" ) );
      connect( action, &QAction::triggered, dirItem, [ dirItem ]
      {
        QgsGui::nativePlatformInterface()->showFileProperties( dirItem->dirPath() );
      } );
    }
  }
}

void QgsAppDirectoryItemGuiProvider::addFavorite( QgsDirectoryItem *item )
{
  if ( !item )
    return;

  QgisApp::instance()->browserModel()->addFavoriteDirectory( item->dirPath() );
}

void QgsAppDirectoryItemGuiProvider::removeFavorite( QgsFavoriteItem *favorite )
{
  QgisApp::instance()->browserModel()->removeFavorite( favorite );
}

void QgsAppDirectoryItemGuiProvider::renameFavorite( QgsFavoriteItem *favorite )
{
  QgsNewNameDialog dlg( tr( "favorite “%1”" ).arg( favorite->name() ), favorite->name() );
  dlg.setWindowTitle( tr( "Rename Favorite" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == favorite->name() )
    return;

  favorite->rename( dlg.name() );
}

void QgsAppDirectoryItemGuiProvider::changeDirectoryColor( QgsDirectoryItem *item )
{
  const QColor oldColor = item->iconColor();

  const QColor color = QgsColorDialog::getColor( oldColor, QgisApp::instance(), tr( "Set Color" ), true );
  if ( !color.isValid() )
    return;

  // store new color for directory
  QgsDirectoryItem::setCustomColor( item->dirPath(), color );
  // and update item's color immediately
  item->setIconColor( color );
}

void QgsAppDirectoryItemGuiProvider::clearDirectoryColor( QgsDirectoryItem *item )
{
  QgsDirectoryItem::setCustomColor( item->dirPath(), QColor() );
  // and update item's color immediately
  item->setIconColor( QColor() );
}

void QgsAppDirectoryItemGuiProvider::hideDirectory( QgsDirectoryItem *item )
{
  if ( ! item )
    return;

  QgisApp::instance()->browserModel()->hidePath( item );
}

void QgsAppDirectoryItemGuiProvider::toggleFastScan( QgsDirectoryItem *item )
{
  QgsSettings settings;
  QStringList fastScanDirs = settings.value( QStringLiteral( "qgis/scanItemsFastScanUris" ),
                             QStringList() ).toStringList();
  int idx = fastScanDirs.indexOf( item->path() );
  if ( idx != -1 )
  {
    fastScanDirs.removeAt( idx );
  }
  else
  {
    fastScanDirs << item->path();
  }
  settings.setValue( QStringLiteral( "qgis/scanItemsFastScanUris" ), fastScanDirs );
}

void QgsAppDirectoryItemGuiProvider::toggleMonitor( QgsDirectoryItem *item )
{
  if ( item->isMonitored() )
    item->setMonitoring( Qgis::BrowserDirectoryMonitoring::NeverMonitor );
  else
    item->setMonitoring( Qgis::BrowserDirectoryMonitoring::AlwaysMonitor );
}

void QgsAppDirectoryItemGuiProvider::showProperties( QgsDirectoryItem *item, QgsDataItemGuiContext context )
{
  if ( ! item )
    return;

  QgsBrowserPropertiesDialog *dialog = new QgsBrowserPropertiesDialog( QStringLiteral( "browser" ), QgisApp::instance() );
  dialog->setAttribute( Qt::WA_DeleteOnClose );

  dialog->setItem( item, context );
  dialog->show();
}


//
// QgsAppFileItemGuiProvider
//

QString QgsAppFileItemGuiProvider::name()
{
  return QStringLiteral( "file_items" );
}

void QgsAppFileItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context )
{

  QAction *openDataSourceManagerAction = nullptr;
  if ( const auto layerItem = qobject_cast< QgsLayerItem * >( item ) )
  {
    const QList<QgsSourceSelectProvider *> sourceSelectProviders { QgsGui::sourceSelectProviderRegistry()->providersByKey( layerItem->providerKey() ) };
    if ( ! sourceSelectProviders.isEmpty() && sourceSelectProviders.first()->capabilities().testFlag( QgsSourceSelectProvider::Capability::ConfigureFromUri ) )
    {
      openDataSourceManagerAction = new QAction( tr( "Open with Data Source Manager…" ), menu );
      connect( openDataSourceManagerAction, &QAction::triggered, this, [ = ]
      {
        QString pageName { layerItem->providerKey() };
        // GPKG special handling
        if ( qobject_cast<QgsGeoPackageVectorLayerItem *>( layerItem ) )
        {
          pageName = QStringLiteral( "GeoPackage" );
        }
        QgisApp::instance()->dataSourceManager( pageName, layerItem->uri() );
      } );
    }
  }
  else if ( const auto collectionItem = qobject_cast< QgsFileDataCollectionItem * >( item ) )
  {
    QSet<QString> providerKeys;
    const QList<QgsProviderSublayerDetails> sublayers { collectionItem->sublayers() };
    for ( const auto &sublayer : std::as_const( sublayers ) )
    {
      providerKeys.insert( sublayer.providerKey() );
    }

    if ( providerKeys.count() == 1 )
    {
      const QgsProviderSublayerDetails &sublayer {sublayers.first() };
      openDataSourceManagerAction = new QAction( tr( "Open with Data Source Manager…" ), menu );
      connect( openDataSourceManagerAction, &QAction::triggered, this, [ = ]
      {
        QString pageName { sublayer.providerKey() };
        // GPKG special handling
        if ( sublayer.driverName() ==  QStringLiteral( "GeoPackage" ) )
        {
          pageName = QStringLiteral( "GeoPackage" );
        }
        else if ( sublayer.driverName() ==  QStringLiteral( "SQLite" ) )
        {
          pageName = QStringLiteral( "Spatialite" );
        }
        QgisApp::instance()->dataSourceManager( pageName, layerItem->uri() );
      } );
    }
  }

  if ( openDataSourceManagerAction )
  {
    menu->addAction( openDataSourceManagerAction );
    menu->addSeparator();
  }

  if ( item->capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile )
  {

    // Check for certain file items
    const QString filename = item->path();
    const QFileInfo fi( filename );
    if ( !filename.isEmpty() )
    {
      const static QList< std::pair< QString, QString > > sStandardFileTypes =
      {
        { QStringLiteral( "pdf" ), QObject::tr( "Document" )},
        { QStringLiteral( "xls" ), QObject::tr( "Spreadsheet" )},
        { QStringLiteral( "xlsx" ), QObject::tr( "Spreadsheet" )},
        { QStringLiteral( "ods" ), QObject::tr( "Spreadsheet" )},
        { QStringLiteral( "csv" ), QObject::tr( "CSV File" )},
        { QStringLiteral( "txt" ), QObject::tr( "Text File" )},
        { QStringLiteral( "png" ), QObject::tr( "PNG Image" )},
        { QStringLiteral( "jpg" ), QObject::tr( "JPEG Image" )},
        { QStringLiteral( "jpeg" ), QObject::tr( "JPEG Image" )},
        { QStringLiteral( "tif" ), QObject::tr( "TIFF Image" )},
        { QStringLiteral( "tiff" ), QObject::tr( "TIFF Image" )},
        { QStringLiteral( "svg" ), QObject::tr( "SVG File" )}
      };
      for ( const auto &it : sStandardFileTypes )
      {
        const QString ext = it.first;
        const QString name = it.second;
        if ( fi.suffix().compare( ext, Qt::CaseInsensitive ) == 0 )
        {
          QAction *viewAction = new QAction( tr( "Open %1 Externally…" ).arg( name ), menu );
          connect( viewAction, &QAction::triggered, this, [ = ]
          {
            QDesktopServices::openUrl( QUrl::fromLocalFile( filename ) );
          } );

          // we want this action to be at the top
          QAction *beforeAction = menu->actions().value( 0 );
          if ( beforeAction )
          {
            menu->insertAction( beforeAction, viewAction );
            menu->insertSeparator( beforeAction );
          }
          else
          {
            menu->addAction( viewAction );
            menu->addSeparator();
          }
          // will only find one!
          break;
        }
      }
    }

    if ( qobject_cast< QgsDataCollectionItem * >( item ) )
    {
      QAction *actionRefresh = new QAction( QObject::tr( "Refresh" ), menu );
      connect( actionRefresh, &QAction::triggered, item, [item] { item->refresh(); } );
      QAction *separatorAction = new QAction( menu );
      separatorAction->setSeparator( true );
      if ( !menu->actions().empty() )
      {
        menu->insertAction( menu->actions().constFirst(), separatorAction );
        menu->insertAction( menu->actions().constFirst(), actionRefresh );
      }
      else
      {
        menu->addAction( actionRefresh );
        menu->addAction( separatorAction );
      }
    }

    QMenu *manageFileMenu = new QMenu( tr( "Manage" ), menu );

    QStringList selectedFiles;
    QList< QPointer< QgsDataItem > > selectedParents;
    for ( QgsDataItem *selectedItem : selectedItems )
    {
      if ( selectedItem->capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile )
      {
        selectedFiles.append( selectedItem->path() );
        selectedParents << selectedItem->parent();
      }
    }

    if ( selectedFiles.size() == 1 )
    {
      const QString renameText = tr( "Rename “%1”…" ).arg( fi.fileName() );
      QAction *renameAction = new QAction( renameText, menu );
      connect( renameAction, &QAction::triggered, this, [ = ]
      {
        const QString oldPath = selectedFiles.value( 0 );
        const QStringList existingNames = QFileInfo( oldPath ).dir().entryList();

        QgsNewNameDialog dlg( tr( "file" ), QFileInfo( oldPath ).fileName(), QStringList(), existingNames, Qt::CaseInsensitive, menu );
        dlg.setWindowTitle( tr( "Rename %1" ).arg( QFileInfo( oldPath ).fileName() ) );
        dlg.setHintString( tr( "Rename “%1” to" ).arg( QFileInfo( oldPath ).fileName() ) );
        dlg.setOverwriteEnabled( false );
        dlg.setConflictingNameWarning( tr( "A file with this name already exists." ) );
        if ( dlg.exec() != QDialog::Accepted || dlg.name().isEmpty() )
          return;

        QString newName = dlg.name();
        if ( QFileInfo( newName ).suffix().isEmpty() )
          newName = newName + '.' + QFileInfo( oldPath ).suffix();

        rename( oldPath, newName, context, selectedParents );
      } );
      manageFileMenu->addAction( renameAction );
    }

    const QString deleteText = selectedFiles.count() == 1 ? tr( "Delete “%1”…" ).arg( fi.fileName() )
                               : tr( "Delete Selected Files…" );
    QAction *deleteAction = new QAction( deleteText, menu );
    connect( deleteAction, &QAction::triggered, this, [ = ]
    {
      // Check if the files correspond to paths in the project
      QList<QgsMapLayer *> layersList;
      for ( const QString &path : std::as_const( selectedFiles ) )
      {
        layersList << QgsProjectUtils::layersMatchingPath( QgsProject::instance(), path );
      }

      // now expand out the list of files to include all sidecar files (e.g. .aux.xml files)
      QSet< QString > allFilesWithSidecars;
      for ( const QString &file : std::as_const( selectedFiles ) )
      {
        allFilesWithSidecars.insert( file );
        allFilesWithSidecars.unite( QgsFileUtils::sidecarFilesForPath( file ) );
      }
      QStringList sortedAllFilesWithSidecars( qgis::setToList( allFilesWithSidecars ) );
      std::sort( sortedAllFilesWithSidecars.begin(), sortedAllFilesWithSidecars.end(), []( const QString & a, const QString & b )
      {
        return a.compare( b, Qt::CaseInsensitive ) < 0;
      } );

      bool removeLayers = false;
      if ( layersList.empty() )
      {
        // generic warning
        QMessageBox message( QMessageBox::Warning, sortedAllFilesWithSidecars.size() > 1 ? tr( "Delete Files" ) : tr( "Delete %1" ).arg( QFileInfo( selectedFiles.at( 0 ) ).fileName() ),
                             sortedAllFilesWithSidecars.size() > 1 ? tr( "Permanently delete %n file(s)?", nullptr, sortedAllFilesWithSidecars.size() )
                             : tr( "Permanently delete “%1”?" ).arg( QFileInfo( selectedFiles.at( 0 ) ).fileName() ),
                             QMessageBox::Yes | QMessageBox::No );
        message.setDefaultButton( QMessageBox::No );

        if ( sortedAllFilesWithSidecars.size() > 1 )
        {
          QStringList fileNames;
          fileNames.reserve( sortedAllFilesWithSidecars.size() );
          for ( const QString &file : std::as_const( sortedAllFilesWithSidecars ) )
          {
            fileNames << QFileInfo( file ).fileName();
          }
          message.setDetailedText( tr( "The following files will be deleted:" ) + QStringLiteral( "\n\n• %1" ).arg( fileNames.join( QStringLiteral( "\n• " ) ) ) );
        }

        int res = message.exec();
        if ( res == QMessageBox::No )
          return;
      }
      else
      {
        QMessageBox message( QMessageBox::Warning, sortedAllFilesWithSidecars.size() > 1 ? tr( "Delete Files" ) : tr( "Delete %1" ).arg( QFileInfo( selectedFiles.at( 0 ) ).fileName() ),
                             sortedAllFilesWithSidecars.size() > 1 ? tr( "One or more selected files exist in the current project. Are you sure you want to delete these files?" )
                             : tr( "The file %1 exists in the current project. Are you sure you want to delete it?" ).arg( QFileInfo( selectedFiles.at( 0 ) ).fileName() ),
                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
        message.setDefaultButton( QMessageBox::Cancel );
        message.setButtonText( QMessageBox::Yes, tr( "Delete and Remove Layers" ) );
        message.setButtonText( QMessageBox::No, tr( "Delete and Retain Layers" ) );

        QStringList layerNames;
        layerNames.reserve( layersList.size() );
        for ( const QgsMapLayer *layer : std::as_const( layersList ) )
        {
          layerNames << layer->name();
        }
        QString detailedText = tr( "The following layers will be affected:" ) + QStringLiteral( "\n\n• %1" ).arg( layerNames.join( QStringLiteral( "\n• " ) ) );

        if ( sortedAllFilesWithSidecars.size() > 1 )
        {
          QStringList fileNames;
          fileNames.reserve( sortedAllFilesWithSidecars.size() );
          for ( const QString &file : std::as_const( sortedAllFilesWithSidecars ) )
          {
            fileNames << QFileInfo( file ).fileName();
          }
          detailedText += QStringLiteral( "\n\n" ) + tr( "The following files will be deleted:" ) + QStringLiteral( "\n\n• %1" ).arg( fileNames.join( QStringLiteral( "\n• " ) ) );
        }
        message.setDetailedText( detailedText );

        int res = message.exec();
        if ( res == QMessageBox::Cancel )
          return;

        if ( res == QMessageBox::Yes )
          removeLayers = true;
      }

      QStringList errors;
      errors.reserve( allFilesWithSidecars.size() );
      for ( const QString &path : std::as_const( allFilesWithSidecars ) )
      {
        // delete file
        const QFileInfo fi( path );
        if ( fi.isFile() )
        {
          if ( !QFile::remove( path ) )
            errors << path;
        }
        else if ( fi.isDir() )
        {
          QDir dir( path );
          if ( !dir.removeRecursively() )
            errors << path;
        }
      }

      for ( const QPointer< QgsDataItem > &parent : selectedParents )
      {
        if ( parent )
          parent->refresh();
      }

      if ( !layersList.empty() )
      {
        if ( removeLayers )
        {
          QgsProject::instance()->removeMapLayers( layersList );
          QgsProject::instance()->setDirty( true );
        }
        else
        {
          // we just update the layer source to get it to recognize that it's now broken in the UI
          for ( QgsMapLayer *layer : std::as_const( layersList ) )
          {
            layer->setDataSource( layer->source(), layer->name(), layer->providerType() );
          }
        }
      }

      if ( !errors.empty() )
      {
        if ( errors.size() == 1 )
          notify( QString(), tr( "Could not delete %1" ).arg( QFileInfo( errors.at( 0 ) ).fileName() ), context, Qgis::MessageLevel::Critical );
        else
          notify( QString(), tr( "Could not delete %n file(s)", nullptr, errors.size() ), context, Qgis::MessageLevel::Critical );
      }
    } );
    manageFileMenu->addAction( deleteAction );

    menu->addMenu( manageFileMenu );

    if ( QgsGui::nativePlatformInterface()->capabilities() & QgsNative::NativeFilePropertiesDialog )
    {
      if ( QFileInfo::exists( item->path() ) )
      {
        if ( !menu->isEmpty() )
          menu->addSeparator();

        QAction *showInFilesAction = menu->addAction( tr( "Show in Files" ) );
        connect( showInFilesAction, &QAction::triggered, this, [ = ]
        {
          QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( item->path() );
        } );

        QAction *filePropertiesAction = menu->addAction( tr( "File Properties…" ) );
        connect( filePropertiesAction, &QAction::triggered, this, [ = ]
        {
          QgsGui::nativePlatformInterface()->showFileProperties( item->path() );
        } );
      }
    }
  }
}

int QgsAppFileItemGuiProvider::precedenceWhenPopulatingMenus() const
{
  // we want this provider to be called last -- file items should naturally always appear at the bottom of the menu.
  return std::numeric_limits< int >::max();
}

bool QgsAppFileItemGuiProvider::rename( QgsDataItem *item, const QString &name, QgsDataItemGuiContext context )
{
  if ( !( item->capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile ) )
    return false;

  const QString oldPath = item->path();

  QString newName = name;

  if ( QFileInfo( newName ).suffix().isEmpty() )
    newName = newName + '.' + QFileInfo( oldPath ).suffix();

  const QString newPath = QFileInfo( oldPath ).dir().filePath( newName );
  if ( newPath == oldPath )
    return false; // don't need to do anything

  const QStringList existingNames = QFileInfo( oldPath ).dir().entryList();
  if ( existingNames.contains( newName ) )
  {
    notify( QString(), tr( "Cannot rename to “%1”: A file with this name already exists" ).arg( newName ), context, Qgis::MessageLevel::Warning );
    return false;
  }

  return rename( oldPath, newName, context, { item->parent() } );
}

bool QgsAppFileItemGuiProvider::rename( const QString &oldPath, const QString &newName, QgsDataItemGuiContext context, const QList<QPointer<QgsDataItem> > &parentItems )
{
  // Check if the file corresponds to paths in the project
  const QList<QgsMapLayer *> layersList = QgsProjectUtils::layersMatchingPath( QgsProject::instance(), oldPath );

  const QString newPath = QFileInfo( oldPath ).dir().filePath( newName );

  bool updateLayers = false;
  if ( !layersList.empty() )
  {
    QMessageBox message( QMessageBox::Warning, tr( "Rename %1" ).arg( QFileInfo( oldPath ).fileName() ),
                         tr( "The file %1 exists in the current project. Are you sure you want to rename it?" ).arg( QFileInfo( oldPath ).fileName() ),
                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );
    message.setDefaultButton( QMessageBox::Cancel );
    message.setButtonText( QMessageBox::Yes, tr( "Rename and Update Layer Paths" ) );
    message.setButtonText( QMessageBox::No, tr( "Rename but Leave Layer Paths" ) );

    QStringList layerNames;
    layerNames.reserve( layersList.size() );
    for ( const QgsMapLayer *layer : std::as_const( layersList ) )
    {
      layerNames << layer->name();
    }
    const QString detailedText = tr( "The following layers will be affected:" ) + QStringLiteral( "\n\n• %1" ).arg( layerNames.join( QStringLiteral( "\n• " ) ) );
    message.setDetailedText( detailedText );

    int res = message.exec();
    if ( res == QMessageBox::Cancel )
      return false;

    if ( res == QMessageBox::Yes )
      updateLayers = true;
  }

  QString error;
  const bool result = QgsFileUtils::renameDataset( oldPath, newPath, error );

  for ( const QPointer< QgsDataItem > &parentItem : parentItems )
  {
    if ( parentItem )
      parentItem->refresh();
  }

  if ( !layersList.empty() )
  {
    if ( updateLayers )
    {
      QgsProjectUtils::updateLayerPath( QgsProject::instance(), oldPath, newPath );
      QgsProject::instance()->setDirty( true );
    }
    else
    {
      // we just update the layer source to get it to recognize that it's now broken in the UI
      for ( QgsMapLayer *layer : std::as_const( layersList ) )
      {
        layer->setDataSource( layer->source(), layer->name(), layer->providerType() );
      }
    }
  }

  if ( !result )
  {
    notify( QString(), error, context, Qgis::MessageLevel::Critical );
  }

  return true;
}

//
// QgsProjectHomeItemGuiProvider
//

QString QgsProjectHomeItemGuiProvider::name()
{
  return QStringLiteral( "project_home_item" );
}

void QgsProjectHomeItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( !qobject_cast< QgsProjectHomeItem * >( item ) )
    return;

  if ( !menu->actions().empty() )
    menu->insertSeparator( menu->actions().at( 0 ) );

  QAction *setHome = new QAction( tr( "Set Project Home…" ), menu );
  connect( setHome, &QAction::triggered, this, [ = ]
  {
    QString oldHome = QgsProject::instance()->homePath();
    QString newPath = QFileDialog::getExistingDirectory( QgisApp::instance(), tr( "Select Project Home Directory" ), oldHome );
    if ( !newPath.isEmpty() )
    {
      QgsProject::instance()->setPresetHomePath( newPath );
    }
  } );

  // ensure item is the first one shown
  if ( !menu->actions().empty() )
    menu->insertAction( menu->actions().at( 0 ), setHome );
  else
    menu->addAction( setHome );
}

//
// QgsFavoritesItemGuiProvider
//

QString QgsFavoritesItemGuiProvider::name()
{
  return QStringLiteral( "favorites_item" );
}

void QgsFavoritesItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( item->type() != Qgis::BrowserItemType::Favorites )
    return;

  QAction *addAction = new QAction( tr( "Add a Directory…" ), menu );
  connect( addAction, &QAction::triggered, this, [ = ]
  {
    QString directory = QFileDialog::getExistingDirectory( QgisApp::instance(), tr( "Add Directory to Favorites" ) );
    if ( !directory.isEmpty() )
    {
      QgisApp::instance()->browserModel()->addFavoriteDirectory( directory );
    }
  } );
  menu->addAction( addAction );
}

//
// QgsLayerItemGuiProvider
//

QString QgsLayerItemGuiProvider::name()
{
  return QStringLiteral( "layer_item" );
}

void QgsLayerItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context )
{
  if ( item->type() != Qgis::BrowserItemType::Layer )
    return;

  QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item );
  if ( layerItem && ( layerItem->mapLayerType() == Qgis::LayerType::Vector ||
                      layerItem->mapLayerType() == Qgis::LayerType::Raster ) )
  {
    QMenu *exportMenu = new QMenu( tr( "Export Layer" ), menu );

    // if there's a "Manage" menu, we want this to come just before it
    QAction *beforeAction = nullptr;
    QList<QAction *> actions = menu->actions();
    for ( QAction *action : std::as_const( actions ) )
    {
      if ( action->text() == tr( "Manage" ) )
      {
        beforeAction = action;
        break;
      }
    }
    if ( beforeAction )
      menu->insertMenu( beforeAction, exportMenu );
    else
      menu->addMenu( exportMenu );

    QAction *toFileAction = new QAction( tr( "To File…" ), exportMenu );
    exportMenu->addAction( toFileAction );
    connect( toFileAction, &QAction::triggered, layerItem, [ layerItem ]
    {
      switch ( layerItem->mapLayerType() )
      {
        case Qgis::LayerType::Vector:
        {
          const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
          std::unique_ptr<QgsVectorLayer> layer( new QgsVectorLayer( layerItem->uri(), layerItem->name(), layerItem->providerKey(), options ) );
          if ( layer && layer->isValid() )
          {
            QgisApp::instance()->saveAsFile( layer.get(), false, false );
          }
          break;
        }

        case Qgis::LayerType::Raster:
        {
          std::unique_ptr<QgsRasterLayer> layer( new QgsRasterLayer( layerItem->uri(), layerItem->name(), layerItem->providerKey() ) );
          if ( layer && layer->isValid() )
          {
            QgisApp::instance()->saveAsFile( layer.get(), false, false );
          }
          break;
        }

        case Qgis::LayerType::Plugin:
        case Qgis::LayerType::Annotation:
        case Qgis::LayerType::Mesh:
        case Qgis::LayerType::VectorTile:
        case Qgis::LayerType::PointCloud:
        case Qgis::LayerType::Group:
        case Qgis::LayerType::TiledScene:
          break;
      }
    } );
  }

  if ( item->capabilities2() & Qgis::BrowserItemCapability::Delete )
  {
    QStringList selectedDeletableItemPaths;
    for ( QgsDataItem *selectedItem : selectedItems )
    {
      if ( qobject_cast<QgsLayerItem *>( selectedItem ) && ( selectedItem->capabilities2() & Qgis::BrowserItemCapability::Delete ) )
        selectedDeletableItemPaths.append( qobject_cast<QgsLayerItem *>( selectedItem )->uri() );
    }

    const QString deleteText = selectedDeletableItemPaths.count() == 1 ? tr( "Delete Layer “%1”…" ).arg( layerItem->name() )
                               : tr( "Delete Selected Layers…" );
    QAction *deleteAction = new QAction( deleteText, menu );
    connect( deleteAction, &QAction::triggered, this, [ = ]
    {
      deleteLayers( selectedDeletableItemPaths, context );
    } );

    // this action should sit in the Manage menu. If one does not exist, create it now
    bool foundExistingManageMenu = false;
    QList<QAction *> actions = menu->actions();
    for ( QAction *action : std::as_const( actions ) )
    {
      if ( action->text() == tr( "Manage" ) )
      {
        action->menu()->addAction( deleteAction );
        foundExistingManageMenu = true;
        break;
      }
    }
    if ( !foundExistingManageMenu )
    {
      QMenu *manageLayerMenu = new QMenu( tr( "Manage" ), menu );
      manageLayerMenu->addAction( deleteAction );
      menu->addMenu( manageLayerMenu );
    }
  }

  if ( !menu->isEmpty() )
    menu->addSeparator();

  const QString addText = selectedItems.count() == 1 ? tr( "Add Layer to Project" )
                          : tr( "Add Selected Layers to Project" );
  QAction *addAction = new QAction( addText, menu );
  connect( addAction, &QAction::triggered, this, [ = ]
  {
    addLayersFromItems( selectedItems );
  } );
  menu->addAction( addAction );

  for ( QAction *action : menu->actions() )
  {
    const QString insertAfter = action->property( "insertAfter" ).toString();
    if ( !insertAfter.isEmpty() && insertAfter == "addLayerToProject" )
    {
      // temporarily unset the parent -- we don't want the action to be deleted automatically if it was parented to the menu
      QObject *prevParent = action->parent();
      action->setParent( nullptr );

      menu->removeAction( action );
      menu->addAction( action );

      action->setParent( prevParent );
    }
  }

  QAction *propertiesAction = new QAction( tr( "Layer Properties…" ), menu );
  connect( propertiesAction, &QAction::triggered, this, [ = ]
  {
    showPropertiesForItem( layerItem, context );
  } );
  menu->addAction( propertiesAction );
}

int QgsLayerItemGuiProvider::precedenceWhenPopulatingMenus() const
{
  // we want this provider to be called second last (last place is reserved for QgsAppFileItemGuiProvider)
  return std::numeric_limits< int >::max() - 1;
}

bool QgsLayerItemGuiProvider::handleDoubleClick( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( !item || item->type() != Qgis::BrowserItemType::Layer )
    return false;

  if ( QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item ) )
  {
    const QgsMimeDataUtils::UriList layerUriList = layerItem->mimeUris();
    QgisApp::instance()->handleDropUriList( layerUriList );
    return true;
  }
  else
  {
    return false;
  }
}

void QgsLayerItemGuiProvider::addLayersFromItems( const QList<QgsDataItem *> &items )
{
  QgsTemporaryCursorOverride cursor( Qt::WaitCursor );

  // If any of the layer items are QGIS we just open and exit the loop
  // TODO - maybe this logic is wrong?
  for ( const QgsDataItem *item : items )
  {
    if ( item && item->type() == Qgis::BrowserItemType::Project )
    {
      if ( const QgsProjectItem *projectItem = qobject_cast<const QgsProjectItem *>( item ) )
        QgisApp::instance()->openProject( projectItem->path() );

      return;
    }
  }

  QgsMimeDataUtils::UriList layerUriList;
  layerUriList.reserve( items.size() );
  // add items in reverse order so they are in correct order in the layers dock
  for ( int i = items.size() - 1; i >= 0; i-- )
  {
    QgsDataItem *item = items.at( i );
    if ( item && item->type() == Qgis::BrowserItemType::Layer )
    {
      if ( QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item ) )
        layerUriList.append( layerItem->mimeUris() );
    }
  }
  if ( !layerUriList.isEmpty() )
    QgisApp::instance()->handleDropUriList( layerUriList );
}

void QgsLayerItemGuiProvider::deleteLayers( const QStringList &itemPaths, QgsDataItemGuiContext context )
{
  for ( const QString &itemPath : itemPaths )
  {
    //get the item from browserModel by its path
    QgsLayerItem *item = qobject_cast<QgsLayerItem *>( QgisApp::instance()->browserModel()->dataItem( QgisApp::instance()->browserModel()->findUri( itemPath ) ) );
    if ( !item )
    {
      QgsMessageLog::logMessage( tr( "Item with path %1 no longer exists." ).arg( itemPath ) );
      return;
    }

    // first try to use the new API - through QgsDataItemGuiProvider. If that fails, try to use the legacy API...
    bool usedNewApi = false;
    const QList<QgsDataItemGuiProvider *> providers = QgsGui::dataItemGuiProviderRegistry()->providers();
    for ( QgsDataItemGuiProvider *provider : providers )
    {
      if ( provider->deleteLayer( item, context ) )
      {
        usedNewApi = true;
        break;
      }
    }

    if ( !usedNewApi )
    {
      Q_NOWARN_DEPRECATED_PUSH
      bool res = item->deleteLayer();
      Q_NOWARN_DEPRECATED_POP

      if ( !res )
        notify( tr( "Delete Layer" ), tr( "Item Layer %1 cannot be deleted." ).arg( item->name() ), context, Qgis::MessageLevel::Warning );
    }
  }
}

void QgsLayerItemGuiProvider::showPropertiesForItem( QgsLayerItem *item, QgsDataItemGuiContext context )
{
  if ( ! item )
    return;

  QgsBrowserPropertiesDialog *dialog = new QgsBrowserPropertiesDialog( QStringLiteral( "browser" ), QgisApp::instance() );
  dialog->setAttribute( Qt::WA_DeleteOnClose );
  dialog->setItem( item, context );
  dialog->show();
}

//
// QgsProjectItemGuiProvider
//

QString QgsProjectItemGuiProvider::name()
{
  return QStringLiteral( "project_items" );
}

void QgsProjectItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext context )
{
  if ( !item || item->type() != Qgis::BrowserItemType::Project )
    return;

  if ( QgsProjectItem *projectItem = qobject_cast<QgsProjectItem *>( item ) )
  {
    QAction *openAction = new QAction( tr( "Open Project" ), menu );
    const QString projectPath = projectItem->path();
    connect( openAction, &QAction::triggered, this, [projectPath]
    {
      QgisApp::instance()->openProject( projectPath );
    } );
    menu->addAction( openAction );

    QAction *extractAction = new QAction( tr( "Extract Symbols…" ), menu );
    connect( extractAction, &QAction::triggered, this, [projectPath, context]
    {
      QgsStyle style;
      style.createMemoryDatabase();

      QgsSaveToStyleVisitor visitor( &style );

      QgsProject p( nullptr, Qgis::ProjectCapabilities() );
      QgsTemporaryCursorOverride override( Qt::WaitCursor );
      if ( p.read( projectPath, Qgis::ProjectReadFlag::DontResolveLayers | Qgis::ProjectReadFlag::DontStoreOriginalStyles | Qgis::ProjectReadFlag::DontLoad3DViews ) )
      {
        p.accept( &visitor );
        override.release();
        QgsStyleManagerDialog dlg( &style, QgisApp::instance(), Qt::WindowFlags(), true );
        dlg.setFavoritesGroupVisible( false );
        dlg.setSmartGroupsVisible( false );
        QFileInfo fi( projectPath );
        dlg.setBaseStyleName( fi.baseName() );
        dlg.exec();
      }
      else if ( context.messageBar() )
      {
        context.messageBar()->pushWarning( tr( "Extract Symbols" ), tr( "Could not read project file" ) );
      }
    } );
    menu->addAction( extractAction );
  }
}

bool QgsProjectItemGuiProvider::handleDoubleClick( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( !item || item->type() != Qgis::BrowserItemType::Project )
    return false;

  if ( QgsProjectItem *projectItem = qobject_cast<QgsProjectItem *>( item ) )
  {
    QgisApp::instance()->openProject( projectItem->path() );
    return true;
  }
  else
  {
    return false;
  }
}

QString QgsFieldsItemGuiProvider::name()
{
  return QStringLiteral( "fields_item" );
}

void QgsFieldsItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context )
{
  Q_UNUSED( selectedItems )

  if ( !item || item->type() != Qgis::BrowserItemType::Fields )
    return;


  if ( QgsFieldsItem *fieldsItem = qobject_cast<QgsFieldsItem *>( item ) )
  {
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( fieldsItem->providerKey() ) };
    if ( md )
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( fieldsItem->connectionUri(), {} ) ) };
      // Check if it is supported
      if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::AddField ) )
      {
        QAction *addColumnAction = new QAction( tr( "Add New Field…" ), menu );
        QPointer<QgsDataItem>itemPtr { item };
        const QSet< QString > illegalFieldNames = conn->illegalFieldNames();

        connect( addColumnAction, &QAction::triggered, fieldsItem, [ md, fieldsItem, context, itemPtr, menu, illegalFieldNames ]
        {
          std::unique_ptr<QgsVectorLayer> layer { fieldsItem->layer() };
          if ( layer )
          {
            QgsAddAttrDialog dialog( layer.get(), menu );
            dialog.setIllegalFieldNames( illegalFieldNames );
            if ( dialog.exec() == QDialog::Accepted )
            {
              std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn2 { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( fieldsItem->connectionUri(), {} ) ) };
              try
              {
                conn2->addField( dialog.field(), fieldsItem->schema(), fieldsItem->tableName() );
                if ( itemPtr )
                  itemPtr->refresh();
              }
              catch ( const QgsProviderConnectionException &ex )
              {
                notify( tr( "New Field" ), tr( "Failed to add the new field to '%1': %2" ).arg( fieldsItem->tableName(), ex.what() ), context, Qgis::MessageLevel::Critical );
              }
            }
          }
          else
          {
            notify( tr( "New Field" ), tr( "Failed to load layer '%1'. Check application logs and user permissions." ).arg( fieldsItem->tableName() ), context, Qgis::MessageLevel::Critical );
          }
        } );
        menu->addAction( addColumnAction );
      }
    }
  }
}

QWidget *QgsFieldsItemGuiProvider::createParamWidget( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( QgsFieldsItem *fieldsItem = qobject_cast<QgsFieldsItem *>( item ) )
  {
    return new QgsFieldsDetailsWidget( nullptr, fieldsItem->providerKey(), fieldsItem->connectionUri(), fieldsItem->schema(), fieldsItem->tableName() );
  }
  else
  {
    return nullptr;
  }
}

QString QgsFieldItemGuiProvider::name()
{
  return QStringLiteral( "field_item" );
}

void QgsFieldItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context )
{
  Q_UNUSED( selectedItems )

  if ( !item || item->type() != Qgis::BrowserItemType::Field )
    return;

  if ( QgsFieldItem *fieldItem = qobject_cast<QgsFieldItem *>( item ) )
  {
    // Retrieve the connection from the parent
    QPointer< QgsFieldsItem > fieldsItem { qobject_cast<QgsFieldsItem *>( fieldItem->parent() ) };
    if ( fieldsItem )
    {
      const QString connectionUri = fieldsItem->connectionUri();
      const QString providerKey = fieldsItem->providerKey();
      const QString schema = fieldsItem->schema();
      const QString tableName = fieldsItem->tableName();
      const QString fieldName = fieldItem->field().name();
      const QString domainName = fieldItem->field().constraints().domainName();
      const QString alias = fieldItem->field().alias();
      const QString comment = fieldItem->field().comment();

      // Check if it is supported
      QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
      if ( md )
      {
        std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };

        if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::SetFieldDomain )
             && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::ListFieldDomains ) )
        {
          const QStringList domains = conn->fieldDomainNames();
          if ( !domains.isEmpty() )
          {
            QMenu *setFieldDomainMenu = new QMenu( tr( "Set Field Domain" ), menu );
            menu->addMenu( setFieldDomainMenu );

            for ( const QString &domain : domains )
            {
              QAction *setDomainAction = new QAction( domain, setFieldDomainMenu );
              setFieldDomainMenu->addAction( setDomainAction );

              if ( domain == domainName )
              {
                // show current domain as checked
                setDomainAction->setCheckable( true );
                setDomainAction->setChecked( true );
              }

              connect( setDomainAction, &QAction::triggered, this, [connectionUri, providerKey, schema, tableName, fieldName, domain, context, fieldsItem]
              {
                if ( QMessageBox::question( nullptr, tr( "Set Field Domain" ),
                                            tr( "Set field domain for %1 to %2?" ).arg( fieldName, domain ),
                                            QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
                {
                  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
                  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn2 { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };
                  try
                  {
                    conn2->setFieldDomainName( fieldName, schema, tableName, domain );
                    if ( fieldsItem )
                      fieldsItem->refresh();
                  }
                  catch ( const QgsProviderConnectionException &ex )
                  {
                    notify( tr( "Set Field Domain" ), ex.what(), context, Qgis::MessageLevel::Critical );
                  }
                }
              } );
            }
          }
        }
        if ( !domainName.isEmpty() && conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::SetFieldDomain ) )
        {
          QAction *clearDomainAction = new QAction( tr( "Unset Field Domain (%1)…" ).arg( domainName ), menu );
          menu->addAction( clearDomainAction );

          connect( clearDomainAction, &QAction::triggered, this, [connectionUri, providerKey, schema, tableName, fieldName, domainName, context, fieldsItem]
          {
            if ( QMessageBox::question( nullptr, tr( "Unset Field Domain" ),
                                        tr( "Unset %1 field domain from %2?" ).arg( domainName, fieldName ),
                                        QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
            {
              QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
              std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn2 { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };
              try
              {
                conn2->setFieldDomainName( fieldName, schema, tableName, QString() );
                if ( fieldsItem )
                  fieldsItem->refresh();
              }
              catch ( const QgsProviderConnectionException &ex )
              {
                notify( tr( "Unset Field Domain" ), ex.what(), context, Qgis::MessageLevel::Critical );
              }
            }
          } );
        }

        if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::RenameField ) )
        {
          QAction *renameFieldAction = new QAction( tr( "Rename Field…" ), menu );
          const QString itemName { item->name() };

          connect( renameFieldAction, &QAction::triggered, fieldsItem, [ md, fieldsItem, itemName, context ]
          {
            // Confirmation dialog
            QgsNewNameDialog dlg( tr( "field “%1”" ).arg( itemName ), itemName );
            dlg.setWindowTitle( tr( "Rename Field" ) );
            if ( dlg.exec() != QDialog::Accepted || dlg.name() == itemName )
              return;

            std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn2 { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( fieldsItem->connectionUri(), {} ) ) };
            try
            {
              conn2->renameField( fieldsItem->schema(), fieldsItem->tableName(), itemName, dlg.name() );
              fieldsItem->refresh();
            }
            catch ( const QgsProviderConnectionException &ex )
            {
              notify( tr( "Rename Field" ), tr( "Failed to rename field '%1': %2" ).arg( itemName, ex.what() ), context, Qgis::MessageLevel::Critical );
            }
          } );

          menu->addAction( renameFieldAction );
        }

        if ( conn && conn->capabilities2().testFlag( Qgis::DatabaseProviderConnectionCapability2::SetFieldAlias ) )
        {
          QAction *setAliasAction = new QAction( tr( "Set Alias…" ), menu );
          const QString itemName { item->name() };

          connect( setAliasAction, &QAction::triggered, fieldsItem, [ md, fieldsItem, itemName, alias, context ]
          {
            bool ok = false;

            const QString newAlias = QInputDialog::getText( QgisApp::instance(), tr( "Set Alias For %1" ).arg( itemName ), tr( "Alias" ), QLineEdit::Normal, alias, &ok );
            if ( ok && alias != newAlias )
            {
              std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn2 { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( fieldsItem->connectionUri(), {} ) ) };
              try
              {
                conn2->setFieldAlias( itemName, fieldsItem->schema(), fieldsItem->tableName(), newAlias );
                fieldsItem->refresh();
              }
              catch ( const QgsProviderConnectionException &ex )
              {
                notify( tr( "Set Field Alias" ), tr( "Failed to set alias for field '%1': %2" ).arg( itemName, ex.what() ), context, Qgis::MessageLevel::Critical );
              }
            }
          } );

          menu->addAction( setAliasAction );
        }

        if ( conn && conn->capabilities2().testFlag( Qgis::DatabaseProviderConnectionCapability2::SetFieldComment ) )
        {
          QAction *setCommentAction = new QAction( tr( "Set Comment…" ), menu );
          const QString itemName { item->name() };

          connect( setCommentAction, &QAction::triggered, fieldsItem, [ md, fieldsItem, itemName, comment, context ]
          {
            bool ok = false;

            const QString newComment = QInputDialog::getText( QgisApp::instance(), tr( "Set Comment For %1" ).arg( itemName ), tr( "Comment" ), QLineEdit::Normal, comment, &ok );
            if ( ok && comment != newComment )
            {
              std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn2 { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( fieldsItem->connectionUri(), {} ) ) };
              try
              {
                conn2->setFieldComment( itemName, fieldsItem->schema(), fieldsItem->tableName(), newComment );
                fieldsItem->refresh();
              }
              catch ( const QgsProviderConnectionException &ex )
              {
                notify( tr( "Set Field Comment" ), tr( "Failed to set comment for field '%1': %2" ).arg( itemName, ex.what() ), context, Qgis::MessageLevel::Critical );
              }
            }
          } );

          menu->addAction( setCommentAction );
        }

        if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::DeleteField ) )
        {
          QAction *deleteFieldAction = new QAction( tr( "Delete Field…" ), menu );
          const bool supportsCascade { conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::DeleteFieldCascade ) };
          const QString itemName { item->name() };

          connect( deleteFieldAction, &QAction::triggered, fieldsItem, [ md, fieldsItem, itemName, context, supportsCascade ]
          {
            // Confirmation dialog
            QString message {  tr( "Delete '%1' permanently?" ).arg( itemName ) };
            if ( fieldsItem->tableProperty() && fieldsItem->tableProperty()->primaryKeyColumns().contains( itemName ) )
            {
              message.append( tr( "\nThis field is part of a primary key, its removal may make the table unusable by QGIS!" ) );
            }
            if ( fieldsItem->tableProperty() && fieldsItem->tableProperty()->geometryColumn() == itemName )
            {
              message.append( tr( "\nThis field is a geometry column, its removal may make the table unusable by QGIS!" ) );
            }
            QMessageBox msgbox{QMessageBox::Icon::Question, tr( "Delete Field" ), message, QMessageBox::Ok | QMessageBox::Cancel };
            QCheckBox *cb = new QCheckBox( tr( "Delete all related objects (CASCADE)?" ) );
            msgbox.setCheckBox( cb );
            msgbox.setDefaultButton( QMessageBox::Cancel );

            if ( ! supportsCascade )
            {
              cb->hide();
            }

            if ( msgbox.exec() == QMessageBox::Ok )
            {
              std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn2 { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( fieldsItem->connectionUri(), {} ) ) };
              try
              {
                conn2->deleteField( itemName, fieldsItem->schema(), fieldsItem->tableName(), supportsCascade && cb->isChecked() );
                fieldsItem->refresh();
              }
              catch ( const QgsProviderConnectionException &ex )
              {
                notify( tr( "Delete Field" ), tr( "Failed to delete field '%1': %2" ).arg( itemName, ex.what() ), context, Qgis::MessageLevel::Critical );
              }
            }
          } );
          if ( !menu->isEmpty() )
            menu->addSeparator();

          menu->addAction( deleteFieldAction );
        }
      }
    }
    else
    {
      // This should never happen!
      QgsDebugError( QStringLiteral( "Error getting parent fields for %1" ).arg( item->name() ) );
    }
  }
}

bool QgsFieldItemGuiProvider::rename( QgsDataItem *item, const QString &name, QgsDataItemGuiContext context )
{
  if ( QgsFieldItem *fieldItem = qobject_cast<QgsFieldItem *>( item ) )
  {
    QPointer< QgsFieldsItem > fieldsItem { qobject_cast<QgsFieldsItem *>( fieldItem->parent() ) };
    if ( fieldsItem )
    {
      const QString connectionUri = fieldsItem->connectionUri();
      const QString providerKey = fieldsItem->providerKey();

      // Check if it is supported
      QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
      if ( md )
      {
        std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };
        if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::RenameField ) )
        {
          const QString itemName { item->name() };

          try
          {
            conn->renameField( fieldsItem->schema(), fieldsItem->tableName(), itemName, name );
            fieldsItem->refresh();
          }
          catch ( const QgsProviderConnectionException &ex )
          {
            notify( tr( "Rename Field" ), tr( "Failed to rename field '%1': %2" ).arg( itemName, ex.what() ), context, Qgis::MessageLevel::Critical );
          }
        }
      }
    }
    return true;
  }
  return false;
}

QWidget *QgsFieldItemGuiProvider::createParamWidget( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( QgsFieldItem *fieldItem = qobject_cast<QgsFieldItem *>( item ) )
  {
    QgsFieldsItem *fieldsItem { static_cast<QgsFieldsItem *>( fieldItem->parent() ) };
    if ( fieldsItem )
    {
      return new QgsFieldDetailsWidget( nullptr, fieldsItem->providerKey(), fieldsItem->connectionUri(), fieldsItem->schema(), fieldsItem->tableName(), fieldItem->field() );
    }
  }
  return nullptr;
}

QgsDatabaseItemGuiProvider::QgsDatabaseItemGuiProvider()
{
  if ( QgsDatabaseQueryHistoryProvider *historyProvider = qobject_cast< QgsDatabaseQueryHistoryProvider * >( QgsGui::historyProviderRegistry()->providerById( QStringLiteral( "dbquery" ) ) ) )
  {
    connect( historyProvider, &QgsDatabaseQueryHistoryProvider::openSqlDialog, this, &QgsDatabaseItemGuiProvider::openSqlDialogGeneric );
  }
}

QString QgsDatabaseItemGuiProvider::name()
{
  return QStringLiteral( "database" );
}

void QgsDatabaseItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context )
{
  Q_UNUSED( selectedItems )

  // Add create new table for collection items but not not if it is a root item
  if ( ! qobject_cast<QgsConnectionsRootItem *>( item ) )
  {
    std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( item->databaseConnection() );

    if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::CreateVectorTable ) )
    {
      QAction *newTableAction = new QAction( QObject::tr( "New Table…" ), menu );

      QObject::connect( newTableAction, &QAction::triggered, item, [ item, context]
      {
        std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn2( item->databaseConnection() );
        // This should never happen but let's play safe
        if ( ! conn2 )
        {
          QgsMessageLog::logMessage( tr( "Connection to the database (%1) was lost." ).arg( item->name() ) );
          return;
        }

        QgsNewVectorTableDialog dlg { conn2.get(), nullptr };
        dlg.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );

        const bool isSchema { qobject_cast<QgsDatabaseSchemaItem *>( item ) != nullptr };

        if ( isSchema )
        {
          dlg.setSchemaName( item->name() );
        }

        if ( dlg.exec() == QgsNewVectorTableDialog::DialogCode::Accepted )
        {
          const QgsFields fields { dlg.fields() };
          const QString tableName { dlg.tableName() };
          const QString schemaName { dlg.schemaName() };
          const QString geometryColumn { dlg.geometryColumnName() };
          const Qgis::WkbType geometryType { dlg.geometryType() };
          const bool createSpatialIndex = dlg.createSpatialIndex() &&
                                          geometryType != Qgis::WkbType::NoGeometry &&
                                          geometryType != Qgis::WkbType::Unknown;
          const QgsCoordinateReferenceSystem crs { dlg.crs( ) };
          // This flag tells to the provider that field types do not need conversion
          // also prevents  GDAL to create a spatial index by default for GPKG, we are
          // going to create it afterwards in a unified manner for all providers.
          QMap<QString, QVariant> options { { QStringLiteral( "skipConvertFields" ), true },
            { QStringLiteral( "layerOptions" ), QStringLiteral( "SPATIAL_INDEX=NO" ) } };

          if ( ! geometryColumn.isEmpty() )
          {
            options[ QStringLiteral( "geometryColumn" ) ] = geometryColumn;
          }

          try
          {
            conn2->createVectorTable( schemaName, tableName, fields, geometryType, crs, true, &options );
            if ( createSpatialIndex && conn2->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::CreateSpatialIndex ) )
            {
              try
              {
                conn2->createSpatialIndex( schemaName, tableName );
              }
              catch ( QgsProviderConnectionException &ex )
              {
                notify( QObject::tr( "Create Spatial Index" ), QObject::tr( "Could not create spatial index for table '%1':%2." ).arg( tableName, ex.what() ), context, Qgis::MessageLevel::Warning );
              }
            }
            // Ok, here is the trick: we cannot refresh the connection item because the refresh is not
            // recursive.
            // So, we check if the item is a schema or not, if it's not it means we initiated the new table from
            // the parent connection item, hence we search for the schema item and refresh it instead of refreshing
            // the connection item (the parent) with no effects.
            if ( ! isSchema && conn2->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::Schemas ) )
            {
              const auto constChildren { item->children() };
              for ( const auto &c : constChildren )
              {
                if ( c->name() == schemaName )
                {
                  c->refresh();
                }
              }
            }
            else
            {
              item->refresh( );
            }
            notify( QObject::tr( "New Table Created" ), QObject::tr( "Table '%1' was created successfully." ).arg( tableName ), context, Qgis::MessageLevel::Success );
          }
          catch ( QgsProviderConnectionException &ex )
          {
            notify( QObject::tr( "New Table Creation Error" ), QObject::tr( "Error creating new table '%1': %2" ).arg( tableName, ex.what() ), context, Qgis::MessageLevel::Critical );
          }

        }
      } );
      menu->addAction( newTableAction );
    }

    // SQL dialog
    if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::ExecuteSql ) )
    {
      QAction *sqlAction = new QAction( QObject::tr( "Execute SQL…" ), menu );

      const QString connectionUri = conn->uri();
      const QString providerKey = conn->providerKey();
      const QString itemName = item->name();
      QString itemParentName;

      QString tableName;
      if ( qobject_cast<QgsLayerItem *>( item ) )
      {
        if ( QgsProviderSublayerItem *sublayerItem = qobject_cast< QgsProviderSublayerItem * >( item ) )
        {
          tableName = sublayerItem->sublayerDetails().name();
        }
        if ( tableName.isEmpty() )
        {
          tableName = itemName;
        }

        itemParentName = item->parent()->name();
      }

      QObject::connect( sqlAction, &QAction::triggered, item, [ connectionUri, providerKey, itemName, tableName, itemParentName, context, this ]
      {
        QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
        if ( !md )
          return;

        std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn2( qgis::down_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, QVariantMap() ) ) );

        // This should never happen but let's play safe
        if ( ! conn2 )
        {
          QgsMessageLog::logMessage( tr( "Connection to the database (%1) was lost." ).arg( itemName ) );
          return;
        }

        // If this is a layer item (or below the hierarchy) we can pre-set the query to something
        // meaningful
        QString sql;

        if ( !itemParentName.isEmpty() )
        {
          std::unique_ptr< QgsProviderSqlQueryBuilder > queryBuilder( conn2->queryBuilder() );
          sql = queryBuilder->createLimitQueryForTable(
                  conn2->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::Schemas ) ? itemParentName : QString(),
                  tableName, 10 );
        }

        openSqlDialog( connectionUri, providerKey, sql, context, itemName );
      } );
      menu->addAction( sqlAction );
    }

    // Check if domain creation is supported
    if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::Vacuum ) )
    {
      QAction *compactAction = new QAction( tr( "Compact Database (VACUUM)" ), menu );
      menu->addAction( compactAction );

      const QString connectionUri = conn->uri();
      const QString providerKey = conn->providerKey();

      connect( compactAction, &QAction::triggered, compactAction, [context, connectionUri, providerKey]
      {
        QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
        if ( !md )
          return;

        std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( qgis::down_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, QVariantMap() ) ) );
        QString errCause;
        if ( conn )
        {
          try
          {
            QgsTemporaryCursorOverride override( Qt::WaitCursor );
            conn->vacuum( QString(), QString() );
          }
          catch ( QgsProviderConnectionException &ex )
          {
            errCause = ex.what();
          }
        }
        else
        {
          errCause = QObject::tr( "There was an error retrieving the connection to %1" ).arg( connectionUri );
        }

        if ( !errCause.isEmpty() )
        {
          notify( tr( "Database compact (VACUUM)" ), errCause, context, Qgis::MessageLevel::Critical );
        }
        else if ( context.messageBar() )
        {
          context.messageBar()->pushMessage( tr( "Database compacted" ), Qgis::MessageLevel::Success );
        }
      } );
    }
  }
}

bool QgsDatabaseItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  // WARNING: This method will be called from the MAIN THREAD AND WILL BLOCK QGIS WHILE THE
  // BROWSER IS BEING POPULATED
  // We are limited to VERY VERY cheap calculations only!!
  // DO NOT UNDER *****ANY***** CIRCUMSTANCES OPEN DATASETS HERE!!!!
  QgsFileDataCollectionItem *fileDataCollectionItem = qobject_cast< QgsFileDataCollectionItem * >( item );
  if ( !fileDataCollectionItem )
    return false;

  if ( qobject_cast< QgsGeoPackageCollectionItem * >( item ) )
    return false; // GPKG is handled elsewhere (QgsGeoPackageItemGuiProvider)

  if ( fileDataCollectionItem->canAddVectorLayers() )
  {
    return true;
  }

  // WARNING: This method will be called from the MAIN THREAD AND WILL BLOCK QGIS WHILE THE
  // BROWSER IS BEING POPULATED
  // We are limited to VERY VERY cheap calculations only!!
  // DO NOT UNDER *****ANY***** CIRCUMSTANCES OPEN DATASETS HERE!!!!
  return false;
}

bool QgsDatabaseItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction )
{
  if ( !qobject_cast< QgsFileDataCollectionItem * >( item ) )
    return false;

  if ( qobject_cast< QgsGeoPackageCollectionItem * >( item ) )
    return false; // GPKG is handled elsewhere (QgsGeoPackageItemGuiProvider)

  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  QString uri;

  QStringList importResults;
  bool hasError = false;

  // Main task
  std::unique_ptr< QgsTaskWithSerialSubTasks > mainTask( new QgsTaskWithSerialSubTasks( tr( "Layer import" ) ) );
  bool hasSubTasks = false;

  const QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  for ( const QgsMimeDataUtils::Uri &dropUri : lst )
  {
    // Check that we are not copying over self
    if ( dropUri.uri.startsWith( item->path() ) )
    {
      importResults.append( tr( "You cannot import layer %1 over itself!" ).arg( dropUri.name ) );
      hasError = true;
    }
    else
    {
      QgsVectorLayer *srcLayer = nullptr;
      bool owner;
      QString error;
      if ( dropUri.layerType == QLatin1String( "vector" ) )
      {
        // open the source layer
        srcLayer = dropUri.vectorLayer( owner, error );
      }
      else
      {
        // unsupported
        hasError = true;
        continue;
      }

      if ( !srcLayer )
      {
        importResults.append( tr( "%1: %2" ).arg( dropUri.name, error ) );
        hasError = true;
        continue;
      }

      if ( srcLayer->isValid() )
      {
        uri = item->path();

        // check if the destination layer already exists
        bool exists = false;
        const QVector< QgsDataItem * > c( item->children() );
        for ( const QgsDataItem *child : c )
        {
          if ( child->name() == dropUri.name )
          {
            exists = true;
          }
        }

        if ( ! exists || QMessageBox::question( nullptr, tr( "Overwrite Layer" ),
                                                tr( "Destination layer <b>%1</b> already exists. Do you want to overwrite it?" ).arg( dropUri.name ), QMessageBox::Yes |  QMessageBox::No ) == QMessageBox::Yes )
        {
          QgsVectorLayer *vectorSrcLayer = qobject_cast < QgsVectorLayer * >( srcLayer );
          QVariantMap options;
          //  options.insert( QStringLiteral( "driverName" ), QStringLiteral( "GPKG" ) );
          options.insert( QStringLiteral( "update" ), true );
          options.insert( QStringLiteral( "overwrite" ), true );
          options.insert( QStringLiteral( "layerName" ), dropUri.name );
          QgsVectorLayerExporterTask *exportTask = new QgsVectorLayerExporterTask( vectorSrcLayer, uri, QStringLiteral( "ogr" ), vectorSrcLayer->crs(), options, owner );
          mainTask->addSubTask( exportTask );
          hasSubTasks = true;
          // when export is successful:
          connect( exportTask, &QgsVectorLayerExporterTask::exportComplete, item, [ = ]()
          {
            notify( tr( "Import to database" ), tr( "Import was successful." ), context, Qgis::MessageLevel::Success );
            item->refresh();
          } );

          // when an error occurs:
          connect( exportTask, &QgsVectorLayerExporterTask::errorOccurred, item, [ = ]( Qgis::VectorExportResult error, const QString & errorMessage )
          {
            if ( error != Qgis::VectorExportResult::UserCanceled )
            {
              QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
              output->setTitle( tr( "Import to database" ) );
              output->setMessage( tr( "Failed to import some vector layers!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
              output->showMessage();
            }
          } );
        }
      }
      else
      {
        importResults.append( tr( "%1: Not a valid layer!" ).arg( dropUri.name ) );
        hasError = true;
      }
    }
  }

  if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QLatin1Char( '\n' ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }
  if ( hasSubTasks )
  {
    QgsApplication::taskManager()->addTask( mainTask.release() );
  }
  return true;
}

void QgsDatabaseItemGuiProvider::openSqlDialog( const QString &connectionUri, const QString &provider, const QString &query, QgsDataItemGuiContext context, const QString &identifierName )
{
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( provider ) };
  if ( !md )
    return;

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( qgis::down_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, QVariantMap() ) ) );

  // Create the SQL dialog: this might become an independent class dialog in the future, for now
  // we are still prototyping the features that this dialog will have.

  QMainWindow *dialog = new QMainWindow();
  dialog->setObjectName( QStringLiteral( "SQLCommandsDialog" ) );
  if ( !identifierName.isEmpty() )
    dialog->setWindowTitle( tr( "%1 — Execute SQL" ).arg( identifierName ) );
  else
    dialog->setWindowTitle( tr( "Execute SQL" ) );

  QgsGui::enableAutoGeometryRestore( dialog );
  dialog->setAttribute( Qt::WA_DeleteOnClose );

  QgsQueryResultWidget *widget { new QgsQueryResultWidget( nullptr, conn.release() ) };
  widget->setQuery( query );
  dialog->setCentralWidget( widget );

  connect( widget, &QgsQueryResultWidget::createSqlVectorLayer, widget, [ provider, connectionUri, context ]( const QString &, const QString &, const QgsAbstractDatabaseProviderConnection::SqlVectorLayerOptions & options )
  {
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( provider ) };
    if ( !md )
      return;

    std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( qgis::down_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, QVariantMap() ) ) );
    if ( !conn )
      return;

    try
    {
      QgsMapLayer *sqlLayer { conn->createSqlVectorLayer( options ) };
      QgsProject::instance()->addMapLayers( { sqlLayer } );
    }
    catch ( QgsProviderConnectionException &ex )
    {
      notify( QObject::tr( "New SQL Layer Creation Error" ), QObject::tr( "Error creating new SQL layer: %1" ).arg( ex.what() ), context, Qgis::MessageLevel::Critical );
    }

  } );
  dialog->show();
}

void QgsDatabaseItemGuiProvider::openSqlDialogGeneric( const QString &connectionUri, const QString &provider, const QString &query )
{
  QgsDataItemGuiContext context;
  context.setMessageBar( QgisApp::instance()->messageBar() );

  openSqlDialog( connectionUri, provider, query, context );
}


//
// QgsFieldDomainItemGuiProvider
//

QString QgsFieldDomainItemGuiProvider::name()
{
  return QStringLiteral( "field_domain_item" );
}

void QgsFieldDomainItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext context )
{
  if ( qobject_cast< QgsFieldDomainsItem * >( item )
       || qobject_cast< QgsGeoPackageCollectionItem * >( item )
       || qobject_cast< QgsFileDataCollectionItem * >( item ) )
  {
    QString providerKey;
    QString connectionUri;

    if ( QgsFieldDomainsItem *fieldDomainsItem = qobject_cast< QgsFieldDomainsItem * >( item ) )
    {
      providerKey = fieldDomainsItem->providerKey();
      connectionUri = fieldDomainsItem->connectionUri();
    }
    else if ( QgsGeoPackageCollectionItem *gpkgItem = qobject_cast< QgsGeoPackageCollectionItem * >( item ) )
    {
      providerKey = QStringLiteral( "ogr" );
      connectionUri = gpkgItem->path().remove( QStringLiteral( "gpkg:/" ) );
    }
    else if ( QgsFileDataCollectionItem *fileItem = qobject_cast< QgsFileDataCollectionItem * >( item ) )
    {
      providerKey = QStringLiteral( "ogr" );
      connectionUri = fileItem->path();
    }

    // Check if domain creation is supported
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
    if ( md )
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };
      if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::AddFieldDomain ) )
      {
        QMenu *createFieldDomainMenu = new QMenu( tr( "New Field Domain" ), menu );
        menu->addMenu( createFieldDomainMenu );

        QPointer< QgsDataItem > itemWeakPointer( item );

        auto createDomain = [context, itemWeakPointer, md, connectionUri]( Qgis::FieldDomainType type )
        {
          QgsFieldDomainDialog dialog( type, QgisApp::instance() );
          dialog.setWindowTitle( tr( "New Field Domain" ) );
          if ( dialog.exec() )
          {
            std::unique_ptr< QgsFieldDomain > newDomain( dialog.createFieldDomain() );
            std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };
            try
            {
              conn->addFieldDomain( *newDomain, QString() );
              notify( QObject::tr( "New Field Domain Created" ), QObject::tr( "Field domain '%1' was created successfully." ).arg( newDomain->name() ), context, Qgis::MessageLevel::Success );
              if ( itemWeakPointer )
              {
                itemWeakPointer->refresh();
              }
            }
            catch ( QgsProviderConnectionException &ex )
            {
              notify( QObject::tr( "Field Domain Creation Error" ), QObject::tr( "Error creating new field domain '%1': %2" ).arg( newDomain->name(), ex.what() ), context, Qgis::MessageLevel::Critical );
            }
          }
        };

        const QList< Qgis::FieldDomainType > supportedDomainTypes = conn->supportedFieldDomainTypes();

        if ( supportedDomainTypes.contains( Qgis::FieldDomainType::Range ) )
        {
          QAction *rangeDomainAction = new QAction( QObject::tr( "New Range Domain…" ) );
          createFieldDomainMenu->addAction( rangeDomainAction );
          connect( rangeDomainAction, &QAction::triggered, this, [ = ]
          {
            createDomain( Qgis::FieldDomainType::Range );
          } );
        }

        if ( supportedDomainTypes.contains( Qgis::FieldDomainType::Coded ) )
        {
          QAction *codedDomainAction = new QAction( QObject::tr( "New Coded Values Domain…" ) );
          createFieldDomainMenu->addAction( codedDomainAction );
          connect( codedDomainAction, &QAction::triggered, this, [ = ]
          {
            createDomain( Qgis::FieldDomainType::Coded );
          } );
        }

        if ( supportedDomainTypes.contains( Qgis::FieldDomainType::Glob ) )
        {
          QAction *globDomainAction = new QAction( QObject::tr( "New Glob Domain…" ) );
          createFieldDomainMenu->addAction( globDomainAction );
          connect( globDomainAction, &QAction::triggered, this, [ = ]
          {
            createDomain( Qgis::FieldDomainType::Glob );
          } );
        }
      }
    }
  }
}

QWidget *QgsFieldDomainItemGuiProvider::createParamWidget( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( QgsFieldDomainItem *fieldDomainItem = qobject_cast< QgsFieldDomainItem * >( item ) )
  {
    const QgsFieldDomain *domain = fieldDomainItem->fieldDomain();
    return new QgsFieldDomainDetailsWidget( nullptr, domain );
  }
  else if ( QgsFieldDomainsItem *fieldDomainsItem = qobject_cast< QgsFieldDomainsItem * >( item ) )
  {
    return new QgsFieldDomainsDetailsWidget( nullptr, fieldDomainsItem->providerKey(), fieldDomainsItem->connectionUri() );
  }
  else
  {
    return nullptr;
  }
}

//
// QgsFieldDomainDetailsWidget
//

QgsFieldDomainDetailsWidget::QgsFieldDomainDetailsWidget( QWidget *parent, const QgsFieldDomain *domain )
  : QWidget( parent )
{
  setupUi( this );

  mDomain.reset( domain->clone() );

  const QString style = QgsApplication::reportStyleSheet();
  mTextBrowser->document()->setDefaultStyleSheet( style );


  QString metadata = QStringLiteral( "<html>\n<body>\n" );
  metadata += htmlMetadata( mDomain.get(), mDomain->name() );

  mTextBrowser->setHtml( metadata );
}

QString QgsFieldDomainDetailsWidget::htmlMetadata( QgsFieldDomain *domain, const QString &title )
{
  QString metadata;
  metadata += QStringLiteral( "<h1>" ) + title + QStringLiteral( "</h1>\n<hr>\n" ) + QStringLiteral( "<table class=\"list-view\">\n" );

  if ( title != domain->name() )
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Name" ) + QStringLiteral( "</td><td>" ) + domain->name() + QStringLiteral( "</td></tr>\n" );

  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Description" ) + QStringLiteral( "</td><td>" ) + domain->description() + QStringLiteral( "</td></tr>\n" );

  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Type" ) + QStringLiteral( "</td><td>" ) + domain->typeName() + QStringLiteral( "</td></tr>\n" );

  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Field type" ) + QStringLiteral( "</td><td>" );
  metadata += QgsVariantUtils::typeToDisplayString( domain->fieldType() );
  metadata += QLatin1String( "</td></tr>\n" );

  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Split policy" ) + QStringLiteral( "</td><td>" );
  switch ( domain->splitPolicy() )
  {
    case Qgis::FieldDomainSplitPolicy::DefaultValue:
      metadata += tr( "Use default field value" );
      break;
    case Qgis::FieldDomainSplitPolicy::Duplicate:
      metadata +=  tr( "Duplicate field value" );
      break;
    case Qgis::FieldDomainSplitPolicy::GeometryRatio:
      metadata +=  tr( "Use geometry ratio" );
      break;
    case Qgis::FieldDomainSplitPolicy::UnsetField:
      metadata +=  tr( "Unset field" );
      break;
  }
  metadata += QLatin1String( "</td></tr>\n" );

  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Merge policy" ) + QStringLiteral( "</td><td>" );
  switch ( domain->mergePolicy() )
  {
    case Qgis::FieldDomainMergePolicy::DefaultValue:
      metadata +=  tr( "Use default field value" );
      break;
    case Qgis::FieldDomainMergePolicy::Sum:
      metadata +=  tr( "Sum field values" );
      break;
    case Qgis::FieldDomainMergePolicy::GeometryWeighted:
      metadata +=  tr( "Use geometry weighted value" );
      break;
  }

  metadata += QLatin1String( "</table>\n<br><br>" );

  switch ( domain->type() )
  {
    case Qgis::FieldDomainType::Coded:
    {
      metadata += QStringLiteral( "<h1>" ) + tr( "Coded values" ) + QStringLiteral( "</h1>\n<hr>\n" );
      metadata += QLatin1String( "<table class=\"list-view\">\n" );

      const QgsCodedFieldDomain *codedDomain = qgis::down_cast< QgsCodedFieldDomain *>( domain );
      const QList< QgsCodedValue > values = codedDomain->values();
      for ( const QgsCodedValue &value : values )
      {
        metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + value.code().toString() + QStringLiteral( "</td><td>" ) + value.value() + QStringLiteral( "</td></tr>\n" );
      }
      metadata += QLatin1String( "</table>\n<br><br>\n" );
      break;
    }

    case Qgis::FieldDomainType::Range:
    {
      const QgsRangeFieldDomain *rangeDomain = qgis::down_cast< QgsRangeFieldDomain *>( domain );

      metadata += QStringLiteral( "<h1>" ) + tr( "Range" ) + QStringLiteral( "</h1>\n<hr>\n" );
      metadata += QLatin1String( "<table class=\"list-view\">\n" );


      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Minimum" ) + QStringLiteral( "</td><td>" )
                  + QStringLiteral( "%1 %2" ).arg( rangeDomain->minimum().toString(),
                      rangeDomain->minimumIsInclusive() ? tr( "(inclusive)" ) : tr( "(exclusive)" ) )
                  + QStringLiteral( "</td></tr>\n" );
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Maximum" ) + QStringLiteral( "</td><td>" )
                  + QStringLiteral( "%1 %2" ).arg( rangeDomain->maximum().toString(),
                      rangeDomain->maximumIsInclusive() ? tr( "(inclusive)" ) : tr( "(exclusive)" ) )
                  + QStringLiteral( "</td></tr>\n" );

      metadata += QLatin1String( "</table>\n<br><br>\n" );
      break;
    }

    case Qgis::FieldDomainType::Glob:
    {
      const QgsGlobFieldDomain *globDomain = qgis::down_cast< QgsGlobFieldDomain *>( domain );

      metadata += QStringLiteral( "<h1>" ) + tr( "Glob" ) + QStringLiteral( "</h1>\n<hr>\n" );
      metadata += QLatin1String( "<table class=\"list-view\">\n" );

      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Pattern" ) + QStringLiteral( "</td><td>" ) + globDomain->glob() + QStringLiteral( "</td></tr>\n" );

      metadata += QLatin1String( "</table>\n<br><br>\n" );
      break;
    }

  }
  return metadata;
}

QgsFieldDomainDetailsWidget::~QgsFieldDomainDetailsWidget() = default;

//
// QgsFieldDomainsDetailsWidget
//

QgsFieldDomainsDetailsWidget::QgsFieldDomainsDetailsWidget( QWidget *parent, const QString &providerKey, const QString &uri )
  : QWidget( parent )
{
  setupUi( this );

  const QString style = QgsApplication::reportStyleSheet();
  mTextBrowser->document()->setDefaultStyleSheet( style );

  try
  {
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
    if ( md )
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( uri, {} ) ) };
      if ( conn && ( conn->capabilities() & QgsAbstractDatabaseProviderConnection::Capability::RetrieveFieldDomain ) )
      {
        QString domainError;
        QStringList fieldDomains;
        try
        {
          fieldDomains = conn->fieldDomainNames();
        }
        catch ( QgsProviderConnectionException &ex )
        {
          domainError = ex.what();
        }

        QString metadata = QStringLiteral( "<html>\n<body>\n" );
        metadata += QStringLiteral( "<h1>" ) + tr( "Field Domains" ) + QStringLiteral( "</h1>\n<hr>\n" ) + QStringLiteral( "<table class=\"list-view\">\n" );

        for ( const QString &name : std::as_const( fieldDomains ) )
        {
          try
          {
            std::unique_ptr< QgsFieldDomain > domain( conn->fieldDomain( name ) );
            if ( domain )
            {
              metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + domain->name() + QStringLiteral( "</td><td>" ) + domain->typeName()
                          + QStringLiteral( "</td><td>" ) + domain->description() + QStringLiteral( "</td></tr>\n" );
            }
          }
          catch ( QgsProviderConnectionException &ex )
          {
            QgsMessageLog::logMessage( ex.what() );
          }
        }
        metadata += QLatin1String( "</table>\n<br><br>\n" );

        if ( !domainError.isEmpty() )
        {
          mTextBrowser->setPlainText( domainError );
        }
        else
        {
          mTextBrowser->setHtml( metadata );
        }
      }
    }
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    mTextBrowser->setPlainText( ex.what() );
  }
}





//
// QgsRelationshipItemGuiProvider
//

QString QgsRelationshipItemGuiProvider::name()
{
  return QStringLiteral( "relationship_item" );
}

void QgsRelationshipItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext context )
{
  if ( QgsRelationshipItem *relationshipItem = qobject_cast< QgsRelationshipItem * >( item ) )
  {
    if ( QgsRelationshipsItem *relationshipsItem = qobject_cast< QgsRelationshipsItem * >( relationshipItem->parent() ) )
    {
      const QString providerKey = relationshipsItem->providerKey();
      const QString connectionUri = relationshipsItem->connectionUri();
      const QgsWeakRelation relation = relationshipItem->relation();

      // Check if relationship operations are supported
      if ( QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( providerKey ) )
      {
        std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };
        if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::UpdateRelationship ) )
        {
          QAction *editRelationshipAction = new QAction( tr( "Edit Relationship…" ), menu );

          QPointer< QgsDataItem > itemWeakPointer( item );

          connect( editRelationshipAction, &QAction::triggered, this, [ = ]
          {
            std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };

            QgsDbRelationDialog dialog( conn.release(), QgisApp::instance() );
            dialog.setWindowTitle( tr( "Edit Relationship" ) );
            dialog.setRelationship( relation );
            if ( dialog.exec() )
            {
              QgsWeakRelation relation = dialog.relationship();
              std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };
              try
              {
                conn->updateRelationship( relation );
                notify( QObject::tr( "Relationship Updated" ), QObject::tr( "Relationship '%1' was updated successfully." ).arg( relation.name() ), context, Qgis::MessageLevel::Success );
                if ( itemWeakPointer )
                {
                  itemWeakPointer->refresh();
                }
              }
              catch ( QgsProviderConnectionException &ex )
              {
                notify( QObject::tr( "Relationship Update Error" ), QObject::tr( "Error updating relationship '%1': %2" ).arg( relation.name(), ex.what() ), context, Qgis::MessageLevel::Critical );
              }
            }
          } );

          menu->addAction( editRelationshipAction );
        }
        if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::DeleteRelationship ) )
        {
          QAction *deleteRelationshipAction = new QAction( tr( "Delete Relationship…" ), menu );

          QPointer< QgsDataItem > itemWeakPointer( item );

          connect( deleteRelationshipAction, &QAction::triggered, this, [ = ]
          {
            if ( QMessageBox::question( nullptr, tr( "Delete Relationship" ),
                                        tr( "Are you sure you want to delete the %1 relationship?" ).arg( relation.name() ),
                                        QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
            {
              std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };
              try
              {
                conn->deleteRelationship( relation );
                notify( QObject::tr( "Relationship Deleted" ), QObject::tr( "Relationship '%1' was deleted successfully." ).arg( relation.name() ), context, Qgis::MessageLevel::Success );
                if ( itemWeakPointer )
                {
                  itemWeakPointer->parent()->refresh();
                }
              }
              catch ( QgsProviderConnectionException &ex )
              {
                notify( QObject::tr( "Relationship Deletion Error" ), QObject::tr( "Error deleting relationship '%1': %2" ).arg( relation.name(), ex.what() ), context, Qgis::MessageLevel::Critical );
              }
            }
          } );

          menu->addAction( deleteRelationshipAction );
        }
      }
    }
  }
  else if ( qobject_cast< QgsRelationshipsItem * >( item )
            || qobject_cast< QgsGeoPackageCollectionItem * >( item )
            || qobject_cast< QgsFileDataCollectionItem * >( item ) )
  {
    QString providerKey;
    QString connectionUri;
    if ( QgsRelationshipsItem *relationshipsItem = qobject_cast< QgsRelationshipsItem * >( item ) )
    {
      providerKey = relationshipsItem->providerKey();
      connectionUri = relationshipsItem->connectionUri();
    }
    else if ( QgsGeoPackageCollectionItem *gpkgItem = qobject_cast< QgsGeoPackageCollectionItem * >( item ) )
    {
      providerKey = QStringLiteral( "ogr" );
      connectionUri = gpkgItem->path().remove( QStringLiteral( "gpkg:/" ) );
    }
    else if ( QgsFileDataCollectionItem *fileItem = qobject_cast< QgsFileDataCollectionItem * >( item ) )
    {
      providerKey = QStringLiteral( "ogr" );
      connectionUri = fileItem->path();
    }

    // Check if relationship creation is supported
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
    if ( md )
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };

      if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::AddRelationship ) )
      {
        QAction *createRelationshipAction = new QAction( tr( "New Relationship…" ), menu );

        QPointer< QgsDataItem > itemWeakPointer( item );

        connect( createRelationshipAction, &QAction::triggered, this, [ = ]
        {
          std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };

          if ( conn->tables().isEmpty() )
          {
            notify( QString(), QObject::tr( "Relationships cannot be created in an empty database" ), context, Qgis::MessageLevel::Warning );
            return;
          }

          QgsDbRelationDialog dialog( conn.release(), QgisApp::instance() );
          dialog.setWindowTitle( tr( "New Relationship" ) );
          if ( dialog.exec() )
          {
            QgsWeakRelation relation = dialog.relationship();
            std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) };
            try
            {
              conn->addRelationship( relation );
              notify( QObject::tr( "New Relationship Created" ), QObject::tr( "Relationship '%1' was created successfully." ).arg( relation.name() ), context, Qgis::MessageLevel::Success );
              if ( itemWeakPointer )
              {
                itemWeakPointer->refresh();
              }
            }
            catch ( QgsProviderConnectionException &ex )
            {
              notify( QObject::tr( "Relationship Creation Error" ), QObject::tr( "Error creating new relationship '%1': %2" ).arg( relation.name(), ex.what() ), context, Qgis::MessageLevel::Critical );
            }
          }
        } );

        menu->addAction( createRelationshipAction );
      }
    }
  }
}

QWidget *QgsRelationshipItemGuiProvider::createParamWidget( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( QgsRelationshipItem *relationshipItem = qobject_cast< QgsRelationshipItem * >( item ) )
  {
    return new QgsRelationshipDetailsWidget( nullptr, relationshipItem->relation() );
  }
  else if ( QgsRelationshipsItem *relationsItem = qobject_cast< QgsRelationshipsItem * >( item ) )
  {
    return new QgsRelationshipsDetailsWidget( nullptr, relationsItem->providerKey(), relationsItem->connectionUri(), relationsItem->schema(), relationsItem->tableName() );
  }
  else
  {
    return nullptr;
  }
}

//
// QgsRelationshipDetailsWidget
//

QgsRelationshipDetailsWidget::QgsRelationshipDetailsWidget( QWidget *parent, const QgsWeakRelation &relation )
  : QWidget( parent )
  , mRelation( relation )
{
  setupUi( this );

  const QString style = QgsApplication::reportStyleSheet();
  mTextBrowser->document()->setDefaultStyleSheet( style );


  QString metadata = QStringLiteral( "<html>\n<body>\n" );
  metadata += htmlMetadata( mRelation, mRelation.name() );

  mTextBrowser->setHtml( metadata );
}

QString QgsRelationshipDetailsWidget::htmlMetadata( const QgsWeakRelation &relation, const QString &title )
{
  QString metadata;
  metadata += QStringLiteral( "<h1>" ) + title + QStringLiteral( "</h1>\n<hr>\n" ) + QStringLiteral( "<table class=\"list-view\">\n" );

  if ( title != relation.name() )
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Name" ) + QStringLiteral( "</td><td>" ) + relation.name() + QStringLiteral( "</td></tr>\n" );

  if ( relation.cardinality() != Qgis::RelationshipCardinality::ManyToMany )
  {
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Tables" ) + QStringLiteral( "</td><td>" )
                + QStringLiteral( "%1 → %2" ).arg( relation.referencedLayerName(),
                    relation.referencingLayerName() ) + QStringLiteral( "</td></tr>\n" );

    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Fields" ) + QStringLiteral( "</td><td>" );
    QStringList fieldMetadata;
    for ( int i = 0; i < std::min( relation.referencedLayerFields().size(), relation.referencingLayerFields().size() ); ++i )
    {
      fieldMetadata << QStringLiteral( "%1.%2 → %3.%4" ).arg(
                      relation.referencedLayerName(),
                      relation.referencedLayerFields().at( i ),
                      relation.referencingLayerName(),
                      relation.referencingLayerFields().at( i ) );
    }
    metadata += fieldMetadata.join( QLatin1String( "<br>" ) );
    metadata += QLatin1String( "</td></tr>\n" );
  }
  else
  {
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Tables" ) + QStringLiteral( "</td><td>" )
                + QStringLiteral( "%1 → %2 → %3" ).arg( relation.referencedLayerName(),
                    relation.mappingTableName(),
                    relation.referencingLayerName() ) + QStringLiteral( "</td></tr>\n" );

    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Fields" ) + QStringLiteral( "</td><td>" );
    QStringList fieldMetadata;
    for ( int i = 0; i < std::min( relation.referencedLayerFields().size(), relation.mappingReferencedLayerFields().size() ); ++i )
    {
      fieldMetadata << QStringLiteral( "%1.%2 → %3.%4" ).arg(
                      relation.referencedLayerName(),
                      relation.referencedLayerFields().at( i ),
                      relation.mappingTableName(),
                      relation.mappingReferencedLayerFields().at( i ) );
    }
    for ( int i = 0; i < std::min( relation.referencingLayerFields().size(), relation.mappingReferencingLayerFields().size() ); ++i )
    {
      fieldMetadata << QStringLiteral( "%1.%2 → %3.%4" ).arg(
                      relation.mappingTableName(),
                      relation.mappingReferencingLayerFields().at( i ),
                      relation.referencingLayerName(),
                      relation.referencingLayerFields().at( i )
                    );
    }
    metadata += fieldMetadata.join( QLatin1String( "<br>" ) );
    metadata += QLatin1String( "</td></tr>\n" );
  }

  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Cardinality" ) + QStringLiteral( "</td><td>" ) + QgsRelation::cardinalityToDisplayString( relation.cardinality() ) + QStringLiteral( "</td></tr>\n" );
  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Strength" ) + QStringLiteral( "</td><td>" ) + QgsRelation::strengthToDisplayString( relation.strength() ) + QStringLiteral( "</td></tr>\n" );

  if ( !relation.forwardPathLabel().isEmpty() )
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Forward label" ) + QStringLiteral( "</td><td>" ) + relation.forwardPathLabel() + QStringLiteral( "</td></tr>\n" );
  if ( !relation.backwardPathLabel().isEmpty() )
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Backward label" ) + QStringLiteral( "</td><td>" ) + relation.backwardPathLabel() + QStringLiteral( "</td></tr>\n" );

  if ( !relation.relatedTableType().isEmpty() )
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Relation type" ) + QStringLiteral( "</td><td>" ) + relation.relatedTableType() + QStringLiteral( "</td></tr>\n" );

  metadata += QLatin1String( "</table>" );

  return metadata;
}

QgsRelationshipDetailsWidget::~QgsRelationshipDetailsWidget() = default;

//
// QgsRelationshipsDetailsWidget
//

QgsRelationshipsDetailsWidget::QgsRelationshipsDetailsWidget( QWidget *parent, const QString &providerKey, const QString &uri, const QString &schema, const QString &tableName )
  : QWidget( parent )
{
  setupUi( this );

  const QString style = QgsApplication::reportStyleSheet();
  mTextBrowser->document()->setDefaultStyleSheet( style );

  try
  {
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
    if ( md )
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( uri, {} ) ) };
      if ( conn && ( conn->capabilities() & QgsAbstractDatabaseProviderConnection::Capability::RetrieveRelationships ) )
      {
        QString relationError;
        QList< QgsWeakRelation > relationships;
        try
        {
          relationships = conn->relationships( schema, tableName );
        }
        catch ( QgsProviderConnectionException &ex )
        {
          relationError = ex.what();
        }

        QString metadata = QStringLiteral( "<html>\n<body>\n" );
        metadata += QStringLiteral( "<h1>" ) + tr( "Relationships" ) + QStringLiteral( "</h1>\n<hr>\n" ) + QStringLiteral( "<table width=\"100%\" class=\"tabular-view\">\n" );

        int i = 0;

        for ( const QgsWeakRelation &relation : std::as_const( relationships ) )
        {
          QString rowClass;
          if ( i % 2 )
            rowClass = QStringLiteral( "class=\"odd-row\"" );

          metadata += QStringLiteral( "<tr %1><td class=\"highlight\">" ).arg( rowClass ) + relation.name() + QStringLiteral( "</td><td>" ) + relation.referencedLayerName()
                      + QStringLiteral( " → " ) + relation.referencingLayerName()
                      + QStringLiteral( "</td><td>" ) + QObject::tr( "%1 (%2)" ).arg( QgsRelation::cardinalityToDisplayString( relation.cardinality() ),
                          QgsRelation::strengthToDisplayString( relation.strength() ) )
                      + QStringLiteral( "</td></tr>\n" );
          i++;
        }
        metadata += QLatin1String( "</table>\n<br><br>\n" );

        if ( !relationError.isEmpty() )
        {
          mTextBrowser->setPlainText( relationError );
        }
        else
        {
          mTextBrowser->setHtml( metadata );
        }
      }
    }
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    mTextBrowser->setPlainText( ex.what() );
  }
}


//
// QgsFieldsDetailsWidget
//

QgsFieldsDetailsWidget::QgsFieldsDetailsWidget( QWidget *parent, const QString &providerKey, const QString &uri, const QString &schema,
    const QString &tableName )
  : QWidget( parent )
{
  setupUi( this );

  const QString style = QgsApplication::reportStyleSheet();
  mTextBrowser->document()->setDefaultStyleSheet( style );

  try
  {
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
    if ( md )
    {
      QString metadata = QStringLiteral( "<html>\n<body>\n" );

      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( uri, {} ) ) };
      if ( conn )
      {
        const QgsFields fields = conn->fields( schema, tableName );
        metadata += QStringLiteral( "<h1>" ) + tr( "Fields" ) + QStringLiteral( "</h1>\n<hr>\n" ) + QStringLiteral( "<table class=\"list-view\">\n" );

        // count fields
        metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Count" ) + QStringLiteral( "</td><td>" ) + QString::number( fields.size() ) + QStringLiteral( "</td></tr>\n" );

        metadata += QLatin1String( "</table>\n<br><table width=\"100%\" class=\"tabular-view\">\n" );
        metadata += QLatin1String( "<tr><th>" ) + tr( "Field" ) + QLatin1String( "</th><th>" ) + tr( "Type" ) + QLatin1String( "</th><th>" ) + tr( "Length" ) + QLatin1String( "</th><th>" ) + tr( "Precision" ) + QLatin1String( "</th><th>" ) + tr( "Comment" ) + QLatin1String( "</th></tr>\n" );

        for ( int i = 0; i < fields.size(); ++i )
        {
          QgsField myField = fields.at( i );
          QString rowClass;
          if ( i % 2 )
            rowClass = QStringLiteral( "class=\"odd-row\"" );
          metadata += QLatin1String( "<tr " ) + rowClass + QLatin1String( "><td>" ) + myField.displayNameWithAlias() + QLatin1String( "</td><td>" ) + myField.typeName() + QLatin1String( "</td><td>" ) + QString::number( myField.length() ) + QLatin1String( "</td><td>" ) + QString::number( myField.precision() ) + QLatin1String( "</td><td>" ) + myField.comment() + QLatin1String( "</td></tr>\n" );
        }
        metadata += QLatin1String( "</table>\n<br><br>\n" );

        mTextBrowser->setHtml( metadata );
      }
      else
      {
        mTextBrowser->setPlainText( tr( "Could not load layer fields" ) );
      }
    }
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    mTextBrowser->setPlainText( ex.what() );
  }
}

//
// QgsFieldDetailsWidget
//

QgsFieldDetailsWidget::QgsFieldDetailsWidget( QWidget *parent, const QString &providerKey, const QString &uri, const QString &, const QString &, const QgsField &field )
  : QWidget( parent )
{
  setupUi( this );

  const QString style = QgsApplication::reportStyleSheet();
  mTextBrowser->document()->setDefaultStyleSheet( style );

  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
  if ( md )
  {
    QString metadata = QStringLiteral( "<html>\n<body>\n" );

    metadata += QStringLiteral( "<h1>" ) + field.name() + QStringLiteral( "</h1>\n<hr>\n" ) + QStringLiteral( "<table class=\"list-view\">\n" );

    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Alias" ) + QStringLiteral( "</td><td>" ) + field.alias() + QStringLiteral( "</td></tr>\n" );
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Type" ) + QStringLiteral( "</td><td>" ) + field.displayType() + QStringLiteral( "</td></tr>\n" );
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Comment" ) + QStringLiteral( "</td><td>" ) + field.comment() + QStringLiteral( "</td></tr>\n" );

    metadata += QLatin1String( "</table>\n<br><br>\n" );
    if ( !field.constraints().domainName().isEmpty() )
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( uri, {} ) ) };
      bool foundDomainMetadata = false;
      try
      {
        // try to retrieve full domain details if possible
        std::unique_ptr< QgsFieldDomain > domain( conn->fieldDomain( field.constraints().domainName() ) );
        if ( domain )
        {
          metadata += QgsFieldDomainDetailsWidget::htmlMetadata( domain.get(), tr( "Domain" ) );
          foundDomainMetadata = true;
        }
      }
      catch ( const QgsProviderConnectionException & )
      {}

      if ( !foundDomainMetadata )
      {
        metadata += QStringLiteral( "<h1>" ) + tr( "Domain" ) + QStringLiteral( "</h1>\n<hr>\n" ) + QStringLiteral( "<table class=\"list-view\">\n" );
        metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Name" ) + QStringLiteral( "</td><td>" ) + field.constraints().domainName() + QStringLiteral( "</td></tr>\n" );
        metadata += QLatin1String( "</table>\n<br><br>\n" );
      }
    }
    mTextBrowser->setHtml( metadata );
  }
  else
  {
    mTextBrowser->setPlainText( tr( "Could not load field" ) );
  }
}
