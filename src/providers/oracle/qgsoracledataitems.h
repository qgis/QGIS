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
#include "qgsdataitemprovider.h"

class QSqlDatabase;

class QgsOracleRootItem;
class QgsOracleConnectionItem;
class QgsOracleOwnerItem;
class QgsOracleLayerItem;
class QgsProxyProgressTask;

class QgsOracleRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsOracleRootItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 5; }

    QWidget *paramWidget() override;

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
    QgsOracleConnectionItem( QgsDataItem *parent, const QString &name, const QString &path );
    ~QgsOracleConnectionItem() override;

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;
    QList<QAction *> actions( QWidget *parent ) override;

    bool acceptDrop() override { return true; }
    bool handleDrop( const QMimeData *data, Qt::DropAction action ) override;

    void refresh() override;

  signals:
    void addGeometryColumn( QgsOracleLayerProperty );

  public slots:
    void editConnection();
    void deleteConnection();
    void refreshConnection();

    void setLayerType( const QgsOracleLayerProperty &layerProperty );

    void threadStarted();
    void threadFinished();

  private:
    void stop();
    QMap<QString, QgsOracleOwnerItem * > mOwnerMap;
    QgsOracleColumnTypeThread *mColumnTypeThread = nullptr;
    QgsProxyProgressTask *mColumnTypeTask = nullptr;
    void setAllAsPopulated();
};

class QgsOracleOwnerItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOracleOwnerItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    void addLayer( const QgsOracleLayerProperty &layerProperty );

    // QgsDataItem interface
  public:
    bool layerCollection() const override;
};

Q_NOWARN_DEPRECATED_PUSH // deleteLayer deprecated
class QgsOracleLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsOracleLayerItem( QgsDataItem *parent, const QString &name, const QString &path, QgsLayerItem::LayerType layerType, const QgsOracleLayerProperty &layerProperties );

    QString createUri();

    QList<QAction *> actions( QWidget *parent ) override;

  public slots:
    bool deleteLayer() override;

  private:
    QgsOracleLayerProperty mLayerProperty;
};
Q_NOWARN_DEPRECATED_POP

//! Provider for ORACLE root data item
class QgsOracleDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    QString dataProviderKey() const override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;
};

#endif // QGSORACLEDATAITEMS_H
