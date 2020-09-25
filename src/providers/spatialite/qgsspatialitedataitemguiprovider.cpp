/***************************************************************************
  qgsspatialitedataitemguiprovider.cpp
  --------------------------------------
  Date                 : June 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsspatialitedataitemguiprovider.h"
#include "qgsspatialiteconnection.h"
#include "qgsspatialitedataitems.h"
#include "qgsspatialitesourceselect.h"

#include "qgsapplication.h"
#include "qgsmessageoutput.h"
#include "qgsmimedatautils.h"
#include "qgssettings.h"

#include <QFileDialog>
#include <QMessageBox>


void QgsSpatiaLiteDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{
  if ( QgsSLRootItem *rootItem = qobject_cast< QgsSLRootItem * >( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), this );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );

    QAction *actionCreateDatabase = new QAction( tr( "Create Database…" ), this );
    connect( actionCreateDatabase, &QAction::triggered, this, [rootItem] { createDatabase( rootItem ); } );
    menu->addAction( actionCreateDatabase );
  }

  if ( QgsSLConnectionItem *connItem = qobject_cast< QgsSLConnectionItem * >( item ) )
  {
    QAction *actionDelete = new QAction( tr( "Delete" ), this );
    connect( actionDelete, &QAction::triggered, this, [connItem] { deleteConnection( connItem ); } );
    menu->addAction( actionDelete );
  }
}

bool QgsSpatiaLiteDataItemGuiProvider::deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context )
{
  if ( QgsSLLayerItem *layerItem = qobject_cast< QgsSLLayerItem * >( item ) )
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Object" ),
                                QObject::tr( "Are you sure you want to delete %1?" ).arg( layerItem->name() ),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return false;

    QgsDataSourceUri uri( layerItem->uri() );
    QString errCause;
    if ( !SpatiaLiteUtils::deleteLayer( uri.database(), uri.table(), errCause ) )
    {
      notify( tr( "Delete Layer" ), errCause, context, Qgis::MessageLevel::Warning );
    }
    else
    {
      notify( tr( "Delete Layer" ), tr( "Layer deleted successfully." ), context, Qgis::MessageLevel::Success );
      if ( layerItem->parent() )
        layerItem->parent()->refresh();
      return true;
    }
  }
  return false;
}

bool QgsSpatiaLiteDataItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( qobject_cast< QgsSLConnectionItem * >( item ) )
    return true;

  return false;
}

bool QgsSpatiaLiteDataItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext, const QMimeData *data, Qt::DropAction action )
{
  if ( QgsSLConnectionItem *connItem = qobject_cast< QgsSLConnectionItem * >( item ) )
  {
    return handleDropConnectionItem( connItem, data, action );
  }
  return false;
}


void QgsSpatiaLiteDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  if ( QgsSpatiaLiteSourceSelect::newConnection( nullptr ) )
  {
    item->refreshConnections();
  }
}

void QgsSpatiaLiteDataItemGuiProvider::createDatabase( QgsDataItem *item )
{
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSpatiaLiteDir" ), QDir::homePath() ).toString();

  QString filename = QFileDialog::getSaveFileName( nullptr, tr( "New SpatiaLite Database File" ),
                     lastUsedDir,
                     tr( "SpatiaLite" ) + " (*.sqlite *.db *.sqlite3 *.db3 *.s3db)" );
  if ( filename.isEmpty() )
    return;

  QString errCause;
  if ( SpatiaLiteUtils::createDb( filename, errCause ) )
  {
    // add connection
    settings.setValue( "/SpatiaLite/connections/" + QFileInfo( filename ).fileName() + "/sqlitepath", filename );

    item->refresh();
  }
  else
  {
    QMessageBox::critical( nullptr, tr( "Create SpatiaLite database" ), tr( "Failed to create the database:\n" ) + errCause );
  }
}

void QgsSpatiaLiteDataItemGuiProvider::deleteConnection( QgsDataItem *item )
{
  if ( QMessageBox::question( nullptr, QObject::tr( "Delete Connection" ),
                              QObject::tr( "Are you sure you want to delete the connection to %1?" ).arg( item->name() ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsSpatiaLiteConnection::deleteConnection( item->name() );
  // the parent should be updated
  item->parent()->refreshConnections();
}

bool QgsSpatiaLiteDataItemGuiProvider::handleDropConnectionItem( QgsSLConnectionItem *connItem, const QMimeData *data, Qt::DropAction )
{
  if ( !QgsMimeDataUtils::isUriList( data ) )
    return false;

  // TODO: probably should show a GUI with settings etc

  QgsDataSourceUri destUri;
  destUri.setDatabase( connItem->databasePath() );

  QStringList importResults;
  bool hasError = false;

  QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
  const auto constLst = lst;
  for ( const QgsMimeDataUtils::Uri &u : constLst )
  {
    // open the source layer
    bool owner;
    QString error;
    QgsVectorLayer *srcLayer = u.vectorLayer( owner, error );
    if ( !srcLayer )
    {
      importResults.append( tr( "%1: %2" ).arg( u.name, error ) );
      hasError = true;
      continue;
    }

    if ( srcLayer->isValid() )
    {
      destUri.setDataSource( QString(), u.name, srcLayer->geometryType() != QgsWkbTypes::NullGeometry ? QStringLiteral( "geom" ) : QString() );
      QgsDebugMsg( "URI " + destUri.uri() );

      std::unique_ptr< QgsVectorLayerExporterTask > exportTask( new QgsVectorLayerExporterTask( srcLayer, destUri.uri(), QStringLiteral( "spatialite" ), srcLayer->crs(), QVariantMap(), owner ) );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, connItem, [ = ]()
      {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to SpatiaLite database" ), tr( "Import was successful." ) );
        connItem->refresh();
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, connItem, [ = ]( int error, const QString & errorMessage )
      {
        if ( error != QgsVectorLayerExporter::ErrUserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to SpatiaLite database" ) );
          output->setMessage( tr( "Failed to import layer!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
          output->showMessage();
        }
        connItem->refresh();
      } );

      QgsApplication::taskManager()->addTask( exportTask.release() );
    }
    else
    {
      importResults.append( tr( "%1: Not a valid layer!" ).arg( u.name ) );
      hasError = true;
    }
  }

  if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to SpatiaLite database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QStringLiteral( "\n" ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}
