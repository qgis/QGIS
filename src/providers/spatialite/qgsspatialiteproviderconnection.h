/***************************************************************************
  QgsSpatialiteProviderConnection.h - QgsSpatialiteProviderConnection

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
#ifndef QGSSPATIALITEPROVIDERCONNECTION_H
#define QGSSPATIALITEPROVIDERCONNECTION_H

#include "qgsabstractdatabaseproviderconnection.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsSpatiaLiteProviderConnection : public QgsAbstractDatabaseProviderConnection
{
  public:

    QgsSpatiaLiteProviderConnection( const QString &name );
    // Note: URI must be in PG QgsDataSourceUri format ( "dbname='path_to_sqlite.db'" )
    QgsSpatiaLiteProviderConnection( const QString &uri, const QVariantMap &configuration );


    // QgsAbstractProviderConnection interface
  public:
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QString tableUri( const QString &schema, const QString &name ) const override;
    void createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, QgsWkbTypes::Type wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    void renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const override;
    QList<QList<QVariant>> executeSql( const QString &sql ) const override;
    void vacuum( const QString &schema, const QString &name ) const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema = QString(),
        const TableFlags &flags = nullptr ) const override;

  private:

    void setDefaultCapabilities();
    //! Use GDAL to execute SQL
    QList<QVariantList> executeSqlPrivate( const QString &sql ) const;

    //! extract the path from the DS URI (which is in "PG" form: 'dbname=\'/path_to.sqlite\' table="table_name" (geom_col_name)')
    QString pathFromUri() const;

};

///@endcond
#endif // QGSSPATIALITEPROVIDERCONNECTION_H
