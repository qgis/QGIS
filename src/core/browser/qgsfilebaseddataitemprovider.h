/***************************************************************************
  qgsfilebaseddataitemprovider.h
  --------------------------------------
  Date                 : July 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFILEBASEDDATAITEMPROVIDER_H
#define QGSFILEBASEDDATAITEMPROVIDER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsdataitemprovider.h"
#include "qgsdatacollectionitem.h"
#include "qgslayeritem.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include <QString>
#include <QVector>

class QgsProviderSublayerDetails;

#define SIP_NO_FILE

class QgsDataItem;

/**
 * \ingroup core
 * \brief A generic data item for file based layers.
 *
 * This is a generic data item for file based layers. It is created by a QgsFileBasedDataItemProvider
 * for files which represent a single layer, or as children of a QgsFileDataCollectionItem for
 * files which contain multiple layers.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsProviderSublayerItem final: public QgsLayerItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsProviderSublayerItem.
     * \param parent parent item
     * \param name data item name (this should match either the layer's name or the filename of a single-layer file)
     * \param details sublayer details
     * \param filePath path to file (for sublayer items which directly represent a file)
     */
    QgsProviderSublayerItem( QgsDataItem *parent, const QString &name, const QgsProviderSublayerDetails &details, const QString &filePath );
    QString layerName() const override;
    QVector<QgsDataItem *> createChildren() override;
    QgsAbstractDatabaseProviderConnection *databaseConnection() const override;

    /**
     * Returns the sublayer details for the item.
     *
     * \since QGIS 3.28
     */
    QgsProviderSublayerDetails sublayerDetails() const;

  private:

    static Qgis::BrowserLayerType layerTypeFromSublayer( const QgsProviderSublayerDetails &sublayer );

    QgsProviderSublayerDetails mDetails;

};


/**
 * \ingroup core
 * \brief A data collection item for grouping of the content in file based data collections (e.g. FileGeodatabase files).
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsFileDataCollectionGroupItem final: public QgsDataCollectionItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsFileDataCollectionGroupItem.
     * \param parent parent item
     * \param groupName group name
     * \param path item path
     */
    QgsFileDataCollectionGroupItem( QgsDataItem *parent, const QString &groupName, const QString &path );

    /**
     * Adds a \a sublayer to the group.
     */
    void appendSublayer( const QgsProviderSublayerDetails &sublayer );

    bool hasDragEnabled() const override;
    QgsMimeDataUtils::UriList mimeUris() const override;

  private:

    QList< QgsProviderSublayerDetails > mSublayers;
};

/**
 * \ingroup core
 * \brief A data collection item for file based data collections (e.g. NetCDF files).
 *
 * This is a generic data collection item, which is created by a QgsFileBasedDataItemProvider
 * for datasets which may potentially contain multiple sublayers.
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsFileDataCollectionItem final: public QgsDataCollectionItem
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsFileDataCollectionItem.
     * \param parent parent item
     * \param name data item name (this should usually match the filename of the dataset)
     * \param path path to dataset
     * \param sublayers list of sublayers to initially populate the item with. If the sublayer details are incomplete
     * (see QgsProviderUtils::sublayerDetailsAreIncomplete()) then the item will be populated in a background thread when
     * expanded.
     * \param extraUriParts optional map of extra components to append to URIs generated for the \a path. The provider-specific encodeUri methods will be used to handle these URI additions. Since QGIS 3.40.
     */
    QgsFileDataCollectionItem( QgsDataItem *parent,
                               const QString &name,
                               const QString &path,
                               const QList< QgsProviderSublayerDetails> &sublayers,
                               const QVariantMap &extraUriParts = QVariantMap() );

    QVector<QgsDataItem *> createChildren() override;
    bool hasDragEnabled() const override;

    /**
     * Returns TRUE if the file is likely to support addition of vector layers.
     *
     * This method is designed to be cheap to evaluate, so that it is safe to call
     * within the main thread.
     *
     * By default it relies solely on a basic check of the associated driver's theoretical
     * capabilities, and does not actually open the dataset to determine whether the particular
     * file definitely can support layer additions.
     *
     * If a connection has previously been opened for this item, then the results will
     * be updated to use the actual capabilities determined by that connection.
     *
     * \since QGIS 3.32
     */
    bool canAddVectorLayers() const;

    QgsMimeDataUtils::UriList mimeUris() const override;
    QgsAbstractDatabaseProviderConnection *databaseConnection() const override;

    /**
     * Returns the associated connection capabilities, if a databaseConnection() is available.
     *
     * \see databaseConnectionCapabilities2()
     * \since QGIS 3.32
     */
    QgsAbstractDatabaseProviderConnection::Capabilities databaseConnectionCapabilities() const;

    /**
     * Returns extended connection capabilities, if a databaseConnection() is available.
     *
     * \see databaseConnectionCapabilities()
     * \since QGIS 3.32
     */
    Qgis::DatabaseProviderConnectionCapabilities2 databaseConnectionCapabilities2() const;

    /**
     * Returns the sublayers.
     * \since QGIS 3.38
     */
    QList<QgsProviderSublayerDetails> sublayers() const;

  private:

    QList< QgsProviderSublayerDetails> mSublayers;
    QVariantMap mExtraUriParts;
    mutable bool mHasCachedCapabilities = false;
    mutable QgsAbstractDatabaseProviderConnection::Capabilities mCachedCapabilities;
    mutable Qgis::DatabaseProviderConnectionCapabilities2 mCachedCapabilities2;
    mutable bool mHasCachedDropSupport = false;
    mutable bool mCachedSupportsDrop = false;
};


/**
 * \ingroup core
 * \brief A data item provider for file based data sources.
 *
 * This is a generic data item provider, which creates data items for file based data sources from
 * registered providers (using the QgsProviderRegistry::querySublayers() API).
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsFileBasedDataItemProvider : public QgsDataItemProvider
{
  public:

    QString name() override;
    Qgis::DataItemProviderCapabilities capabilities() const override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override SIP_FACTORY;

    /**
     * Static method to create a data item for sublayers corresponding to a file-like \a path.
     *
     * \param path file like path to create item for
     * \param parentItem parent data item
     * \param providers list of data providers to include when scanning for sublayers for the path. Must be populated.
     * \param extraUriParts map of optional extra components to append to URIs generated for the \a path. The provider-specific encodeUri methods will be used to handle these URI additions.
     * \param queryFlags flags controlling sublayer querying
     *
     * \returns data item, if \a path corresponds to a layer or an item with multiple sublayers
     *
     * \since QGIS 3.40
     */
    static QgsDataItem *createLayerItemForPath( const QString &path, QgsDataItem *parentItem, const QStringList &providers,
        const QVariantMap &extraUriParts,
        Qgis::SublayerQueryFlags queryFlags );

    bool handlesDirectoryPath( const QString &path ) override;

  private:

    static QgsDataItem *createDataItemForPathPrivate( const QString &path, QgsDataItem *parentItem, const QStringList *allowedProviders,
        Qgis::SublayerQueryFlags queryFlags,
        const QVariantMap &extraUriParts );
};

#endif // QGSFILEBASEDDATAITEMPROVIDER_H
