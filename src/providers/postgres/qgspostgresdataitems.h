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
#include "qgsmimedatautils.h"
#include "qgsvectorlayerexporter.h"

class QgsPGRootItem;
class QgsPGConnectionItem;
class QgsPGSchemaItem;
class QgsPGLayerItem;

class QgsPGRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGRootItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

#ifdef HAVE_GUI
    virtual QWidget *paramWidget() override;

    QList<QAction *> actions( QWidget *parent ) override;
#endif

    static QMainWindow *sMainWindow;

  public slots:
#ifdef HAVE_GUI
    void onConnectionsChanged();
    void newConnection();
#endif
};

class QgsPGConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGConnectionItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;
    virtual bool equal( const QgsDataItem *other ) override;
#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;
#endif

    virtual bool acceptDrop() override { return true; }
    virtual bool handleDrop( const QMimeData *data, Qt::DropAction action ) override;

    bool handleDrop( const QMimeData *data, const QString &toSchema );

  signals:
    void addGeometryColumn( const QgsPostgresLayerProperty & );

  public slots:
#ifdef HAVE_GUI
    void editConnection();
    void deleteConnection();
    void refreshConnection();
    void createSchema();
#endif

    // refresh specified schema or all schemas if schema name is empty
    void refreshSchema( const QString &schema );

};

class QgsPGSchemaItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGSchemaItem( QgsDataItem *parent, const QString &connectionName, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;
#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;
#endif

    virtual bool acceptDrop() override { return true; }
    virtual bool handleDrop( const QMimeData *data, Qt::DropAction action ) override;

  public slots:
#ifdef HAVE_GUI
    void deleteSchema();
    void renameSchema();
#endif

  private:
    QgsPGLayerItem *createLayer( QgsPostgresLayerProperty layerProperty );

    QString mConnectionName;
};

class QgsPGLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsPGLayerItem( QgsDataItem *parent, const QString &name, const QString &path, QgsLayerItem::LayerType layerType, const QgsPostgresLayerProperty &layerProperties );

    QString createUri();

#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;
#endif
    virtual QString comments() const override;

  public slots:
#ifdef HAVE_GUI
    void deleteLayer();
    void renameLayer();
    void truncateTable();
#endif

  private:
    QgsPostgresLayerProperty mLayerProperty;
};

#endif // QGSPOSTGRESDATAITEMS_H
