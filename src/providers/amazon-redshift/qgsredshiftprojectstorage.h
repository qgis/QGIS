/***************************************************************************
   qgsredshiftprojectstorage.h
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSREDSHIFTPROJECTSTORAGE_H
#define QGSREDSHIFTPROJECTSTORAGE_H

#include "qgsconfig.h"
#include "qgsdatasourceuri.h"
#include "qgsprojectstorage.h"
#include "qgsredshiftconn.h"

//! Stores information parsed from Redshift project URI
struct QgsRedshiftProjectUri
{
  bool valid;

  //! QGIS URI of Redshift database where project is stored.
  QgsDataSourceUri connInfo;

  QString schemaName;
  QString projectName;

};

//! Implements storage of QGIS projects inside a Redshift table
class QgsRedshiftProjectStorage : public QgsProjectStorage
{
  public:
    QString type() override
    {
      return QStringLiteral( "redshift" );
    }

    QStringList listProjects( const QString &uri ) override;

    bool readProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) override;

    bool writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) override;

    bool removeProject( const QString &uri ) override;

    bool readProjectStorageMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata ) override;

    static QString encodeUri( const QgsRedshiftProjectUri &postUri );
    static QgsRedshiftProjectUri decodeUri( const QString &uri );

  private:
    static bool getProjectUriAndConn( const QString &uri, QgsRedshiftProjectUri &projectUri, QgsRedshiftConn *&conn, QString &error_msg );
};

#endif // QGSREDSHIFTPROJECTSTORAGE_H
