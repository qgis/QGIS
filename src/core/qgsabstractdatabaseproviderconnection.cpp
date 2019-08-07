/***************************************************************************
  qgsabstractdatabaseproviderconnection.cpp - QgsAbstractDatabaseProviderConnection

 ---------------------
 begin                : 2.8.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsexception.h"
#include <QVariant>
#include <QObject>

QgsAbstractDatabaseProviderConnection::QgsAbstractDatabaseProviderConnection( const QString &name ):
  QgsAbstractProviderConnection( name )
{

}

QgsAbstractDatabaseProviderConnection::QgsAbstractDatabaseProviderConnection( const QString &name, const QString &uri ):
  QgsAbstractProviderConnection( name, uri )
{

}
QgsAbstractDatabaseProviderConnection::Capabilities QgsAbstractDatabaseProviderConnection::capabilities() const
{
  return mCapabilities;
}

void QgsAbstractDatabaseProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString, QVariant> *
    options )
{
  Q_UNUSED( schema );
  Q_UNUSED( name );
  Q_UNUSED( fields );
  Q_UNUSED( srs );
  Q_UNUSED( overwrite );
  Q_UNUSED( options );
  Q_UNUSED( wkbType );
  throw QgsProviderConnectionException( QObject::tr( "Operation 'createVectorTable' is not supported" ) );
}

void QgsAbstractDatabaseProviderConnection::renameTable( const QString &, const QString &, const QString & )
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'renameTable' is not supported" ) );
}

void QgsAbstractDatabaseProviderConnection::createRasterTable( const QString &, const QString & )
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'createRasterTable' is not supported" ) );
}

void QgsAbstractDatabaseProviderConnection::dropTable( const QString &, const QString & )
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'dropTable' is not supported" ) );
}

void QgsAbstractDatabaseProviderConnection::createSchema( const QString & )
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'createSchema' is not supported" ) );
}

void QgsAbstractDatabaseProviderConnection::dropSchema( const QString &, bool )
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'dropSchema' is not supported" ) );
}

void QgsAbstractDatabaseProviderConnection::renameSchema( const QString &, const QString & )
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'renameSchema' is not supported" ) );
}

void QgsAbstractDatabaseProviderConnection::executeSql( const QString & )
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'executeSql' is not supported" ) );
}

void QgsAbstractDatabaseProviderConnection::vacuum( const QString &, const QString & )
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'vacuum' is not supported" ) );
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsAbstractDatabaseProviderConnection::tables( const QString &, const QgsAbstractDatabaseProviderConnection::TableFlags & )
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'tables' is not supported" ) );
}

QStringList QgsAbstractDatabaseProviderConnection::schemas( )
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'schemas' is not supported" ) );
}
