/***************************************************************************
    qgsoracleprojectstorage.h
    ---------------------
    begin                : March 2022
    copyright            : (C) 2022 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSORACLEPROJECTSTORAGE_H
#define QGSORACLEPROJECTSTORAGE_H

#include "qgsconfig.h"
#include "qgsprojectstorage.h"

#include "qgsdatasourceuri.h"


//! Stores information parsed from oracle project URI
typedef struct
{
  bool valid;

  QgsDataSourceUri connInfo;  // using only the bits about connection info (server, port, username, password, service, ssl mode)

  QString owner;
  QString projectName;

} QgsOracleProjectUri;


//! Implements storage of QGIS projects inside a OracleQL table
class QgsOracleProjectStorage : public QgsProjectStorage
{
  public:

    QString type() override { return QStringLiteral( "oracle" ); }

    QStringList listProjects( const QString &uri ) override;

    bool readProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) override;

    bool writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) override;

    bool removeProject( const QString &uri ) override;

    bool readProjectStorageMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata ) override;

    static QString encodeUri( const QgsOracleProjectUri &postUri );
    static QgsOracleProjectUri decodeUri( const QString &uri );
};


#endif // QGSORACLEPROJECTSTORAGE_H
