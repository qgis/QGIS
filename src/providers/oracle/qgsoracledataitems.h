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

#include "qgsdataitem.h"

#include "qgsoracletablemodel.h"
#include "qgsoraclesourceselect.h"
#include "qgsmimedatautils.h"
#include "qgsvectorlayerimport.h"

class QSqlDatabase;

class QgsOracleRootItem;
class QgsOracleConnectionItem;
class QgsOracleOwnerItem;
class QgsOracleLayerItem;

class QgsOracleRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOracleRootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsOracleRootItem();

    QVector<QgsDataItem*> createChildren();

    virtual QWidget * paramWidget();

    virtual QList<QAction*> actions();

  public slots:
    void connectionsChanged();
    void newConnection();
};

class QgsOracleConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOracleConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsOracleConnectionItem();

    QVector<QgsDataItem*> createChildren();
    virtual bool equal( const QgsDataItem *other );
    virtual QList<QAction*> actions();

    virtual bool acceptDrop() { return true; }
    virtual bool handleDrop( const QMimeData * data, Qt::DropAction action );

    void refresh();

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
    QgsOracleColumnTypeThread *mColumnTypeThread;
};

class QgsOracleOwnerItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOracleOwnerItem( QgsDataItem* parent, QString name, QString path );
    ~QgsOracleOwnerItem();

    QVector<QgsDataItem*> createChildren();

    void addLayer( QgsOracleLayerProperty layerProperty );
};

class QgsOracleLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsOracleLayerItem( QgsDataItem* parent, QString name, QString path, QgsLayerItem::LayerType layerType, QgsOracleLayerProperty layerProperties );
    ~QgsOracleLayerItem();

    QString createUri();

    virtual QList<QAction*> actions();

  public slots:
    void deleteLayer();

  private:
    QgsOracleLayerProperty mLayerProperty;
};

#endif // QGSORACLEDATAITEMS_H
