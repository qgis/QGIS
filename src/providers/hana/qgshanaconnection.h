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
#include "qgshanadatatypes.h"
#include "qgsdatasourceuri.h"
#include "qgshanatablemodel.h"
#include "qgshanaresultset.h"
#include "qgsvectordataprovider.h"

#include "odbc/Forwards.h"

struct AttributeField
{
    QString schemaName;
    QString tableName;
    QString name;
    QgsHanaDataType type = QgsHanaDataType::Unknown;
    int srid = -1;
    QString typeName;
    int size = 0;
    int precision = 0;
    bool isAutoIncrement = false;
    bool isNullable = false;
    bool isSigned = false;
    bool isUnique = false;
    QString comment;

    QgsField toQgsField() const;
};

using AttributeFields = QVector<AttributeField>;

class QgsHanaConnection : public QObject
{
    Q_OBJECT
  public:
    ~QgsHanaConnection() override;

    QString connInfo() const;

    void execute( const QString &sql );
    bool execute( const QString &sql, QString *errorMessage );
    QgsHanaResultSetRef executeQuery( const QString &sql );
    QgsHanaResultSetRef executeQuery( const QString &sql, const QVariantList &args );
    size_t executeCountQuery( const QString &sql );
    size_t executeCountQuery( const QString &sql, const QVariantList &args );
    QVariant executeScalar( const QString &sql );
    QVariant executeScalar( const QString &sql, const QVariantList &args );

    NS_ODBC::PreparedStatementRef prepareStatement( const QString &sql );

    void commit();
    void rollback();

    QList<QgsVectorDataProvider::NativeType> getNativeTypes();
    const QString &getDatabaseVersion();
    const QString &getDatabaseCloudVersion();
    const QString &getUserName();
    QgsCoordinateReferenceSystem getCrs( int srid );
    QVector<QgsHanaLayerProperty> getLayers(
      const QString &schemaName,
      bool allowGeometrylessTables,
      bool userTablesOnly = true,
      const std::function<bool( const QgsHanaLayerProperty &layer )> &layerFilter = nullptr
    );
    QVector<QgsHanaLayerProperty> getLayersFull(
      const QString &schemaName,
      bool allowGeometrylessTables,
      bool userTablesOnly = true,
      const std::function<bool( const QgsHanaLayerProperty &layer )> &layerFilter = nullptr
    );
    void readLayerInfo( QgsHanaLayerProperty &layerProperty );
    void readQueryFields( const QString &schemaName, const QString &sql, const std::function<void( const AttributeField &field )> &callback );
    void readTableFields( const QString &schemaName, const QString &tableName, const std::function<void( const AttributeField &field )> &callback );
    QVector<QgsHanaSchemaProperty> getSchemas( const QString &ownerName );
    QStringList getLayerPrimaryKey( const QString &schemaName, const QString &tableName );
    Qgis::WkbType getColumnGeometryType( const QString &querySource, const QString &columnName );
    Qgis::WkbType getColumnGeometryType( const QString &schemaName, const QString &tableName, const QString &columnName );
    QString getColumnDataType( const QString &schemaName, const QString &tableName, const QString &columnName );
    int getColumnSrid( const QString &schemaName, const QString &tableName, const QString &columnName );
    int getColumnSrid( const QString &sql, const QString &columnName );
    QgsHanaResultSetRef getColumns( const QString &schemaName, const QString &tableName, const QString &fieldName );
    bool isTable( const QString &schemaName, const QString &tableName );

    static QgsHanaConnection *createConnection( const QgsDataSourceUri &uri );
    static QgsHanaConnection *createConnection( const QgsDataSourceUri &uri, bool *canceled );
    static QgsHanaConnection *createConnection( const QgsDataSourceUri &uri, bool *canceled, QString *errorMessage );

    static QStringList connectionList();

  private:
    QgsHanaConnection( NS_ODBC::ConnectionRef connection, const QgsDataSourceUri &uri );

    QStringList getPrimaryKeyCandidates( const QgsHanaLayerProperty &layerProperty );

    NS_ODBC::PreparedStatementRef createPreparedStatement( const QString &sql, const QVariantList &args );

  private:
    NS_ODBC::ConnectionRef mConnection;
    const QgsDataSourceUri mUri;
    QString mDatabaseVersion;
    QString mDatabaseCloudVersion;
    QString mUserName;
};

#endif // QGSHANACONNECTION_H
