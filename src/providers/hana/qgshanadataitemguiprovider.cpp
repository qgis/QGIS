/***************************************************************************
   qgshanadataitemguiprovider.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgshanadataitems.h"
#include "qgshanadataitemguiprovider.h"
#include "moc_qgshanadataitemguiprovider.cpp"
#include "qgshananewconnection.h"
#include "qgshanaproviderconnection.h"
#include "qgshanasourceselect.h"
#include "qgsnewnamedialog.h"
#include "qgsdataitemguiproviderutils.h"
#include "qgssettings.h"
#include "qgshanautils.h"
#include "qgsapplication.h"
#include "qgstaskmanager.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerexporter.h"
#include "qgsmessageoutput.h"
#include "qgsdbimportvectorlayerdialog.h"

#include <QInputDialog>
#include <QMessageBox>

void QgsHanaDataItemGuiProvider::populateContextMenu(
  QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context
)
{
  if ( QgsHanaRootItem *rootItem = qobject_cast<QgsHanaRootItem *>( item ) )
  {
    QAction *actionNew = new QAction( tr( "New Connection…" ), this );
    connect( actionNew, &QAction::triggered, this, [rootItem] { newConnection( rootItem ); } );
    menu->addAction( actionNew );
  }

  if ( QgsHanaConnectionItem *connItem = qobject_cast<QgsHanaConnectionItem *>( item ) )
  {
    const QList<QgsHanaConnectionItem *> hanaConnectionItems = QgsDataItem::filteredItems<QgsHanaConnectionItem>( selection );

    if ( hanaConnectionItems.size() == 1 )
    {
      QAction *actionRefresh = new QAction( tr( "Refresh" ), this );
      connect( actionRefresh, &QAction::triggered, this, [connItem] { refreshConnection( connItem ); } );
      menu->addAction( actionRefresh );

      menu->addSeparator();

      QAction *actionEdit = new QAction( tr( "Edit Connection…" ), this );
      connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
      menu->addAction( actionEdit );

      QAction *actionDuplicate = new QAction( tr( "Duplicate Connection" ), this );
      connect( actionDuplicate, &QAction::triggered, this, [connItem] { duplicateConnection( connItem ); } );
      menu->addAction( actionDuplicate );
    }

    QAction *actionDelete = new QAction( hanaConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [hanaConnectionItems, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( hanaConnectionItems, []( const QString &connectionName ) { QgsHanaSettings::removeConnection( connectionName ); }, context );
    } );
    menu->addAction( actionDelete );

    if ( hanaConnectionItems.size() == 1 )
    {
      menu->addSeparator();
      QAction *actionCreateSchema = new QAction( tr( "New Schema…" ), this );
      connect( actionCreateSchema, &QAction::triggered, this, [connItem, context] { createSchema( connItem, context ); } );
      menu->addAction( actionCreateSchema );
    }
  }

  if ( QgsHanaSchemaItem *schemaItem = qobject_cast<QgsHanaSchemaItem *>( item ) )
  {
    QAction *actionRefresh = new QAction( tr( "Refresh" ), this );
    connect( actionRefresh, &QAction::triggered, this, [schemaItem] { schemaItem->refresh(); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QMenu *maintainMenu = new QMenu( tr( "Schema Operations" ), menu );

    QAction *actionRename = new QAction( tr( "Rename Schema…" ), this );
    connect( actionRename, &QAction::triggered, this, [schemaItem, context] { renameSchema( schemaItem, context ); } );
    maintainMenu->addAction( actionRename );

    QAction *actionDelete = new QAction( tr( "Delete Schema…" ), this );
    connect( actionDelete, &QAction::triggered, this, [schemaItem, context] { deleteSchema( schemaItem, context ); } );
    maintainMenu->addAction( actionDelete );

    menu->addMenu( maintainMenu );
  }

  if ( QgsHanaLayerItem *layerItem = qobject_cast<QgsHanaLayerItem *>( item ) )
  {
    const QgsHanaLayerProperty &layerInfo = layerItem->layerInfo();
    if ( !layerInfo.isView )
    {
      QMenu *maintainMenu = new QMenu( tr( "Table Operations" ), menu );

      QAction *actionRenameLayer = new QAction( tr( "Rename Table…" ), this );
      connect( actionRenameLayer, &QAction::triggered, this, [layerItem, context] { renameLayer( layerItem, context ); } );
      maintainMenu->addAction( actionRenameLayer );

      menu->addMenu( maintainMenu );
    }
  }
}

bool QgsHanaDataItemGuiProvider::deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context )
{
  if ( QgsHanaLayerItem *layerItem = qobject_cast<QgsHanaLayerItem *>( item ) )
  {
    const QgsHanaLayerProperty &layerInfo = layerItem->layerInfo();
    const QString layerName = QStringLiteral( "%1.%2" ).arg( layerInfo.schemaName, layerInfo.tableName );
    const QString caption = tr( layerInfo.isView ? "Delete View" : "Delete Table" );
    if ( QMessageBox::question( nullptr, caption, tr( "Are you sure you want to delete '%1'?" ).arg( layerName ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return false;

    QString errorMsg;
    try
    {
      const QgsHanaProviderConnection providerConn( layerItem->uri(), {} );
      providerConn.dropVectorTable( layerInfo.schemaName, layerInfo.tableName );
    }
    catch ( const QgsProviderConnectionException &ex )
    {
      errorMsg = ex.what();
    }

    if ( errorMsg.isEmpty() )
    {
      notify( caption, tr( "'%1' deleted successfully." ).arg( layerName ), context, Qgis::MessageLevel::Success );

      if ( layerItem->parent() )
        layerItem->parent()->refresh();
      return true;
    }
    else
    {
      notify( caption, errorMsg, context, Qgis::MessageLevel::Warning );
    }
  }

  return false;
}

bool QgsHanaDataItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( qobject_cast<QgsHanaConnectionItem *>( item ) )
    return true;
  if ( qobject_cast<QgsHanaSchemaItem *>( item ) )
    return true;

  return false;
}

bool QgsHanaDataItemGuiProvider::handleDrop(
  QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction
)
{
  if ( QgsHanaConnectionItem *connItem = qobject_cast<QgsHanaConnectionItem *>( item ) )
  {
    return handleDrop( connItem, data, QString(), context );
  }
  else if ( QgsHanaSchemaItem *schemaItem = qobject_cast<QgsHanaSchemaItem *>( item ) )
  {
    QgsHanaConnectionItem *connItem = qobject_cast<QgsHanaConnectionItem *>( schemaItem->parent() );
    if ( !connItem )
      return false;

    return handleDrop( connItem, data, schemaItem->name(), context );
  }
  return false;
}

QWidget *QgsHanaDataItemGuiProvider::createParamWidget( QgsDataItem *root, QgsDataItemGuiContext )
{
  QgsHanaRootItem *rootItem = qobject_cast<QgsHanaRootItem *>( root );
  if ( rootItem == nullptr )
    return nullptr;
  QgsHanaSourceSelect *select = new QgsHanaSourceSelect( nullptr, QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsHanaSourceSelect::connectionsChanged, rootItem, &QgsHanaRootItem::onConnectionsChanged );
  return select;
}

void QgsHanaDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsHanaNewConnection nc( nullptr );
  if ( nc.exec() )
  {
    item->refresh();
  }
}

void QgsHanaDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsHanaNewConnection nc( nullptr, item->name() );
  if ( nc.exec() )
  {
    // the parent should be updated
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
}

void QgsHanaDataItemGuiProvider::duplicateConnection( QgsDataItem *item )
{
  const QString connectionName = item->name();
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "/HANA/connections" ) );
  const QStringList connections = settings.childGroups();
  settings.endGroup();

  const QString newConnectionName = QgsDataItemGuiProviderUtils::uniqueName( connectionName, connections );

  QgsHanaSettings::duplicateConnection( connectionName, newConnectionName );

  if ( item->parent() )
  {
    item->parent()->refreshConnections();
  }
}

void QgsHanaDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsHanaDataItemGuiProvider::createSchema( QgsDataItem *item, QgsDataItemGuiContext context )
{
  const QString schemaName = QInputDialog::getText( nullptr, tr( "Create Schema" ), tr( "Schema name:" ) );
  if ( schemaName.isEmpty() )
    return;

  QString errorMsg;
  try
  {
    const QgsHanaProviderConnection providerConn( item->name() );
    providerConn.createSchema( schemaName );
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    errorMsg = ex.what();
  }

  if ( errorMsg.isEmpty() )
  {
    notify( tr( "New Schema" ), tr( "Schema '%1' created successfully." ).arg( schemaName ), context, Qgis::MessageLevel::Success );

    item->refresh();
    // the parent should be updated
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
  else
  {
    notify( tr( "New Schema" ), tr( "Unable to create schema '%1'\n%2" ).arg( schemaName, errorMsg ), context, Qgis::MessageLevel::Warning );
  }
}

void QgsHanaDataItemGuiProvider::deleteSchema( QgsHanaSchemaItem *schemaItem, QgsDataItemGuiContext context )
{
  const QString schemaName = schemaItem->name();
  const QString caption = tr( "Delete Schema" );
  QString errorMsg;
  try
  {
    const QgsHanaProviderConnection providerConn( schemaItem->connectionName() );
    const auto tables = providerConn.tables( schemaName );
    if ( tables.empty() )
    {
      if ( QMessageBox::question( nullptr, caption, tr( "Are you sure you want to delete '%1'?" ).arg( schemaName ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
        return;
    }
    else
    {
      const int MAXIMUM_LISTED_ITEMS = 10;
      QString tableNames;
      for ( int i = 0; i < tables.size(); ++i )
      {
        const auto &tableProperty = tables.at( i );
        if ( i < MAXIMUM_LISTED_ITEMS )
          tableNames += tableProperty.tableName() + QLatin1Char( '\n' );
        else
        {
          tableNames += QStringLiteral( "\n[%1 additional objects not listed]" ).arg( tables.size() - MAXIMUM_LISTED_ITEMS );
          break;
        }
      }

      if ( QMessageBox::question( nullptr, caption, tr( "Schema '%1' contains objects:\n\n%2\n\nAre you sure you want to delete the schema and all these objects?" ).arg( schemaName, tableNames ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
        return;
    }

    providerConn.dropSchema( schemaName, !tables.empty() );
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    errorMsg = ex.what();
  }

  if ( errorMsg.isEmpty() )
  {
    notify( caption, tr( "Schema '%1' deleted successfully." ).arg( schemaName ), context, Qgis::MessageLevel::Success );
    if ( schemaItem->parent() )
      schemaItem->parent()->refresh();
  }
  else
  {
    notify( caption, tr( "Unable to delete schema '%1'\n%2" ).arg( schemaName, errorMsg ), context, Qgis::MessageLevel::Warning );
  }
}

void QgsHanaDataItemGuiProvider::renameSchema( QgsHanaSchemaItem *schemaItem, QgsDataItemGuiContext context )
{
  const QString schemaName = schemaItem->name();
  const QString caption = tr( "Rename Schema" );
  QgsNewNameDialog dlg( tr( "schema '%1'" ).arg( schemaName ), schemaName );
  dlg.setWindowTitle( caption );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == schemaName )
    return;

  const QString newSchemaName = dlg.name();
  QString errorMsg;
  try
  {
    const QgsHanaProviderConnection providerConn( schemaItem->connectionName() );
    providerConn.renameSchema( schemaName, newSchemaName );
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    errorMsg = ex.what();
  }

  if ( errorMsg.isEmpty() )
  {
    notify( caption, tr( "Schema '%1' renamed successfully to '%2'." ).arg( schemaName, newSchemaName ), context, Qgis::MessageLevel::Success );
    if ( schemaItem->parent() )
      schemaItem->parent()->refresh();
  }
  else
  {
    notify( caption, tr( "Unable to rename schema '%1'\n%2" ).arg( schemaName, errorMsg ), context, Qgis::MessageLevel::Warning );
  }
}

void QgsHanaDataItemGuiProvider::renameLayer( QgsHanaLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsHanaLayerProperty &layerInfo = layerItem->layerInfo();
  const QString caption = tr( "Rename Table" );
  QgsNewNameDialog dlg( tr( "table '%1.%2'" ).arg( layerInfo.schemaName, layerInfo.tableName ), layerInfo.tableName );
  dlg.setWindowTitle( caption );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == layerInfo.tableName )
    return;

  const QString newLayerName = dlg.name();
  QString errorMsg;
  try
  {
    const QgsHanaProviderConnection providerConn( layerItem->uri(), {} );
    providerConn.renameVectorTable( layerInfo.schemaName, layerInfo.tableName, newLayerName );
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    errorMsg = ex.what();
  }

  if ( errorMsg.isEmpty() )
  {
    notify( caption, tr( "'%1' renamed successfully to '%2'." ).arg( layerInfo.tableName, newLayerName ), context, Qgis::MessageLevel::Success );
    if ( layerItem->parent() )
      layerItem->parent()->refresh();
  }
  else
  {
    notify( caption, tr( "Unable to rename '%1'\n%2" ).arg( layerInfo.tableName, errorMsg ), context, Qgis::MessageLevel::Warning );
  }
}

bool QgsHanaDataItemGuiProvider::handleDrop( QgsHanaConnectionItem *connectionItem, const QMimeData *data, const QString &toSchema, QgsDataItemGuiContext context )
{
  if ( !QgsMimeDataUtils::isUriList( data ) || !connectionItem )
    return false;

  const QgsMimeDataUtils::UriList sourceUris = QgsMimeDataUtils::decodeUriList( data );
  if ( sourceUris.size() == 1 )
  {
    return handleDropUri( connectionItem, sourceUris.at( 0 ), toSchema, context );
  }

  QStringList importResults;
  bool hasError = false;

  QPointer< QgsHanaConnectionItem > connectionItemPointer( connectionItem );

  QgsDataSourceUri uri = connectionItem->connectionUri();
  QgsHanaConnectionRef conn( uri );
  // TODO: when dropping multiple layers, we need a dedicated "bulk import" dialog for settings which apply to ALL layers

  if ( !conn.isNull() )
  {
    const QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList( data );
    for ( const QgsMimeDataUtils::Uri &u : lst )
    {
      if ( u.layerType != QLatin1String( "vector" ) )
      {
        importResults.append( tr( "%1: Not a vector layer!" ).arg( u.name ) );
        hasError = true; // only vectors can be imported
        continue;
      }

      // open the source layer
      bool owner;
      QString error;
      QgsVectorLayer *srcLayer = u.vectorLayer( owner, error );
      if ( !srcLayer )
      {
        importResults.append( QStringLiteral( "%1: %2" ).arg( u.name, error ) );
        hasError = true;
        continue;
      }

      if ( srcLayer->isValid() )
      {
        QgsDataSourceUri dsUri( u.uri );
        QString geomColumn = dsUri.geometryColumn();
        if ( geomColumn.isEmpty() )
        {
          bool fieldsInUpperCase = QgsHanaUtils::countFieldsWithFirstLetterInUppercase( srcLayer->fields() ) > srcLayer->fields().size() / 2;
          geomColumn = ( srcLayer->geometryType() != Qgis::GeometryType::Null ) ? ( fieldsInUpperCase ? QStringLiteral( "GEOM" ) : QStringLiteral( "geom" ) ) : nullptr;
        }

        uri.setDataSource( toSchema, u.name, geomColumn, QString(), dsUri.keyColumn() );
        uri.setWkbType( srcLayer->wkbType() );

        std::unique_ptr<QgsVectorLayerExporterTask> exportTask(
          new QgsVectorLayerExporterTask( srcLayer, uri.uri( false ), QStringLiteral( "hana" ), srcLayer->crs(), QVariantMap(), owner )
        );

        // when export is successful:
        connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [=]() {
          QMessageBox::information( nullptr, tr( "Import to SAP HANA database" ), tr( "Import was successful." ) );
          if ( connectionItemPointer )
            connectionItemPointer->refreshSchema( toSchema );
        } );

        // when an error occurs:
        connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [=]( Qgis::VectorExportResult error, const QString &errorMessage ) {
          if ( error != Qgis::VectorExportResult::UserCanceled )
          {
            QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
            output->setTitle( tr( "Import to SAP HANA database" ) );
            output->setMessage( tr( "Failed to import some layers!\n\n" ) + errorMessage, QgsMessageOutput::MessageText );
            output->showMessage();
          }
          if ( connectionItemPointer )
            connectionItemPointer->refreshSchema( toSchema );
        } );

        QgsApplication::taskManager()->addTask( exportTask.release() );
      }
      else
      {
        importResults.append( tr( "%1: Not a valid layer!" ).arg( u.name ) );
        hasError = true;
      }
    }
  }
  else
  {
    importResults.append( tr( "Connection failed" ) );
    hasError = true;
  }

  if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to SAP HANA database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QLatin1Char( '\n' ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

bool QgsHanaDataItemGuiProvider::handleDropUri( QgsHanaConnectionItem *connectionItem, const QgsMimeDataUtils::Uri &sourceUri, const QString &toSchema, QgsDataItemGuiContext context )
{
  QPointer< QgsHanaConnectionItem > connectionItemPointer( connectionItem );
  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connectionItem->databaseConnection() );
  if ( !databaseConnection )
    return false;

  QgsDbImportVectorLayerDialog dialog( databaseConnection.release(), context.messageBar() ? context.messageBar()->parentWidget() : nullptr );
  dialog.setSourceUri( sourceUri );
  dialog.setDestinationSchema( toSchema );
  dialog.exec();
  return true;
}
