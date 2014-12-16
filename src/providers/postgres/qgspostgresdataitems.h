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

    QVector<QgsDataItem*> createChildren();

    virtual QWidget * paramWidget();

    virtual QList<QAction*> actions();

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

    QVector<QgsDataItem*> createChildren();
    virtual bool equal( const QgsDataItem *other );
    virtual QList<QAction*> actions();

    virtual bool acceptDrop() { return true; }
    virtual bool handleDrop( const QMimeData * data, Qt::DropAction action );

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

    QVector<QgsDataItem*> createChildren();
    virtual QList<QAction*> actions();

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

    virtual QList<QAction*> actions();

  public slots:
    void deleteLayer();

  private:
    QgsPostgresLayerProperty mLayerProperty;
};

#endif // QGSPOSTGRESDATAITEMS_H
