/***************************************************************************
   qgsredshiftdataitems.h
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
#ifndef QGSREDSHIFTDATAITEMS_H
#define QGSREDSHIFTDATAITEMS_H

#include <QMainWindow>

#include "qgsconnectionsitem.h"
#include "qgsdatacollectionitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdatabaseschemaitem.h"
#include "qgslayeritem.h"

#include "qgsdataitemprovider.h"
#include "qgsmimedatautils.h"
#include "qgsredshiftconn.h"
#include "qgswkbtypes.h"

class QgsRSRootItem;
class QgsRSConnectionItem;
class QgsRSSchemaItem;
class QgsRSLayerItem;

class QgsRSRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsRSRootItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override
    {
      return 3;
    }

    static QMainWindow *sMainWindow;

  public slots:
    void onConnectionsChanged();
};

class QgsRSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsRSConnectionItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

    using QgsDataCollectionItem::handleDrop;
    bool handleDrop( const QMimeData *data, const QString &toSchema );

  signals:
    void addGeometryColumn( const QgsRedshiftLayerProperty & );

  public slots:

    // refresh specified schema or all schemas if schema name is empty
    void refreshSchema( const QString &schema );
};

class QgsRSSchemaItem : public QgsDatabaseSchemaItem
{
    Q_OBJECT
  public:
    QgsRSSchemaItem( QgsDataItem *parent, const QString &connectionName, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QString connectionName() const
    {
      return mConnectionName;
    }

  private:
    QgsRSLayerItem *createLayer( QgsRedshiftLayerProperty layerProperty );

    QString mConnectionName;

    // QgsDataItem interface
  public:
    bool layerCollection() const override;
};

class QgsRSLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsRSLayerItem( QgsDataItem *parent, const QString &name, const QString &path,
                    Qgis::BrowserLayerType layerType,
                    const QgsRedshiftLayerProperty &layerProperties );

    QString createUri();

    QString comments() const override;

    const QgsRedshiftLayerProperty &layerInfo() const
    {
      return mLayerProperty;
    }

    QVector<QgsDataItem *> createChildren() override;

  private:
    QgsRedshiftLayerProperty mLayerProperty;
};

//! Provider for Redshift data item
class QgsRedshiftDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;

    QString dataProviderKey() const override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;
};

#endif // QGSREDSHIFTDATAITEMS_H
