//
// Created by myarjunar on 25/04/17.
//

#ifndef QGSGEONODEDATAITEMS_H
#define QGSGEONODEDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgsdatasourceuri.h"
#include "qgsgeonodeconnection.h"

class QgsGeoNodeConnectionItem : public QgsDataCollectionItem
{
  Q_OBJECT
public:
  QgsGeoNodeConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri );
  QVector<QgsDataItem *> createChildren() override;
  virtual QList<QAction *> actions() override;

private:
  void editConnection();
  void deleteConnection()
  {
    QgsGeoNodeConnection::deleteConnection( mConnection.mConnName );
    mParent->refresh();
  };

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
  void replacePath( QgsDataItem *item, QString before, QString after );
  QString mName;
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

  virtual QList<QAction *> actions() override;

private slots:
  void newConnection();
};

//! Provider for Geonode root data item
class QgsGeoNodeDataItemProvider : public QgsDataItemProvider
{
public:
  virtual QString name() override { return QStringLiteral( "GeoNode" ); }

  virtual int capabilities() override { return QgsDataProvider::Net; }

  virtual QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

#endif //QGSGEONODEDATAITEMS_H
