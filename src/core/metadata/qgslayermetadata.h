/***************************************************************************
                             qgslayermetadata.h
                             -------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERMETADATA_H
#define QGSLAYERMETADATA_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsbox3d.h"
#include "qgsrange.h"
#include "qgsabstractmetadatabase.h"

class QgsMapLayer;

/**
 * \ingroup core
 * \class QgsLayerMetadata
 * \brief A structured metadata store for a map layer.
 *
 * QgsLayerMetadata handles storage and management of the metadata
 * for a QgsMapLayer. This class is an internal QGIS format with a common
 * metadata structure, which allows for code to access the metadata properties for
 * layers in a uniform way.
 *
 * The metadata store is designed to be compatible with the Dublin Core metadata
 * specifications, and will be expanded to allow compatibility with ISO specifications
 * in future releases. However, the QGIS internal schema does not represent a superset
 * of all existing metadata schemas and accordingly conversion from specific
 * metadata formats to QgsLayerMetadata may result in a loss of information.
 *
 * This class is designed to follow the specifications detailed in
 * the schema definition available at resources/qgis-resource-metadata.xsd
 * within the QGIS source code.
 *
 * Metadata can be validated through the use of QgsLayerMetadataValidator
 * subclasses. E.g. validating against the native QGIS metadata schema can be performed
 * using QgsNativeMetadataValidator.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayerMetadata : public QgsAbstractMetadataBase
{
  public:

    /**
     * Metadata spatial extent structure.
     */
    struct CORE_EXPORT SpatialExtent
    {

      /**
       * Coordinate reference system for spatial extent.
       * The CRS should match the CRS defined in the QgsLayerMetadata CRS property.
       * \see QgsLayerMetadata::crs()
       * \see spatial
       */
      QgsCoordinateReferenceSystem extentCrs;

      /**
       * Geospatial extent of the resource. X and Y coordinates are in the
       * CRS defined by the metadata (see extentCrs).
       *
       * While the spatial extent can include a Z dimension, this is not
       * compulsory.
       * \see extentCrs
       */
      QgsBox3d bounds;

      // TODO c++20 - replace with = default
      bool operator==( const QgsLayerMetadata::SpatialExtent &other ) const;
    };

    /**
     * Metadata extent structure.
     */
    struct CORE_EXPORT Extent
    {
      public:

        /**
         * Spatial extents of the resource.
         * \see setSpatialExtents()
         */
        QList< QgsLayerMetadata::SpatialExtent > spatialExtents() const;

        /**
         * Sets the spatial \a extents of the resource.
         * \see spatialExtents()
         */
        void setSpatialExtents( const QList< QgsLayerMetadata::SpatialExtent > &extents );

        /**
         * Temporal extents of the resource. Use QgsDateTimeRange::isInstant() to determine
         * whether the temporal extent is a range or a single point in time.
         * If QgsDateTimeRange::isInfinite() returns TRUE then the temporal extent
         * is considered to be indeterminate and continuous.
         * \see setTemporalExtents()
         */
        QList< QgsDateTimeRange > temporalExtents() const;

        /**
         * Sets the temporal \a extents of the resource.
         * \see temporalExtents()
         */
        void setTemporalExtents( const QList< QgsDateTimeRange > &extents );

        // TODO c++20 - replace with = default
        bool operator==( const QgsLayerMetadata::Extent &other ) const;

#ifndef SIP_RUN
      private:

        QList< QgsLayerMetadata::SpatialExtent > mSpatialExtents;
        QList< QgsDateTimeRange > mTemporalExtents;

#endif

    };

    /**
     * Metadata constraint structure.
     */
    struct CORE_EXPORT Constraint
    {

      /**
       * Constructor for Constraint.
       */
      Constraint( const QString &constraint = QString(), const QString &type = QString() )
        : type( type )
        , constraint( constraint )
      {}

      /**
       * Constraint type. Standard values include 'access' and 'other', however any
       * string can be used for the type.
       */
      QString type;

      /**
       * Free-form constraint string.
       */
      QString constraint;

      // TODO c++20 - replace with = default
      bool operator==( const QgsLayerMetadata::Constraint &other ) const;

    };

    /**
     * A list of constraints.
     */
    typedef QList< QgsLayerMetadata::Constraint > ConstraintList;

    /**
     * Constructor for QgsLayerMetadata.
     */
    QgsLayerMetadata() = default;


    QgsLayerMetadata *clone() const override SIP_FACTORY;

    /**
     * Returns any fees associated with using the resource.
     * An empty string will be returned if no fees are set.
     * \see setFees()
     */
    QString fees() const;

    /**
     * Sets the \a fees associated with using the resource.
     * Use an empty string if no fees are set.
     * \see fees()
     */
    void setFees( const QString &fees );

    /**
     * Returns a list of constraints associated with using the resource.
     * \see setConstraints()
     */
    QgsLayerMetadata::ConstraintList constraints() const;

    /**
     * Adds an individual constraint to the existing constraints.
     * \see constraints()
     * \see setConstraints()
     */
    void addConstraint( const QgsLayerMetadata::Constraint &constraint );

    /**
     * Sets the list of \a constraints associated with using the resource.
     * \see constraints()
     */
    void setConstraints( const QgsLayerMetadata::ConstraintList &constraints );

    /**
     * Returns a list of attribution or copyright strings associated with the resource.
     * \see setRights()
     */
    QStringList rights() const;

    /**
     * Sets a list of \a rights (attribution or copyright strings) associated with the resource.
     * \see rights()
     */
    void setRights( const QStringList &rights );

    /**
     * Returns a list of licenses associated with the resource (examples: http://opendefinition.org/licenses/).
     * \see setLicenses()
     */
    QStringList licenses() const;

    /**
     * Sets a list of \a licenses associated with the resource.
     * (examples: http://opendefinition.org/licenses/).
     * \see licenses()
     */
    void setLicenses( const QStringList &licenses );

    /**
     * Returns the character encoding of the data in the resource. An empty string will be returned if no encoding is set.
     * \see setEncoding()
     */
    QString encoding() const;

    /**
     * Sets the character \a encoding of the data in the resource. Use an empty string if no encoding is set.
     * \see encoding()
     */
    void setEncoding( const QString &encoding );

    /**
     * Returns the spatial and temporal extents associated with the resource.
     * \see setExtent()
     */
    SIP_SKIP const QgsLayerMetadata::Extent &extent() const;

    /**
     * Returns the spatial and temporal extents associated with the resource.
     * \see setExtent()
     */
    QgsLayerMetadata::Extent &extent();

    /**
     * Sets the spatial and temporal extents associated with the resource.
     * \see setExtent()
     */
    void setExtent( const QgsLayerMetadata::Extent &extent );

    /**
     * Returns the coordinate reference system described by the layer's metadata.
     *
     * Note that this has no link to QgsMapLayer::crs(). While in most cases these
     * two systems are likely to be identical, it is possible to have a layer
     * with a different CRS described by it's accompanying metadata versus the
     * CRS which is actually used to display and manipulate the layer within QGIS.
     * This may be the case when a layer has an incorrect CRS within its metadata
     * and a user has manually overridden the layer's CRS within QGIS.
     *
     * The CRS described here should either match the CRS from the layer QgsMapLayer::crs()
     * or the CRS from the data provider.
     *
     * This property should also match the CRS property used in the spatial extent.
     *
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Sets the coordinate reference system for the layer's metadata.
     *
     * Note that this has no link to QgsMapLayer::setCrs(). Setting the layer's
     * CRS via QgsMapLayer::setCrs() does not affect the layer's metadata CRS,
     * and changing the CRS from the metadata will not change the layer's
     * CRS or how it is projected within QGIS.
     *
     * While ideally these two systems are likely to be identical, it is possible to have a layer
     * with a different CRS described by it's accompanying metadata versus the
     * CRS which is actually used to display and manipulate the layer within QGIS.
     * This may be the case when a layer has an incorrect CRS within its metadata
     * and a user has manually overridden the layer's CRS within QGIS.
     *
     * The CRS described here should either match the CRS from the layer QgsMapLayer::crs()
     * or the CRS from the data provider.
     *
     * This property should also match the CRS property used in the spatial extent.
     *
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Saves the metadata to a \a layer's custom properties (see QgsMapLayer::setCustomProperty() ).
     * \see readFromLayer()
     */
    void saveToLayer( QgsMapLayer *layer ) const;

    /**
     * Reads the metadata state from a \a layer's custom properties (see QgsMapLayer::customProperty() ).
     * \see saveToLayer()
     */
    void readFromLayer( const QgsMapLayer *layer );

    bool readMetadataXml( const QDomElement &metadataElement ) override;
    bool writeMetadataXml( QDomElement &metadataElement, QDomDocument &document ) const override;
    void combine( const QgsAbstractMetadataBase *other ) override;

    bool operator==( const QgsLayerMetadata &metadataOther ) const;

    /**
     * Returns TRUE if the metadata identifier, title, abstract, keywords or categories
     * contain \a searchString using case-insensitive search.
     *
     * If \a searchString is empty this method returns FALSE.
     *
     * \since QGIS 3.28
     */
    bool contains( const QString &searchString ) const;

    /**
     * Returns TRUE if the metadata identifier, title, abstract, keywords or categories
     * matches any regular expression from \a searchReList.
     *
     * \since QGIS 3.28
     */
    bool matches( const QVector<QRegularExpression> &searchReList ) const;

  private:

    /*
     * IMPORTANT!!!!!!
     *
     * Do NOT add anything to this class without also updating the schema
     * definition located at resources/qgis-resource-metadata.xsd
     *
     */

    QString mFees;
    ConstraintList mConstraints;
    QStringList mRights;
    QStringList mLicenses;

    // IMPORTANT - look up before adding anything here!!

    QString mEncoding;
    QgsCoordinateReferenceSystem mCrs;

    Extent mExtent;

    /*
     * IMPORTANT!!!!!!
     *
     * Do NOT add anything to this class without also updating the schema
     * definition located at resources/qgis-resource-metadata.xsd
     *
     */

};

Q_DECLARE_METATYPE( QgsLayerMetadata::ConstraintList )
Q_DECLARE_METATYPE( QgsLayerMetadata::Extent )
Q_DECLARE_METATYPE( QgsLayerMetadata )

#endif // QGSLAYERMETADATA_H
