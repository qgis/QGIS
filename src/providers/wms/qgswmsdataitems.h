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
    ~QgsWMSConnectionItem();

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
    virtual void deleteLater() override;

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
    ~QgsWMSLayerItem();

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
    ~QgsWMTSLayerItem();

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
    ~QgsWMSRootItem();

    QVector<QgsDataItem *> createChildren() override;

#ifdef HAVE_GUI
    virtual QList<QAction *> actions() override;
    virtual QWidget *paramWidget() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void newConnection();
#endif

};


//! Provider for WMS root data item
class QgsWmsDataItemProvider : public QgsDataItemProvider
{
  public:
    virtual QString name() override { return QStringLiteral( "WMS" ); }

    virtual int capabilities() override { return QgsDataProvider::Net; }

    virtual QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;

    virtual QVector<QgsDataItem *> createDataItems( const QString &path, QgsDataItem *parentItem ) override;
};


//! Root item for XYZ tile layers
class QgsXyzTileRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsXyzTileRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

#ifdef HAVE_GUI
    virtual QList<QAction *> actions() override;
#endif

  private slots:
#ifdef HAVE_GUI
    void newConnection();
#endif
};

//! Item implementation for XYZ tile layers
class QgsXyzLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsXyzLayerItem( QgsDataItem *parent, QString name, QString path, const QString &encodedUri );

#ifdef HAVE_GUI
    virtual QList<QAction *> actions() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void editConnection();
    void deleteConnection();
#endif
};


//! Provider for XYZ root data item
class QgsXyzTileDataItemProvider : public QgsDataItemProvider
{
  public:
    virtual QString name() override { return QStringLiteral( "XYZ Tiles" ); }

    virtual int capabilities() override { return QgsDataProvider::Net; }

    virtual QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override
    {
      if ( path.isEmpty() )
        return new QgsXyzTileRootItem( parentItem, QStringLiteral( "XYZ Tiles" ), QStringLiteral( "xyz:" ) );
      return nullptr;
    }

    virtual QVector<QgsDataItem *> createDataItems( const QString &path, QgsDataItem *parentItem ) override;
};


#endif // QGSWMSDATAITEMS_H
