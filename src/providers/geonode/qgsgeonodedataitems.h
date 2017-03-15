//
// Created by myarjunar on 15/03/17.
//

#ifndef QGSGEONODEDATAITEMS_H
#define QGSGEONODEDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdatasourceuri.h"
#include "qgswfsdataitems.h"
#include "qgswmsdataitems.h"
#include "qgswcsdataitems.h"

class QgsGeoNodeCapabilitiesDownload;
class QgsGeoNodeCapabilitiesUpload;

class QgsGeoNodeConnectionItem : public QgsDataCollectionItem
{
  Q_OBJECT
public:
  QgsGeoNodeConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri );
  ~QgsGeoNodeConnectionItem();

  QVector<QgsDataItem *> createChildren() override;
  virtual bool equal( const QgsDataItem *other ) override;

  virtual QList<QAction *> actions() override;

public slots:
  void editConnection();
  void deleteConnection();
  virtual void deleteLater() override;

private:
  QString mUri;
  QgsGeoNodeCapabilitiesDownload *mCapabilitiesDownload = nullptr;
  QgsGeoNodeCapabilitiesUpload *mCapabilitiesUpload = nullptr;
};

class QgsGeoNodeWfsLayerItem : public QgsWfsLayerItem
{
  Q_OBJECT

public:
  QgsGeoNodeWfsLayerItem( QgsDataItem *parent, QString name, const QgsDataSourceUri &uri, QString featureType, QString title, QString crsString );
  ~QgsGeoNodeWfsLayerItem();

};

class QgsGeoNodeWmsLayerItem : public QgsWMSLayerItem
{
  Q_OBJECT
public:
  QgsGeoNodeWmsLayerItem( QgsDataItem *parent, QString name, QString path,
                   const QgsWmsCapabilitiesProperty &capabilitiesProperty,
                   const QgsDataSourceUri &dataSourceUri,
                   const QgsWmsLayerProperty &layerProperty );
  ~QgsGeoNodeWmsLayerItem();

  QString createUri();

  QgsWmsCapabilitiesProperty mCapabilitiesProperty;
  QgsDataSourceUri mDataSourceUri;
  QgsWmsLayerProperty mLayerProperty;
};

class QgsGeoNodeWcsLayerItem : public QgsWCSLayerItem
{
  Q_OBJECT
public:
  QgsGeoNodeWcsLayerItem( QgsDataItem *parent, QString name, QString path,
                   const QgsWcsCapabilitiesProperty &capabilitiesProperty,
                   const QgsDataSourceUri &dataSourceUri, const QgsWcsCoverageSummary &coverageSummary );
  ~QgsGeoNodeWcsLayerItem();

  QString createUri();

  QgsWcsCapabilitiesProperty mCapabilities;
  QgsDataSourceUri mDataSourceUri;
  QgsWcsCoverageSummary mCoverageSummary;
};

class QgsGeoNodeRootItem : public QgsDataCollectionItem
{
  Q_OBJECT
public:
  QgsGeoNodeRootItem( QgsDataItem *parent, QString name, QString path );
  ~QgsGeoNodeRootItem();

  QVector<QgsDataItem *> createChildren() override;

  virtual QList<QAction *> actions() override;

  virtual QWidget *paramWidget() override;

public slots:
  void connectionsChanged();

  void newConnection();
};


#endif //QGSGEONODEDATAITEMS_H