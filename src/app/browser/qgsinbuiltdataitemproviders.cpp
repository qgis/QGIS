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
#include "qgsnewnamedialog.h"
#include "qgsbrowsermodel.h"
#include "qgsbrowserdockwidget_p.h"
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QFileDialog>

QString QgsAppDirectoryItemGuiProvider::name()
{
  return QStringLiteral( "directory_items" );
}

void QgsAppDirectoryItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( item->type() != QgsDataItem::Directory )
    return;

  QgsDirectoryItem *directoryItem = qobject_cast< QgsDirectoryItem * >( item );

  QgsSettings settings;


  QAction *createFolder = new QAction( tr( "New Directory…" ), menu );
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
  menu->addAction( createFolder );

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
      menu->addSeparator();
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

  QAction *openFolder = new QAction( tr( "Open Directory…" ), menu );
  connect( openFolder, &QAction::triggered, this, [ = ]
  {
    QDesktopServices::openUrl( QUrl::fromLocalFile( directoryItem->dirPath() ) );
  } );
  menu->addAction( openFolder );

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
