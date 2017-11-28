/***************************************************************************
    qgsspatialitedataitems.h
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
#ifndef QGSSPATIALITEDATAITEMS_H
#define QGSSPATIALITEDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsspatialiteconnection.h"

class SpatialiteDbInfo;

/**
 * Item that represents a layer that can be opened with the Spatialite Provider
 * Used in QgsBrowserWatcher/Model logic to display as a member in the list of Layers (Geometry or Raster) contained in the Sqlite-Container
 * \note
 *  called from dataItem when given path is not empty and only 1 Layer or from a QgsSpatialiteCollectionItem where > 1 Layer
 *  represent a SpatialiteDbLayer inside a SpatialiteDbInfo
 * QgsSpatialiteCollectionItem::createChildren calls this for each Layer found
 * For the given Layer-Name, getDbLayerInfo will retrieve the needed metadata to set all values needed to display the Layer-Type
 * and through getDbLayerUris to retrieve the needed Uri of the Layer
 * mProviderKey will be set from the value retrieved from getDbLayerInfo [default: 'spatialite', but also 'rasterlite2', 'gdal' and 'ogr' are possible]
 * \see SpatialiteDbInfo::getDbLayerInfo
 * \see SpatialiteDbInfo::getDbLayerUris
 * \since QGIS 3.0
 */
class QgsSpatialiteLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsSpatialiteLayerItem( QgsDataItem *parent, QString filePath, QString sLayerName, SpatialiteDbInfo *spatialiteDbInfo = nullptr );

#ifdef HAVE_GUI

    /**
     * Returns the list of actions available for this item. This is usually used for the popup menu on right-clicking
     * the item. Subclasses should override this to provide actions.
     *
     * Subclasses should ensure that ownership of created actions is correctly handled by parenting them
     * to the specified parent widget.
     */
    QList<QAction *> actions( QWidget *parent ) override;
#endif

  public slots:
#ifdef HAVE_GUI
    void deleteLayer();
#endif
  private:

    /**
     * SpatialiteDbInfo Object
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;

    /**
     * Layer-Info
     * \note
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see SpatialiteDbInfo::parseLayerInfo
     * \since QGIS 3.0
     */
    QString mLayerInfo;

    /**
     * The Spatialite Geometry-Type of the Layer
     * - representing mGeometryType
     * \note
     *  - QgsWkbTypes::displayString(mGeometryType)
     *  \see setGeometryType
     * \since QGIS 3.0
     */
    QgsWkbTypes::Type mGeometryType = QgsWkbTypes::Unknown;

    /**
     * Srid, as retrieved from Layer-Info
     * \note
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see SpatialiteDbInfo::parseLayerInfo
     * \since QGIS 3.0
     */
    int mLayerSrid = -1;

    /**
     * Provider, as retrieved from Layer-Info
     * \note
     * - Value: GeometryType and Srid formatted as 'geometry_type:srid:provider:layertype'
     * \see SpatialiteDbInfo::parseLayerInfo
     * \since QGIS 3.0
     */
    QString mProvider = "spatialite";

    /**
     * The Spatialite Layer-Type of the Layer
     * \note
     * - SpatialTable, SpatialView, VirtualShape, RasterLite1, RasterLite2
     * - SpatialiteTopology, TopologyExport, GeoPackageVector, GeoPackageRaster
     * - MBTilesTable, MBTilesView
     *  \see SpatialiteDbInfo::SpatialiteLayerTypeFromName
     *  \see mLayerTypeString
     * \since QGIS 3.0
     */
    SpatialiteDbInfo::SpatialiteLayerType mLayerTypeSpatialite = SpatialiteDbInfo::SpatialiteUnknown;

    /**
     * Returns QIcon representation of the enum of Layer-Types of the Sqlite3 Container
     * - SpatialiteDbInfo::SpatialiteLayerType
     * \note
     * - SpatialTable, SpatialView, VirtualShape, RasterLite1, RasterLite2
     * - SpatialiteTopology, TopologyExport, GeoPackageVector, GeoPackageRaster
     * - MBTilesTable, MBTilesView
     * \see SpatialiteDbInfo::SpatialiteLayerTypeIcon::SpatialiteLayerTypeNameIcon
     * \since QGIS 3.0
     */
    QIcon mLayerTypeIcon;

    /**
     * Returns QIcon representation of the enum of a Geometry-Type
     * - QgsWkbTypes::Type
     * \note
     * - QgsWkbTypes::Point, Multi and 25D
     * - QgsWkbTypes::LineString, Multi and 25D
     * - QgsWkbTypes::Polygon, Multi and 25D
     * \see SpatialiteDbInfo::SpatialGeometryTypeIcon
     * \since QGIS 3.0
     */
    QIcon mGeometryTypeIcon;
};

/**
 * A Collection: logical collection of layers or subcollections
 * used by QgsBrowserWatcher/Model logic
 * \note
 *  called from dataItem when given path is not empty and the amout of Layers > 1
 * - collection all Layers in a Spatialite-Database, as QgsSpatialiteLayerItem
 *  represent a SpatialiteDbInfo that contains SpatialiteDbLayer objects
 * \see QgsSpatialiteLayerItem
 * \see SpatialiteDbLayer
 * \see SpatialiteDbInfo
 * \since QGIS 3.0
 */
class QgsSpatialiteCollectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsSpatialiteCollectionItem( QgsDataItem *parent, const QString name, const QString filePath, SpatialiteDbInfo *spatialiteDbInfo = nullptr );

    /**
     * Create children. Children are not expected to have parent set.
     * used by QgsBrowserWatcher/Model logic
     * \note
     *  A list of unique Layer-Names will be retrieved from SpatialiteDbInfo
     * - for each Layer-Name QgsSpatialiteLayerItem will be called
     *  This method MUST BE THREAD SAFE.
     * \see QgsSpatialiteLayerItem
     * \see SpatialiteDbLayer
     * \see SpatialiteDbInfo:.getDbLayersType
     * \since QGIS 3.0
     */
    QVector<QgsDataItem *> createChildren() override;

    /**
     * Returns true if this item is equal to another item (by testing item type and path).
     */
    virtual bool equal( const QgsDataItem *other ) override;

#ifdef HAVE_GUI

    /**
     * Returns the list of actions available for this item. This is usually used for the popup menu on right-clicking
     * the item. Subclasses should override this to provide actions.
     *
     * Subclasses should ensure that ownership of created actions is correctly handled by parenting them
     * to the specified parent widget.
     */
    QList<QAction *> actions( QWidget *parent ) override;
#endif

  protected:

    /**
     * SpatialiteDbInfo Object
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;
};

/**
 * A Collection: QgsSpatiaLiteConnection::connectionList
 * Used in QgsBrowserWatcher/Model logic to display the list of connections stored in the settings
 * \note
 *  called from dataItem, through QgsSpatialiteRootItem, with an empty path
 * QgsSpatialiteRootItem::createChildren calls this for each connection entry found
 * \see QgsSpatialiteRootItem
 * \since QGIS 3.0
 */
class QgsSpatialiteConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsSpatialiteConnectionItem( QgsDataItem *parent, const QString name, const QString filePath );

    /**
     * Returns true if this item is equal to another item (by testing item type and path).
     */
    virtual bool equal( const QgsDataItem *other ) override;

#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;
#endif

    virtual bool acceptDrop() override { return true; }

    /**
     * Attempts to process the mime data dropped on this item. Subclasses must override this and acceptDrop() if they
     * accept dropped layers.
     * \see acceptDrop()
     */
    virtual bool handleDrop( const QMimeData *data, Qt::DropAction action ) override;

  public slots:
#ifdef HAVE_GUI
    void editConnection();
    void deleteConnection();
#endif

  protected:
    QString mDbPath;

    /**
     * SpatialiteDbInfo Object
     * - containing all Information about Database file
     * \note
     * - isDbValid() return if the connection contains layers that are supported by
     * -- QgsSpatiaLiteProvider, QgsGdalProvider and QgsOgrProvider
     * \see SpatialiteDbInfo::isDbValid()
     * \since QGIS 3.0
     */
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;
};

/**
 * A Collection: logical collection of layers or subcollections
 * Used in QgsBrowserWatcher/Model logic to display (as Parent) to display the list of connections stored in the settings
 * \note
 *  called from dataItem when given path is not empty
 * - will show the Database name, with the Layers shown as children
 * \see QgsSpatialiteConnectionItem
 * \since QGIS 3.0
 */
class QgsSpatialiteRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsSpatialiteRootItem( QgsDataItem *parent, const QString &name, const QString &filePath );

    /**
     * Create children. Children are not expected to have parent set.
     * used by QgsBrowserWatcher/Model logic
     * \note
     *  A list of unique connections will be retrieved from QgsSpatiaLiteConnection
     * - for each connection QgsSpatialiteConnectionItem will be called
     *  This method MUST BE THREAD SAFE.
     * \see QgsSpatialiteConnectionItem
     * \see QgsSpatiaLiteConnection::connectionList
     * \since QGIS 3.0
     */
    QVector<QgsDataItem *> createChildren() override;

#ifdef HAVE_GUI
    virtual QWidget *paramWidget() override;

    /**
     * Returns the list of actions available for this item. This is usually used for the popup menu on right-clicking
     * the item. Subclasses should override this to provide actions.
     *
     * Subclasses should ensure that ownership of created actions is correctly handled by parenting them
     * to the specified parent widget.
     */
    QList<QAction *> actions( QWidget *parent ) override;
#endif

  public slots:
#ifdef HAVE_GUI
    void onConnectionsChanged();
    void newConnection();
#endif
    void createDatabase();
};


#endif // QGSSPATIALITEDATAITEMS_H
