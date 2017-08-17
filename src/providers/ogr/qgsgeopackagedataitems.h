/***************************************************************************
    qgsgeopackagedataitems.h
    ---------------------
    begin                : August 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGEOPACKAGEDATAITEMS_H
#define QGSGEOPACKAGEDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"

/**
 * \brief The QgsGeoPackageAbstractLayerItem class is the base class for GeoPackage raster and vector layers
 */
class QgsGeoPackageAbstractLayerItem : public QgsLayerItem
{
    Q_OBJECT

  protected:
    QgsGeoPackageAbstractLayerItem( QgsDataItem *parent, QString name, QString path, QString uri, LayerType layerType, QString providerKey );

    /** Subclasses need to implement this function with
     * the real deletion implementation
     */
    virtual bool executeDeleteLayer( QString &errCause );
#ifdef HAVE_GUI
    QList<QAction *> actions();
  public slots:
    virtual void deleteLayer();
#endif
};


class QgsGeoPackageRasterLayerItem : public QgsGeoPackageAbstractLayerItem
{
    Q_OBJECT

  public:
    QgsGeoPackageRasterLayerItem( QgsDataItem *parent, QString name, QString path, QString uri );
  protected:
    virtual bool executeDeleteLayer( QString &errCause ) override;
};


class QgsGeoPackageVectorLayerItem : public QgsGeoPackageAbstractLayerItem
{
    Q_OBJECT

  public:
    QgsGeoPackageVectorLayerItem( QgsDataItem *parent, QString name, QString path, QString uri, LayerType layerType );
  protected:
    virtual bool executeDeleteLayer( QString &errCause ) override;
};


class QgsGeoPackageConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT

  public:
    QgsGeoPackageConnectionItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;
    virtual bool equal( const QgsDataItem *other ) override;

#ifdef HAVE_GUI
    virtual bool acceptDrop() override { return true; }
    virtual bool handleDrop( const QMimeData *data, Qt::DropAction action ) override;
    QList<QAction *> actions() override;
#endif

    //! Return the layer type from \a geometryType
    static QgsLayerItem::LayerType layerTypeFromDb( const QString &geometryType );

  public slots:
#ifdef HAVE_GUI
    void editConnection();
    void deleteConnection();
    void addTable();
#endif

  protected:
    QString mPath;
};


class QgsGeoPackageRootItem : public QgsDataCollectionItem
{
    Q_OBJECT

  public:
    QgsGeoPackageRootItem( QgsDataItem *parent, QString name, QString path );
    ~QgsGeoPackageRootItem();

    QVector<QgsDataItem *> createChildren() override;

#ifdef HAVE_GUI
    virtual QWidget *paramWidget() override;
    QList<QAction *> actions() override;

  public slots:
    void newConnection();
    void connectionsChanged();
    void createDatabase();
#endif

  private:
    bool storeConnection( const QString &path );

};


//! Provider for geopackage data item
class QgsGeoPackageDataItemProvider : public QgsDataItemProvider
{
  public:
    virtual QString name() override { return QStringLiteral( "GPKG" ); }
    virtual int capabilities() override { return QgsDataProvider::Database; }
    virtual QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};



#endif // QGSGEOPACKAGEDATAITEMS_H
