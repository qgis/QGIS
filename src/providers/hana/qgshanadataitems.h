/***************************************************************************
   qgshanadataitems.h
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
#ifndef QGSHANADATAITEMS_H
#define QGSHANADATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgshanatablemodel.h"
#include "qgsmimedatautils.h"
#include "qgsvectorlayerexporter.h"

#include <QMainWindow>

class QgsHanaRootItem;
class QgsHanaConnectionItem;
class QgsHanaSchemaItem;
class QgsHanaLayerItem;

class QgsHanaRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsHanaRootItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 3; }

    QWidget *paramWidget() override;

    static QMainWindow *sMainWindow;

  public slots:
    void onConnectionsChanged();
};

class QgsHanaConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsHanaConnectionItem(QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

    bool handleDrop( const QMimeData *data, const QString &toSchema );

private:
  void updateToolTip(const QString& userName, const QString &dbmsVersion);

  signals:
    void addGeometryColumn( const QgsHanaLayerProperty &layerProperty);

  public slots:
    // refresh specified schema or all schemas if schema name is empty
    void refreshSchema( const QString &schema );
};

class QgsHanaSchemaItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsHanaSchemaItem( QgsDataItem *parent, const QString &connectionName, const QString &name,
      const QString &path );

    QVector<QgsDataItem *> createChildren() override;

  private:
    QgsHanaLayerItem *createLayer(const QgsHanaLayerProperty &layerProperty );

    QString mSchemaName;
    QString mConnectionName;
};

class QgsHanaLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsHanaLayerItem( QgsDataItem *parent, const QString &name, const QString &path,
      QgsLayerItem::LayerType layerType, const QgsHanaLayerProperty &layerProperties );

    QString createUri() const;

    QString comments() const override;

    const QgsHanaLayerProperty &layerInfo() const { return mLayerProperty; }

  private:
    QgsHanaLayerProperty mLayerProperty;
};

//! Provider for HANA data items
class QgsHanaDataItemProvider : public QgsDataItemProvider
{
public:
  QString name() override { return QStringLiteral("HANA"); }
  int capabilities() const override { return QgsDataProvider::Database; }
  QgsDataItem *createDataItem(const QString &pathIn, QgsDataItem *parentItem) override;
};


#endif // QGSHANADATAITEMS_H
