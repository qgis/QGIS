/***************************************************************************
  qgsOracleproviderconnection.h - QgsOracleProviderConnection

 ---------------------
 begin                : 28.12.2020
 copyright            : (C) 2020 by Julien Cabieces
 email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSORACLEPROVIDERCONNECTION_H
#define QGSORACLEPROVIDERCONNECTION_H
#include "qgsabstractdatabaseproviderconnection.h"

#include <QSqlQuery>

class QgsOracleQuery;

struct QgsOracleProviderResultIterator : public QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator
{
    QgsOracleProviderResultIterator( int columnCount, std::unique_ptr<QgsOracleQuery> query );

    QVariantList nextRowPrivate() override;
    bool hasNextRowPrivate() const override;

  private:
    int mColumnCount = 0;
    std::unique_ptr<QgsOracleQuery> mQuery;
    QVariantList mNextRow;

    QVariantList nextRowInternal();
    long long rowCountPrivate() const override;
};

class QgsOracleProviderConnection : public QgsAbstractDatabaseProviderConnection

{
  public:
    explicit QgsOracleProviderConnection( const QString &name );
    QgsOracleProviderConnection( const QString &uri, const QVariantMap &configuration );

    // QgsAbstractProviderConnection interface

    void createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const override;

    QString tableUri( const QString &schema, const QString &name ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    void renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const override;

    QgsAbstractDatabaseProviderConnection::QueryResult execSql( const QString &sql, QgsFeedback *feedback = nullptr ) const override;
    void createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options = QgsAbstractDatabaseProviderConnection::SpatialIndexOptions() ) const override;
    bool spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    void deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    QStringList schemas() const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema, const TableFlags &flags = TableFlags(), QgsFeedback *feedback = nullptr ) const override;
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QIcon icon() const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;
    QMultiMap<Qgis::SqlKeywordCategory, QStringList> sqlDictionary() override;
    QgsVectorLayer *createSqlVectorLayer( const SqlVectorLayerOptions &options ) const override;

  private:
    QgsAbstractDatabaseProviderConnection::QueryResult executeSqlPrivate( const QString &sql, QgsFeedback *feedback = nullptr ) const;
    void setDefaultCapabilities();


    // QgsAbstractDatabaseProviderConnection interface
  public:
    SqlVectorLayerOptions sqlOptions( const QString &layerSource ) override;
};


#endif // QGSORACLEPROVIDERCONNECTION_H
