/***************************************************************************
  qgsdataitemguiprovider.cpp
  --------------------------------------
  Date                 : October 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdataitemguiprovider.h"
#include "qgsdataitem.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsnewvectortabledialog.h"
#include "qgsmessagebar.h"
//
// QgsDataItemGuiContext
//

QgsMessageBar *QgsDataItemGuiContext::messageBar() const
{
  return mMessageBar;
}

void QgsDataItemGuiContext::setMessageBar( QgsMessageBar *messageBar )
{
  mMessageBar = messageBar;
}

//
// QgsDataItemGuiProvider
//

void QgsDataItemGuiProvider::populateContextMenu( QgsDataItem *, QMenu *, const QList<QgsDataItem *> &, QgsDataItemGuiContext )
{

}

void QgsDataItemGuiProvider::populateDatabaseContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context )
{
  Q_UNUSED( selectedItems )
  // Add create new table for connections
  if ( QgsDataCollectionItem * collectionItem { qobject_cast<QgsDataCollectionItem *>( item ) } )
  {
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( collectionItem->providerKey() ) };
    if ( md )
    {
      const bool isSchema { qobject_cast<QgsDatabaseSchemaItem *>( item ) };
      const QString connectionName { isSchema ? collectionItem->parent()->name() : collectionItem->name() };
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionName ) ) );
      if ( conn && conn->capabilities().testFlag( QgsAbstractDatabaseProviderConnection::Capability::CreateVectorTable ) )
      {
        QAction *newTableAction = new QAction( QObject::tr( "New Table…" ), menu );
        QObject::connect( newTableAction, &QAction::triggered, collectionItem, [ collectionItem, connectionName, md, isSchema, context]
        {
          std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn2 { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionName ) ) };
          QgsNewVectorTableDialog dlg { conn2.get(), nullptr };
          // TODO: dlg.setCrs( QgsProject::instance()->defaultCrsForNewLayers() );
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
            const QgsCoordinateReferenceSystem crs { dlg.crs( ) };
            QMap<QString, QVariant> options;
            if ( ! geometryColumn.isEmpty() )
            {
              options[ QStringLiteral( "geometryColumn" ) ] = geometryColumn;
            }
            try
            {
              conn2->createVectorTable( schemaName, tableName, fields, geometryType, crs, true, &options );
              collectionItem->refresh();
              if ( context.messageBar() )
              {
                context.messageBar()->pushSuccess( QObject::tr( "New Table Created" ), QObject::tr( "Table '%1' was created successfully." ).arg( tableName ) );
              }
            }
            catch ( QgsProviderConnectionException &ex )
            {
              if ( context.messageBar() )
              {
                context.messageBar()->pushCritical( QObject::tr( "New Table Creation Error" ), QObject::tr( "Error creating new table '%1': %2" ).arg( tableName, ex.what() ) );
              }
            }
          }
        } );
        menu->addAction( newTableAction );
      }
    }
  }
}

bool QgsDataItemGuiProvider::rename( QgsDataItem *, const QString &, QgsDataItemGuiContext )
{
  return false;
}

bool QgsDataItemGuiProvider::deleteLayer( QgsLayerItem *, QgsDataItemGuiContext )
{
  return false;
}

bool QgsDataItemGuiProvider::handleDoubleClick( QgsDataItem *, QgsDataItemGuiContext )
{
  return false;
}

bool QgsDataItemGuiProvider::acceptDrop( QgsDataItem *, QgsDataItemGuiContext )
{
  return false;
}

bool QgsDataItemGuiProvider::handleDrop( QgsDataItem *, QgsDataItemGuiContext, const QMimeData *, Qt::DropAction )
{
  return false;
}

QWidget *QgsDataItemGuiProvider::createParamWidget( QgsDataItem *, QgsDataItemGuiContext )
{
  return nullptr;
}
