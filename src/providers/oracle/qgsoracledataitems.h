/***************************************************************************
    qgsoracledataitems.h
    ---------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSORACLEDATAITEMS_H
#define QGSORACLEDATAITEMS_H

#include <QSqlDatabase>
#include <QMainWindow>

#include "qgsdataitem.h"

#include "qgsoracletablemodel.h"
#include "qgsoraclesourceselect.h"
#include "qgsmimedatautils.h"
#include "qgsvectorlayerexporter.h"

class QSqlDatabase;

class QgsOracleRootItem;
class QgsOracleConnectionItem;
class QgsOracleOwnerItem;
class QgsOracleLayerItem;

class QgsOracleRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOracleRootItem( QgsDataItem *parent, QString name, QString path );
    ~QgsOracleRootItem();

    QVector<QgsDataItem *> createChildren() override;

    virtual QWidget *paramWidget() override;

    QList<QAction *> actions( QWidget *parent ) override;

    static QMainWindow *sMainWindow;

  public slots:
    void connectionsChanged();
    void newConnection();
};

class QgsOracleConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOracleConnectionItem( QgsDataItem *parent, QString name, QString path );
    ~QgsOracleConnectionItem();

    QVector<QgsDataItem *> createChildren() override;
    virtual bool equal( const QgsDataItem *other ) override;
    QList<QAction *> actions( QWidget *parent ) override;

    virtual bool acceptDrop() override { return true; }
    virtual bool handleDrop( const QMimeData *data, Qt::DropAction action ) override;

    void refresh() override;

  signals:
    void addGeometryColumn( QgsOracleLayerProperty );

  public slots:
    void editConnection();
    void deleteConnection();
    void refreshConnection();

    void setLayerType( QgsOracleLayerProperty layerProperty );

    void threadStarted();
    void threadFinished();

  private:
    void stop();
    QMap<QString, QgsOracleOwnerItem * > mOwnerMap;
    QgsOracleColumnTypeThread *mColumnTypeThread = nullptr;
    void setAllAsPopulated();
};

class QgsOracleOwnerItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOracleOwnerItem( QgsDataItem *parent, QString name, QString path );
    ~QgsOracleOwnerItem();

    QVector<QgsDataItem *> createChildren();

    void addLayer( QgsOracleLayerProperty layerProperty );
};

class QgsOracleLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsOracleLayerItem( QgsDataItem *parent, QString name, QString path, QgsLayerItem::LayerType layerType, QgsOracleLayerProperty layerProperties );
    ~QgsOracleLayerItem();

    QString createUri();

    QList<QAction *> actions( QWidget *parent ) override;

  public slots:
    void deleteLayer();

  private:
    QgsOracleLayerProperty mLayerProperty;
};

#endif // QGSORACLEDATAITEMS_H
