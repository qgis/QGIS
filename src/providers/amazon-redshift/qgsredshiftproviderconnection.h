/***************************************************************************
   qgsredshiftproviderconnection.h
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSREDSHIFTPROVIDERCONNECTION_H
#define QGSREDSHIFTPROVIDERCONNECTION_H
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsredshiftconn.h"

struct QgsRedshiftProviderResultIterator
  : public QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator
{
    QgsRedshiftProviderResultIterator( bool resolveTypes )
      : mResolveTypes( resolveTypes )
    {
    }

    QMap<int, QVariant::Type> typeMap;
    std::unique_ptr<QgsRedshiftResult> result;

  private:
    QVariantList nextRowPrivate() override;
    bool hasNextRowPrivate() const override;
    long long rowCountPrivate() const override;

    bool mResolveTypes = true;
    long long mRowIndex = 0;
};

class QgsRedshiftProviderConnection : public QgsAbstractDatabaseProviderConnection

{
  public:
    QgsRedshiftProviderConnection( const QString &name );
    QgsRedshiftProviderConnection( const QString &uri, const QVariantMap &configuration );

    // QgsAbstractProviderConnection interface

  public:
    void createVectorTable( const QString &schema, const QString &name, const QgsFields &fields,
                            Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite,
                            const QMap<QString, QVariant> *options ) const override;

    QString tableUri( const QString &schema, const QString &name ) const override;
    QgsAbstractDatabaseProviderConnection::QueryResult execSql( const QString &sql,
        QgsFeedback *feedback = nullptr ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    void renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const override;
    void createSchema( const QString &name ) const override;
    void dropSchema( const QString &name, bool force = false ) const override;
    void renameSchema( const QString &name, const QString &newName ) const override;
    void vacuum( const QString &schema, const QString &name ) const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables(
      const QString &schema, const TableFlags &flags = TableFlags() ) const override;
    QStringList schemas() const override;
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QIcon icon() const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;
//    QgsVectorLayer *createSqlVectorLayer( const SqlVectorLayerOptions &options ) const override;
//    QMultiMap<Qgis::SqlKeywordCategory, QStringList> sqlDictionary() override;
//    SqlVectorLayerOptions sqlOptions( const QString &layerSource ) override;

  private:
    QList<QVariantList> executeSqlPrivate( const QString &sql, bool resolveTypes = true, QgsFeedback *feedback = nullptr,
                                           std::shared_ptr<class QgsPoolRedshiftConn> pgconn = nullptr ) const;
    QgsAbstractDatabaseProviderConnection::QueryResult execSqlPrivate(
      const QString &sql, bool resolveTypes = true, QgsFeedback *feedback = nullptr,
      std::shared_ptr<class QgsPoolRedshiftConn> pgconn = nullptr ) const;
    void setDefaultCapabilities();
    void dropTablePrivate( const QString &schema, const QString &name ) const;
    void renameTablePrivate( const QString &schema, const QString &name, const QString &newName ) const;
};

#endif // QGSREDSHIFTPROVIDERCONNECTION_H
