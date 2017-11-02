/***************************************************************************
  qgsogrdbitems.h - QgsOgrDbItems are data items for file-based OGR/GDAL
                    DBs like GeoPackage and SpatiaLite

 ---------------------
 begin                : 18.9.2017
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
#ifndef QGSOGRDBITEMS_H
#define QGSOGRDBITEMS_H


class QgsOgrDbItems
{
  public:
    QgsOgrDbItems();
};



#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"


/**
 * \brief The QgsOgrDbAbstractLayerItem class is the base class for OgrDb raster and vector layers
 */
class QgsOgrDbAbstractLayerItem : public QgsLayerItem
{
    Q_OBJECT

  protected:
    QgsOgrDbAbstractLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, LayerType layerType, const QString &providerKey );

    /**
     * Subclasses need to implement this function with
     * the real deletion implementation
     */
    virtual bool executeDeleteLayer( QString &errCause );
#ifdef HAVE_GUI
    QList<QAction *> actions();
  public slots:
    virtual void deleteLayer();
#endif
};


class QgsOgrDbRasterLayerItem : public QgsOgrDbAbstractLayerItem
{
    Q_OBJECT

  public:
    QgsOgrDbRasterLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri );
  protected:
    virtual bool executeDeleteLayer( QString &errCause ) override;
};


class QgsOgrDbVectorLayerItem : public QgsOgrDbAbstractLayerItem
{
    Q_OBJECT

  public:
    QgsOgrDbVectorLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, LayerType layerType );
  protected:
    virtual bool executeDeleteLayer( QString &errCause ) override;
};


/**
 * \brief The QgsOgrDbCollectionItem class is the base class for
 *        OgrDb container
 */
class QgsOgrDbCollectionItem : public QgsDataCollectionItem
{
    Q_OBJECT

  public:
    QgsOgrDbCollectionItem( QgsDataItem *parent, const QString &name, const QString &path );
    QVector<QgsDataItem *> createChildren() override;
    virtual bool equal( const QgsDataItem *other ) override;

#ifdef HAVE_GUI
    virtual bool acceptDrop() override { return true; }
    virtual bool handleDrop( const QMimeData *data, Qt::DropAction action ) override;
    QList<QAction *> actions() override;
#endif

    //! Return the layer type from \a geometryType
    static QgsLayerItem::LayerType layerTypeFromDb( const QString &geometryType );

    //! Delete a OgrDb layer
    static bool deleteOgrDbRasterLayer( const QString &uri, QString &errCause );

  public slots:
#ifdef HAVE_GUI
    void addTable();
    void addConnection();
#endif

  protected:
    QString mPath;
};


/**
 * \brief The QgsOgrDbConnectionItem class adds the stored
 *        connection management to QgsOgrDbCollectionItem
 */
class QgsOgrDbConnectionItem : public QgsOgrDbCollectionItem
{
    Q_OBJECT

  public:
    QgsOgrDbConnectionItem( QgsDataItem *parent, const QString &name, const QString &path );
    virtual bool equal( const QgsDataItem *other ) override;

#ifdef HAVE_GUI
    QList<QAction *> actions() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void editConnection();
    void deleteConnection();
#endif

};


#endif // QGSOGRDBITEMS_H
