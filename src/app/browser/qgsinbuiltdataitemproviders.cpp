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
#include "qgswindowmanagerinterface.h"
#include "qgsrasterlayer.h"
#include "qgsnewvectorlayerdialog.h"
#include "qgsnewgeopackagelayerdialog.h"
#include "qgsfileutils.h"
#include "qgsapplication.h"
#include "processing/qgsprojectstylealgorithms.h"
#include "qgsstylemanagerdialog.h"
#include "qgsproviderregistry.h"
#include "qgsaddattrdialog.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsprovidermetadata.h"
#include "qgsnewvectortabledialog.h"
#include "qgsdataitemproviderregistry.h"

#include <QFileInfo>
#include <QMenu>
#include <QInputDialog>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

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

void QgsLayerItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context )
{
  if ( item->type() != QgsDataItem::Layer )
    return;

  QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item );

  if ( layerItem )
  {
    // Check for certain file items
    QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( layerItem->providerKey(), layerItem->uri() );
    const QString filename = parts.value( QStringLiteral( "path" ) ).toString();
    if ( !filename.isEmpty() )
    {
      QFileInfo fi( filename );

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
          // pdf file
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
  }

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
          const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
          std::unique_ptr<QgsVectorLayer> layer( new QgsVectorLayer( layerItem->uri(), layerItem->name(), layerItem->providerKey(), options ) );
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
        case QgsMapLayerType::VectorTileLayer:
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
      deleteLayers( selectedDeletableItemPaths, context );
    } );
    menu->addAction( deleteAction );
  }

  QAction *propertiesAction = new QAction( tr( "Layer Properties…" ), menu );
  connect( propertiesAction, &QAction::triggered, this, [ = ]
  {
    showPropertiesForItem( layerItem, context );
  } );
  menu->addAction( propertiesAction );

  if ( QgsGui::nativePlatformInterface()->capabilities() & QgsNative::NativeFilePropertiesDialog )
  {
    bool isFile = false;
    if ( layerItem )
    {
      // Also check for postgres layers (rasters are handled by GDAL)
      isFile = ( layerItem->providerKey() == QStringLiteral( "ogr" ) ||
                 layerItem->providerKey() == QStringLiteral( "gdal" ) ) &&
               ! layerItem->uri().startsWith( QStringLiteral( "PG:" ) );
    }
    else
    {
      isFile = QFileInfo::exists( item->path() );
    }
    if ( isFile )
    {
      QAction *action = menu->addAction( tr( "File Properties…" ) );
      connect( action, &QAction::triggered, this, [ = ]
      {
        QgsGui::nativePlatformInterface()->showFileProperties( item->path() );
      } );
    }
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

    QAction *extractAction = new QAction( tr( "Extract Symbols…" ), menu );
    connect( extractAction, &QAction::triggered, this, [projectPath, context]
    {
      QgsStyle style;
      style.createMemoryDatabase();

      QgsSaveToStyleVisitor visitor( &style );

      QgsProject p;
      QgsTemporaryCursorOverride override( Qt::WaitCursor );
      if ( p.read( projectPath, QgsProject::ReadFlag::FlagDontResolveLayers ) )
      {
        p.accept( &visitor );
        override.release();
        QgsStyleManagerDialog dlg( &style, QgisApp::instance(), nullptr, true );
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

QString QgsFieldsItemGuiProvider::name()
{
  return QStringLiteral( "fields_item" );
}

void QgsFieldsItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context )
{
  Q_UNUSED( selectedItems )

  if ( !item || item->type() != QgsDataItem::Type::Fields )
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
        const QString itemName { item->name() };

        connect( addColumnAction, &QAction::triggered, fieldsItem, [ md, fieldsItem, context, itemPtr, menu ]
        {
          std::unique_ptr<QgsVectorLayer> layer { fieldsItem->layer() };
          if ( layer )
          {
            QgsAddAttrDialog dialog( layer.get(), menu );
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
            notify( tr( "New Field" ), tr( "Failed to load layer'%1'. Check application logs and user permissions." ).arg( fieldsItem->tableName() ), context, Qgis::MessageLevel::Critical );
          }
        } );
        menu->addAction( addColumnAction );
      }
    }
  }
}

QString QgsFieldItemGuiProvider::name()
{
  return QStringLiteral( "field_item" );
}

void QgsFieldItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context )
{
  Q_UNUSED( selectedItems )

  if ( !item || item->type() != QgsDataItem::Type::Field )
    return;

  if ( QgsFieldItem *fieldItem = qobject_cast<QgsFieldItem *>( item ) )
  {
    // Retrieve the connection from the parent
    QgsFieldsItem *fieldsItem { static_cast<QgsFieldsItem *>( fieldItem->parent() ) };
    if ( fieldsItem )
    {
      // Check if it is supported
      QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( fieldsItem->providerKey() ) };
      if ( md )
      {
        std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( fieldsItem->connectionUri(), {} ) ) };
        if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::DeleteField ) )
        {
          QAction *deleteFieldAction = new QAction( tr( "Delete Field…" ), menu );
          const bool supportsCascade { conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::DeleteFieldCascade ) };
          const QString itemName { item->name() };

          connect( deleteFieldAction, &QAction::triggered, fieldsItem, [ md, fieldsItem, itemName, context, supportsCascade ]
          {
            // Confirmation dialog
            QMessageBox msgbox{QMessageBox::Icon::Question, tr( "Delete Field" ), tr( "Delete '%1' permanently?" ).arg( itemName ), QMessageBox::Ok | QMessageBox::Cancel };
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
          menu->addAction( deleteFieldAction );
        }
      }
    }
    else
    {
      // This should never happen!
      QgsDebugMsg( QStringLiteral( "Error getting parent fields for %1" ).arg( item->name() ) );
    }
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
    if ( QgsDataCollectionItem * collectionItem { qobject_cast<QgsDataCollectionItem *>( item ) } )
    {
      // This is super messy: we need the QgsDataProvider key and NOT the QgsDataItemProvider key!
      const QString dataProviderKey { QgsApplication::dataItemProviderRegistry()->dataProviderKey( collectionItem->providerKey() ) };
      QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( dataProviderKey ) };
      if ( md )
      {
        // Note: we could have used layerCollection() but casting to QgsDatabaseSchemaItem is more explicit
        const bool isSchema { qobject_cast<QgsDatabaseSchemaItem *>( item ) != nullptr };
        const QString connectionName { isSchema ? collectionItem->parent()->name() : collectionItem->name() };
        std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionName ) ) );
        if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::CreateVectorTable ) )
        {
          QAction *newTableAction = new QAction( QObject::tr( "New Table…" ), menu );
          QObject::connect( newTableAction, &QAction::triggered, collectionItem, [ collectionItem, connectionName, md, isSchema, context]
          {
            std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn2 { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionName ) ) };
            QgsNewVectorTableDialog dlg { conn2.get(), nullptr };
            dlg.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );
            if ( isSchema )
            {
              dlg.setSchemaName( collectionItem->name() );
            }
            if ( dlg.exec() == QgsNewVectorTableDialog::DialogCode::Accepted )
            {
              const QgsFields fields { dlg.fields() };
              const QString tableName { dlg.tableName() };
              const QString schemaName { dlg.schemaName() };
              const QString geometryColumn { dlg.geometryColumnName() };
              const QgsWkbTypes::Type geometryType { dlg.geometryType() };
              const bool createSpatialIndex { dlg.createSpatialIndex() &&
                                              geometryType != QgsWkbTypes::NoGeometry &&
                                              geometryType != QgsWkbTypes::Unknown };
              const QgsCoordinateReferenceSystem crs { dlg.crs( ) };
              // This flag tells to the provider that field types do not need conversion
              QMap<QString, QVariant> options { { QStringLiteral( "skipConvertFields" ), true } };

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
                  const auto constChildren { collectionItem->children() };
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
                  collectionItem->refresh( );
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
      }
    }
  }
}
