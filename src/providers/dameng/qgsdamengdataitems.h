/***************************************************************************
    qgsdamengdataitems.h
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
#ifndef QGSDAMENGDATAITEMS_H
#define QGSDAMENGDATAITEMS_H

#include <QMainWindow>

#include "qgsconnectionsitem.h"
#include "qgsdatacollectionitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdatabaseschemaitem.h"
#include "qgslayeritem.h"

#include "qgsdamengconn.h"
#include "qgsmimedatautils.h"
#include "qgswkbtypes.h"

class QgsDamengRootItem;
class QgsDamengConnectionItem;
class QgsDamengSchemaItem;
class QgsDamengLayerItem;

class QgsDamengRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsDamengRootItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 3; }

  public slots:
    void onConnectionsChanged();
};

class QgsDamengConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsDamengConnectionItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

    using QgsDataCollectionItem::handleDrop;
    bool handleDrop( const QMimeData *data, const QString &toSchema );

  signals:
    void addGeometryColumn( const QgsDamengLayerProperty & );

  public slots:

    // refresh specified schema or all schemas if schema name is empty
    void refreshSchema( const QString &schema );
};

class QgsDamengSchemaItem : public QgsDatabaseSchemaItem
{
    Q_OBJECT
  public:
    QgsDamengSchemaItem( QgsDataItem *parent, const QString &connectionName, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QString connectionName() const { return mConnectionName; }

  private:
    QgsDamengLayerItem *createLayer( QgsDamengLayerProperty layerProperty );

    QString mConnectionName;

    // QgsDataItem interface
  public:
    bool layerCollection() const override;
};

class QgsDamengLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsDamengLayerItem( QgsDataItem *parent, const QString &name, const QString &path, Qgis::BrowserLayerType layerType, const QgsDamengLayerProperty &layerProperties );

    QString createUri();

    QString comments() const override;

    const QgsDamengLayerProperty &layerInfo() const { return mLayerProperty; }

    QVector<QgsDataItem *> createChildren() override;

  private:
    QgsDamengLayerProperty mLayerProperty;
};


//! Provider for Dameng data item
class QgsDamengDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;

    QString dataProviderKey() const override;

    Qgis::DataItemProviderCapabilities capabilities() const override;

    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;
};

#endif // QGSDAMENGDATAITEMS_H
