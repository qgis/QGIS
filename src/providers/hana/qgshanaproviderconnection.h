/***************************************************************************
   qgshanaproviderconnection.h  -  QgsHanaProviderConnection
   --------------------------------------
   Date      : 07-04-2020
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANAPROVIDERCONNECTION_H
#define QGSHANAPROVIDERCONNECTION_H

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgshanaconnection.h"
#include "qgshanaresultset.h"

struct QgsHanaEmptyProviderResultIterator: public QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator
{
    // QueryResultIterator interface
  private:
    QVariantList nextRowPrivate() override;
    bool hasNextRowPrivate() const override;
    long long rowCountPrivate() const override { return 0; };
};

struct QgsHanaProviderResultIterator: public QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator
{
    QgsHanaProviderResultIterator( QgsHanaResultSetRef &&resultSet );

  private:
    QgsHanaResultSetRef mResultSet;
    unsigned short mNumColumns = 0;
    bool mNextRow = false;

    // QueryResultIterator interface
  private:
    QVariantList nextRowPrivate() override;
    bool hasNextRowPrivate() const override;
    long long rowCountPrivate() const override;
};

class QgsHanaConnectionRef;

class QgsHanaProviderConnection : public QgsAbstractDatabaseProviderConnection
{
  public:
    QgsHanaProviderConnection( const QString &name );
    QgsHanaProviderConnection( const QString &uri, const QVariantMap &configuration );

  public:
    void createVectorTable( const QString &schema,
                            const QString &name,
                            const QgsFields &fields,
                            QgsWkbTypes::Type wkbType,
                            const QgsCoordinateReferenceSystem &srs, bool overwrite,
                            const QMap<QString, QVariant> *options ) const override;

    QString tableUri( const QString &schema, const QString &name ) const override;
    void dropVectorTable( const QString &schema, const QString &name ) const override;
    void renameVectorTable( const QString &schema, const QString &name, const QString &newName ) const override;
    void createSchema( const QString &name ) const override;
    void dropSchema( const QString &name, bool force = false ) const override;
    void renameSchema( const QString &name, const QString &newName ) const override;
    QueryResult execSql( const QString &sql, QgsFeedback *feedback = nullptr ) const override;
    QgsAbstractDatabaseProviderConnection::TableProperty table( const QString &schema, const QString &table ) const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema,
        const TableFlags &flags = TableFlags() ) const override;
    QStringList schemas( ) const override;
    QgsFields fields( const QString &schema, const QString &table ) const override;
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QIcon icon() const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;
    QgsVectorLayer *createSqlVectorLayer( const SqlVectorLayerOptions &options ) const override;
    QMultiMap<Qgis::SqlKeywordCategory, QStringList> sqlDictionary() override;
    SqlVectorLayerOptions sqlOptions( const QString &layerSource ) override;

  private:
    QgsHanaConnectionRef createConnection() const;
    void executeSqlStatement( const QString &sql ) const;
    void setCapabilities();
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tablesWithFilter( const QString &schema,
        const TableFlags &flags = TableFlags(), const std::function<bool( const QgsHanaLayerProperty &layer )> &layerFilter = nullptr ) const;
};

#endif // QGSHANAPROVIDERCONNECTION_H
