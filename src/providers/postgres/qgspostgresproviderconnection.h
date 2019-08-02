/***************************************************************************
  qgspostgresproviderconnection.h - QgsPostgresProviderConnection

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
#ifndef QGSPOSTGRESPROVIDERCONNECTION_H
#define QGSPOSTGRESPROVIDERCONNECTION_H
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsdatasourceuri.h"

class QgsPostgresProviderConnection : public QgsAbstractDatabaseProviderConnection
{
  public:

    QgsPostgresProviderConnection( const QgsDataSourceUri &uri );

  private:

    QgsDataSourceUri mUri;

    // QgsAbstractProviderConnection interface
  public:

    bool createTable( const QString &name, const QString &schema, QString &errCause ) override;
    bool dropTable( const QString &name, const QString &schema, QString &errCause ) override;
    bool renameTable( const QString &name, const QString &schema, const QString &newName, QString &errCause ) override;
    bool createSchema( const QString &name, QString &errCause ) override;
    bool dropSchema( const QString &name, QString &errCause ) override;
    bool renameSchema( const QString &name, const QString &newName, QString &errCause ) override;
    QVariant executeSql( const QString &sql, QString &errCause ) override;
    bool vacuum( const QString &name, QString &errCause ) override;
    QStringList tables( const QString &schema, QString &errCause ) override;
    QStringList schemas( QString &errCause ) override;
};

#endif // QGSPOSTGRESPROVIDERCONNECTION_H
