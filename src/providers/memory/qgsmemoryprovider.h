/***************************************************************************
    memoryprovider.h - provider with storage in memory
    ------------------
    begin                : June 2008
    copyright            : (C) 2008 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectordataprovider.h"
#include "qgscoordinatereferencesystem.h"


typedef QMap<QgsFeatureId, QgsFeature> QgsFeatureMap;

class QgsSpatialIndex;

class QgsMemoryFeatureIterator;

class QgsMemoryProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:
    QgsMemoryProvider( QString uri = QString() );

    virtual ~QgsMemoryProvider();

    /* Implementation of functions from QgsVectorDataProvider */

    virtual QgsAbstractFeatureSource* featureSource() const OVERRIDE;

    /**
     * Returns the permanent storage type for this layer as a friendly name.
     */

    virtual QString dataSourceUri() const OVERRIDE;

    /**
     * Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const OVERRIDE;

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) OVERRIDE;

    /**
     * Get feature type.
     * @return int representing the feature type
     */
    virtual QGis::WkbType geometryType() const OVERRIDE;

    /**
     * Number of features in the layer
     * @return long containing number of features
     */
    virtual long featureCount() const OVERRIDE;

    /**
     * Return a map of indexes with field names for this layer
     * @return map of fields
     */
    virtual const QgsFields & fields() const OVERRIDE;


    /**
      * Adds a list of features
      * @return true in case of success and false in case of failure
      */
    virtual bool addFeatures( QgsFeatureList & flist ) OVERRIDE;

    /**
      * Deletes a feature
      * @param id list containing feature ids to delete
      * @return true in case of success and false in case of failure
      */
    virtual bool deleteFeatures( const QgsFeatureIds & id ) OVERRIDE;


    /**
     * Adds new attributes
     * @param attributes map with attribute name as key and type as value
     * @return true in case of success and false in case of failure
     */
    virtual bool addAttributes( const QList<QgsField> &attributes ) OVERRIDE;

    /**
     * Deletes existing attributes
     * @param attributes a set containing names of attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool deleteAttributes( const QgsAttributeIds& attributes ) OVERRIDE;

    /**
     * Changes attribute values of existing features.
     * @param attr_map a map containing changed attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap & attr_map ) OVERRIDE;

    /**
     * Changes geometries of existing features
     * @param geometry_map   A std::map containing the feature IDs to change the geometries of.
     *                       the second map parameter being the new geometries themselves
     * @return               true in case of success and false in case of failure
     */
    virtual bool changeGeometryValues( QgsGeometryMap & geometry_map ) OVERRIDE;

    /** Accessor for sql where clause used to limit dataset */
    QString subsetString() OVERRIDE;

    /** mutator for sql where clause used to limit dataset size */
    bool setSubsetString( QString theSQL, bool updateFeatureCount = true ) OVERRIDE;

    virtual bool supportsSubsetString() OVERRIDE { return true; }

    /**
     * Creates a spatial index
     * @return true in case of success
     */
    virtual bool createSpatialIndex() OVERRIDE;

    /** Returns a bitmask containing the supported capabilities
    Note, some capabilities may change depending on whether
    a spatial filter is active on this provider, so it may
    be prudent to check this value per intended operation.
     */
    virtual int capabilities() const OVERRIDE;


    /* Implementation of functions from QgsDataProvider */

    /**
     * return a provider name
     */
    QString name() const OVERRIDE;

    /**
     * return description
     */
    QString description() const OVERRIDE;

    /**
     * Return the extent for this data layer
     */
    virtual QgsRectangle extent() OVERRIDE;

    /**
     * Returns true if this is a valid provider
     */
    bool isValid() OVERRIDE;

    virtual QgsCoordinateReferenceSystem crs() OVERRIDE;

  protected:

    // called when added / removed features or geometries has been changed
    void updateExtent();

  private:
    // Coordinate reference system
    QgsCoordinateReferenceSystem mCrs;

    // fields
    QgsFields mFields;
    QGis::WkbType mWkbType;
    QgsRectangle mExtent;

    // features
    QgsFeatureMap mFeatures;
    QgsFeatureId mNextFeatureId;

    // indexing
    QgsSpatialIndex* mSpatialIndex;

    QString mSubsetString;

    friend class QgsMemoryFeatureSource;
};
