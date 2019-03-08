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
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsnative.h"
#include "qgisapp.h"
#include "qgsmessagebar.h"
#include "qgsmessagelog.h"
#include "qgsnewnamedialog.h"
#include "qgsbrowsermodel.h"
#include "qgsbrowserdockwidget_p.h"
#include "qgswindowmanagerinterface.h"
#include "qgsrasterlayer.h"
#include "qgsnewvectorlayerdialog.h"
#include "qgsnewgeopackagelayerdialog.h"
#include "qgsfileutils.h"
#include "qgsapplication.h"
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QFileDialog>

QString QgsAppDirectoryItemGuiProvider::name()
{
  return QStringLiteral( "directory_items" );
}

void QgsAppDirectoryItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext context )
{
  if ( item->type() != QgsDataItem::Directory )
    return;

  QgsDirectoryItem *directoryItem = qobject_cast< QgsDirectoryItem * >( item );

  QgsSettings settings;

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
        QMessageBox::critical( QgisApp::instance(), tr( "Create Directory" ), tr( "The path “%1” already exists." ).arg( QDir::toNativeSeparators( dir.absoluteFilePath( name ) ) ) );
      }
      else if ( !dir.mkdir( name ) )
      {
        QMessageBox::critical( QgisApp::instance(), tr( "Create Directory" ), tr( "Could not create directory “%1”." ).arg( QDir::toNativeSeparators( dir.absoluteFilePath( name ) ) ) );
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
    QgsNewGeoPackageLayerDialog dialog( QgisApp::instance() );
    QDir dir( directoryItem->dirPath() );
    dialog.setDatabasePath( dir.filePath( QStringLiteral( "new_geopackage" ) ) );
    dialog.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );
    dialog.setAddToProject( false );
    if ( dialog.exec() )
    {
      QString file = dialog.databasePath();
      file = QgsFileUtils::ensureFileNameHasExtension( file, QStringList() << QStringLiteral( "gpkg" ) );
      context.messageBar()->pushSuccess( tr( "New GeoPackage" ), tr( "Created <a href=\"%1\">%2</a>" ).arg(
                                           QUrl::fromLocalFile( file ).toString(), QDir::toNativeSeparators( file ) ) );
      item->refresh();
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

  menu->addMenu( newMenu );

  menu->addSeparator();

  bool inFavDirs = item->parent() && item->parent()->type() == QgsDataItem::Favorites;
  if ( item->parent() && !inFavDirs )
  {
    // only non-root directories can be added as favorites
    QAction *addAsFavorite = new QAction( tr( "Add as a Favorite" ), menu );
    addAsFavorite->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFavourites.svg" ) ) );
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

  QAction *fastScanAction = new QAction( tr( "Fast Scan this Directory" ), menu );
  connect( fastScanAction, &QAction::triggered, this, [ = ]
  {
    toggleFastScan( directoryItem );
  } );
  menu->addAction( fastScanAction );
  fastScanAction->setCheckable( true );
  fastScanAction->setChecked( settings.value( QStringLiteral( "qgis/scanItemsFastScanUris" ),
                              QStringList() ).toStringList().contains( item->path() ) );

  menu->addSeparator();

  QAction *openFolder = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mIconFolder.svg" ) ), tr( "Open Directory…" ), menu );
  connect( openFolder, &QAction::triggered, this, [ = ]
  {
    QDesktopServices::openUrl( QUrl::fromLocalFile( directoryItem->dirPath() ) );
  } );
  menu->addAction( openFolder );

  if ( QgsGui::instance()->nativePlatformInterface()->capabilities() & QgsNative::NativeOpenTerminalAtPath )
  {
    QAction *openTerminal = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionTerminal.svg" ) ), tr( "Open in Terminal…" ), menu );
    connect( openTerminal, &QAction::triggered, this, [ = ]
    {
      QgsGui::instance()->nativePlatformInterface()->openTerminalAtPath( directoryItem->dirPath() );
    } );
    menu->addAction( openTerminal );
    menu->addSeparator();
  }

  QAction *propertiesAction = new QAction( tr( "Properties…" ), menu );
  connect( propertiesAction, &QAction::triggered, this, [ = ]
  {
    showProperties( directoryItem );
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

void QgsAppDirectoryItemGuiProvider::showProperties( QgsDirectoryItem *item )
{
  if ( ! item )
    return;

  QgsBrowserPropertiesDialog *dialog = new QgsBrowserPropertiesDialog( QStringLiteral( "browser" ), QgisApp::instance() );
  dialog->setAttribute( Qt::WA_DeleteOnClose );

  dialog->setItem( item );
  dialog->show();
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
  if ( item->type() != QgsDataItem::Favorites )
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

void QgsLayerItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext )
{
  if ( item->type() != QgsDataItem::Layer )
    return;

  QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item );
  if ( layerItem && ( layerItem->mapLayerType() == QgsMapLayerType::VectorLayer ||
                      layerItem->mapLayerType() == QgsMapLayerType::RasterLayer ) )
  {
    QMenu *exportMenu = new QMenu( tr( "Export Layer" ), menu );
    menu->addMenu( exportMenu );
    QAction *toFileAction = new QAction( tr( "To File…" ), exportMenu );
    exportMenu->addAction( toFileAction );
    connect( toFileAction, &QAction::triggered, layerItem, [ layerItem ]
    {
      switch ( layerItem->mapLayerType() )
      {
        case QgsMapLayerType::VectorLayer:
        {
          std::unique_ptr<QgsVectorLayer> layer( new QgsVectorLayer( layerItem->uri(), layerItem->name(), layerItem->providerKey() ) );
          if ( layer && layer->isValid() )
          {
            QgisApp::instance()->saveAsFile( layer.get(), false, false );
          }
          break;
        }

        case QgsMapLayerType::RasterLayer:
        {
          std::unique_ptr<QgsRasterLayer> layer( new QgsRasterLayer( layerItem->uri(), layerItem->name(), layerItem->providerKey() ) );
          if ( layer && layer->isValid() )
          {
            QgisApp::instance()->saveAsFile( layer.get(), false, false );
          }
          break;
        }

        case QgsMapLayerType::PluginLayer:
        case QgsMapLayerType::MeshLayer:
          break;
      }
    } );
  }

  const QString addText = selectedItems.count() == 1 ? tr( "Add Layer to Project" )
                          : tr( "Add Selected Layers to Project" );
  QAction *addAction = new QAction( addText, menu );
  connect( addAction, &QAction::triggered, this, [ = ]
  {
    addLayersFromItems( selectedItems );
  } );
  menu->addAction( addAction );

  if ( item->capabilities2() & QgsDataItem::Delete )
  {
    QStringList selectedDeletableItemPaths;
    for ( QgsDataItem *selectedItem : selectedItems )
    {
      if ( qobject_cast<QgsLayerItem *>( selectedItem ) && ( selectedItem->capabilities2() & QgsDataItem::Delete ) )
        selectedDeletableItemPaths.append( qobject_cast<QgsLayerItem *>( selectedItem )->uri() );
    }

    const QString deleteText = selectedDeletableItemPaths.count() == 1 ? tr( "Delete Layer" )
                               : tr( "Delete Selected Layers" );
    QAction *deleteAction = new QAction( deleteText, menu );
    connect( deleteAction, &QAction::triggered, this, [ = ]
    {
      deleteLayers( selectedDeletableItemPaths );
    } );
    menu->addAction( deleteAction );
  }

  QAction *propertiesAction = new QAction( tr( "Layer Properties…" ), menu );
  connect( propertiesAction, &QAction::triggered, this, [ = ]
  {
    showPropertiesForItem( layerItem );
  } );
  menu->addAction( propertiesAction );

  if ( QgsGui::nativePlatformInterface()->capabilities() & QgsNative::NativeFilePropertiesDialog )
  {
    QAction *action = menu->addAction( tr( "File Properties…" ) );
    connect( action, &QAction::triggered, this, [ = ]
    {
      QgsGui::nativePlatformInterface()->showFileProperties( item->path() );
    } );
  }
}

bool QgsLayerItemGuiProvider::handleDoubleClick( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( !item || item->type() != QgsDataItem::Layer )
    return false;

  if ( QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item ) )
  {
    const QgsMimeDataUtils::UriList layerUriList = QgsMimeDataUtils::UriList() << layerItem->mimeUri();
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
    if ( item && item->type() == QgsDataItem::Project )
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
    if ( item && item->type() == QgsDataItem::Layer )
    {
      if ( QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item ) )
        layerUriList << layerItem->mimeUri();
    }
  }
  if ( !layerUriList.isEmpty() )
    QgisApp::instance()->handleDropUriList( layerUriList );
}

void QgsLayerItemGuiProvider::deleteLayers( const QStringList &itemPaths )
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
    if ( !item->deleteLayer() )
      QMessageBox::information( QgisApp::instance(), tr( "Delete Layer" ), tr( "Item Layer %1 cannot be deleted." ).arg( item->name() ) );
  }
}

void QgsLayerItemGuiProvider::showPropertiesForItem( QgsLayerItem *item )
{
  if ( ! item )
    return;

  QgsBrowserPropertiesDialog *dialog = new QgsBrowserPropertiesDialog( QStringLiteral( "browser" ), QgisApp::instance() );
  dialog->setAttribute( Qt::WA_DeleteOnClose );
  dialog->setItem( item );
  dialog->show();
}

//
// QgsProjectItemGuiProvider
//

QString QgsProjectItemGuiProvider::name()
{
  return QStringLiteral( "project_items" );
}

void QgsProjectItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( !item || item->type() != QgsDataItem::Project )
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

    if ( QgsGui::nativePlatformInterface()->capabilities() & QgsNative::NativeFilePropertiesDialog )
    {
      QAction *action = menu->addAction( tr( "File Properties…" ) );
      connect( action, &QAction::triggered, this, [projectPath]
      {
        QgsGui::nativePlatformInterface()->showFileProperties( projectPath );
      } );
    }
  }
}

bool QgsProjectItemGuiProvider::handleDoubleClick( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( !item || item->type() != QgsDataItem::Project )
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
