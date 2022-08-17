/***************************************************************************
  qgslayermetadataprovider.h - QgsLayerMetadataProvider

 ---------------------
 begin                : 17.8.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSABSTRACTLAYERMETADATAPROVIDER_H
#define QGSABSTRACTLAYERMETADATAPROVIDER_H

#include <QObject>

#include "qgis_core.h"
#include "qgis.h"

#include "qgslayermetadata.h"
#include "qgsrectangle.h"
#include "qgspolygon.h"


class QgsFeedback;

/**
 * \ingroup core
 * \brief Result record of layer metadata provider search.
 * The results contains metadata information and all information
 * that is required by QGIS to load the layer.
 *
 * \since QGIS 3.28
 */
struct CORE_EXPORT QgsLayerMetadataProviderResult
{
  //! Metadata
  QgsLayerMetadata metadata;
  //! Metadata extent in EPSG:4326
  QgsPolygon geographicExtent;
  //! Layer geometry type (Point, Polygon, Linestring)
  QgsWkbTypes::GeometryType geometryType;
  //! Layer CRS authid
  QString crs;
  //! Layer QgsDataSourceUri string
  QString uri;
  //! Layer data provider name
  QString dataProviderName;
  //! Layer type (vector, raster etc.)
  QgsMapLayerType layerType;
  //! Metadata standard uri, QGIS QMD metadata format uses "http://mrcc.com/qgis.dtd"
  QString standardUri;

  // Convenience accessors to metadata fields:

  //! Metadata identifier
  QString identifier() const;
  //! Metadata title
  QString title() const;
  //! Metadata abstract
  QString abstract() const;
};

/**
 * \ingroup core
 * \brief Container of result records from a layer metadata search.
 *
 * Contains the records of the layer metadata provider that matched the
 * search criteria and the list of the errors that occurred while
 * searching for metadata.
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsLayerMetadataSearchResult
{

  public:

    /**
     * Returns the list of metadata results.
     */
    const QList<QgsLayerMetadataProviderResult> &metadata() const;

    /**
     * Adds a \a newMetadata record to the list of results.
     */
    void addMetadata( const QgsLayerMetadataProviderResult &newMetadata );

    /**
     * Returns the list of errors occurred during a metadata search.
     */
    const QStringList &errors() const;

    /**
     * Adds a \a newError to the list of errors.
     */
    void addError( const QString &newError );

  private:

    //! List of metadata that matched the search criteria
    QList<QgsLayerMetadataProviderResult> mMetadata;
    //! List of errors occurred while searching
    QStringList mErrors;
};

/**
 * \ingroup core
 * \brief Layer metadata provider backends interface.
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsAbstractLayerMetadataProvider
{

  public:

    /**
     * Returns the name of the layer metadata provider implementation, usually the name of the data provider
     * but if can be another unique identifier.
     */
    virtual QString type() const = 0;

    /**
     * Searches for metadata optionally filtering by search string and geographic extent.
     * \param searchString defines a filter to limit the results to the records where the search string appears in the "identifier", "title" or "abstract" metadata fields, a case-insensitive comparison is used for the match.
     * \param geographicExtent defines a filter where the spatial extent matches the given extent in EPSG:4326
     * \param feedback can be used to monitor and control the search process.
     * \returns a QgsLayerMetadataSearchResult object with a list of metadata and errors
     */
    virtual QgsLayerMetadataSearchResult search( const QString &searchString = QString(), const QgsRectangle &geographicExtent = QgsRectangle(), QgsFeedback *feedback = nullptr ) const = 0;

    virtual ~QgsAbstractLayerMetadataProvider() = default;

};

#endif // QGSABSTRACTLAYERMETADATAPROVIDER_H
