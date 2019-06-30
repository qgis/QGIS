/***************************************************************************
    qgspostgresprojectstorage.h
    ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOSTGRESPROJECTSTORAGE_H
#define QGSPOSTGRESPROJECTSTORAGE_H

#include "qgsconfig.h"
#include "qgsprojectstorage.h"

#include "qgsdatasourceuri.h"


//! Stores information parsed from postgres project URI
typedef struct
{
  bool valid;

  QgsDataSourceUri connInfo;  // using only the bits about connection info (server, port, username, password, service, ssl mode)

  QString schemaName;
  QString projectName;

} QgsPostgresProjectUri;


//! Implements storage of QGIS projects inside a PostgreSQL table
class QgsPostgresProjectStorage : public QgsProjectStorage
{
  public:

    QString type() override { return QStringLiteral( "postgresql" ); }

    QStringList listProjects( const QString &uri ) override;

    bool readProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) override;

    bool writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) override;

    bool removeProject( const QString &uri ) override;

    bool readProjectStorageMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata ) override;

    static QString encodeUri( const QgsPostgresProjectUri &postUri );
    static QgsPostgresProjectUri decodeUri( const QString &uri );
};


#endif // QGSPOSTGRESPROJECTSTORAGE_H
