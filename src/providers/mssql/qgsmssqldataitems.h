/***************************************************************************
                         qgsmssqldataitems.h  -  description
                         -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#ifndef QGSMSSQLDATAITEMS_H
#define QGSMSSQLDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsmssqltablemodel.h"

class QgsMssqlGeomColumnTypeThread;

class QgsMssqlRootItem;
class QgsMssqlConnectionItem;
class QgsMssqlSchemaItem;
class QgsMssqlLayerItem;

class QgsMssqlRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsMssqlRootItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 4; }

#ifdef HAVE_GUI
    QWidget *paramWidget() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void onConnectionsChanged();
#endif
};

class QgsMssqlConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsMssqlConnectionItem( QgsDataItem *parent, const QString &name, const QString &path );
    ~QgsMssqlConnectionItem() override;

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

    bool handleDrop( const QMimeData *data, const QString &toSchema );

    QString connInfo() const { return mConnInfo; }
    bool allowGeometrylessTables() const { return mAllowGeometrylessTables; }

  signals:
    void addGeometryColumn( const QgsMssqlLayerProperty & );

  public slots:

    void setAllowGeometrylessTables( const bool allow );

    void setLayerType( QgsMssqlLayerProperty layerProperty );

    void refresh() override;

  private slots:
    void setAsPopulated();

  private:
    QString mConnInfo;
    QString mService;
    QString mHost;
    QString mDatabase;
    QString mUsername;
    QString mPassword;
    bool mUseGeometryColumns;
    bool mUseEstimatedMetadata;
    bool mAllowGeometrylessTables;
    QgsMssqlGeomColumnTypeThread *mColumnTypeThread = nullptr;

    void readConnectionSettings();
    void stop();
};

class QgsMssqlSchemaItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsMssqlSchemaItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QgsMssqlLayerItem *addLayer( const QgsMssqlLayerProperty &layerProperty, bool refresh );
    void refresh() override {} // do not refresh directly
    void addLayers( QgsDataItem *newLayers );
};

class QgsMssqlLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsMssqlLayerItem( QgsDataItem *parent, const QString &name, const QString &path, QgsLayerItem::LayerType layerType, const QgsMssqlLayerProperty &layerProperties );

    QString createUri();

    QgsMssqlLayerItem *createClone();

    bool disableInvalidGeometryHandling() const;

    const QgsMssqlLayerProperty &layerInfo() const { return mLayerProperty; }

  private:
    QgsMssqlLayerProperty mLayerProperty;
    bool mDisableInvalidGeometryHandling = false;
};

//! Provider for GDAL root data item
class QgsMssqlDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;
};

#endif // QGSMSSQLDATAITEMS_H
