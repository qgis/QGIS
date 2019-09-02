/***************************************************************************
    qgswmsdataitems.h
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
#ifndef QGSWMSDATAITEMS_H
#define QGSWMSDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdatasourceuri.h"
#include "qgswmsprovider.h"
#include "qgsgeonodeconnection.h"

class QgsWmsCapabilitiesDownload;

class QgsWMSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWMSConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri );
    ~QgsWMSConnectionItem() override;

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  public slots:
    void deleteLater() override;

  private:
    QString mUri;
    QgsWmsCapabilitiesDownload *mCapabilitiesDownload = nullptr;
};

// WMS Layers may be nested, so that they may be both QgsDataCollectionItem and QgsLayerItem
// We have to use QgsDataCollectionItem and support layer methods if necessary
class QgsWMSLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsWMSLayerItem( QgsDataItem *parent, QString name, QString path,
                     const QgsWmsCapabilitiesProperty &capabilitiesProperty,
                     const QgsDataSourceUri &dataSourceUri,
                     const QgsWmsLayerProperty &layerProperty );

    QString createUri();

    QgsWmsCapabilitiesProperty mCapabilitiesProperty;
    QgsDataSourceUri mDataSourceUri;
    QgsWmsLayerProperty mLayerProperty;
};

class QgsWMTSLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsWMTSLayerItem( QgsDataItem *parent,
                      const QString &name,
                      const QString &path,
                      const QgsDataSourceUri &dataSourceUri,
                      const QString &id,
                      const QString &format,
                      const QString &style,
                      const QString &tileMatrixSet,
                      const QString &crs,
                      const QString &title );

    QString createUri();
    QString layerName() const override { return mTitle; }

  private:
    QgsDataSourceUri mDataSourceUri;
    QString mId, mFormat, mStyle, mTileMatrixSet, mCrs, mTitle;
};

class QgsWMSRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWMSRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 7; }

  public slots:
};


//! Provider for WMS root data item
class QgsWmsDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override { return QStringLiteral( "WMS" ); }

    int capabilities() const override { return QgsDataProvider::Net; }

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;

    QVector<QgsDataItem *> createDataItems( const QString &path, QgsDataItem *parentItem ) override;
};


//! Root item for XYZ tile layers
class QgsXyzTileRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsXyzTileRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 8; }

};

//! Item implementation for XYZ tile layers
class QgsXyzLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsXyzLayerItem( QgsDataItem *parent, QString name, QString path, const QString &encodedUri );

};


//! Provider for XYZ root data item
class QgsXyzTileDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;

    QVector<QgsDataItem *> createDataItems( const QString &path, QgsDataItem *parentItem ) override;
};


#endif // QGSWMSDATAITEMS_H
