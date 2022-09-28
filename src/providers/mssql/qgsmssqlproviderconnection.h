/***************************************************************************
  qgsmssqlproviderconnection.h - QgsMssqlProviderConnection

 ---------------------
 begin                : 10.3.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMSSQLPROVIDERCONNECTION_H
#define QGSMSSQLPROVIDERCONNECTION_H

#include "qgsabstractdatabaseproviderconnection.h"

#include <QSqlQuery>


struct QgssMssqlProviderResultIterator: public QgsAbstractDatabaseProviderConnection::QueryResult::QueryResultIterator
{

    QgssMssqlProviderResultIterator( bool resolveTypes, int columnCount, std::unique_ptr<QSqlQuery> query );

  private:

    bool mResolveTypes = true;
    int mColumnCount = 0;
    std::unique_ptr<QSqlQuery> mQuery;
    QVariantList mNextRow;

    QVariantList nextRowPrivate() override;
    bool hasNextRowPrivate() const override;

    QVariantList nextRowInternal();


    // QueryResultIterator interface
    long long rowCountPrivate() const override;
};

class QgsMssqlProviderConnection : public QgsAbstractDatabaseProviderConnection

{
  public:

    QgsMssqlProviderConnection( const QString &name );
    QgsMssqlProviderConnection( const QString &uri, const QVariantMap &configuration );

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
    void createSchema( const QString &name ) const override;
    void dropSchema( const QString &name, bool force = false ) const override;
    QgsAbstractDatabaseProviderConnection::QueryResult execSql( const QString &sql, QgsFeedback *feedback ) const override;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> tables( const QString &schema,
        const TableFlags &flags = TableFlags() ) const override;
    QStringList schemas( ) const override;
    void store( const QString &name ) const override;
    void remove( const QString &name ) const override;
    QIcon icon() const override;
    QList<QgsVectorDataProvider::NativeType> nativeTypes() const override;
    QgsProviderSqlQueryBuilder *queryBuilder() const override;

  private:

    QgsAbstractDatabaseProviderConnection::QueryResult executeSqlPrivate( const QString &sql, bool resolveTypes = true, QgsFeedback *feedback = nullptr ) const;
    void setDefaultCapabilities();
    void dropTablePrivate( const QString &schema, const QString &name ) const;
    void renameTablePrivate( const QString &schema, const QString &name, const QString &newName ) const;

    static const QStringList EXTRA_CONNECTION_PARAMETERS;

};



#endif // QGSMSSQLPROVIDERCONNECTION_H
