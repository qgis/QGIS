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
    QgsGeoPackageProviderConnection( const QString &uri, const QVariantMap &configuration );


    // QgsAbstractProviderConnection interface
  public:
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QString tableUri( const QString &schema, const QString &name ) const override;
    void createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, QgsWkbTypes::Type wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    void dropRasterTable( const QString &schema, const QString &name ) const override;
    void renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const override;
    QList<QList<QVariant>> executeSql( const QString &sql ) const override;
    void vacuum( const QString &schema, const QString &name ) const override;
    void createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options = QgsAbstractDatabaseProviderConnection::SpatialIndexOptions() ) const override;
    bool spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    void deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema = QString(),
        const TableFlags &flags = nullptr ) const override;
    QIcon icon() const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;

  private:

    void setDefaultCapabilities();
    //! Use GDAL to execute SQL
    QList<QVariantList> executeGdalSqlPrivate( const QString &sql ) const;

};



///@endcond
#endif // QGSGEOPACKAGEPROVIDERCONNECTION_H
