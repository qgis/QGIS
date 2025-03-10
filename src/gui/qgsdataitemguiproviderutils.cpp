/***************************************************************************
  qgsdataitemguiproviderutils.cpp
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdataitemguiproviderutils.h"
#include "qgsdataitem.h"
#include "qgsdataitemguiprovider.h"
#include "qgsdbimportvectorlayerdialog.h"
#include "qgsmessagebar.h"
#include "qgsvectorlayerexporter.h"
#include "qgsapplication.h"
#include "qgstaskmanager.h"
#include "qgsmessagebaritem.h"
#include "qgsmessageoutput.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"

#include <QPointer>
#include <QMessageBox>
#include <QPushButton>

void QgsDataItemGuiProviderUtils::deleteConnectionsPrivate( const QStringList &connectionNames, const std::function<void( const QString & )> &deleteConnection, QPointer<QgsDataItem> firstParent )
{
  if ( connectionNames.size() > 1 )
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Remove Connections" ), QObject::tr( "Are you sure you want to remove all %1 selected connections?" ).arg( connectionNames.size() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;
  }
  else
  {
    if ( QMessageBox::question( nullptr, QObject::tr( "Remove Connection" ), QObject::tr( "Are you sure you want to remove the connection to “%1”?" ).arg( connectionNames.at( 0 ) ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;
  }

  for ( const QString &connectionName : std::as_const( connectionNames ) )
  {
    deleteConnection( connectionName );
  }

  if ( firstParent )
    firstParent->refreshConnections();
}

const QString QgsDataItemGuiProviderUtils::uniqueName( const QString &name, const QStringList &connectionNames )
{
  int i = 0;
  QString newConnectionName( name );
  while ( connectionNames.contains( newConnectionName ) )
  {
    ++i;
    newConnectionName = QObject::tr( "%1 (copy %2)" ).arg( name ).arg( i );
  }

  return newConnectionName;
}

bool QgsDataItemGuiProviderUtils::handleDropUriForConnection( std::unique_ptr<QgsAbstractDatabaseProviderConnection> connection, const QgsMimeDataUtils::Uri &sourceUri, const QString &destinationSchema, QgsDataItemGuiContext context, const QString &shortTitle, const QString &longTitle, const QVariantMap &destinationProviderOptions, const std::function<void()> &onSuccessfulCompletion, const std::function<void( Qgis::VectorExportResult, const QString & )> &onError, QObject *connectionContext )
{
  if ( !connection )
    return false;

  const QString connectionUri = connection->uri();
  const QString connectionProvider = connection->providerKey();

  QgsDbImportVectorLayerDialog dialog( connection.release(), context.messageBar() ? context.messageBar()->parentWidget() : nullptr );
  dialog.setSourceUri( sourceUri );
  dialog.setDestinationSchema( destinationSchema );
  if ( !dialog.exec() )
    return false;

  std::unique_ptr< QgsVectorLayerExporterTask > exportTask = dialog.createExporterTask( destinationProviderOptions );
  if ( !exportTask )
    return false;

  const QString destSchema = dialog.schema();
  const QString destTableName = dialog.tableName();
  const QString tableComment = dialog.tableComment();

  auto pushError = [shortTitle, longTitle, context]( const QString &error ) {
    QgsMessageBarItem *item = new QgsMessageBarItem( shortTitle, QObject::tr( "Import failed." ), Qgis::MessageLevel::Warning, 0, nullptr );
    QPushButton *detailsButton = new QPushButton( QObject::tr( "Details…" ) );
    QObject::connect( detailsButton, &QPushButton::clicked, detailsButton, [=] {
      QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
      output->setTitle( longTitle );
      output->setMessage( error, QgsMessageOutput::MessageText );
      output->showMessage();
    } );
    item->layout()->addWidget( detailsButton );
    context.messageBar()->pushWidget( item, Qgis::MessageLevel::Warning );
  };

  // when export is successful:
  QObject::connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, connectionContext, [onSuccessfulCompletion, connectionUri, longTitle, pushError, connectionProvider, destSchema, destTableName, tableComment, shortTitle, context]() {
    if ( !tableComment.isEmpty() )
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> connection;
      try
      {
        QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( connectionProvider );
        connection.reset( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) );
      }
      catch ( QgsProviderConnectionException &e )
      {
        pushError( QObject::tr( "Could not retrieve connection details:\n\n%1" ).arg( e.what() ) );
        return;
      }

      try
      {
        connection->setTableComment( destSchema, destTableName, tableComment );
      }
      catch ( QgsProviderConnectionException &e )
      {
        pushError( QObject::tr( "Failed to set new table comment!\n\n" ) + e.what() );
        return;
      }
    }

    context.messageBar()->pushSuccess( shortTitle, QObject::tr( "Import was successful." ) );
    onSuccessfulCompletion();
  } );

  // when an error occurs:
  QObject::connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, connectionContext, [onError, shortTitle, pushError, longTitle]( Qgis::VectorExportResult error, const QString &errorMessage ) {
    if ( error != Qgis::VectorExportResult::UserCanceled )
    {
      pushError( QObject::tr( "Failed to import layer!\n\n" ) + errorMessage );
    }

    onError( error, errorMessage );
  } );

  QgsApplication::taskManager()->addTask( exportTask.release() );
  return true;
}

void QgsDataItemGuiProviderUtils::handleImportVectorLayerForConnection( std::unique_ptr<QgsAbstractDatabaseProviderConnection> connection, const QString &destinationSchema, QgsDataItemGuiContext context, const QString &shortTitle, const QString &longTitle, const QVariantMap &destinationProviderOptions, const std::function<void()> &onSuccessfulCompletion, const std::function<void( Qgis::VectorExportResult, const QString & )> &onError, QObject *connectionContext )
{
  if ( !connection )
    return;

  const QString connectionUri = connection->uri();
  const QString connectionProvider = connection->providerKey();

  QgsDbImportVectorLayerDialog dialog( connection.release(), context.messageBar() ? context.messageBar()->parentWidget() : nullptr );
  dialog.setDestinationSchema( destinationSchema );
  if ( !dialog.exec() )
    return;

  std::unique_ptr< QgsVectorLayerExporterTask > exportTask = dialog.createExporterTask( destinationProviderOptions );
  if ( !exportTask )
    return;

  const QString destSchema = dialog.schema();
  const QString destTableName = dialog.tableName();
  const QString tableComment = dialog.tableComment();

  auto pushError = [shortTitle, longTitle, context]( const QString &error ) {
    QgsMessageBarItem *item = new QgsMessageBarItem( shortTitle, QObject::tr( "Import failed." ), Qgis::MessageLevel::Warning, 0, nullptr );
    QPushButton *detailsButton = new QPushButton( QObject::tr( "Details…" ) );
    QObject::connect( detailsButton, &QPushButton::clicked, detailsButton, [=] {
      QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
      output->setTitle( longTitle );
      output->setMessage( error, QgsMessageOutput::MessageText );
      output->showMessage();
    } );
    item->layout()->addWidget( detailsButton );
    context.messageBar()->pushWidget( item, Qgis::MessageLevel::Warning );
  };

  // when export is successful:
  QObject::connect( exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, connectionContext, [onSuccessfulCompletion, connectionUri, longTitle, pushError, connectionProvider, destSchema, destTableName, tableComment, shortTitle, context]() {
    if ( !tableComment.isEmpty() )
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> connection;
      try
      {
        QgsProviderMetadata *md = QgsProviderRegistry::instance()->providerMetadata( connectionProvider );
        connection.reset( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionUri, {} ) ) );
      }
      catch ( QgsProviderConnectionException &e )
      {
        pushError( QObject::tr( "Could not retrieve connection details:\n\n%1" ).arg( e.what() ) );
        return;
      }

      try
      {
        connection->setTableComment( destSchema, destTableName, tableComment );
      }
      catch ( QgsProviderConnectionException &e )
      {
        pushError( QObject::tr( "Failed to set new table comment!\n\n" ) + e.what() );
        return;
      }
    }

    context.messageBar()->pushSuccess( shortTitle, QObject::tr( "Import was successful." ) );
    onSuccessfulCompletion();
  } );

  // when an error occurs:
  QObject::connect( exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, connectionContext, [onError, shortTitle, pushError, longTitle]( Qgis::VectorExportResult error, const QString &errorMessage ) {
    if ( error != Qgis::VectorExportResult::UserCanceled )
    {
      pushError( QObject::tr( "Failed to import layer!\n\n" ) + errorMessage );
    }

    onError( error, errorMessage );
  } );

  QgsApplication::taskManager()->addTask( exportTask.release() );
}
