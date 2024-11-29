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
#include "qgsogrutils.h"

///@cond PRIVATE
#define SIP_NO_FILE


struct QgsSpatialiteProviderResultIterator : public QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator
{
    QgsSpatialiteProviderResultIterator( gdal::dataset_unique_ptr hDS, OGRLayerH ogrLayer );

    ~QgsSpatialiteProviderResultIterator();

    void setFields( const QgsFields &fields );

    void setGeometryColumnName( const QString &geometryColumnName );

  private:
    gdal::dataset_unique_ptr mHDS;
    OGRLayerH mOgrLayer;
    QgsFields mFields;
    QVariantList mNextRow;
    QString mGeometryColumnName;

    QVariantList nextRowPrivate() override;
    bool hasNextRowPrivate() const override;
    QVariantList nextRowInternal();

    // QueryResultIterator interface
  private:
    long long rowCountPrivate() const override;
    long long mRowCount = -1;
};


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
    void createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const override;
    QgsVectorLayer *createSqlVectorLayer( const SqlVectorLayerOptions &options ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    void renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const override;
    QgsAbstractDatabaseProviderConnection::QueryResult execSql( const QString &sql, QgsFeedback *feedback = nullptr ) const override;
    void vacuum( const QString &schema, const QString &name ) const override;
    void createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options = QgsAbstractDatabaseProviderConnection::SpatialIndexOptions() ) const override;
    bool spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema = QString(), const TableFlags &flags = TableFlags(), QgsFeedback *feedback = nullptr ) const override;
    QIcon icon() const override;
    void deleteField( const QString &fieldName, const QString &schema, const QString &tableName, bool force ) const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;
    QMultiMap<Qgis::SqlKeywordCategory, QStringList> sqlDictionary() override;
    SqlVectorLayerOptions sqlOptions( const QString &layerSource ) override;

  private:
    void setDefaultCapabilities();
    //! Use GDAL to execute SQL
    QgsAbstractDatabaseProviderConnection::QueryResult executeSqlPrivate( const QString &sql, QgsFeedback *feedback = nullptr ) const;

    //! Executes SQL directly using sqlite3 -- avoids the extra consistency checks which GDAL requires when opening a spatialite database
    bool executeSqlDirect( const QString &sql ) const;

    //! extract the path from the DS URI (which is in "PG" form: 'dbname=\'/path_to.sqlite\' table="table_name" (geom_col_name)')
    QString pathFromUri() const;
};


///@endcond
#endif // QGSSPATIALITEPROVIDERCONNECTION_H
