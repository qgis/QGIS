/***************************************************************************
   qgshanaconnection.h
   --------------------------------------
   Date      : 31-05-2019
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
#ifndef QGSHANACONNECTION_H
#define QGSHANACONNECTION_H

#include "qgscoordinatereferencesystem.h"
#include "qgsdatasourceuri.h"
#include "qgshanatablemodel.h"
#include "qgshanaresultset.h"
#include "qgslogger.h"

#include "odbc/Forwards.h"

class QgsField;

class QgsHanaConnection : public QObject
{
    Q_OBJECT
  public:
    ~QgsHanaConnection() override;

    QString connInfo();

    void execute( const QString &sql );
    bool execute( const QString &sql, QString *errorMessage );
    QgsHanaResultSetRef executeQuery( const QString &sql );
    QgsHanaResultSetRef executeQuery( const QString &sql, const QVariantList &args );
    size_t executeCountQuery( const QString &sql );
    size_t executeCountQuery( const QString &sql, const QVariantList &args );
    QVariant executeScalar( const QString &sql );
    QVariant executeScalar( const QString &sql, const QVariantList &args );

    odbc::PreparedStatementRef prepareStatement( const QString &sql );

    void commit();
    void rollback();

    const QString &getDatabaseVersion();
    const QString &getUserName();
    QgsCoordinateReferenceSystem getCrs( int srid );
    QVector<QgsHanaLayerProperty> getLayers(
      const QString &schemaName,
      bool allowGeometrylessTables,
      bool userTablesOnly = true );
    void readLayerInfo( QgsHanaLayerProperty &layerProperty );
    QVector<QgsHanaSchemaProperty> getSchemas( const QString &ownerName );
    QgsWkbTypes::Type getLayerGeometryType( const QgsHanaLayerProperty &layerProperty );
    QString getColumnDataType( const QString &schemaName, const QString &tableName, const QString &columnName );
    QgsHanaResultSetRef getColumns( const QString &schemaName, const QString &tableName, const QString &fieldName );

    static QgsHanaConnection *createConnection( const QgsDataSourceUri &uri );
    static QgsHanaConnection *createConnection( const QgsDataSourceUri &uri, bool *cancelled );
    static QgsHanaConnection *createConnection( const QgsDataSourceUri &uri, bool *cancelled, QString *errorMessage );

    static QStringList connectionList();

  private:
    QgsHanaConnection( odbc::ConnectionRef connection, const QgsDataSourceUri &uri );

    int getLayerSRID( const QgsHanaLayerProperty &layerProperty );
    QStringList getLayerPrimaryeKeys( const QgsHanaLayerProperty &layerProperty );

    PreparedStatementRef createPreparedStatement( const QString &sql, const QVariantList &args );

  private:
    odbc::ConnectionRef mConnection;
    QgsDataSourceUri mUri;
    QString mDatabaseVersion;
    QString mUserName;
};

class QgsHanaConnectionRef
{
  public:
    QgsHanaConnectionRef() = default;
    QgsHanaConnectionRef( const QString &name );
    QgsHanaConnectionRef( const QgsDataSourceUri &uri );
    ~QgsHanaConnectionRef();

    bool isNull() const { return mConnection.get() == nullptr; }
    QgsHanaConnection &connection() const { return *mConnection; }

    QgsHanaConnection *operator->() { return mConnection.get(); }

  private:
    std::unique_ptr<QgsHanaConnection> mConnection;
};

#endif  // QGSHANACONNECTION_H
