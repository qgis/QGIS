/***************************************************************************
    qgsvectordataprovider.h - DataProvider Interface for vector layers
     --------------------------------------
    Date                 : 23-Sep-2004
    Copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSVECTORDATAPROVIDER_H
#define QGSVECTORDATAPROVIDER_H


class QTextCodec;

#include <QList>
#include <QSet>
#include <QMap>

//QGIS Includes
#include "qgis.h"
#include "qgsdataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsfield.h"

typedef QMap<QString, QString> QgsNewAttributesMap;
typedef QMap<QString, QVariant::Type> QgsNativeTypeMap;

/** \ingroup core
 * This is the base class for vector data providers.
 *
 * Data providers abstract the retrieval and writing (where supported)
 * of feature and attrubute information from a spatial datasource.
 *
 *
 */
class CORE_EXPORT QgsVectorDataProvider : public QgsDataProvider
{
  public:

    // If you add to this, please also add to capabilitiesString()
    /**
     * enumeration with capabilities that providers might implement
     */
    enum Capability
    {
      NoCapabilities =                     0,
      AddFeatures =                        1,
      DeleteFeatures =               1 <<  1,
      ChangeAttributeValues =        1 <<  2,
      AddAttributes =                1 <<  3,
      DeleteAttributes =             1 <<  4,
      SaveAsShapefile =              1 <<  5,
      CreateSpatialIndex =           1 <<  6,
      SelectAtId =                   1 <<  7,
      ChangeGeometries =             1 <<  8,
      SelectGeometryAtId =           1 <<  9,
      RandomSelectGeometryAtId =     1 << 10,
      SequentialSelectGeometryAtId = 1 << 11
    };

    /**
     * Constructor of the vector provider
     * @param uri  uniform resource locator (URI) for a dataset
     */
    QgsVectorDataProvider( QString uri = QString() );

    /**
     * Destructor
     */
    virtual ~QgsVectorDataProvider();

    /**
     * Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const;

    /** Select features based on a bounding rectangle. Features can be retrieved with calls to nextFeature.
     * @param fetchAttributes list of attributes which should be fetched
     * @param rect spatial filter
     * @param fetchGeometry true if the feature geometry should be fetched
     * @param useIntersect true if an accurate intersection test should be used,
     *                     false if a test based on bounding box is sufficient
     */
    virtual void select( QgsAttributeList fetchAttributes = QgsAttributeList(),
                         QgsRectangle rect = QgsRectangle(),
                         bool fetchGeometry = true,
                         bool useIntersect = false ) = 0;

    /**
     * Update the feature count based on current spatial filter. If not
     * overridden in the data provider this function returns -1
     */
    virtual long updateFeatureCount();

    /**
     * Gets the feature at the given feature ID.
     * @param featureId of the feature to be returned
     * @param feature which will receive the data
     * @param fetchGeometry flag which if true, will cause the geometry to be fetched from the provider
     * @param fetchAttributes a list containing the indexes of the attribute fields to copy
     * @return True when feature was found, otherwise false
     *
     * Default implementation traverses all features until it finds the one with correct ID.
     * In case the provider supports reading the feature directly, override this function.
     */
    virtual bool featureAtId( int featureId,
                              QgsFeature& feature,
                              bool fetchGeometry = true,
                              QgsAttributeList fetchAttributes = QgsAttributeList() );

    /**
     * Get the next feature resulting from a select operation.
     * @param feature feature which will receive data from the provider
     * @return true when there was a feature to fetch, false when end was hit
     */
    virtual bool nextFeature( QgsFeature& feature ) = 0;

    /**
     * Get feature type.
     * @return int representing the feature type
     */
    virtual QGis::WkbType geometryType() const = 0;


    /**
     * Number of features in the layer
     * @return long containing number of features
     */
    virtual long featureCount() const = 0;


    /**
     * Number of attribute fields for a feature in the layer
     */
    virtual uint fieldCount() const = 0;

    /**
     * Return a map of indexes with field names for this layer
     * @return map of fields
     * @see QgsFieldMap
     */
    virtual const QgsFieldMap &fields() const = 0;

    /**
     * Return a short comment for the data that this provider is
     * providing access to (e.g. the comment for postgres table).
     */
    virtual QString dataComment() const;

    /** Restart reading features from previous select operation */
    virtual void rewind() = 0;

    /**
     * Returns the minimum value of an attribute
     * @param index the index of the attribute
     *
     * Default implementation walks all numeric attributes and caches minimal
     * and maximal values. If provider has facilities to retrieve minimal
     * value directly, override this function.
     */
    virtual QVariant minimumValue( int index );

    /**
     * Returns the maximum value of an attribute
     * @param index the index of the attribute
     *
     * Default implementation walks all numeric attributes and caches minimal
     * and maximal values. If provider has facilities to retrieve maximal
     * value directly, override this function.
     */
    virtual QVariant maximumValue( int index );

    /**
     * Return unique values of an attribute
     * @param index the index of the attribute
     * @param values reference to the list to fill
     *
     * Default implementation simply iterates the features
     */
    virtual void uniqueValues( int index, QList<QVariant> &uniqueValues );

    /**
     * Adds a list of features
     * @return true in case of success and false in case of failure
     */
    virtual bool addFeatures( QgsFeatureList &flist );

    /**
     * Deletes one or more features
     * @param id list containing feature ids to delete
     * @return true in case of success and false in case of failure
     */
    virtual bool deleteFeatures( const QgsFeatureIds &id );

    /**
     * Adds new attributes
     * @param attributes map with attribute name as key and type as value
     * @return true in case of success and false in case of failure
     */
    virtual bool addAttributes( const QgsNewAttributesMap & attributes );

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
     * Returns the default value for field specified by @c fieldId
     */
    virtual QVariant defaultValue( int fieldId );

    /**
     * Changes geometries of existing features
     * @param geometry_map   A QgsGeometryMap whose index contains the feature IDs
     *                       that will have their geometries changed.
     *                       The second map parameter being the new geometries themselves
     * @return               True in case of success and false in case of failure
     */
    virtual bool changeGeometryValues( QgsGeometryMap & geometry_map );

    /**
     * Creates a spatial index on the datasource (if supported by the provider type).
     * @return true in case of success
     */
    virtual bool createSpatialIndex();

    /** Returns a bitmask containing the supported capabilities
        Note, some capabilities may change depending on whether
        a spatial filter is active on this provider, so it may
        be prudent to check this value per intended operation.
     */
    virtual int capabilities() const;

    /**
     *  Returns the above in friendly format.
     */
    QString capabilitiesString() const;

    /**
     * Set encoding used for accessing data from layer
     */
    virtual void setEncoding( const QString& e );

    /**
     * Get encoding which is used for accessing data
     */
    QString encoding() const;

    /**
     * Returns the index of a field name or -1 if the field does not exist
     */
    int fieldNameIndex( const QString& fieldName ) const;

    /**Return a map where the key is the name of the field and the value is its index*/
    QMap<QString, int> fieldNameMap() const;

    /**
     * Return list of indexes to fetch all attributes in nextFeature()
     */
    virtual QgsAttributeList attributeIndexes();

    /**Returns the names of the numerical types*/
    const QgsNativeTypeMap &supportedNativeTypes() const;

    /**
     * Set whether provider should also return features that don't have
     * associated geometry. FALSE by default
     */
    void enableGeometrylessFeatures( bool fetch );

  protected:
    QVariant convertValue( QVariant::Type type, QString value );

    void fillMinMaxCache();

    bool mCacheMinMaxDirty;
    QMap<int, QVariant> mCacheMinValues, mCacheMaxValues;

    /** Encoding */
    QTextCodec* mEncoding;

    /** should provider fetch also features that don't have geometry? */
    bool mFetchFeaturesWithoutGeom;

    /**True if geometry should be added to the features in nextFeature calls*/
    bool mFetchGeom;

    /**List of attribute indices to fetch with nextFeature calls*/
    QgsAttributeList mAttributesToFetch;

    /**The names of the providers native types*/
    QgsNativeTypeMap mSupportedNativeTypes;
};

#endif
