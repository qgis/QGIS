/***************************************************************************
  qgsabstractproviderconnection.cpp - QgsAbstractProviderConnection

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
#include "qgsabstractproviderconnection.h"
#include <QObject>
#include <QVariant>

QgsAbstractProviderConnection::QgsAbstractProviderConnection()
{

}

QgsAbstractProviderConnection::Capabilities QgsAbstractProviderConnection::capabilities() const
{
  return mCapabilities;
}

bool QgsAbstractProviderConnection::createTable( const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'createTable' is not supported" );
  return false;
}

bool QgsAbstractProviderConnection::dropTable( const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'dropTable' is not supported" );
  return false;
}

bool QgsAbstractProviderConnection::renameTable( const QString &, const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'renameTable' is not supported" );
  return false;
}

bool QgsAbstractProviderConnection::createSchema( const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'createSchema' is not supported" );
  return false;
}

bool QgsAbstractProviderConnection::dropSchema( const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'dropSchema' is not supported" );
  return false;
}

bool QgsAbstractProviderConnection::renameSchema( const QString &, const QString &newName, QString &errCause )
{
  errCause = QObject::tr( "Operation 'renameSchema' is not supported" );
  return false;
}

QVariant QgsAbstractProviderConnection::executeSql( const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'executeSql' is not supported" );
  return QVariant();
}

bool QgsAbstractProviderConnection::vacuum( const QString &, QString &errCause )
{
  errCause = QObject::tr( "Operation 'vacuum' is not supported" );
  return false;
}

QStringList QgsAbstractProviderConnection::tables( const QString, QString &errCause )
{
  errCause = QObject::tr( "Operation 'tables' is not supported" );
  return QStringList();
}

QStringList QgsAbstractProviderConnection::schemas( QString &errCause )
{
  errCause = QObject::tr( "Operation 'schemas' is not supported" );
  return QStringList();
}
