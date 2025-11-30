/***************************************************************************
    qgspostgresdataitems.h
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOSTGRESDATAITEMS_H
#define QGSPOSTGRESDATAITEMS_H

#include "qgsconnectionsitem.h"
#include "qgsdatabaseschemaitem.h"
#include "qgsdatacollectionitem.h"
#include "qgsdataitemprovider.h"
#include "qgslayeritem.h"
#include "qgsmimedatautils.h"
#include "qgspostgresconn.h"
#include "qgspostgresprojectstorage.h"
#include "qgsprojectitem.h"
#include "qgswkbtypes.h"

#include <QMainWindow>

class QgsPGRootItem;
class QgsPGConnectionItem;
class QgsPGSchemaItem;
class QgsPGLayerItem;

class QgsPGRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsPGRootItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    [[nodiscard]] QVariant sortKey() const override { return 3; }

  public slots:
    void onConnectionsChanged();
};

class QgsPGConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGConnectionItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;
    [[nodiscard]] QgsDataSourceUri connectionUri() const;

  signals:
    void addGeometryColumn( const QgsPostgresLayerProperty & );

  public slots:

    // refresh specified schema or all schemas if schema name is empty
    void refreshSchema( const QString &schema );
};

class QgsPGSchemaItem : public QgsDatabaseSchemaItem
{
    Q_OBJECT
  public:
    QgsPGSchemaItem( QgsDataItem *parent, const QString &connectionName, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    [[nodiscard]] QString connectionName() const { return mConnectionName; }

  private:
    QgsPGLayerItem *createLayer( QgsPostgresLayerProperty layerProperty );

    QString mConnectionName;

    // QgsDataItem interface
  public:
    [[nodiscard]] bool layerCollection() const override;
};

class QgsPGLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsPGLayerItem( QgsDataItem *parent, const QString &name, const QString &path, Qgis::BrowserLayerType layerType, const QgsPostgresLayerProperty &layerProperties );

    QString createUri();

    [[nodiscard]] QString comments() const override;

    [[nodiscard]] const QgsPostgresLayerProperty &layerInfo() const { return mLayerProperty; }

    QVector<QgsDataItem *> createChildren() override;

  private:
    QgsPostgresLayerProperty mLayerProperty;
};


//! Provider for Postgres data item
class QgsPostgresDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;

    [[nodiscard]] QString dataProviderKey() const override;

    [[nodiscard]] Qgis::DataItemProviderCapabilities capabilities() const override;

    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;
};

/*
 * Class representing QgsProject stored in Postgres database
 *
 * \since QGIS 3.44
 */
class QgsPGProjectItem : public QgsProjectItem
{
    Q_OBJECT
  public:
    QgsPGProjectItem( QgsDataItem *parent, const QString name, const QgsPostgresProjectUri &postgresProjectUri, const QString &connectionName );

    [[nodiscard]] QString schemaName() const { return mProjectUri.schemaName; }
    [[nodiscard]] QgsPostgresProjectUri postgresProjectUri() const { return mProjectUri; }

    QString uriWithNewName( const QString &newProjectName );

    /*
    * Returns the name of the Postgres connection.
    *
    * \since QGIS 4.0
    */
    [[nodiscard]] QString connectionName() const { return mConnectionName; }


  private:
    QgsPostgresProjectUri mProjectUri;
    QString mConnectionName;
};

#endif // QGSPOSTGRESDATAITEMS_H
