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

#include "qgsdatacollectionitem.h"
#include "qgsdataitemprovider.h"
#include "qgsconnectionsitem.h"
#include "qgsdataprovider.h"
#include "qgslayeritem.h"
#include "qgstaskmanager.h"
#include "qgis_sip.h"

#include <QStringList>

///@cond PRIVATE
#define SIP_NO_FILE


/**
 * \brief The QgsGeoPackageCollectionItem class is the base class for
 *        GeoPackage container
 */
class CORE_EXPORT QgsGeoPackageCollectionItem : public QgsDataCollectionItem
{
    Q_OBJECT

  public:
    QgsGeoPackageCollectionItem( QgsDataItem *parent, const QString &name, const QString &path );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

    //! Returns the layer type from \a geometryType
    static Qgis::BrowserLayerType layerTypeFromDb( const QString &geometryType );

    //! Deletes a geopackage raster layer
    bool deleteRasterLayer( const QString &layerName, QString &errCause );

    //! Deletes a geopackage vector layer
    bool deleteVectorLayer( const QString &layerName, QString &errCause );

    /**
     * Compacts (VACUUM) a geopackage database
     * \param name DB connection name
     * \param path DB connection path
     * \param errCause contains the error message
     * \return TRUE on success
     */
    static bool vacuumGeoPackageDb( const QString &name, const QString &path, QString &errCause );

    void addConnection();
    void deleteConnection();


    // QgsDataItem interface
  public:
    bool layerCollection() const override;
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::UriList mimeUris() const override;
};


/**
 * \brief The QgsGeoPackageAbstractLayerItem class is the base class for GeoPackage raster and vector layers
 */
class CORE_EXPORT QgsGeoPackageAbstractLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:

    /**
     * Returns a list of all table names for the geopackage
     */
    QStringList tableNames() const;

    //! Checks if the data source has any layer in the current project returns them
    QList<QgsMapLayer *> layersInProject() const;

    /**
     * Deletes a layer.
     * Subclasses need to implement this function with
     * the real deletion implementation
     */
    virtual bool executeDeleteLayer( QString &errCause ) = 0;

    /**
     * Returns the parent collection item
     * \since QGIS 3.10
     */
    QgsGeoPackageCollectionItem *collection() const;

  protected:
    QgsGeoPackageAbstractLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, Qgis::BrowserLayerType layerType, const QString &providerKey );

  private:

    //! Store a casted pointer to the parent collection
    QgsGeoPackageCollectionItem *mCollection = nullptr;
};

class CORE_EXPORT QgsGeoPackageRasterLayerItem : public QgsGeoPackageAbstractLayerItem
{
    Q_OBJECT

  public:
    QgsGeoPackageRasterLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri );
    bool executeDeleteLayer( QString &errCause ) override;
};



class CORE_EXPORT QgsGeoPackageVectorLayerItem final: public QgsGeoPackageAbstractLayerItem
{
    Q_OBJECT

  public:
    QgsGeoPackageVectorLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, Qgis::BrowserLayerType layerType );
    bool executeDeleteLayer( QString &errCause ) override;

    // QgsDataItem interface
    QVector<QgsDataItem *> createChildren() override;
};

/**
 * \brief The QgsGeoPackageConnectionItem class adds the stored
 *        connection management to QgsGeoPackageCollectionItem
 */
class CORE_EXPORT QgsGeoPackageConnectionItem final: public QgsGeoPackageCollectionItem
{
    Q_OBJECT

  public:
    QgsGeoPackageConnectionItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool equal( const QgsDataItem *other ) override;
};


class CORE_EXPORT QgsGeoPackageRootItem final: public QgsConnectionsRootItem
{
    Q_OBJECT

  public:
    QgsGeoPackageRootItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 1; }
    QWidget *paramWidget() override;
  public slots:
    void newConnection();
    void onConnectionsChanged();
};


//! Provider for geopackage data item
class QgsGeoPackageDataItemProvider final: public QgsDataItemProvider
{
  public:
    QString name() override;
    QString dataProviderKey() const override;
    Qgis::DataItemProviderCapabilities capabilities() const override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

///@endcond
#endif // QGSGEOPACKAGEDATAITEMS_H
