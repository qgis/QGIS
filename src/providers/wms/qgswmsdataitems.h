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

#include "qgsdatacollectionitem.h"
#include "qgslayeritem.h"
#include "qgsdataitemprovider.h"
#include "qgsdatasourceuri.h"
#include "qgswmsprovider.h"
#include "qgsgeonodeconnection.h"
#include "qgsconnectionsitem.h"

class QgsWmsCapabilitiesDownload;

class QgsWMSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWMSConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri );
    ~QgsWMSConnectionItem() override;

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;
    void refresh() override;

  public slots:
    void deleteLater() override;

  private:
    QString mUri;
    QgsWmsCapabilitiesDownload *mCapabilitiesDownload = nullptr;
};

/**
 * Base class which contains similar basic attributes and functions needed by the
 * wms collection layers and child layers.
 *
 */
class QgsWMSItemBase
{
  public:
    QgsWMSItemBase( const QgsWmsCapabilitiesProperty &capabilitiesProperty,
                    const QgsDataSourceUri &dataSourceUri,
                    const QgsWmsLayerProperty &layerProperty );

    /**
     * Returns the uri for the wms dataitem.
     *
     * The WMS temporal layers can contain the following parameters uri.
     *
     * - "type": the type of the wms provider e.g WMS-T
     * - "timeDimensionExtent": the layer's time dimension extent it is available
     * - "referencetimeDimensionExtent": reference time extent for the bi-temporal dimension layers
     * - "time": time value of the current layer data from the provider
     * - "referenceTime": reference time value of the current of the layer data, this is applicable for the
     *   bi-temporal dimension layers
     * - "allowTemporalUpdates": whether to allow updates on temporal parameters on this uri
     * - "temporalSource": the source of the layer's temporal range, can be either "provider" or "project"
     * - "enableTime": if the provider using time part in the temporal range datetime instances
     *
     * \param withStyle default TRUE, also adds the style to the URL, it should be empty for collection items
     */
    QString createUri( bool withStyle = true );

    //! Stores GetCapabilities response
    QgsWmsCapabilitiesProperty mCapabilitiesProperty;

    //! Stores WMS connection information
    QgsDataSourceUri mDataSourceUri;

    //! WMS Layer properties, can be inherited by subsidiary layers
    QgsWmsLayerProperty mLayerProperty;
};

/**
 * \brief WMS Layer Collection.
 *
 *  This collection contains a WMS Layer element that can enclose other layers.
 */
class QgsWMSLayerCollectionItem : public QgsDataCollectionItem, public QgsWMSItemBase
{
    Q_OBJECT
  public:
    QgsWMSLayerCollectionItem( QgsDataItem *parent, QString name, QString path,
                               const QgsWmsCapabilitiesProperty &capabilitiesProperty,
                               const QgsDataSourceUri &dataSourceUri,
                               const QgsWmsLayerProperty &layerProperty );

    bool equal( const QgsDataItem *other ) override;

    bool hasDragEnabled() const override;

    QgsMimeDataUtils::UriList mimeUris() const override;

  protected:
    //! The URI
    QString mUri;

    // QgsDataItem interface
  public:
    bool layerCollection() const override;
};

// WMS Layers may be nested, so that they may be both QgsDataCollectionItem and QgsLayerItem
// We have to use QgsDataCollectionItem and support layer methods if necessary
class QgsWMSLayerItem : public QgsLayerItem, public QgsWMSItemBase
{
    Q_OBJECT
  public:
    QgsWMSLayerItem( QgsDataItem *parent, QString name, QString path,
                     const QgsWmsCapabilitiesProperty &capabilitiesProperty,
                     const QgsDataSourceUri &dataSourceUri,
                     const QgsWmsLayerProperty &layerProperty );

    bool equal( const QgsDataItem *other ) override;

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
                      const QString &dimension,
                      const QString &dimensionValue,
                      const QString &format,
                      const QString &style,
                      const QString &tileMatrixSet,
                      const QString &crs,
                      const QString &title );

    QString createUri();
    QString layerName() const override { return mTitle; }

  private:
    QgsDataSourceUri mDataSourceUri;
    QString mId;
    QString mDimension;
    QString mDimensionValue;
    QString mFormat;
    QString mStyle;
    QString mTileMatrixSet;
    QString mCrs;
    QString mTitle;
};

class QgsWMSRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsWMSRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 7; }

  public slots:
};

class QgsWMTSRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsWMTSRootItem( QgsDataItem *parent, QString name, QString path );

  public slots:
};


//! Provider for WMS root data item
class QgsWmsDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override { return QStringLiteral( "WMS" ); }
    QString dataProviderKey() const override;
    int capabilities() const override { return QgsDataProvider::Net; }

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;

    QVector<QgsDataItem *> createDataItems( const QString &path, QgsDataItem *parentItem ) override;
};


//! Root item for XYZ tile layers
class QgsXyzTileRootItem : public QgsConnectionsRootItem
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
    QString dataProviderKey() const override;
    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;

    QVector<QgsDataItem *> createDataItems( const QString &path, QgsDataItem *parentItem ) override;
};


#endif // QGSWMSDATAITEMS_H
