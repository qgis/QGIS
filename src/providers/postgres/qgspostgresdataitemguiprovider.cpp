/***************************************************************************
  qgspostgresdataitemguiprovider.cpp
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

#include "qgspostgresdataitemguiprovider.h"

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsapplication.h"
#include "qgsdatabaseschemaselectiondialog.h"
#include "qgsdataitemguiproviderutils.h"
#include "qgsdbimportvectorlayerdialog.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsmessagelog.h"
#include "qgsmessageoutput.h"
#include "qgsnewnamedialog.h"
#include "qgspgnewconnection.h"
#include "qgspgsourceselect.h"
#include "qgspostgresconn.h"
#include "qgspostgresdataitems.h"
#include "qgspostgresimportprojectdialog.h"
#include "qgspostgresutils.h"
#include "qgsproject.h"
#include "qgsprovidermetadata.h"
#include "qgssettings.h"
#include "qgstaskmanager.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerexporter.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPair>

#include "moc_qgspostgresdataitemguiprovider.cpp"

void QgsPostgresDataItemGuiProvider::populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selection, QgsDataItemGuiContext context )
{
  if ( QgsPGRootItem *rootItem = qobject_cast<QgsPGRootItem *>( item ) )
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

  if ( QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( item ) )
  {
    const QList<QgsPGConnectionItem *> pgConnectionItems = QgsDataItem::filteredItems<QgsPGConnectionItem>( selection );

    if ( pgConnectionItems.size() == 1 )
    {
      QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
      connect( actionRefresh, &QAction::triggered, this, [connItem] { refreshConnection( connItem ); } );
      menu->addAction( actionRefresh );

      menu->addSeparator();

      QAction *actionEdit = new QAction( tr( "Edit Connection…" ), menu );
      connect( actionEdit, &QAction::triggered, this, [connItem] { editConnection( connItem ); } );
      menu->addAction( actionEdit );

      QAction *actionDuplicate = new QAction( tr( "Duplicate Connection" ), menu );
      connect( actionDuplicate, &QAction::triggered, this, [connItem] { duplicateConnection( connItem ); } );
      menu->addAction( actionDuplicate );
    }

    QAction *actionDelete = new QAction( pgConnectionItems.size() > 1 ? tr( "Remove Connections…" ) : tr( "Remove Connection…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [pgConnectionItems, context] {
      QgsDataItemGuiProviderUtils::deleteConnections( pgConnectionItems, []( const QString &connectionName ) {
        QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( u"postgres"_s );
        md->deleteConnection( connectionName ); }, context );
    } );
    menu->addAction( actionDelete );

    if ( pgConnectionItems.size() == 1 )
    {
      menu->addSeparator();

      QAction *actionCreateSchema = new QAction( tr( "New Schema…" ), menu );
      connect( actionCreateSchema, &QAction::triggered, this, [connItem, context] { createSchema( connItem, context ); } );
      menu->addAction( actionCreateSchema );
    }
  }

  if ( QgsPGSchemaItem *schemaItem = qobject_cast<QgsPGSchemaItem *>( item ) )
  {
    QAction *importVectorAction = new QAction( QObject::tr( "Import Vector Layer…" ), menu );
    menu->addAction( importVectorAction );
    const QString destinationSchema = schemaItem->name();
    QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( schemaItem->parent() );
    QObject::connect( importVectorAction, &QAction::triggered, item, [connItem, context, destinationSchema, this] { handleImportVector( connItem, destinationSchema, context ); } );

    QAction *actionRefresh = new QAction( tr( "Refresh" ), menu );
    connect( actionRefresh, &QAction::triggered, this, [schemaItem] { schemaItem->refresh(); } );
    menu->addAction( actionRefresh );

    menu->addSeparator();

    QMenu *maintainMenu = new QMenu( tr( "Schema Operations" ), menu );

    QAction *actionRename = new QAction( tr( "Rename Schema…" ), menu );
    connect( actionRename, &QAction::triggered, this, [schemaItem, context] { renameSchema( schemaItem, context ); } );
    maintainMenu->addAction( actionRename );

    QAction *actionDelete = new QAction( tr( "Delete Schema…" ), menu );
    connect( actionDelete, &QAction::triggered, this, [schemaItem, context] { deleteSchema( schemaItem, context ); } );
    maintainMenu->addAction( actionDelete );

    menu->addMenu( maintainMenu );

    if ( QgsPostgresConn::allowProjectsInDatabase( schemaItem->connectionName() ) )
    {
      QMenu *projectMenu = new QMenu( tr( "Project" ), menu );
      menu->addMenu( projectMenu );

      QAction *actionSaveProject = new QAction( tr( "Save Current Project" ), projectMenu );
      connect( actionSaveProject, &QAction::triggered, this, [schemaItem, context] { saveCurrentProject( schemaItem, context ); } );
      projectMenu->addAction( actionSaveProject );

      QAction *actionImportProject = new QAction( tr( "Import Projects…" ), projectMenu );
      projectMenu->addAction( actionImportProject );
      connect( actionImportProject, &QAction::triggered, this, [schemaItem, context] { saveProjects( schemaItem, context ); } );
    }
  }

  if ( QgsPGLayerItem *layerItem = qobject_cast<QgsPGLayerItem *>( item ) )
  {
    const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
    const QString typeName = typeNameFromLayer( layerInfo );

    QMenu *maintainMenu = new QMenu( tr( "%1 Operations" ).arg( typeName ), menu );

    QAction *actionRenameLayer = new QAction( tr( "Rename %1…" ).arg( typeName ), menu );
    connect( actionRenameLayer, &QAction::triggered, this, [layerItem, context] { renameLayer( layerItem, context ); } );
    maintainMenu->addAction( actionRenameLayer );

    if ( layerInfo.relKind != Qgis::PostgresRelKind::View && layerInfo.relKind != Qgis::PostgresRelKind::MaterializedView )
    {
      QAction *actionTruncateLayer = new QAction( tr( "Truncate %1…" ).arg( typeName ), menu );
      connect( actionTruncateLayer, &QAction::triggered, this, [layerItem, context] { truncateTable( layerItem, context ); } );
      maintainMenu->addAction( actionTruncateLayer );
    }

    if ( layerInfo.relKind == Qgis::PostgresRelKind::MaterializedView )
    {
      QAction *actionRefreshMaterializedView = new QAction( tr( "Refresh Materialized View…" ), menu );
      connect( actionRefreshMaterializedView, &QAction::triggered, this, [layerItem, context] { refreshMaterializedView( layerItem, context ); } );
      maintainMenu->addAction( actionRefreshMaterializedView );
    }
    menu->addMenu( maintainMenu );
  }

  if ( QgsPGProjectItem *projectItem = qobject_cast<QgsPGProjectItem *>( item ) )
  {
    if ( selection.count() == 1 )
    {
      QAction *exportProjectToFileAction = new QAction( tr( "Export Project to File…" ), menu );
      connect( exportProjectToFileAction, &QAction::triggered, this, [projectItem, context] { exportProjectToFile( projectItem, context ); } );
      menu->addAction( exportProjectToFileAction );

      QAction *renameProjectAction = new QAction( tr( "Rename Project…" ), menu );
      connect( renameProjectAction, &QAction::triggered, this, [projectItem, context] { renameProject( projectItem, context ); } );
      menu->addAction( renameProjectAction );

      QAction *deleteProjectAction = new QAction( tr( "Delete Project…" ), menu );
      connect( deleteProjectAction, &QAction::triggered, this, [projectItem, context] { deleteProject( projectItem, context ); } );
      menu->addAction( deleteProjectAction );

      QAction *duplicateProjectAction = new QAction( tr( "Duplicate Project…" ), menu );
      connect( duplicateProjectAction, &QAction::triggered, this, [projectItem, context] { duplicateProject( projectItem, context ); } );
      menu->addAction( duplicateProjectAction );

      QAction *moveProjectToSchemaAction = new QAction( tr( "Move Project to Schema…" ), menu );
      connect( moveProjectToSchemaAction, &QAction::triggered, this, [projectItem, context] { moveProjectsToSchema( { projectItem }, context ); } );
      menu->addAction( moveProjectToSchemaAction );

      // Set project comment
      QAction *setProjectCommentAction = new QAction( tr( "Set Comment…" ), menu );
      connect( setProjectCommentAction, &QAction::triggered, this, [projectItem, context] { setProjectComment( projectItem, context ); } );
      menu->addAction( setProjectCommentAction );
    }
    else
    {
      bool allCanBeCast = std::all_of( selection.begin(), selection.end(), []( QgsDataItem *item ) {
        return qobject_cast<QgsPGProjectItem *>( item ) != nullptr;
      } );

      if ( allCanBeCast )
      {
        const QString connectionName = projectItem->connectionName();

        bool allSameConnection = std::all_of( selection.begin() + 1, selection.end(), [&connectionName]( QgsDataItem *item ) {
          if ( QgsPGProjectItem *projItem = qobject_cast<QgsPGProjectItem *>( item ) )
          {
            return projItem->connectionName() == connectionName;
          }
          return false;
        } );

        if ( allSameConnection )
        {
          QList<QgsPGProjectItem *> listOfProjects;

          std::transform( selection.begin(), selection.end(), std::back_inserter( listOfProjects ), []( QgsDataItem *parent ) -> QgsPGProjectItem * {
            return qobject_cast<QgsPGProjectItem *>( parent );
          } );

          QAction *moveProjectsAction = new QAction( tr( "Move Projects to Schema…" ), menu );
          connect( moveProjectsAction, &QAction::triggered, this, [listOfProjects, context] { moveProjectsToSchema( listOfProjects, context ); } );
          menu->addAction( moveProjectsAction );
        }
      }
    }
  }
}


bool QgsPostgresDataItemGuiProvider::deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context )
{
  if ( QgsPGLayerItem *layerItem = qobject_cast<QgsPGLayerItem *>( item ) )
  {
    const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
    const QString typeName = typeNameFromLayer( layerInfo );

    if ( QMessageBox::question( nullptr, tr( "Delete %1" ).arg( typeName ), QObject::tr( "Are you sure you want to delete %1 '%2.%3'?" ).arg( typeName.toLower(), layerInfo.schemaName, layerInfo.tableName ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return false;

    QString errCause;
    const bool res = QgsPostgresUtils::deleteLayer( layerItem->uri(), errCause );
    if ( !res )
    {
      notify( tr( "Delete %1" ).arg( typeName ), errCause, context, Qgis::MessageLevel::Warning );
      return false;
    }
    else
    {
      notify( tr( "Delete %1" ).arg( typeName ), tr( "%1 '%2' deleted successfully." ).arg( typeName, layerInfo.tableName ), context, Qgis::MessageLevel::Success );
      if ( layerItem->parent() )
        layerItem->parent()->refresh();
      return true;
    }
  }
  return false;
}

bool QgsPostgresDataItemGuiProvider::acceptDrop( QgsDataItem *item, QgsDataItemGuiContext )
{
  if ( qobject_cast<QgsPGConnectionItem *>( item ) )
    return true;
  if ( qobject_cast<QgsPGSchemaItem *>( item ) )
    return true;

  return false;
}

bool QgsPostgresDataItemGuiProvider::handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction )
{
  if ( QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( item ) )
  {
    return handleDrop( connItem, data, QString(), context );
  }
  else if ( QgsPGSchemaItem *schemaItem = qobject_cast<QgsPGSchemaItem *>( item ) )
  {
    QgsPGConnectionItem *connItem = qobject_cast<QgsPGConnectionItem *>( schemaItem->parent() );
    if ( !connItem )
      return false;

    return handleDrop( connItem, data, schemaItem->name(), context );
  }
  return false;
}

QWidget *QgsPostgresDataItemGuiProvider::createParamWidget( QgsDataItem *root, QgsDataItemGuiContext )
{
  QgsPGRootItem *pgRootItem = qobject_cast<QgsPGRootItem *>( root );
  if ( pgRootItem )
  {
    QgsPgSourceSelect *select = new QgsPgSourceSelect( nullptr, QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode::Manager );
    connect( select, &QgsPgSourceSelect::connectionsChanged, pgRootItem, &QgsPGRootItem::onConnectionsChanged );
    return select;
  }
  else
  {
    return nullptr;
  }
}

QString QgsPostgresDataItemGuiProvider::typeNameFromLayer( const QgsPostgresLayerProperty &layer )
{
  switch ( layer.relKind )
  {
    case Qgis::PostgresRelKind::View:
      return tr( "View" );

    case Qgis::PostgresRelKind::MaterializedView:
      return tr( "Materialized View" );

    case Qgis::PostgresRelKind::NotSet:
    case Qgis::PostgresRelKind::Unknown:
    case Qgis::PostgresRelKind::OrdinaryTable:
    case Qgis::PostgresRelKind::Index:
    case Qgis::PostgresRelKind::Sequence:
    case Qgis::PostgresRelKind::CompositeType:
    case Qgis::PostgresRelKind::ToastTable:
    case Qgis::PostgresRelKind::ForeignTable:
    case Qgis::PostgresRelKind::PartitionedTable:
      return tr( "Table" );
  }

  BUILTIN_UNREACHABLE
}

void QgsPostgresDataItemGuiProvider::newConnection( QgsDataItem *item )
{
  QgsPgNewConnection nc( QgsApplication::instance()->activeWindow() );
  if ( nc.exec() )
  {
    item->refresh();
  }
}

void QgsPostgresDataItemGuiProvider::editConnection( QgsDataItem *item )
{
  QgsPgNewConnection nc( nullptr, item->name() );
  nc.setWindowTitle( tr( "Edit PostgreSQL Connection" ) );
  if ( nc.exec() )
  {
    // the parent should be updated
    if ( item->parent() )
      item->parent()->refreshConnections();
  }
}

void QgsPostgresDataItemGuiProvider::duplicateConnection( QgsDataItem *item )
{
  const QString connectionName = item->name();
  QgsSettings settings;
  settings.beginGroup( u"/PostgreSQL/connections"_s );
  const QStringList connections = settings.childGroups();
  settings.endGroup();

  const QString newConnectionName = QgsDataItemGuiProviderUtils::uniqueName( connectionName, connections );

  QgsPostgresConn::duplicateConnection( connectionName, newConnectionName );

  if ( item->parent() )
  {
    item->parent()->refreshConnections();
  }
}


void QgsPostgresDataItemGuiProvider::refreshConnection( QgsDataItem *item )
{
  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}

void QgsPostgresDataItemGuiProvider::createSchema( QgsDataItem *item, QgsDataItemGuiContext context )
{
  const QString schemaName = QInputDialog::getText( nullptr, tr( "Create Schema" ), tr( "Schema name:" ) );
  if ( schemaName.isEmpty() )
    return;

  const QgsDataSourceUri uri = QgsPostgresConn::connUri( item->name() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri, false );
  if ( !conn )
  {
    notify( tr( "New Schema" ), tr( "Unable to create schema." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  //create the schema
  const QString sql = u"CREATE SCHEMA %1"_s.arg( QgsPostgresConn::quotedIdentifier( schemaName ) );

  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    notify( tr( "New Schema" ), tr( "Unable to create schema '%1'\n%2" ).arg( schemaName, result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  conn->unref();

  notify( tr( "New Schema" ), tr( "Schema '%1' created successfully." ).arg( schemaName ), context, Qgis::MessageLevel::Success );

  item->refresh();
  // the parent should be updated
  if ( item->parent() )
    item->parent()->refreshConnections();
}


void QgsPostgresDataItemGuiProvider::deleteSchema( QgsPGSchemaItem *schemaItem, QgsDataItemGuiContext context )
{
  // check if schema contains tables/views
  const QgsDataSourceUri uri = QgsPostgresConn::connUri( schemaItem->connectionName() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri, false );
  if ( !conn )
  {
    notify( tr( "Delete Schema" ), tr( "Unable to delete schema." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  const QString sql = u"SELECT table_name FROM information_schema.tables WHERE table_schema=%1"_s.arg( QgsPostgresConn::quotedValue( schemaItem->name() ) );
  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    notify( tr( "Delete Schema" ), tr( "Unable to delete schema." ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  QStringList childObjects;
  const int maxListed = 10;
  for ( int idx = 0; idx < result.PQntuples(); idx++ )
  {
    childObjects << result.PQgetvalue( idx, 0 );
    const QgsPostgresSchemaProperty schema;
    if ( idx == maxListed - 1 )
      break;
  }

  const int count = result.PQntuples();
  if ( count > 0 )
  {
    QString objects = childObjects.join( QLatin1Char( '\n' ) );
    if ( count > maxListed )
    {
      objects += u"\n[%1 additional objects not listed]"_s.arg( count - maxListed );
    }
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Schema" ), QObject::tr( "Schema '%1' contains objects:\n\n%2\n\nAre you sure you want to delete the schema and all these objects?" ).arg( schemaItem->name(), objects ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    {
      conn->unref();
      return;
    }
  }
  else
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Delete Schema" ), QObject::tr( "Are you sure you want to delete schema '%1'?" ).arg( schemaItem->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;
  }

  QString errCause;
  const bool res = QgsPostgresUtils::deleteSchema( schemaItem->name(), uri, errCause, count > 0 );
  if ( !res )
  {
    notify( tr( "Delete Schema" ), tr( "Unable to delete schema: '%1'." ).arg( errCause ), context, Qgis::MessageLevel::Warning );
  }
  else
  {
    notify( tr( "Delete Schema" ), tr( "Schema '%1' deleted successfully." ).arg( schemaItem->name() ), context, Qgis::MessageLevel::Success );
    if ( schemaItem->parent() )
      schemaItem->parent()->refresh();
  }
}

void QgsPostgresDataItemGuiProvider::renameSchema( QgsPGSchemaItem *schemaItem, QgsDataItemGuiContext context )
{
  QgsNewNameDialog dlg( tr( "schema '%1'" ).arg( schemaItem->name() ), schemaItem->name() );
  dlg.setWindowTitle( tr( "Rename Schema" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == schemaItem->name() )
    return;

  const QString schemaName = QgsPostgresConn::quotedIdentifier( schemaItem->name() );
  const QgsDataSourceUri uri = QgsPostgresConn::connUri( schemaItem->connectionName() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri, false );
  if ( !conn )
  {
    notify( tr( "Rename Schema" ), tr( "Unable to rename schema." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  //rename the schema
  const QString sql = u"ALTER SCHEMA %1 RENAME TO %2"_s
                        .arg( schemaName, QgsPostgresConn::quotedIdentifier( dlg.name() ) );

  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    notify( tr( "Rename Schema" ), tr( "Unable to rename schema '%1'\n%2" ).arg( schemaItem->name(), result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  notify( tr( "Rename Schema" ), tr( "Schema '%1' renamed correctly to '%2'." ).arg( schemaItem->name(), dlg.name() ), context, Qgis::MessageLevel::Success );

  conn->unref();
  if ( schemaItem->parent() )
    schemaItem->parent()->refresh();
}

void QgsPostgresDataItemGuiProvider::renameLayer( QgsPGLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
  const QString typeName = typeNameFromLayer( layerInfo );
  const QString lowerTypeName = ( layerInfo.relKind == Qgis::PostgresRelKind::View || layerInfo.relKind == Qgis::PostgresRelKind::MaterializedView )
                                  ? tr( "view" )
                                  : tr( "table" );

  QgsNewNameDialog dlg( tr( "%1 %2.%3" ).arg( lowerTypeName, layerInfo.schemaName, layerInfo.tableName ), layerInfo.tableName );
  dlg.setWindowTitle( tr( "Rename %1" ).arg( typeName ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == layerInfo.tableName )
    return;

  const QString schemaName = layerInfo.schemaName;
  const QString tableName = layerInfo.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  const QString oldName = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );
  const QString newName = QgsPostgresConn::quotedIdentifier( dlg.name() );

  const QgsDataSourceUri dsUri( layerItem->uri() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, false );
  if ( !conn )
  {
    notify( tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename '%1'." ).arg( lowerTypeName ), context, Qgis::MessageLevel::Warning );
    return;
  }

  //rename the layer
  QString sql;
  if ( layerInfo.relKind == Qgis::PostgresRelKind::View || layerInfo.relKind == Qgis::PostgresRelKind::MaterializedView )
  {
    sql = u"ALTER %1 VIEW %2 RENAME TO %3"_s.arg( layerInfo.relKind == Qgis::PostgresRelKind::MaterializedView ? u"MATERIALIZED"_s : QString(), oldName, newName );
  }
  else
  {
    sql = u"ALTER TABLE %1 RENAME TO %2"_s.arg( oldName, newName );
  }

  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    notify( tr( "Rename %1" ).arg( typeName ), tr( "Unable to rename '%1' %2\n%3" ).arg( lowerTypeName, layerItem->name(), result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  notify( tr( "Rename %1" ).arg( typeName ), tr( "%1 '%2' renamed correctly to '%3'." ).arg( typeName, oldName, newName ), context, Qgis::MessageLevel::Success );

  conn->unref();

  if ( layerItem->parent() )
    layerItem->parent()->refresh();
}

void QgsPostgresDataItemGuiProvider::truncateTable( QgsPGLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Truncate Table" ), QObject::tr( "Are you sure you want to truncate \"%1.%2\"?\n\nThis will delete all data within the table." ).arg( layerInfo.schemaName, layerInfo.tableName ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  const QgsDataSourceUri dsUri( layerItem->uri() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, false );
  if ( !conn )
  {
    notify( tr( "Truncate Table" ), tr( "Unable to truncate table." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  const QString schemaName = layerInfo.schemaName;
  const QString tableName = layerInfo.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  const QString tableRef = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );

  const QString sql = u"TRUNCATE TABLE %1"_s.arg( tableRef );

  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    notify( tr( "Truncate Table" ), tr( "Unable to truncate '%1'\n%2" ).arg( tableName, result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  conn->unref();
  notify( tr( "Truncate Table" ), tr( "Table '%1' truncated successfully." ).arg( tableName ), context, Qgis::MessageLevel::Success );
}

void QgsPostgresDataItemGuiProvider::refreshMaterializedView( QgsPGLayerItem *layerItem, QgsDataItemGuiContext context )
{
  const QgsPostgresLayerProperty &layerInfo = layerItem->layerInfo();
  if ( QMessageBox::question( nullptr, QObject::tr( "Refresh Materialized View" ), QObject::tr( "Are you sure you want to refresh the materialized view \"%1.%2\"?\n\nThis will update all data within the table." ).arg( layerInfo.schemaName, layerInfo.tableName ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  const QgsDataSourceUri dsUri( layerItem->uri() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( dsUri, false );
  if ( !conn )
  {
    notify( tr( "Refresh View" ), tr( "Unable to refresh the view." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  const QString schemaName = layerInfo.schemaName;
  const QString tableName = layerInfo.tableName;
  QString schemaTableName;
  if ( !schemaName.isEmpty() )
  {
    schemaTableName = QgsPostgresConn::quotedIdentifier( schemaName ) + '.';
  }
  const QString tableRef = schemaTableName + QgsPostgresConn::quotedIdentifier( tableName );

  const QString sql = u"REFRESH MATERIALIZED VIEW CONCURRENTLY %1"_s.arg( tableRef );

  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sql ) );
  if ( result.PQresultStatus() != PGRES_COMMAND_OK )
  {
    notify( tr( "Refresh View" ), tr( "Unable to refresh the view '%1'\n%2" ).arg( tableRef, result.PQresultErrorMessage() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  conn->unref();
  notify( tr( "Refresh View" ), tr( "Materialized view '%1' refreshed successfully." ).arg( tableName ), context, Qgis::MessageLevel::Success );
}

void QgsPostgresDataItemGuiProvider::saveConnections()
{
  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::PostGIS );
  dlg.exec();
}

void QgsPostgresDataItemGuiProvider::loadConnections( QgsDataItem *item )
{
  const QString fileName = QFileDialog::getOpenFileName( nullptr, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( nullptr, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::PostGIS, fileName );
  if ( dlg.exec() == QDialog::Accepted )
    item->refreshConnections();
}

bool QgsPostgresDataItemGuiProvider::handleDrop( QgsPGConnectionItem *connectionItem, const QMimeData *data, const QString &toSchema, QgsDataItemGuiContext context )
{
  if ( !QgsMimeDataUtils::isUriList( data ) || !connectionItem )
    return false;

  const QgsMimeDataUtils::UriList sourceUris = QgsMimeDataUtils::decodeUriList( data );
  if ( sourceUris.size() == 1 && sourceUris.at( 0 ).layerType == "vector"_L1 )
  {
    return handleDropUri( connectionItem, sourceUris.at( 0 ), toSchema, context );
  }

  QPointer< QgsPGConnectionItem > connectionItemPointer( connectionItem );

  QgsDataSourceUri uri = connectionItem->connectionUri();

  // TODO: when dropping multiple layers, we need a dedicated "bulk import" dialog for settings which apply to ALL layers

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connectionItem->databaseConnection() );
  if ( !databaseConnection )
    return false;

  QStringList importResults;
  bool hasError = false;

  for ( const QgsMimeDataUtils::Uri &u : sourceUris )
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
      // Try to get source col from uri

      QString geomColumn { u"geom"_s };
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

      QgsDebugMsgLevel( "URI " + destUri, 2 );

      auto exportTask = std::make_unique<QgsVectorLayerExporterTask>( srcLayer, destUri, u"postgres"_s, srcLayer->crs(), providerOptions, owner );

      // when export is successful:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this, [connectionItemPointer, toSchema]() {
        // this is gross - TODO - find a way to get access to messageBar from data items
        QMessageBox::information( nullptr, tr( "Import to PostgreSQL database" ), tr( "Import was successful." ) );
        if ( connectionItemPointer )
          connectionItemPointer->refreshSchema( toSchema );
      } );

      // when an error occurs:
      connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this, [connectionItemPointer, toSchema]( Qgis::VectorExportResult error, const QString &errorMessage ) {
        if ( error != Qgis::VectorExportResult::UserCanceled )
        {
          QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
          output->setTitle( tr( "Import to PostgreSQL database" ) );
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

  if ( hasError )
  {
    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Import to PostgreSQL database" ) );
    output->setMessage( tr( "Failed to import some layers!\n\n" ) + importResults.join( QLatin1Char( '\n' ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }

  return true;
}

bool QgsPostgresDataItemGuiProvider::handleDropUri( QgsPGConnectionItem *connectionItem, const QgsMimeDataUtils::Uri &sourceUri, const QString &toSchema, QgsDataItemGuiContext context )
{
  QPointer< QgsPGConnectionItem > connectionItemPointer( connectionItem );
  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connectionItem->databaseConnection() );
  if ( !databaseConnection )
    return false;

  auto onSuccess = [connectionItemPointer, toSchema]() {
    if ( connectionItemPointer )
    {
      if ( connectionItemPointer )
        connectionItemPointer->refreshSchema( toSchema );
    }
  };

  auto onFailure = [connectionItemPointer = std::move( connectionItemPointer ), toSchema]( Qgis::VectorExportResult, const QString & ) {
    if ( connectionItemPointer )
    {
      if ( connectionItemPointer )
        connectionItemPointer->refreshSchema( toSchema );
    }
  };

  return QgsDataItemGuiProviderUtils::handleDropUriForConnection( std::move( databaseConnection ), sourceUri, toSchema, context, tr( "PostgreSQL Import" ), tr( "Import to PostgreSQL database" ), QVariantMap(), onSuccess, onFailure, this );
}

void QgsPostgresDataItemGuiProvider::handleImportVector( QgsPGConnectionItem *connectionItem, const QString &toSchema, QgsDataItemGuiContext context )
{
  if ( !connectionItem )
    return;

  QPointer< QgsPGConnectionItem > connectionItemPointer( connectionItem );
  std::unique_ptr<QgsAbstractDatabaseProviderConnection> databaseConnection( connectionItem->databaseConnection() );
  if ( !databaseConnection )
    return;

  auto onSuccess = [connectionItemPointer, toSchema]() {
    if ( connectionItemPointer )
    {
      if ( connectionItemPointer )
        connectionItemPointer->refreshSchema( toSchema );
    }
  };

  auto onFailure = [connectionItemPointer = std::move( connectionItemPointer ), toSchema]( Qgis::VectorExportResult, const QString & ) {
    if ( connectionItemPointer )
    {
      if ( connectionItemPointer )
        connectionItemPointer->refreshSchema( toSchema );
    }
  };

  QgsDataItemGuiProviderUtils::handleImportVectorLayerForConnection( std::move( databaseConnection ), toSchema, context, tr( "PostgreSQL Import" ), tr( "Import to PostgreSQL database" ), QVariantMap(), onSuccess, onFailure, this );
}

void QgsPostgresDataItemGuiProvider::exportProjectToFile( QgsPGProjectItem *projectItem, QgsDataItemGuiContext context )
{
  QgsSettings settings;
  const QString defaultPath = settings.value( u"UI/lastProjectDir"_s, QDir::homePath() ).toString();

  const Qgis::ProjectFileFormat defaultProjectFileFormat = settings.enumValue( u"/qgis/defaultProjectFileFormat"_s, Qgis::ProjectFileFormat::Qgz );
  const QString qgisProjectExt = tr( "QGIS Project Formats" ) + ( defaultProjectFileFormat == Qgis::ProjectFileFormat::Qgz ? " (*.qgz *.QGZ *.qgs *.QGS)" : " (*.qgs *.QGS *.qgz *.QGZ)" );
  const QString qgzProjectExt = tr( "QGIS Bundled Project Format" ) + " (*.qgz *.QGZ)";
  const QString qgsProjectExt = tr( "QGIS XML Project Format" ) + " (*.qgs *.QGS)";

  QString filter;
  const QString path = QFileDialog::getSaveFileName(
    nullptr,
    tr( "Save Project As" ),
    defaultPath,
    qgisProjectExt + u";;"_s + qgzProjectExt + u";;"_s + qgsProjectExt, &filter
  );

  if ( path.isEmpty() )
    return;

  QFileInfo fullPath( path );
  QgsSettings().setValue( u"UI/lastProjectDir"_s, fullPath.path() );

  const QString ext = fullPath.suffix().toLower();
  if ( filter == qgisProjectExt && ext != "qgz"_L1 && ext != "qgs"_L1 )
  {
    switch ( defaultProjectFileFormat )
    {
      case Qgis::ProjectFileFormat::Qgs:
      {
        fullPath.setFile( fullPath.filePath() + ".qgs" );
        break;
      }
      case Qgis::ProjectFileFormat::Qgz:
      {
        fullPath.setFile( fullPath.filePath() + ".qgz" );
        break;
      }
    }
  }
  else if ( filter == qgzProjectExt && ext != "qgz"_L1 )
  {
    fullPath.setFile( fullPath.filePath() + ".qgz" );
  }
  else if ( filter == qgsProjectExt && ext != "qgs"_L1 )
  {
    fullPath.setFile( fullPath.filePath() + ".qgs" );
  }

  QgsProject project;
  project.read( projectItem->path() );
  project.setFileName( fullPath.filePath() );
  const bool result = project.write();

  if ( !result && context.messageBar() )
  {
    context.messageBar()->pushWarning( tr( "Export Project to File" ), tr( "Could not save project file" ) );
  }
}

void QgsPostgresDataItemGuiProvider::renameProject( QgsPGProjectItem *projectItem, QgsDataItemGuiContext context )
{
  QgsNewNameDialog dlg( tr( "project “%1”" ).arg( projectItem->name() ), projectItem->name() );
  dlg.setWindowTitle( tr( "Rename Project" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == projectItem->name() )
    return;

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( projectItem->postgresProjectUri().connInfo, false );
  if ( !conn )
  {
    notify( tr( "Rename Project" ), tr( "Unable to rename project." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  const QString newUri = projectItem->uriWithNewName( dlg.name() );

  // read the project, set title and new filename
  QgsProject project;
  project.read( projectItem->path() );
  project.setTitle( dlg.name() );
  project.setFileName( newUri );

  // write project to the database
  const bool success = project.write();
  if ( !success )
  {
    notify( tr( "Rename Project" ), tr( "Unable to rename project “%1” to “%2”" ).arg( projectItem->name(), dlg.name() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  if ( !QgsPostgresUtils::deleteProjectFromSchema( conn, projectItem->name(), projectItem->schemaName() ) )
  {
    notify( tr( "Rename Project" ), tr( "Unable to rename project “%1” to “%2”" ).arg( projectItem->name(), dlg.name() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  // refresh
  projectItem->parent()->refresh();

  conn->unref();
}

void QgsPostgresDataItemGuiProvider::deleteProject( QgsPGProjectItem *projectItem, QgsDataItemGuiContext context )
{
  if ( QMessageBox::question( nullptr, tr( "Delete Project" ), tr( "Are you sure you want to delete project “%1”?" ).arg( projectItem->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
    return;

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( projectItem->postgresProjectUri().connInfo, false );
  if ( !conn )
  {
    notify( tr( "Delete Project" ), tr( "Unable to delete project." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  if ( !QgsPostgresUtils::deleteProjectFromSchema( conn, projectItem->name(), projectItem->schemaName() ) )
  {
    notify( tr( "Delete Project" ), tr( "Unable to delete project “%1” " ).arg( projectItem->name() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  // refresh
  projectItem->parent()->refresh();

  conn->unref();
}

void QgsPostgresDataItemGuiProvider::duplicateProject( QgsPGProjectItem *projectItem, QgsDataItemGuiContext context )
{
  QgsNewNameDialog dlg( tr( "Project “%1”" ).arg( projectItem->name() ), projectItem->name() );
  dlg.setWindowTitle( tr( "Duplicate Project" ) );
  if ( dlg.exec() != QDialog::Accepted || dlg.name() == projectItem->name() )
    return;

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( projectItem->postgresProjectUri().connInfo, false );
  if ( !conn )
  {
    notify( tr( "Duplicate Project" ), tr( "Unable to duplicate project." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  const QString newUri = projectItem->uriWithNewName( dlg.name() );

  // read the project, set title and new filename
  QgsProject project;
  project.read( projectItem->path() );
  project.setTitle( dlg.name() );
  project.setFileName( newUri );

  // write project to the database
  const bool success = project.write();
  if ( !success )
  {
    notify( tr( "Duplicate Project" ), tr( "Unable to duplicate project “%1” to “%2”" ).arg( projectItem->name(), dlg.name() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  // refresh
  projectItem->parent()->refresh();
}

void QgsPostgresDataItemGuiProvider::moveProjectsToSchema( const QList<QgsPGProjectItem *> &selection, QgsDataItemGuiContext context )
{
  QgsPGProjectItem *mainItem = selection.first();

  QgsPGSchemaItem *schemaItem = qobject_cast<QgsPGSchemaItem *>( mainItem->parent() );
  if ( !schemaItem )
  {
    notify( tr( "Move Projects to Another Schema" ), tr( "Unable to move projects to another schema." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( schemaItem->databaseConnection() );
  if ( !conn )
  {
    notify( tr( "Move Projects to Another Schema" ), tr( "Unable to move projects to another schema." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  QgsDatabaseSchemaSelectionDialog dlg = QgsDatabaseSchemaSelectionDialog( conn.release() );

  if ( dlg.exec() == QDialog::Accepted )
  {
    const QString newSchemaName = dlg.selectedSchema();

    QgsPostgresConn *conn2 = QgsPostgresConn::connectDb( mainItem->postgresProjectUri().connInfo, false );

    if ( !conn2 )
    {
      notify( tr( "Move Projects to Another Schema" ), tr( "Unable to move projects to another schema." ), context, Qgis::MessageLevel::Warning );
      return;
    }

    if ( !QgsPostgresUtils::createProjectsTable( conn2, newSchemaName ) )
    {
      const QString errCause = tr( "Unable to move projects. It's not possible to create the destination table on the database. Maybe this is due to database permissions (user=%1). Please contact your database admin." ).arg( mainItem->postgresProjectUri().connInfo.username() );

      notify( tr( "Move Projects to Another Schema" ), errCause, context, Qgis::MessageLevel::Warning );
      conn2->unref();
      return;
    }

    if ( !QgsPostgresUtils::addCommentColumnToProjectsTable( conn2, newSchemaName ) )
    {
      const QString errCause = tr( "Unable to move projects. It's not possible to add the comment column to the destination table on the database. Maybe this is due to database permissions (user=%1). Please contact your database admin." ).arg( mainItem->postgresProjectUri().connInfo.username() );

      notify( tr( "Move Projects to Another Schema" ), errCause, context, Qgis::MessageLevel::Warning );
      conn2->unref();
      return;
    }

    int movedProjectCount = 0;
    for ( QgsPGProjectItem *projectItem : selection )
    {
      if ( !QgsPostgresUtils::moveProjectToSchema( conn2, projectItem->schemaName(), projectItem->name(), newSchemaName ) )
      {
        notify( tr( "Move Projects to Another Schema" ), tr( "Unable to move project “%1” to scheme “%2” " ).arg( projectItem->name(), newSchemaName ), context, Qgis::MessageLevel::Warning );
      }
      else
      {
        movedProjectCount++;
      }

      // refresh
      projectItem->parent()->refresh();

      const QVector<QgsDataItem *> children = projectItem->parent()->parent()->children();
      for ( QgsDataItem *item : children )
      {
        if ( QgsPGSchemaItem *schemaItem = qobject_cast<QgsPGSchemaItem *>( item ) )
        {
          if ( schemaItem->name() == newSchemaName )
          {
            schemaItem->refresh();
            break;
          }
        }
      }
    }

    conn2->unref();

    if ( selection.length() == 1 )
    {
      notify( tr( "Move Project to Another Schema" ), tr( "Project “%1” moved to schema “%2” successful." ).arg( mainItem->name(), newSchemaName ), context, Qgis::MessageLevel::Success );
    }
    else
    {
      notify( tr( "Move Projects to Another Schema" ), tr( "Move of %1 projects to schema “%2” successful." ).arg( movedProjectCount ).arg( newSchemaName ), context, Qgis::MessageLevel::Success );
    }
  }
}

void QgsPostgresDataItemGuiProvider::setProjectComment( QgsPGProjectItem *projectItem, QgsDataItemGuiContext context )
{
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( projectItem->postgresProjectUri().connInfo, false );

  if ( !conn )
  {
    notify( tr( "Set Project Comment" ), tr( "Unable to connect to database." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  const QString comment = QgsPostgresUtils::projectComment( conn, projectItem->schemaName(), projectItem->name() );
  bool ok = false;

  const QString newComment = QInputDialog::getMultiLineText( nullptr, tr( "Set Comment For Project %1" ).arg( projectItem->name() ), tr( "Comment" ), comment, &ok );
  if ( ok && newComment != comment )
  {
    const bool res = QgsPostgresUtils::setProjectComment( conn, projectItem->name(), projectItem->schemaName(), newComment );

    if ( !res )
    {
      notify( tr( "Set Project Comment" ), tr( "Failed to set project comment for '%1'" ).arg( projectItem->name() ), context, Qgis::MessageLevel::Warning );
    }
    else
    {
      notify( tr( "Set Project Comment" ), tr( "Comment updated for project '%1'" ).arg( projectItem->name() ), context, Qgis::MessageLevel::Success );
      projectItem->refresh();
    }
  }

  conn->unref();
}

void QgsPostgresDataItemGuiProvider::saveCurrentProject( QgsPGSchemaItem *schemaItem, QgsDataItemGuiContext context )
{
  const QgsDataSourceUri uri = QgsPostgresConn::connUri( schemaItem->connectionName() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri, false );
  if ( !conn )
  {
    notify( tr( "Save Project" ), tr( "Unable to save project to database." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  if ( !QgsPostgresUtils::projectsTableExists( conn, schemaItem->name() ) )
  {
    if ( !QgsPostgresUtils::createProjectsTable( conn, schemaItem->name() ) )
    {
      notify( tr( "Save Project" ), tr( "Unable to create table qgis_projects in schema %1." ).arg( schemaItem->name() ), context, Qgis::MessageLevel::Warning );
      conn->unref();
      return;
    }
  }

  QgsProject *project = QgsProject::instance();
  if ( !project )
  {
    notify( tr( "Save Project" ), tr( "Unable to save project to database." ), context, Qgis::MessageLevel::Warning );
    if ( conn )
      conn->unref();
    return;
  }

  QgsPostgresProjectUri pgProjectUri;
  pgProjectUri.connInfo = conn->uri();
  pgProjectUri.schemaName = schemaItem->name();
  pgProjectUri.projectName = project->title().isEmpty() ? project->baseName() : project->title();

  if ( pgProjectUri.projectName.isEmpty() )
  {
    bool ok;
    const QString projectName = QInputDialog::getText( nullptr, tr( "Set Project Name" ), tr( "Name" ), QLineEdit::Normal, tr( "New Project" ), &ok );
    if ( ok && !projectName.isEmpty() )
    {
      pgProjectUri.projectName = projectName;
    }
    else
    {
      notify( tr( "Save Project" ), tr( "Unable to save project without name to database." ), context, Qgis::MessageLevel::Warning );
      return;
    }
  }

  QString projectUri = QgsPostgresProjectStorage::encodeUri( pgProjectUri );
  const QString sqlProjectExist = u"SELECT EXISTS( SELECT 1 FROM %1.qgis_projects WHERE name = %2);"_s
                                    .arg( QgsPostgresConn::quotedIdentifier( schemaItem->name() ), QgsPostgresConn::quotedValue( pgProjectUri.projectName ) );
  QgsPostgresResult result( conn->LoggedPQexec( "QgsPostgresDataItemGuiProvider", sqlProjectExist ) );

  if ( !( result.PQresultStatus() == PGRES_COMMAND_OK || result.PQresultStatus() == PGRES_TUPLES_OK ) )
  {
    notify( tr( "Save Project" ), tr( "Unable to save project to database." ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  if ( result.PQgetvalue( 0, 0 ) == "t"_L1 )
  {
    notify( tr( "Save Project" ), tr( "Project “%1” exist in the database. Overwriting it." ).arg( pgProjectUri.projectName ), context, Qgis::MessageLevel::Info );
  }

  // read the project, set title and new filename
  QgsProject savedProject;
  savedProject.read( project->fileName() );
  savedProject.setFileName( projectUri );

  // write project to the database
  const bool success = savedProject.write();
  if ( !success )
  {
    notify( tr( "Save Project" ), tr( "Unable to save project “%1” to “%2”." ).arg( savedProject.title(), schemaItem->name() ), context, Qgis::MessageLevel::Warning );
    conn->unref();
    return;
  }

  notify( tr( "Save Project" ), tr( "Project “%1” saved to schema “%2”." ).arg( savedProject.title(), schemaItem->name() ), context, Qgis::MessageLevel::Info );

  // refresh
  schemaItem->refresh();
  conn->unref();
}

void QgsPostgresDataItemGuiProvider::saveProjects( QgsPGSchemaItem *schemaItem, QgsDataItemGuiContext context )
{
  const QgsDataSourceUri uri = QgsPostgresConn::connUri( schemaItem->connectionName() );
  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri, false );
  if ( !conn )
  {
    notify( tr( "Save Project" ), tr( "Could not connect to database." ), context, Qgis::MessageLevel::Warning );
    return;
  }

  if ( !QgsPostgresUtils::projectsTableExists( conn, schemaItem->name() ) )
  {
    if ( !QgsPostgresUtils::createProjectsTable( conn, schemaItem->name() ) )
    {
      notify( tr( "Save Project" ), tr( "Unable to create table qgis_projects in schema %1." ).arg( schemaItem->name() ), context, Qgis::MessageLevel::Warning );
      conn->unref();
    }
  }

  QgsPostgresImportProjectDialog dlg( schemaItem->connectionName(), schemaItem->name() );
  if ( dlg.exec() == QDialog::Accepted )
  {
    QList<QPair<QString, QString>> projectsWithNames = dlg.projectsToSave();

    int projectsSaved = 0;
    int projectsNotSaved = 0;
    QStringList unsavedProjects;

    for ( const QPair<QString, QString> &projectWithName : projectsWithNames )
    {
      QgsPostgresProjectUri pgProjectUri;
      pgProjectUri.connInfo = QgsDataSourceUri( conn->uri() );
      pgProjectUri.schemaName = schemaItem->name();
      pgProjectUri.projectName = projectWithName.second;
      QString projectUri = QgsPostgresProjectStorage::encodeUri( pgProjectUri );

      // read the project and set new filename
      QgsProject savedProject;
      savedProject.read( projectWithName.first );
      savedProject.setFileName( projectUri );

      // write project to the database
      const bool success = savedProject.write();
      if ( success )
      {
        projectsSaved++;
      }
      else
      {
        projectsNotSaved++;
        unsavedProjects << projectWithName.second;
      }
    }

    notify( tr( "Save Projects" ), tr( "Number of projects saved “%1”." ).arg( projectsSaved ), context, Qgis::MessageLevel::Info );

    if ( projectsNotSaved > 0 )
    {
      notify( tr( "Save Projects" ), tr( "Number of projects that could not be saved “%1”." ).arg( projectsNotSaved ), context, Qgis::MessageLevel::Critical );
      QgsMessageLog::logMessage( tr( "Project that could not be imported: %1" ).arg( unsavedProjects.join( ", " ) ), QString(), Qgis::MessageLevel::Critical );
    }

    // refresh
    schemaItem->refresh();
  }

  conn->unref();
}
