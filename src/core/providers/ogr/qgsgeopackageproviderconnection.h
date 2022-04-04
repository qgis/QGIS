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
#include "qgsogrproviderconnection.h"
#include "qgsogrutils.h"

///@cond PRIVATE
#define SIP_NO_FILE



struct QgsGeoPackageProviderResultIterator: public QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator
{

    QgsGeoPackageProviderResultIterator( gdal::ogr_datasource_unique_ptr hDS, OGRLayerH ogrLayer );

    ~QgsGeoPackageProviderResultIterator();

    void setFields( const QgsFields &fields );
    void setGeometryColumnName( const QString &geometryColumnName );
    void setPrimaryKeyColumnName( const QString &primaryKeyColumnName );

  private:

    gdal::ogr_datasource_unique_ptr mHDS;
    OGRLayerH mOgrLayer;
    QgsFields mFields;
    QVariantList mNextRow;
    QString mGeometryColumnName;
    QString mPrimaryKeyColumnName;
    long long mRowCount = -1;

    QVariantList nextRowPrivate() override;
    bool hasNextRowPrivate() const override;
    long long rowCountPrivate() const override;
    QVariantList nextRowInternal();

};

class QgsGeoPackageProviderConnection : public QgsOgrProviderConnection
{
  public:

    QgsGeoPackageProviderConnection( const QString &name );
    QgsGeoPackageProviderConnection( const QString &uri, const QVariantMap &configuration );


    // QgsAbstractProviderConnection interface
  public:
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QString tableUri( const QString &schema, const QString &name ) const override;
    void dropRasterTable( const QString &schema, const QString &name ) const override;
    void renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const override;
    QgsVectorLayer *createSqlVectorLayer( const SqlVectorLayerOptions &options ) const override;
    QueryResult execSql( const QString &sql, QgsFeedback *feedback = nullptr ) const override;
    void vacuum( const QString &schema, const QString &name ) const override;
    void createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options = QgsAbstractDatabaseProviderConnection::SpatialIndexOptions() ) const override;
    bool spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    void deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema = QString(),
        const TableFlags &flags = TableFlags() ) const override;
    QIcon icon() const override;
    QgsFields fields( const QString &schema, const QString &table ) const override;
    QMultiMap<Qgis::SqlKeywordCategory, QStringList> sqlDictionary() override;
    SqlVectorLayerOptions sqlOptions( const QString &layerSource ) override;

  private:

    void setDefaultCapabilities();
    //! Use GDAL to execute SQL
    QueryResult executeGdalSqlPrivate( const QString &sql, QgsFeedback *feedback = nullptr ) const;
    //! Returns PK name for table
    QString primaryKeyColumnName( const QString &table ) const;

};



///@endcond
#endif // QGSGEOPACKAGEPROVIDERCONNECTION_H
