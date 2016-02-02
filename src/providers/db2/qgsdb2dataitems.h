/***************************************************************************
  qgsdb2dataitems.h - Browser Panel object population
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
  Email     : dadler at adtechgeospatial.com
/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#include "qgsdb2provider.h"
#include "qgsdb2sourceselect.h"
#include <qgsdataitem.h>

class QgsDb2RootItem;
class QgsDb2Connection;
class QgsDb2SchemaItem;
class QgsDb2LayerItem;

class QgsDb2RootItem : public QgsDataCollectionItem
{
    Q_OBJECT

  public:
    QgsDb2RootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsDb2RootItem();

    QVector<QgsDataItem*> createChildren() override;

    virtual QWidget * paramWidget() override;

    virtual QList<QAction*> actions() override;

  public slots:
    //void connectionsChanged();
    void newConnection();
};

class QgsDb2ConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsDb2ConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsDb2ConnectionItem();

    QVector<QgsDataItem*> createChildren() override;
    //virtual bool equal( const QgsDataItem *other ) override;
    virtual QList<QAction*> actions() override;

    //virtual bool acceptDrop() override { return true; }
    //virtual bool handleDrop( const QMimeData * data, Qt::DropAction action ) override;
    void refresh() override;

    QString connInfo() const { return mConnInfo; }

  signals:
    void addGeometryColumn( QgsDb2LayerProperty );

  public slots:
    void refreshConnection();
    void editConnection();
    void deleteConnection();
    //void setAllowGeometrylessTables( bool allow );

    //void setLayerType( QgsDb2LayerProperty layerProperty );

  private:
    QString mConnInfo;
    QString mService;
    QString mHost;
    QString mDriver;
    QString mPort;
    QString mDatabase;
    QString mUsername;
    QString mPassword;
    int     mEnvironment;
    bool mUseGeometryColumns;
    bool mUseEstimatedMetadata;
    bool mAllowGeometrylessTables;

    void readConnectionSettings();
};


class QgsDb2SchemaItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsDb2SchemaItem( QgsDataItem* parent, QString name, QString path );
    ~QgsDb2SchemaItem();

    QVector<QgsDataItem*> createChildren() override;

    QgsDb2LayerItem* addLayer( QgsDb2LayerProperty layerProperty, bool refresh );
    void refresh() override {} // do not refresh directly
    void addLayers( QgsDataItem* newLayers );
};

class QgsDb2LayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsDb2LayerItem( QgsDataItem* parent, QString name, QString path, QgsLayerItem::LayerType layerType, QgsDb2LayerProperty layerProperties );
    ~QgsDb2LayerItem();

    QString createUri();

    QgsDb2LayerItem* createClone();

  private:
    QgsDb2LayerProperty mLayerProperty;
};

