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
     */
    QgsFileDataCollectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QList< QgsProviderSublayerDetails> &sublayers );

    QVector<QgsDataItem *> createChildren() override;
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::UriList mimeUris() const override;
    QgsAbstractDatabaseProviderConnection *databaseConnection() const override;

  private:

    QList< QgsProviderSublayerDetails> mSublayers;
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
    int capabilities() const override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override SIP_FACTORY;
    bool handlesDirectoryPath( const QString &path ) override;
};

#endif // QGSFILEBASEDDATAITEMPROVIDER_H
