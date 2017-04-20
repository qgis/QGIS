/***************************************************************************
    qgsowsdataitems.h
    ---------------------
    begin                : May 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSOWSDATAITEMS_H
#define QGSOWSDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgsdatasourceuri.h"
#include "qgsgeonodeconnection.h"

class QgsOWSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOWSConnectionItem( QgsDataItem *parent, QString name, QString path );
    ~QgsOWSConnectionItem();

    QVector<QgsDataItem *> createChildren() override;
    virtual bool equal( const QgsDataItem *other ) override;

#ifdef HAVE_GUI
    virtual QList<QAction *> actions() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void editConnection();
    void deleteConnection();
#endif

  private:
    void replacePath( QgsDataItem *item, QString before, QString after );
};

class QgsOWSRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOWSRootItem( QgsDataItem *parent, QString name, QString path );
    ~QgsOWSRootItem();

    QVector<QgsDataItem *> createChildren() override;

#ifdef HAVE_GUI
    virtual QList<QAction *> actions() override;
    virtual QWidget *paramWidget() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void connectionsChanged();

    void newConnection();
#endif
};

//! Provider for ows root data item
class QgsOwsDataItemProvider : public QgsDataItemProvider
{
  public:
    virtual QString name() override { return QStringLiteral( "OWS" ); }

    virtual int capabilities() override { return QgsDataProvider::Net; }

    virtual QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

class QgsGeoNodeConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsGeoNodeConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri );
    QVector<QgsDataItem *> createChildren() override;
  private:
    QString mUri;
    QgsGeoNodeConnection mConnection;
};

class QgsGeoNodeServiceItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsGeoNodeServiceItem( QgsDataItem *parent, QString connName, QString serviceName, QString path, QString uri );
    QVector<QgsDataItem *> createChildren() override;

  private:
    QString mServiceName;
    QString mUri;
    QgsGeoNodeConnection mConnection;
};

class QgsGeoNodeLayerItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsGeoNodeLayerItem( QgsDataItem *parent, QString connName, QString layerName, QString serviceName );
  private:
    QgsGeoNodeConnection mConnection;
};

class QgsGeoNodeRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsGeoNodeRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

    /*virtual QList<QAction *> actions() override;

  private slots:
    void newConnection();*/
};

//! Provider for Geonode root data item
class QgsGeoNodeDataItemProvider : public QgsDataItemProvider
{
  public:
    virtual QString name() override { return QStringLiteral( "GeoNode" ); }

    virtual int capabilities() override { return QgsDataProvider::Net; }

    virtual QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

#endif // QGSOWSDATAITEMS_H
