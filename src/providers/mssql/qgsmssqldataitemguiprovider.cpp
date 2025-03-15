/***************************************************************************
  qgsmssqldataitemguiprovider.cpp
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

#include "qgsmanageconnectionsdialog.h"
#include "qgsmssqlconnection.h"
#include "qgsmssqldataitemguiprovider.h"
#include "moc_qgsmssqldataitemguiprovider.cpp"
#include "qgsmssqldataitems.h"
#include "qgsmssqlnewconnection.h"
#include "qgsmssqlsourceselect.h"
#include "qgsdataitemguiproviderutils.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgstaskmanager.h"
#include "qgsvectorlayerexporter.h"
#include "qgsmessageoutput.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsdbimportvectorlayerdialog.h"
#include "qgsbrowsertreeview.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>


void QgsMssqlDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context )
{
  if ( QgsMssqlRootItem *rootItem = qobject_cast<QgsMssqlRootItem *>( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), menu );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );

    QAction *actionSaveServers = new QAction( tr( "Save Connections…" ), menu );
    connect( actionSaveServers, &QAction::triggered, this, [] { saveConnections(); } );
    menu->addAction( actionSaveServers );

    QAction *actionLoadServers = new QAction( tr( "Load Connections…" ), menu );
    connect( actionLoadServers, &QAction::triggered, this, [rootItem] { loadConnections( rootItem ); } );
    menu->addAction( actionLoadServers );
  }
  else if ( QgsMssqlConnectionItem *connItem = qobject_cast<QgsMssqlConnectionItem *>( item ) )
  {
    const QList<QgsMssqlConnectionItem *> mssqlConnectionItems = QgsDataItem::filteredItems<QgsMssqlConnectionItem>( selection );

    if ( mssqlConnectionItems.size() == 1 )
    {
      QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
      connect( actionRefresh, &QAction::triggered, this, [connItem] {
        connItem->refresh();
        if ( connItem->parent() )
          connItem->parent()->refreshConnections();
      } );
      menu->addAction( actionRefresh );

      menu->addSeparator();

      QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
      connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
      menu->addAction( actionEdit );

      QAction *actionDuplicate = new QAction( tr( "Duplicate Connection" ), menu );
      connect( actionDuplicate, &QAction::triggered, this, [connItem] { duplicateConnection( connItem ); } );
      menu->addAction( actionDuplicate );
    }

    QAction *actionDelete = new QAction( mssqlConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [mssqlConnectionItems, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( mssqlConnectionItems, []( const QString &connectionName ) { QgsMssqlSourceSelect::deleteConnection( connectionName ); }, context );
    } );
    menu->addAction( actionDelete );

    if ( mssqlConnectionItems.size() == 1 )
    {
      menu->addSeparator();

      QAction *actionShowNoGeom = new QAction( tr( "Show Non-spatial Tables" ), menu );
      actionShowNoGeom->setCheckable( true );
      actionShowNoGeom->setChecked( connItem->allowGeometrylessTables() );
      connect( actionShowNoGeom, &QAction::toggled, connItem, &QgsMssqlConnectionItem::setAllowGeometrylessTables );
      menu->addAction( actionShowNoGeom );

      QAction *actionCreateSchema = new QAction( tr( "New Schema…" ), menu );
      connect( actionCreateSchema, &QAction::triggered, this, [connItem] { createSchema( connItem ); } );
      menu->addAction( actionCreateSchema );
    }
  }
  else if ( QgsMssqlSchemaItem *schemaItem = qobject_cast<QgsMssqlSchemaItem *>( item ) )
  {
    QAction *importVectorAction = new QAction( QObject::tr( "Import Vector Layer…" ), menu );
    menu->addAction( importVectorAction );
    const QString destinationSchema = schemaItem->name();
    QgsMssqlConnectionItem *connItem = qobject_cast<QgsMssqlConnectionItem *>( schemaItem->parent() );
    QObject::connect( importVectorAction, &QAction::triggered, item, [connItem, context, destinationSchema, this] { handleImportVector( connItem, destinationSchema, context ); } );

    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [schemaItem] {
      if ( schemaItem->parent() )
        schemaItem->parent()->refresh();
    } );
    menu->addAction( actionRefresh );
  }
  else if ( QgsMssqlLayerItem *layerItem = qobject_cast<QgsMssqlLayerItem *>( item ) )
  {
    QMenu *maintainMenu = new QMenu( tr( "Table Operations" ), menu );

    // truncate
    QAction *actionTruncateLayer = new QAction( tr( "Truncate Table" ), menu );
    connect( actionTruncateLayer, &QAction::triggered, this, [layerItem] { truncateTable( layerItem ); } );
    maintainMenu->addAction( actionTruncateLayer );

    menu->addMenu( maintainMenu );
  }
}

bool QgsMssqlDataItemGuiProvider::deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context )
{
  if ( QgsMssqlLayerItem *layerItem = qobject_cast<QgsMssqlLayerItem *>( item ) )
  {
    QgsMssqlConnectionItem *connItem = qobject_cast<QgsMssqlConnectionItem *>( layerItem->parent() ? layerItem->parent()->parent() : nullptr );
    const QgsMssqlLayerProperty &layerInfo = layerItem->layerInfo();
    const QString typeName = layerInfo.isView ? tr( "View" ) : tr( "Table" );

    if ( QMessageBox::question( nullptr, QObject::tr( "Delete %1" ).arg( typeName ), QObject::tr( "Are you sure you want to delete [%1].[%2]?" ).arg( layerInfo.schemaName, layerInfo.tableName ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return true;

    QString errCause;
    bool res;
    if ( layerInfo.isView )
      res = QgsMssqlConnection::dropView( layerItem->uri(), &errCause );
    else
      res = QgsMssqlConnection::dropTable( layerItem->uri(), &errCause );

    if ( !res )
    {
      notify( tr( "Delete %1" ).arg( typeName ), errCause, context, Qgis::MessageLevel::Warning );
    }
    else
    {
      notify( tr( "Delete %1" ).arg( typeName ), tr( "%1 deleted successfully." ).arg( typeName ), context, Qgis::MessageLevel::Success );
      if ( connItem )
        connItem->refresh();
    }
    return true;
  }
  return false;
}

bool QgsMssqlDataItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( qobject_cast<QgsMssqlConnectionItem *>( item ) )
    return true;
  if ( qobject_cast<QgsMssqlSchemaItem *>( item ) )
    return true;

  return false;
}

bool QgsMssqlDataItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction )
{
  if ( QgsMssqlConnectionItem *connItem = qobject_cast<QgsMssqlConnectionItem *>( item ) )
  {
    return handleDrop( connItem, data, QString(), context );
  }
  else if ( QgsMssqlSchemaItem *schemaItem = qobject_cast<QgsMssqlSchemaItem *>( item ) )
  {
    QgsMssqlConnectionItem *connItem = qobject_cast<QgsMssqlConnectionItem *>( schemaItem->parent() );
    if ( !connItem )
      return false;

    return handleDrop( connItem, data, schemaItem->name(), context );
  }
  return false;
}

void QgsMssqlDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsMssqlNewConnection nc( nullptr );
  if ( nc.exec() )
  {
    item->refreshConnections();
  }
}

void QgsMssqlDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsMssqlNewConnection nc( nullptr, item->name() );
  if ( nc.exec() )
  {
    // the parent should be updated
    item->parent()->refreshConnections();
    item->refresh();
  }
}

void QgsMssqlDataItemGuiProvider::duplicateConnection( QgsDataItem *item )
{
  const QString connectionName = item->name();
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/MSSQL/connections" ) );
  const QStringList connections = settings.childGroups();
  settings.endGroup();

  const QString newConnectionName = QgsDataItemGuiProviderUtils::uniqueName( connectionName, connections );

  QgsMssqlConnection::duplicateConnection( connectionName, newConnectionName );

  item->parent()->refreshConnections();
  item->refresh();
}


void QgsMssqlDataItemGuiProvider::createSchema( QgsMssqlConnectionItem *connItem )
{
  const QString schemaName = QInputDialog::getText( nullptr, tr( "Create Schema" ), tr( "Schema name:" ) );
  if ( schemaName.isEmpty() )
    return;

  const QString uri = connItem->connectionUri();
  QString error;
  if ( !QgsMssqlConnection::createSchema( uri, schemaName, &error ) )
  {
    QMessageBox::warning( nullptr, tr( "Create Schema" ), tr( "Unable to create schema %1\n%2" ).arg( schemaName, error ) );
    return;
  }

  connItem->refresh();
  // the parent should be updated
  if ( connItem->parent() )
    connItem->parent()->refreshConnections();
}


void QgsMssqlDataItemGuiProvider::truncateTable( QgsMssqlLayerItem *layerItem )
{
  const QgsMssqlLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Truncate Table" ), QObject::tr( "Are you sure you want to truncate [%1].[%2]?\n\nThis will delete all data within the table." ).arg( layerInfo.schemaName, layerInfo.tableName ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QString errCause;
  const bool res = QgsMssqlConnection::truncateTable( layerItem->uri(), &errCause );
  if ( !res )
  {
    QMessageBox::warning( nullptr, tr( "Truncate Table" ), errCause );
  }
  else
  {
    QMessageBox::information( nullptr, tr( "Truncate Table" ), tr( "Table truncated successfully." ) );
  }
}

void QgsMssqlDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::MSSQL );
  dlg.exec();
}

void QgsMssqlDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::MSSQL, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}

bool QgsMssqlDataItemGuiProvider::handleDrop( QgsMssqlConnectionItem *connectionItem, const QMimeData *data, const QString &toSchema, QgsDataItemGuiContext context )
{
  if ( !QgsMimeDataUtils::isUriList( data ) || !connectionItem )
    return false;

  const QgsMimeDataUtils::UriList sourceUris = QgsMimeDataUtils::decodeUriList( data );
  if ( sourceUris.size() == 1 && sourceUris.at( 0 ).layerType == QLatin1String( "vector" ) )
  {
    return handleDropUri( connectionItem, sourceUris.at( 0 ), toSchema, context );
  }

  QPointer< QgsMssqlConnectionItem > connectionItemPointer( connectionItem );

  // TODO: when dropping multiple layers, we need a dedicated "bulk import" dialog for settings which apply to ALL layers

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connectionItem->databaseConnection() );
  if ( !databaseConnection )
    return false;

  QStringList importResults;
  bool hasError = false;

  for ( const QgsMimeDataUtils::Uri &u : sourceUris )
  {
    if ( u.layerType != QLatin1String( "vector" ) )
    {
      importResults.append( tr( "%1: Not a vector layer!" ).arg( u.name ) );
      hasError = true; // only vectors can be imported
      continue;
    }

    // open the source layer
    const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
    QgsVectorLayer *srcLayer = new QgsVectorLayer( u.uri, u.name, u.providerKey, options );

    if ( srcLayer->isValid() )
    {
      QString geomColumn { QStringLiteral( "geom" ) };
      if ( !srcLayer->dataProvider()->geometryColumnName().isEmpty() )
      {
        geomColumn = srcLayer->dataProvider()->geometryColumnName();
      }

      QgsAbstractDatabaseProviderConnection::VectorLayerExporterOptions exporterOptions;
      exporterOptions.layerName = u.name;
      exporterOptions.schema = toSchema;
      exporterOptions.wkbType = srcLayer->wkbType();
      exporterOptions.geometryColumn = geomColumn;

      QVariantMap providerOptions;
      const QString destUri = databaseConnection->createVectorLayerExporterDestinationUri( exporterOptions, providerOptions );

      std::unique_ptr<QgsVectorLayerExporterTask> exportTask( QgsVectorLayerExporterTask::withLayerOwnership( srcLayer, destUri, QStringLiteral( "mssql" ), srcLayer->crs(), providerOptions ) );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [=]() {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to MS SQL Server database" ), tr( "Import was successful." ) );
        if ( connectionItemPointer )
        {
          if ( connectionItemPointer->state() == Qgis::BrowserItemState::Populated )
            connectionItemPointer->refresh();
          else
            connectionItemPointer->populate();
        }
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [=]( Qgis::VectorExportResult error, const QString &errorMessage ) {
        if ( error != Qgis::VectorExportResult::UserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to MS SQL Server database" ) );
          output->setMessage( tr( "Failed to import some layers!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
          output->showMessage();
        }
        if ( connectionItemPointer )
        {
          if ( connectionItemPointer->state() == Qgis::BrowserItemState::Populated )
            connectionItemPointer->refresh();
          else
            connectionItemPointer->populate();
        }
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
    output->setTitle( tr( "Import to MS SQL Server database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QLatin1Char( '\n' ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

bool QgsMssqlDataItemGuiProvider::handleDropUri( QgsMssqlConnectionItem *connectionItem, const QgsMimeDataUtils::Uri &sourceUri, const QString &toSchema, QgsDataItemGuiContext context )
{
  QPointer< QgsMssqlConnectionItem > connectionItemPointer( connectionItem );
  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connectionItem->databaseConnection() );
  if ( !databaseConnection )
    return false;

  auto onSuccess = [connectionItemPointer]() {
    if ( connectionItemPointer )
    {
      if ( connectionItemPointer->state() == Qgis::BrowserItemState::Populated )
        connectionItemPointer->refresh();
      else
        connectionItemPointer->populate();
    }
  };

  auto onFailure = [connectionItemPointer]( Qgis::VectorExportResult, const QString & ) {
    if ( connectionItemPointer )
    {
      if ( connectionItemPointer->state() == Qgis::BrowserItemState::Populated )
        connectionItemPointer->refresh();
      else
        connectionItemPointer->populate();
    }
  };

  return QgsDataItemGuiProviderUtils::handleDropUriForConnection( std::move( databaseConnection ), sourceUri, toSchema, context, tr( "SQL Server Import" ), tr( "Import to SQL Server database" ), QVariantMap(), onSuccess, onFailure, this );
}

void QgsMssqlDataItemGuiProvider::handleImportVector( QgsMssqlConnectionItem *connectionItem, const QString &toSchema, QgsDataItemGuiContext context )
{
  if ( !connectionItem )
    return;

  QPointer< QgsMssqlConnectionItem > connectionItemPointer( connectionItem );
  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connectionItem->databaseConnection() );
  if ( !databaseConnection )
    return;

  auto onSuccess = [connectionItemPointer]() {
    if ( connectionItemPointer )
    {
      if ( connectionItemPointer->state() == Qgis::BrowserItemState::Populated )
        connectionItemPointer->refresh();
      else
        connectionItemPointer->populate();
    }
  };

  auto onFailure = [connectionItemPointer]( Qgis::VectorExportResult, const QString & ) {
    if ( connectionItemPointer )
    {
      if ( connectionItemPointer->state() == Qgis::BrowserItemState::Populated )
        connectionItemPointer->refresh();
      else
        connectionItemPointer->populate();
    }
  };

  QgsDataItemGuiProviderUtils::handleImportVectorLayerForConnection( std::move( databaseConnection ), toSchema, context, tr( "SQL Server Import" ), tr( "Import to SQL Server database" ), QVariantMap(), onSuccess, onFailure, this );
}
