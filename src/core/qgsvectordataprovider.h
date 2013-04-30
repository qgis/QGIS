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
#ifndef QGSVECTORDATAPROVIDER_H
#define QGSVECTORDATAPROVIDER_H

class QTextCodec;

#include <QList>
#include <QSet>
#include <QMap>
#include <QHash>

//QGIS Includes
#include "qgis.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsrectangle.h"

typedef QList<int> QgsAttributeList;
typedef QSet<int> QgsAttributeIds;
typedef QHash<int, QString> QgsAttrPalIndexNameHash;

class QgsFeatureIterator;

#include "qgsfeaturerequest.h"

/** \ingroup core
 * This is the base class for vector data providers.
 *
 * Data providers abstract the retrieval and writing (where supported)
 * of feature and attribute information from a spatial datasource.
 *
 *
 */
class CORE_EXPORT QgsVectorDataProvider : public QgsDataProvider
{
    Q_OBJECT

  public:

    // If you add to this, please also add to capabilitiesString()
    /**
     * enumeration with capabilities that providers might implement
     */
    enum Capability
    {
      /** provider has no capabilities */
      NoCapabilities =                     0,
      /** allows adding features */
      AddFeatures =                        1,
      /** allows deletion of features */
      DeleteFeatures =               1 <<  1,
      /** allows modification of attribute values */
      ChangeAttributeValues =        1 <<  2,
      /** allows addition of new attributes (fields) */
      AddAttributes =                1 <<  3,
      /** allows deletion of attributes (fields) */
      DeleteAttributes =             1 <<  4,
      /** DEPRECATED - do not use */
      SaveAsShapefile =              1 <<  5,
      /** allows creation of spatial index */
      CreateSpatialIndex =           1 <<  6,
      /** fast access to features using their ID */
      SelectAtId =                   1 <<  7,
      /** allows modifications of geometries */
      ChangeGeometries =             1 <<  8,
      /** DEPRECATED - do not use */
      SelectGeometryAtId =           1 <<  9,
      /** DEPRECATED - do not use */
      RandomSelectGeometryAtId =     1 << 10,
      /** DEPRECATED - do not use */
      SequentialSelectGeometryAtId = 1 << 11,
      CreateAttributeIndex =         1 << 12,
      /** allows user to select encoding */
      SelectEncoding =               1 << 13,
    };

    /** bitmask of all provider's editing capabilities */
    const static int EditingCapabilities = AddFeatures | DeleteFeatures |
                                           ChangeAttributeValues | ChangeGeometries | AddAttributes | DeleteAttributes;

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

    /**
     * Query the provider for features specified in request.
     */
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request = QgsFeatureRequest() ) = 0;

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
     * Return a map of indexes with field names for this layer
     * @return map of fields
     * @see QgsFieldMap
     */
    virtual const QgsFields &fields() const = 0;

    /**
     * Return a short comment for the data that this provider is
     * providing access to (e.g. the comment for postgres table).
     */
    virtual QString dataComment() const;

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
     * @param uniqueValues values reference to the list to fill
     * @param limit maxmum number of the values to return (added in 1.4)
     *
     * Default implementation simply iterates the features
     */
    virtual void uniqueValues( int index, QList<QVariant> &uniqueValues, int limit = -1 );

    /**Returns the possible enum values of an attribute. Returns an empty stringlist if a provider does not support enum types
      or if the given attribute is not an enum type.
     * @param index the index of the attribute
     * @param enumList reference to the list to fill
      @note: added in version 1.2*/
    virtual void enumValues( int index, QStringList& enumList ) { Q_UNUSED( index ); enumList.clear(); }

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
     * @param attributes list of new attributes
     * @return true in case of success and false in case of failure
     * @note added in 1.2
     */
    virtual bool addAttributes( const QList<QgsField> &attributes );

    /**
     * Deletes existing attributes
     * @param attributes a set containing indices of attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool deleteAttributes( const QgsAttributeIds &attributes );

    /**
     * Changes attribute values of existing features.
     * @param attr_map a map containing changed attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map );

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

    /**Create an attribute index on the datasource*/
    virtual bool createAttributeIndex( int field );

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

    /**
     * Return list of indexes of fields that make up the primary key
     * @note added in 2.0
     */
    virtual QgsAttributeList pkAttributeIndexes() { return QgsAttributeList(); }

    /**
     * Return list of indexes to names for QgsPalLabeling fix
     */
    virtual QgsAttrPalIndexNameHash palAttributeIndexNames() const { return mAttrPalIndexName; }

    /**
     * check if provider supports type of field
     * @note added in 1.2
     */
    bool supportedType( const QgsField &field ) const;

    struct NativeType
    {
      NativeType( QString typeDesc, QString typeName, QVariant::Type type, int minLen = 0, int maxLen = 0, int minPrec = 0, int maxPrec = 0 ) :
          mTypeDesc( typeDesc ), mTypeName( typeName ), mType( type ), mMinLen( minLen ), mMaxLen( maxLen ), mMinPrec( minPrec ), mMaxPrec( maxPrec ) {};

      QString mTypeDesc;
      QString mTypeName;
      QVariant::Type mType;
      int mMinLen;
      int mMaxLen;
      int mMinPrec;
      int mMaxPrec;
    };

    /**
     * Returns the names of the supported types
     * @note added in 1.2
     */
    const QList< NativeType > &nativeTypes() const;

    /** Returns true if the provider is strict about the type of inserted features
          (e.g. no multipolygon in a polygon layer)
          @note: added in version 1.4*/
    virtual bool doesStrictFeatureTypeCheck() const { return true;}

    /** Returns a list of available encodings */
    static const QStringList &availableEncodings();

    /* provider has errors to report
     * @note added in 1.7
     */
    bool hasErrors();

    /* clear recorded errors
     * @note added in 1.7
     */
    void clearErrors();

    /* get recorded errors
     * @note added in 1.7
     */
    QStringList errors();

<<<<<<< HEAD
<<<<<<< HEAD
    /**
     * It returns false by default.
     * Must be implemented by providers that support saving and loading styles to db returning true
     */
    virtual bool isSaveAndLoadStyleToDBSupported() { return false; }

=======
  signals:
    /** Is emitted, when editing has started */
    void editingStarted();

    /** Is emitted, when editing stopped */
    void editingStopped();
>>>>>>> f3842775a5147b7a8da1639a4bec667a769b7fac
=======
>>>>>>> 009ba1b995536ad9936991524ccd0f53a1a5b558

  protected:
    QVariant convertValue( QVariant::Type type, QString value );

    void clearMinMaxCache();
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
    QList< NativeType > mNativeTypes;

    void pushError( QString msg );

    /** Old-style mapping of index to name for QgsPalLabeling fix */
    QgsAttrPalIndexNameHash mAttrPalIndexName;

  private:
    /** old notation **/
    QMap<QString, QVariant::Type> mOldTypeList;

    // list of errors
    QStringList mErrors;

    static QStringList smEncodings;

};


#endif
