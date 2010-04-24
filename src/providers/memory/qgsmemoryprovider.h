/***************************************************************************
    memoryprovider.h - provider with storage in memory
    ------------------
    begin                : June 2008
    copyright            : (C) 2008 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectordataprovider.h"


typedef QMap<int, QgsFeature> QgsFeatureMap;

class QgsSpatialIndex;

class QgsMemoryProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:
    QgsMemoryProvider( QString uri = QString() );

    virtual ~QgsMemoryProvider();

    /* Implementation of functions from QgsVectorDataProvider */

    /**
     * Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const;

    /** Select features based on a bounding rectangle. Features can be retrieved with calls to nextFeature.
     *  @param fetchAttributes list of attributes which should be fetched
     *  @param rect spatial filter
     *  @param fetchGeometry true if the feature geometry should be fetched
     *  @param useIntersect true if an accurate intersection test should be used,
     *                     false if a test based on bounding box is sufficient
     */
    virtual void select( QgsAttributeList fetchAttributes = QgsAttributeList(),
                         QgsRectangle rect = QgsRectangle(),
                         bool fetchGeometry = true,
                         bool useIntersect = false );

    /**
     * Get the next feature resulting from a select operation.
     * @param feature feature which will receive data from the provider
     * @return true when there was a feature to fetch, false when end was hit
     *
     * mFile should be open with the file pointer at the record of the next
     * feature, or EOF.  The feature found on the current line is parsed.
     */
    virtual bool nextFeature( QgsFeature& feature );

    /**
      * Gets the feature at the given feature ID.
      * @param featureId id of the feature
      * @param feature feature which will receive the data
      * @param fetchGeoemtry if true, geometry will be fetched from the provider
      * @param fetchAttributes a list containing the indexes of the attribute fields to copy
      * @return True when feature was found, otherwise false
      */
    virtual bool featureAtId( int featureId,
                              QgsFeature& feature,
                              bool fetchGeometry = true,
                              QgsAttributeList fetchAttributes = QgsAttributeList() );

    /**
     * Get feature type.
     * @return int representing the feature type
     */
    virtual QGis::WkbType geometryType() const;

    /**
     * Number of features in the layer
     * @return long containing number of features
     */
    virtual long featureCount() const;

    /**
     * Number of attribute fields for a feature in the layer
     */
    virtual uint fieldCount() const;

    /**
     * Return a map of indexes with field names for this layer
     * @return map of fields
     */
    virtual const QgsFieldMap & fields() const;

    /** Restart reading features from previous select operation */
    virtual void rewind();


    /**
      * Adds a list of features
      * @return true in case of success and false in case of failure
      */
    virtual bool addFeatures( QgsFeatureList & flist );

    /**
      * Deletes a feature
      * @param id list containing feature ids to delete
      * @return true in case of success and false in case of failure
      */
    virtual bool deleteFeatures( const QgsFeatureIds & id );


    /**
     * Adds new attributes
     * @param attributes map with attribute name as key and type as value
     * @return true in case of success and false in case of failure
     */
    virtual bool addAttributes( const QList<QgsField> &attributes );

    /**
     * Deletes existing attributes
     * @param attributes a set containing names of attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool deleteAttributes( const QgsAttributeIds& attributes );

    /**
     * Changes attribute values of existing features.
     * @param attr_map a map containing changed attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap & attr_map );

    /**
     * Changes geometries of existing features
     * @param geometry_map   A std::map containing the feature IDs to change the geometries of.
     *                       the second map parameter being the new geometries themselves
     * @return               true in case of success and false in case of failure
     */
    virtual bool changeGeometryValues( QgsGeometryMap & geometry_map );

    /**
     * Creates a spatial index
     * @return true in case of success
     */
    virtual bool createSpatialIndex();

    /** Returns a bitmask containing the supported capabilities
    Note, some capabilities may change depending on whether
    a spatial filter is active on this provider, so it may
    be prudent to check this value per intended operation.
     */
    virtual int capabilities() const;


    /* Implementation of functions from QgsDataProvider */

    /**
     * return a provider name
     */
    QString name() const;

    /**
     * return description
     */
    QString description() const;

    /**
     * Return the extent for this data layer
     */
    virtual QgsRectangle extent();

    /**
     * Returns true if this is a valid provider
     */
    bool isValid();

    virtual QgsCoordinateReferenceSystem crs();

  protected:

    // called when added / removed features or geometries has been changed
    void updateExtent();

  private:
    // fields
    QgsFieldMap mFields;
    QGis::WkbType mWkbType;
    QgsRectangle mExtent;

    // features
    QgsFeatureMap mFeatures;
    int mNextFeatureId;

    // selection
    QgsAttributeList mSelectAttrs;
    QgsRectangle mSelectRect;
    QgsGeometry* mSelectRectGeom;
    bool mSelectGeometry, mSelectUseIntersect;
    QgsFeatureMap::iterator mSelectIterator;
    bool mSelectUsingSpatialIndex;
    QList<int> mSelectSI_Features;
    QList<int>::iterator mSelectSI_Iterator;

    // indexing
    QgsSpatialIndex* mSpatialIndex;

};
