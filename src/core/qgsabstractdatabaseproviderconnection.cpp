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

#include <QVariant>
#include <QObject>

QgsAbstractDatabaseProviderConnection::QgsAbstractDatabaseProviderConnection( const QString &name ):
  QgsAbstractProviderConnection( name )
{

}

QgsAbstractDatabaseProviderConnection::QgsAbstractDatabaseProviderConnection( const QString &name, const QgsDataSourceUri &uri ):
  QgsAbstractProviderConnection( name, uri )
{

}
QgsAbstractDatabaseProviderConnection::Capabilities QgsAbstractDatabaseProviderConnection::capabilities() const
{
  return mCapabilities;
}

bool QgsAbstractDatabaseProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString, QVariant> *
    options, QString &errCause )
{
  Q_UNUSED( schema );
  Q_UNUSED( name );
  Q_UNUSED( fields );
  Q_UNUSED( srs );
  Q_UNUSED( overwrite );
  Q_UNUSED( options );
  Q_UNUSED( wkbType );
  errCause = QObject::tr( "Operation 'createVectorTable' is not supported" );
  return false;
}

bool QgsAbstractDatabaseProviderConnection::renameTable( const QString &, const QString &, const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'renameTable' is not supported" );
  return false;
}

bool QgsAbstractDatabaseProviderConnection::createRasterTable( const QString &, const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'createRasterTable' is not supported" );
  return false;
}

bool QgsAbstractDatabaseProviderConnection::dropTable( const QString &, const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'dropTable' is not supported" );
  return false;
}


bool QgsAbstractDatabaseProviderConnection::createSchema( const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'createSchema' is not supported" );
  return false;
}

bool QgsAbstractDatabaseProviderConnection::dropSchema( const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'dropSchema' is not supported" );
  return false;
}

bool QgsAbstractDatabaseProviderConnection::renameSchema( const QString &, const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'renameSchema' is not supported" );
  return false;
}

bool QgsAbstractDatabaseProviderConnection::executeSql( const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'executeSql' is not supported" );
  return false;
}

bool QgsAbstractDatabaseProviderConnection::vacuum( const QString &, const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'vacuum' is not supported" );
  return false;
}

QStringList QgsAbstractDatabaseProviderConnection::tables( const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'tables' is not supported" );
  return QStringList();
}

QStringList QgsAbstractDatabaseProviderConnection::schemas( QString &errCause )
{
  errCause = QObject::tr( "Operation 'schemas' is not supported" );
  return QStringList();
}
