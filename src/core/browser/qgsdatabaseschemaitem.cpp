/***************************************************************************
                             qgsdatabaseschemaitem.cpp
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatabaseschemaitem.h"
#include "moc_qgsdatabaseschemaitem.cpp"
#include "qgsapplication.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatabaseproviderconnection.h"

QgsDatabaseSchemaItem::QgsDatabaseSchemaItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &providerKey )
  : QgsDataCollectionItem( parent, name, path, providerKey )
{

}

QgsDatabaseSchemaItem::~QgsDatabaseSchemaItem()
{

}

QgsAbstractDatabaseProviderConnection *QgsDatabaseSchemaItem::databaseConnection() const
{
  const QString dataProviderKey { QgsApplication::dataItemProviderRegistry()->dataProviderKey( providerKey() ) };
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( dataProviderKey ) };
  if ( ! md )
  {
    return nullptr;
  }
  const QString connectionName { parent()->name() };
  try
  {
    return static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( connectionName ) );
  }
  catch ( QgsProviderConnectionException & )
  {
    // This is expected and it is not an error in case the provider does not implement
    // the connections API
  }
  return nullptr;
}
