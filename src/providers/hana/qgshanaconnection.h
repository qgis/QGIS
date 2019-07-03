/***************************************************************************
   qgshanaconnection.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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

#include "qgsdatasourceuri.h"
#include "qgshanatablemodel.h"
#include "qgslogger.h"

#include "odbc/Forwards.h"

class QgsField;

class QgsHanaConnection : public QObject
{
    Q_OBJECT
  public:
    ~QgsHanaConnection() override;

    QString connInfo();
    void disconnect();

    bool dropTable(const QString& schemaName, const QString& tableName, QString* errMessage);

    const QString& getDatabaseVersion();
    const QString& getUserName();
    QVector<QgsHanaLayerProperty> getLayers(
      const QString& schemaName,
      bool allowGeometrylessTables,
      bool userTablesOnly = true);
    void readLayerInfo(QgsHanaLayerProperty& layerProperty);
    QVector<QgsHanaSchemaProperty> getSchemas(const QString& ownerName);

    odbc::ConnectionRef& getNativeRef() { return mConnection; }

    static QgsHanaConnection* createConnection(const QgsDataSourceUri& uri);
    static void setConnectionAttemptCanceled(bool value) {
      sConnectionAttemptCanceled = value;
    }
    static bool isConnectionAttemptCanceled() {
      return sConnectionAttemptCanceled;
    }

  private:
    explicit QgsHanaConnection(const QgsDataSourceUri& uri);

    QgsWkbTypes::Type getLayerGeometryType(const QgsHanaLayerProperty& layerProperty);
    int getLayerSRID(const QgsHanaLayerProperty& layerProperty);
    QStringList getLayerPrimaryeKeys(const QgsHanaLayerProperty& layerProperty);

    static bool connect(
      odbc::ConnectionRef& conn,
      const QgsDataSourceUri& uri,
      QString& errorMessage);

  private:
    odbc::ConnectionRef mConnection;
    QgsDataSourceUri mUri;
    QString mDatabaseVersion;
    QString mUserName;

    static bool sConnectionAttemptCanceled;
};

class QgsHanaConnectionRef
{
  public:
    QgsHanaConnectionRef() = default;
    QgsHanaConnectionRef(const QString& name);
    QgsHanaConnectionRef(const QgsDataSourceUri& uri);
    ~QgsHanaConnectionRef();

    bool isNull() const { return mConnection.get() == nullptr; }
    QgsHanaConnection& connection() const { return *mConnection; }
    void release();

    QgsHanaConnection* operator->() { return mConnection.get(); }

  private:
    std::unique_ptr<QgsHanaConnection> mConnection;
};

#endif  // QGSHANACONNECTION_H
