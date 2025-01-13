/***************************************************************************
    qgsdamengproviderconnection.h - QgsDamengProviderConnection
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
#ifndef QGSDAMENGPROVIDERCONNECTION_H
#define QGSDAMENGPROVIDERCONNECTION_H

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsdamengconnpool.h"

struct QgsDamengProviderResultIterator: public QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator
{
    QgsDamengProviderResultIterator( bool resolveTypes )
      : mResolveTypes( resolveTypes )
    {}

    QMap<int, QMetaType::Type> typeMap;
    std::unique_ptr<QgsDamengResult> result;

  private:

    QVariantList nextRowPrivate() override;
    bool hasNextRowPrivate() const override;
    long long rowCountPrivate() const override;

    int mColumnCount = 0;
    bool mResolveTypes = true;
    long long mRowIndex = 0;
};

class QgsDamengProviderConnection : public QgsAbstractDatabaseProviderConnection
{
  public:
    QgsDamengProviderConnection( const QString &name );
    QgsDamengProviderConnection( const QString &uri, const QVariantMap &configuration );

    // QgsAbstractProviderConnection interface

  public:
    void createVectorTable( const QString &schema, const QString &name, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, const QMap<QString, QVariant> *options ) const override;

    QString tableUri( const QString &schema, const QString &name ) const override;
    QgsFields fields( const QString &schema, const QString &table, QgsFeedback *feedback = nullptr ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    void renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const override;
    void createSchema( const QString &name ) const override;
    void dropSchema( const QString &name, bool force = false ) const override;
    void renameSchema( const QString &name, const QString &newName ) = delete;
    QgsAbstractDatabaseProviderConnection::QueryResult execSql( const QString &sql, QgsFeedback *feedback = nullptr ) const override;
    void createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options = QgsAbstractDatabaseProviderConnection::SpatialIndexOptions() ) const override;
    bool spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    void deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema, const TableFlags &flags = TableFlags(), QgsFeedback *feedback = nullptr ) const override;
    QgsAbstractDatabaseProviderConnection::TableProperty table( const QString &schema, const QString &table, QgsFeedback *feedback = nullptr ) const override;
    QStringList schemas() const override;
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QIcon icon() const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;
    QgsVectorLayer *createSqlVectorLayer( const SqlVectorLayerOptions &options ) const override;
    QMultiMap<Qgis::SqlKeywordCategory, QStringList> sqlDictionary() override;
    SqlVectorLayerOptions sqlOptions( const QString &layerSource ) override;

    static const QStringList CONFIGURATION_PARAMETERS;
    static const QString SETTINGS_BASE_KEY;

  private:
    QList<QVariantList> executeSqlPrivate( const QString &sql, bool resolveTypes = true, QgsFeedback *feedback = nullptr, std::shared_ptr< class QgsPoolDamengConn > dmconn = nullptr ) const;
    QgsAbstractDatabaseProviderConnection::QueryResult execSqlPrivate( const QString &sql, bool resolveTypes = true, QgsFeedback *feedback = nullptr, std::shared_ptr< class QgsPoolDamengConn > dmconn = nullptr ) const;
    void setDefaultCapabilities();
    void dropTablePrivate( const QString &schema, const QString &name ) const;
    void renameTablePrivate( const QString &schema, const QString &name, const QString &newName ) const;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tablesPrivate( const QString &schema, const QString &table, const TableFlags &flags = TableFlags(), QgsFeedback *feedback = nullptr ) const;
};


#endif // QGSDAMENGPROVIDERCONNECTION_H
