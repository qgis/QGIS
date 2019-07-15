/***************************************************************************
    qgsgeopackageprojectstorage.h
    ---------------------
    begin                : March 2019
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

#ifndef QGSGEOPACKAGEPROJECTSTORAGE_H
#define QGSGEOPACKAGEPROJECTSTORAGE_H

#include "qgsconfig.h"
#include "qgsprojectstorage.h"
#include "qgsdatasourceuri.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

//! Stores information parsed from postgres project URI
typedef struct
{
  bool valid;
  QString database;
  QString projectName;

} QgsGeoPackageProjectUri;


class CORE_EXPORT QgsGeoPackageProjectStorage : public QgsProjectStorage
{
  public:

    // QgsProjectStorage interface
  public:
    QString type() override { return QStringLiteral( "geopackage" ); }
    QStringList listProjects( const QString &uri ) override;
    bool readProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) override;
    bool writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) override;
    bool removeProject( const QString &uri ) override;
    bool renameProject( const QString &uri, const QString &uriNew ) override;
    bool readProjectStorageMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata ) override;
    static QString encodeUri( const QgsGeoPackageProjectUri &postUri );
    static QgsGeoPackageProjectUri decodeUri( const QString &uri );
    virtual QString filePath( const QString &uri ) override;

  private:
    QString _executeSql( const QString &uri, const QString &sql );
};

///@endcond
#endif // QGSGEOPACKAGEPROJECTSTORAGE_H
