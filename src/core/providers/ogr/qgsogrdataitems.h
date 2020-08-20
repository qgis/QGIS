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
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

/**
 * Holds the information about a gpkg layer
 */
class CORE_EXPORT QgsOgrDbLayerInfo
{
  public:
    QgsOgrDbLayerInfo( const QString &path,
                       const QString &uri,
                       const QString &name,
                       const QString &theGeometryColumn,
                       const QString &theGeometryType,
                       const QgsLayerItem::LayerType &theLayerType,
                       const QString &driverName )
      : mPath( path )
      , mUri( uri )
      , mName( name )
      , mGeometryColumn( theGeometryColumn )
      , mGeometryType( theGeometryType )
      , mLayerType( theLayerType )
      , mDriverName( driverName )
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
    QString mDriverName;
};

/**
 * The QgsOgrLayerException class is thrown by QgsOgrLayerItem when the layer is not valid
 */
class CORE_EXPORT QgsOgrLayerNotValidException: public QgsException
{
  public:

    /**
     * Constructor for QgsOgrLayerNotValidException, with the specified error \a message.
     */
    QgsOgrLayerNotValidException( const QString &message )
      : QgsException( message )
    {}
};

class CORE_EXPORT QgsOgrLayerItem final: public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsOgrLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, LayerType layerType, const QString &driverName = QString(), bool isSubLayer = false );

    QString layerName() const override;

    /**
     * Retrieve sub layers from a DB ogr layer \a path with the specified \a driver
     * If the layer is not valid, throw a std::exception
     */
    static QList<QgsOgrDbLayerInfo *> subLayers( const QString &path, const QString &driver );
    //! Returns a LayerType from a geometry type string
    static QgsLayerItem::LayerType layerTypeFromDb( const QString &geometryType );
    bool isSubLayer() const;

    QVector<QgsDataItem *> createChildren() override;

  private:
    QString mDriverName;
    bool mIsSubLayer;


};




class CORE_EXPORT QgsOgrDataCollectionItem final: public QgsDataCollectionItem
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
    static bool saveConnection( const QString &path, const QString &ogrDriverName );

    /**
     * Utility function to create and store a new DB connection
     * \param name is the translatable name of the managed layers (e.g. "GeoPackage")
     * \param extensions is a string with file extensions (e.g. "GeoPackage Database (*.gpkg *.GPKG)")
     * \param ogrDriverName the OGR/GDAL driver name (e.g. "GPKG")
     */
    static bool createConnection( const QString &name, const QString &extensions, const QString &ogrDriverName );
};

//! Provider for OGR root data item
class QgsOgrDataItemProvider final: public QgsDataItemProvider
{
  public:
    QString name() override;
    QString dataProviderKey() const override;
    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;

    bool handlesDirectoryPath( const QString &path ) override;
};

///@endcond
#endif // QGSOGRDATAITEMS_H
