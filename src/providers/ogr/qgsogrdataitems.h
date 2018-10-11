/***************************************************************************
                             qgsogrdataitems.h
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRDATAITEMS_H
#define QGSOGRDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsogrprovider.h"
#include "qgsdataitemprovider.h"


/**
 * Holds the information about a gpkg layer
 */
class QgsOgrDbLayerInfo
{
  public:
    QgsOgrDbLayerInfo( const QString &path, const QString &uri, const QString &name, const QString &theGeometryColumn, const QString &theGeometryType, const QgsLayerItem::LayerType &theLayerType )
      : mPath( path )
      , mUri( uri )
      , mName( name )
      , mGeometryColumn( theGeometryColumn )
      , mGeometryType( theGeometryType )
      , mLayerType( theLayerType )
    {
    }
    const QString path() const { return mPath; }
    const QString uri() const { return mUri; }
    const QString name() const { return mName; }
    const QString geometryColumn() const { return mGeometryColumn; }
    const QString geometryType() const { return mGeometryType; }
    QgsLayerItem::LayerType layerType() const { return mLayerType; }

  private:
    QString mPath;
    QString mUri;
    QString mName;
    QString mGeometryColumn;
    QString mGeometryType;
    QgsLayerItem::LayerType mLayerType = QgsLayerItem::LayerType::NoType;
};


class QgsOgrLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsOgrLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, LayerType layerType, bool isSubLayer = false );

    bool setCrs( const QgsCoordinateReferenceSystem &crs ) override;

    QString layerName() const override;
    //! Retrieve sub layers from a DB ogr layer \a path with the specified \a driver
    static QList<QgsOgrDbLayerInfo *> subLayers( const QString &path, const QString &driver );
    //! Returns a LayerType from a geometry type string
    static QgsLayerItem::LayerType layerTypeFromDb( const QString &geometryType );

#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;

    static void deleteLayer( bool isSubLayer, const QString &uri, const QString &name, QPointer< QgsDataItem > parent );
#endif
  private:
    bool mIsSubLayer;
};


class QgsOgrDataCollectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsOgrDataCollectionItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    /**
     * Utility function to store DB connections
     * \param path to the DB
     * \param ogrDriverName the OGR/GDAL driver name (e.g. "GPKG")
     */
    static bool storeConnection( const QString &path, const QString &ogrDriverName );

    /**
     * Utility function to create and store a new DB connection
     * \param name is the translatable name of the managed layers (e.g. "GeoPackage")
     * \param extensions is a string with file extensions (e.g. "GeoPackage Database (*.gpkg *.GPKG)")
     * \param ogrDriverName the OGR/GDAL driver name (e.g. "GPKG")
     */
    static bool createConnection( const QString &name, const QString &extensions, const QString &ogrDriverName );

};

//! Provider for OGR root data item
class QgsOgrDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override { return QStringLiteral( "OGR" ); }

    int capabilities() override { return QgsDataProvider::File | QgsDataProvider::Dir | QgsDataProvider::Net; }

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;

    bool handlesDirectoryPath( const QString &path ) override;
};



#endif // QGSOGRDATAITEMS_H
