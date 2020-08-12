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


class QgsPostgresProviderConnection : public QgsAbstractDatabaseProviderConnection

{
  public:

    QgsPostgresProviderConnection( const QString &name );
    QgsPostgresProviderConnection( const QString &uri, const QVariantMap &configuration );

    // QgsAbstractProviderConnection interface

  public:

    void createVectorTable( const QString &schema,
                            const QString &name,
                            const QgsFields &fields,
                            QgsWkbTypes::Type wkbType,
                            const QgsCoordinateReferenceSystem &srs, bool overwrite,
                            const QMap<QString, QVariant> *options ) const override;

    QString tableUri( const QString &schema, const QString &name ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    void dropRasterTable( const QString &schema, const QString &name ) const override;
    void renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const override;
    void renameRasterTable( const QString &schema, const QString &name, const QString &newName ) const override;
    void createSchema( const QString &name ) const override;
    void dropSchema( const QString &name, bool force = false ) const override;
    void renameSchema( const QString &name, const QString &newName ) const override;
    QList<QVariantList> executeSql( const QString &sql ) const override;
    void vacuum( const QString &schema, const QString &name ) const override;
    void createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options = QgsAbstractDatabaseProviderConnection::SpatialIndexOptions() ) const override;
    bool spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    void deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema,
        const TableFlags &flags = nullptr ) const override;
    QStringList schemas( ) const override;
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QIcon icon() const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;

  private:

    QList<QVariantList> executeSqlPrivate( const QString &sql, bool resolveTypes = true ) const;
    void setDefaultCapabilities();
    void dropTablePrivate( const QString &schema, const QString &name ) const;
    void renameTablePrivate( const QString &schema, const QString &name, const QString &newName ) const;


};



#endif // QGSPOSTGRESPROVIDERCONNECTION_H
