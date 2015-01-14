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

#include <QMainWindow>

#include "qgsdataitem.h"

#include "qgspostgresconn.h"
#include "qgspgsourceselect.h"
#include "qgsmimedatautils.h"
#include "qgsvectorlayerimport.h"

class QgsPGRootItem;
class QgsPGConnectionItem;
class QgsPGSchemaItem;
class QgsPGLayerItem;

class QgsPGRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGRootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsPGRootItem();

    QVector<QgsDataItem*> createChildren() OVERRIDE;

    virtual QWidget * paramWidget() OVERRIDE;

    virtual QList<QAction*> actions() OVERRIDE;

    static QMainWindow *sMainWindow;

  public slots:
    void connectionsChanged();
    void newConnection();
};

class QgsPGConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsPGConnectionItem();

    QVector<QgsDataItem*> createChildren() OVERRIDE;
    virtual bool equal( const QgsDataItem *other ) OVERRIDE;
    virtual QList<QAction*> actions() OVERRIDE;

    virtual bool acceptDrop() OVERRIDE { return true; }
    virtual bool handleDrop( const QMimeData * data, Qt::DropAction action ) OVERRIDE;

  signals:
    void addGeometryColumn( QgsPostgresLayerProperty );

  public slots:
    void editConnection();
    void deleteConnection();
    void refreshConnection();

  private:
    QgsPostgresConn *mConn;
    QMap<QString, QgsPGSchemaItem * > mSchemaMap;
};

class QgsPGSchemaItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGSchemaItem( QgsDataItem* parent, QString connectionName, QString name, QString path );
    ~QgsPGSchemaItem();

    QVector<QgsDataItem*> createChildren() OVERRIDE;
    virtual QList<QAction*> actions() OVERRIDE;

  private:
    QgsPGLayerItem * createLayer( QgsPostgresLayerProperty layerProperty );

    QString mConnectionName;
};

class QgsPGLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsPGLayerItem( QgsDataItem* parent, QString name, QString path, QgsLayerItem::LayerType layerType, QgsPostgresLayerProperty layerProperties );
    ~QgsPGLayerItem();

    QString createUri();

    virtual QList<QAction*> actions() OVERRIDE;

  public slots:
    void deleteLayer();

  private:
    QgsPostgresLayerProperty mLayerProperty;
};

#endif // QGSPOSTGRESDATAITEMS_H
