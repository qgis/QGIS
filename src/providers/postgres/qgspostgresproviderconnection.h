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
#include "qgspostgresconnpool.h"

struct QgsPostgresProviderResultIterator: public QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator
{
    QgsPostgresProviderResultIterator( bool resolveTypes )
      : mResolveTypes( resolveTypes )
    {}

    QMap<int, QVariant::Type> typeMap;
    std::unique_ptr<QgsPostgresResult> result;

  private:

    QVariantList nextRowPrivate() override;
    bool hasNextRowPrivate() const override;
    long long rowCountPrivate() const override;

    bool mResolveTypes = true;
    long long mRowIndex = 0;
};

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
    QgsFields fields( const QString &schema, const QString &table ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    void dropRasterTable( const QString &schema, const QString &name ) const override;
    void renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const override;
    void renameRasterTable( const QString &schema, const QString &name, const QString &newName ) const override;
    void createSchema( const QString &name ) const override;
    void dropSchema( const QString &name, bool force = false ) const override;
    void renameSchema( const QString &name, const QString &newName ) const override;
    QgsAbstractDatabaseProviderConnection::QueryResult execSql( const QString &sql, QgsFeedback *feedback = nullptr ) const override;
    void vacuum( const QString &schema, const QString &name ) const override;
    void createSpatialIndex( const QString &schema, const QString &name, const QgsAbstractDatabaseProviderConnection::SpatialIndexOptions &options = QgsAbstractDatabaseProviderConnection::SpatialIndexOptions() ) const override;
    bool spatialIndexExists( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    void deleteSpatialIndex( const QString &schema, const QString &name, const QString &geometryColumn ) const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema,
        const TableFlags &flags = TableFlags() ) const override;
    QStringList schemas( ) const override;
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QIcon icon() const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;
    QgsVectorLayer *createSqlVectorLayer( const SqlVectorLayerOptions &options ) const override;
    QMultiMap<Qgis::SqlKeywordCategory, QStringList> sqlDictionary() override;
    SqlVectorLayerOptions sqlOptions( const QString &layerSource ) override;
    QList<QgsLayerMetadataProviderResult> searchLayerMetadata( const QgsMetadataSearchContext &searchContext, const QString &searchString, const QgsRectangle &geographicExtent, QgsFeedback *feedback ) const override;

    static const QStringList CONFIGURATION_PARAMETERS;
    static const QString SETTINGS_BASE_KEY;

  private:

    QList<QVariantList> executeSqlPrivate( const QString &sql, bool resolveTypes = true, QgsFeedback *feedback = nullptr, std::shared_ptr< class QgsPoolPostgresConn > pgconn = nullptr ) const;
    QgsAbstractDatabaseProviderConnection::QueryResult execSqlPrivate( const QString &sql, bool resolveTypes = true, QgsFeedback *feedback = nullptr, std::shared_ptr< class QgsPoolPostgresConn > pgconn = nullptr ) const;
    void setDefaultCapabilities();
    void dropTablePrivate( const QString &schema, const QString &name ) const;
    void renameTablePrivate( const QString &schema, const QString &name, const QString &newName ) const;


};


#endif // QGSPOSTGRESPROVIDERCONNECTION_H
