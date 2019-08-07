/***************************************************************************
  QgsGeoPackageProviderConnection.h - QgsGeoPackageProviderConnection

 ---------------------
 begin                : 6.8.2019
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
#ifndef QGSGEOPACKAGEPROVIDERCONNECTION_H
#define QGSGEOPACKAGEPROVIDERCONNECTION_H

#include "qgsabstractdatabaseproviderconnection.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsGeoPackageProviderConnection : public QgsAbstractDatabaseProviderConnection
{
  public:

    QgsGeoPackageProviderConnection( const QString &name );
    QgsGeoPackageProviderConnection( const QString &name, const QString &uri );


    // QgsAbstractProviderConnection interface
  public:
    void store( QVariantMap guiConfig ) override;
    void remove() override;
    void createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, QgsWkbTypes::Type wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) override;
    void dropTable( const QString &schema, const QString &name ) override;
    void renameTable( const QString &schema, const QString &name, const QString &newName ) override;
    void executeSql( const QString &sql ) override;
    void vacuum( const QString &schema, const QString &name ) override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema = QString() ) override;

  private:

    void setDefaultCapabilities();
    //! Use GDAL to execute SQL
    void executeGdalSqlPrivate( const QString &sql );
    //! Use sqlite to execute SQL
    void executeSqliteSqlPrivate( const QString &sql );

};

///@endcond
#endif // QGSGEOPACKAGEPROVIDERCONNECTION_H
