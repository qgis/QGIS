/***************************************************************************
    qgsdamengprojectstorage.h
    ---------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGPROJECTSTORAGE_H
#define QGSDAMENGPROJECTSTORAGE_H

#include "qgsconfig.h"
#include "qgsprojectstorage.h"

#include "qgsdatasourceuri.h"


//! Stores information parsed from dameng project URI
typedef struct
{
  bool valid;

  QgsDataSourceUri connInfo;  // using only the bits about connection info ( server, port, username, password, service, ssl mode )

  QString schemaName;
  QString projectName;

} QgsDamengProjectUri;


//! Implements storage of QGIS projects inside a Dameng table
class QgsDamengProjectStorage : public QgsProjectStorage
{
  public:

    QString type() override { return QStringLiteral( "dameng" ); }

    QStringList listProjects( const QString &uri ) override;

    bool readProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) override;

    bool writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) override;

    bool removeProject( const QString &uri ) override;

    bool readProjectStorageMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata ) override;

    static QString encodeUri( const QgsDamengProjectUri &postUri );
    static QgsDamengProjectUri decodeUri( const QString &uri );
};


#endif // QGSDAMENGPROJECTSTORAGE_H
