/***************************************************************************
  qgsdb2dataitems.h - Browser Panel object population
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
****************************************************************************
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

/**
 * @class QgsDb2RootItem
 * @brief Browser Panel DB2 root object.
 */
class QgsDb2RootItem : public QgsDataCollectionItem
{
    Q_OBJECT

  public:
    QgsDb2RootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsDb2RootItem();

    /**
     * Add saved connections as children.
     */
    QVector<QgsDataItem*> createChildren() override;

    virtual QWidget * paramWidget() override;

    virtual QList<QAction*> actions() override;

  public slots:
    //void connectionsChanged();
    void newConnection();
};

/**
 * @class QgsDb2ConnectionItem
 * @brief Browser Panel DB2 connection object (under root).
 */
class QgsDb2ConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsDb2ConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsDb2ConnectionItem();

    static bool ConnInfoFromSettings( const QString connName,
                                      QString &connInfo, QString &errorMsg );

    static bool ConnInfoFromParameters(
      const QString &service,
      const QString &driver,
      const QString &host,
      const QString &port,
      const QString &database,
      const QString &username,
      const QString &password,
      const QString &authcfg,
      QString &connInfo,
      QString &errorMsg );
    /**
     * Fetch geometry column data from server and populate Browser Panel with
     * schemas and layers.
     */
    QVector<QgsDataItem*> createChildren() override;
    virtual bool equal( const QgsDataItem *other ) override;

    /**
     * Add Refresh, Edit, and Delete actions for every QgsDb2ConnectionItem.
     */
    virtual QList<QAction*> actions() override;

    virtual bool acceptDrop() override { return true; }
    virtual bool handleDrop( const QMimeData * data, Qt::DropAction action ) override;
    bool handleDrop( const QMimeData * data, const QString& toSchema );
    void refresh() override;

    QString connInfo() const { return mConnInfo; }

  signals:
    void addGeometryColumn( QgsDb2LayerProperty );

  public slots:
    /**
     * Refresh with saved connection data.
     */
    void refreshConnection();

    /**
     * Show dialog to edit and save connection data.
     */
    void editConnection();

    /**
     * Delete saved connection data and remove from Browser Panel.
     */
    void deleteConnection();
    //void setAllowGeometrylessTables( bool allow );

    //void setLayerType( QgsDb2LayerProperty layerProperty );

  private:
    QString mConnInfo;

    void readConnectionSettings();
};

/**
 * @class QgsDb2SchemaItem
 * @brief Browser Panel DB2 schema object (under connections).
 */
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
    virtual bool acceptDrop() override { return true; }
    virtual bool handleDrop( const QMimeData * data, Qt::DropAction action ) override;
};

/**
 * @class QgsDb2LayerItem
 * @brief Browser Panel DB2 layer object (under schemas).
 */
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

