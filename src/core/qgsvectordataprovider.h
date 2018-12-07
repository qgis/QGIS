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

#include "qgis_core.h"
#include <QList>
#include <QSet>
#include <QMap>
#include <QHash>

//QGIS Includes
#include "qgis.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsaggregatecalculator.h"
#include "qgsmaplayerdependency.h"
#include "qgsrelation.h"
#include "qgsfeaturesink.h"
#include "qgsfeaturesource.h"

typedef QList<int> QgsAttributeList SIP_SKIP;
typedef QSet<int> QgsAttributeIds SIP_SKIP;
typedef QHash<int, QString> QgsAttrPalIndexNameHash;

class QgsFeatureIterator;
class QgsTransaction;
class QgsFeedback;
class QgsFeatureRenderer;
class QgsAbstractVectorLayerLabeling;

#include "qgsfeaturerequest.h"

/**
 * \ingroup core
 * This is the base class for vector data providers.
 *
 * Data providers abstract the retrieval and writing (where supported)
 * of feature and attribute information from a spatial datasource.
 *
 *
 */
class CORE_EXPORT QgsVectorDataProvider : public QgsDataProvider, public QgsFeatureSink, public QgsFeatureSource
{
    Q_OBJECT

    friend class QgsTransaction;
    friend class QgsVectorLayerEditBuffer;

  public:

    // If you add to this, please also add to capabilitiesString()

    /**
     * enumeration with capabilities that providers might implement
     */
    enum Capability
    {
      NoCapabilities = 0,       //!< Provider has no capabilities
      AddFeatures = 1,       //!< Allows adding features
      DeleteFeatures = 1 <<  1, //!< Allows deletion of features
      ChangeAttributeValues = 1 <<  2, //!< Allows modification of attribute values
      AddAttributes = 1 <<  3, //!< Allows addition of new attributes (fields)
      DeleteAttributes = 1 <<  4, //!< Allows deletion of attributes (fields)
      CreateSpatialIndex = 1 <<  6, //!< Allows creation of spatial index
      SelectAtId = 1 <<  7, //!< Fast access to features using their ID
      ChangeGeometries = 1 <<  8, //!< Allows modifications of geometries
      SelectEncoding = 1 << 13, //!< Allows user to select encoding
      CreateAttributeIndex = 1 << 12, //!< Can create indexes on provider's fields
      SimplifyGeometries = 1 << 14, //!< Supports simplification of geometries on provider side according to a distance tolerance
      SimplifyGeometriesWithTopologicalValidation = 1 << 15, //!< Supports topological simplification of geometries on provider side according to a distance tolerance
      TransactionSupport = 1 << 16, //!< Supports transactions
      CircularGeometries = 1 << 17, //!< Supports circular geometry types (circularstring, compoundcurve, curvepolygon)
      ChangeFeatures = 1 << 18, /**  Supports joint updates for attributes and geometry
                                                               *  Providers supporting this should still define
                                                               *  ChangeGeometries | ChangeAttributeValues */
      RenameAttributes = 1 << 19, //!< Supports renaming attributes (fields). Since QGIS 2.16
      FastTruncate = 1 << 20, //!< Supports fast truncation of the layer (removing all features). Since QGIS 3.0
      ReadLayerMetadata = 1 << 21, //!< Provider can read layer metadata from data store. Since QGIS 3.0. See QgsDataProvider::layerMetadata()
      WriteLayerMetadata = 1 << 22, //!< Provider can write layer metadata to the data store. Since QGIS 3.0. See QgsDataProvider::writeLayerMetadata()
      CancelSupport = 1 << 23, //!< Supports interruption of pending queries from a separated thread. Since QGIS 3.2
      CreateRenderer = 1 << 24, //!< Provider can create feature renderers using backend-specific formatting information. Since QGIS 3.2. See QgsVectorDataProvider::createRenderer().
      CreateLabeling = 1 << 25, //!< Provider can set labeling settings using backend-specific formatting information. Since QGIS 3.6. See QgsVectorDataProvider::createLabeling().
    };

    Q_DECLARE_FLAGS( Capabilities, Capability )

    //! Bitmask of all provider's editing capabilities
    static const int EditingCapabilities = AddFeatures | DeleteFeatures |
                                           ChangeAttributeValues | ChangeGeometries | AddAttributes | DeleteAttributes |
                                           RenameAttributes;

    /**
     * Enumeration of feature count states
     */
    enum FeatureCountState
    {
      //! Feature count not yet computed
      Uncounted = -2,
      //! Provider returned an unknown feature count
      UnknownCount = -1,
    };

    /**
     * Constructor for a vector data provider.
     *
     * The \a uri argument specifies the uniform resource locator (URI) for the associated dataset.
     *
     * Additional creation options are specified within the \a options value.
     */
    QgsVectorDataProvider( const QString &uri = QString(), const QgsDataProvider::ProviderOptions &options = QgsDataProvider::ProviderOptions() );

    /**
     * Returns feature source object that can be used for querying provider's data. The returned feature source
     * is independent from provider - any changes to provider's state (e.g. change of subset string) will not be
     * reflected in the feature source, therefore it can be safely used for processing in background without
     * having to care about possible changes within provider that may happen concurrently. Also, even in the case
     * of provider being deleted, any feature source obtained from the provider will be kept alive and working
     * (they are independent and owned by the caller).
     *
     * Sometimes there are cases when some data needs to be shared between vector data provider and its feature source.
     * In such cases, the implementation must ensure that the data is not susceptible to run condition. For example,
     * if it is possible that both feature source and provider may need reading/writing to some shared data at the
     * same time, some synchronization mechanisms must be used (e.g. mutexes) to prevent data corruption.
     *
     * \returns new instance of QgsAbstractFeatureSource (caller is responsible for deleting it)
     * \since QGIS 2.4
     */
    virtual QgsAbstractFeatureSource *featureSource() const = 0 SIP_FACTORY;

    /**
     * Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const;

    /**
     * Query the provider for features specified in request.
     * \param request feature request describing parameters of features to return
     * \returns iterator for matching features from provider
     */
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const override = 0;

    /**
     * Returns the geometry type which is returned by this layer
     */
    QgsWkbTypes::Type wkbType() const override = 0;

    /**
     * Number of features in the layer
     * \returns long containing number of features
     */
    long featureCount() const override = 0;

    /**
     * Returns true if the layer contains at least one feature.
     *
     * \since QGIS 3.4
     */
    virtual bool empty() const;

    /**
     * Will always return FeatureAvailability::FeaturesAvailable or
     * FeatureAvailability::NoFeaturesAvailable.
     *
     * Calls empty() internally. Providers should override empty()
     * instead if they provide an optimized version of this call.
     *
     * \see empty()
     * \since QGIS 3.4
     */
    QgsFeatureSource::FeatureAvailability hasFeatures() const override;

    /**
     * Returns the fields associated with this data provider.
     */
    QgsFields fields() const override = 0;

    QgsCoordinateReferenceSystem sourceCrs() const override;
    QgsRectangle sourceExtent() const override;
    QString sourceName() const override { return QString(); }

    /**
     * Returns a short comment for the data that this provider is
     * providing access to (e.g. the comment for postgres table).
     */
    virtual QString dataComment() const;

    /**
     * Returns the minimum value of an attribute
     * \param index the index of the attribute
     *
     * Default implementation walks all numeric attributes and caches minimal
     * and maximal values. If provider has facilities to retrieve minimal
     * value directly, override this function.
     */
    QVariant minimumValue( int index ) const override;

    /**
     * Returns the maximum value of an attribute
     * \param index the index of the attribute
     *
     * Default implementation walks all numeric attributes and caches minimal
     * and maximal values. If provider has facilities to retrieve maximal
     * value directly, override this function.
     */
    QVariant maximumValue( int index ) const override;

    /**
     * Returns unique string values of an attribute which contain a specified subset string. Subset
     * matching is done in a case-insensitive manner.
     * \param index the index of the attribute
     * \param substring substring to match (case insensitive)
     * \param limit maxmum number of the values to return, or -1 to return all unique values
     * \param feedback optional feedback object for canceling request
     * \returns list of unique strings containing substring
     */
    virtual QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
        QgsFeedback *feedback = nullptr ) const;

    /**
     * Calculates an aggregated value from the layer's features. The base implementation does nothing,
     * but subclasses can override this method to handoff calculation of aggregates to the provider.
     * \param aggregate aggregate to calculate
     * \param index the index of the attribute to calculate aggregate over
     * \param parameters parameters controlling aggregate calculation
     * \param context expression context for filter
     * \param ok will be set to true if calculation was successfully performed by the data provider
     * \returns calculated aggregate value
     * \since QGIS 2.16
     */
    virtual QVariant aggregate( QgsAggregateCalculator::Aggregate aggregate,
                                int index,
                                const QgsAggregateCalculator::AggregateParameters &parameters,
                                QgsExpressionContext *context,
                                bool &ok ) const;

    /**
     * Returns the possible enum values of an attribute. Returns an empty stringlist if a provider does not support enum types
     * or if the given attribute is not an enum type.
     * \param index the index of the attribute
     * \param enumList reference to the list to fill
     */
    virtual void enumValues( int index, QStringList &enumList SIP_OUT ) const { Q_UNUSED( index ); enumList.clear(); }

    bool addFeatures( QgsFeatureList &flist SIP_INOUT, QgsFeatureSink::Flags flags = nullptr ) override;

    /**
     * Deletes one or more features from the provider. This requires the DeleteFeatures capability.
     * \param id list containing feature ids to delete
     * \returns true in case of success and false in case of failure
     * \see truncate()
     */
    virtual bool deleteFeatures( const QgsFeatureIds &id );

    /**
     * Removes all features from the layer. This requires either the FastTruncate or DeleteFeatures capability.
     * Providers with the FastTruncate capability will use an optimised method to truncate the layer.
     * \returns true in case of success and false in case of failure.
     * \see deleteFeatures()
     * \since QGIS 3.0
     */
    virtual bool truncate();

    /**
     * Cancels the current reloading of data.
     * \returns true if the reloading has been correctly interrupted, false otherwise
     * \see reloadData()
     * \since QGIS 3.2
     */
    virtual bool cancelReload();

    /**
     * Adds new \a attributes to the provider. Returns true in case of success and false in case of failure.
     * If attributes are added using this method then QgsVectorLayer::updateFields() must be called
     * manually to ensure that the layer's field are correctly reported.
     */
    virtual bool addAttributes( const QList<QgsField> &attributes );

    /**
     * Deletes existing \a attributes from the provider.
     * If attributes are deleted using this method then QgsVectorLayer::updateFields() must be called
     * manually to ensure that the layer's field are correctly reported.
     * \param attributes a set containing indices of attributes
     * \returns true in case of success and false in case of failure
     */
    virtual bool deleteAttributes( const QgsAttributeIds &attributes );

    /**
     * Renames existing attributes.
     * If attributes are renamed using this method then QgsVectorLayer::updateFields() must be called
     * manually to ensure that the layer's field are correctly reported.
     * \param renamedAttributes map of attribute index to new attribute name
     * \returns true in case of success and false in case of failure
     * \since QGIS 2.16
     */
    virtual bool renameAttributes( const QgsFieldNameMap &renamedAttributes );

    /**
     * Changes attribute values of existing features. This should
     * succeed if the provider reports the ChangeAttributeValues capability.
     * \param attr_map a map containing changed attributes
     * \returns true in case of success and false in case of failure
     */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map );

    /**
     * Changes attribute values and geometries of existing features. This should
     * succeed if the provider reports both the ChangeAttributeValues and
     * ChangeGeometries capabilities. Providers which report the ChangeFeatures
     * capability implement an optimised version of this method.
     * \param attr_map a map containing changed attributes
     * \param geometry_map   A QgsGeometryMap whose index contains the feature IDs
     *                       that will have their geometries changed.
     *                       The second map parameter being the new geometries themselves
     * \returns true in case of success and false in case of failure
     */
    virtual bool changeFeatures( const QgsChangedAttributesMap &attr_map,
                                 const QgsGeometryMap &geometry_map );

    /**
     * Returns any literal default values which are present at the provider for a specified
     * field index. Important - this should ONLY be called when creating an attribute to insert
     * directly into the database. Do not call this method for non-feature creation or modification,
     * e.g., when validating an attribute or to compare it against an existing value, as calling it
     * can cause changes to the underlying data source (e.g., Postgres provider where the default value
     * is calculated as a result of a sequence). It is recommended that you instead use the methods
     * in QgsVectorLayerUtils such as QgsVectorLayerUtils::createFeature()
     * so that default value handling and validation is automatically carried out.
     * \see defaultValueClause()
     */
    virtual QVariant defaultValue( int fieldIndex ) const;

    /**
     * Returns any default value clauses which are present at the provider for a specified
     * field index. These clauses are usually SQL fragments which must be evaluated by the
     * provider, e.g., sequence values.
     * \see defaultValue()
     * \since QGIS 3.0
     */
    virtual QString defaultValueClause( int fieldIndex ) const;

    /**
     * Returns any constraints which are present at the provider for a specified
     * field index.
     * \see skipConstraintCheck()
     * \since QGIS 3.0
     */
    QgsFieldConstraints::Constraints fieldConstraints( int fieldIndex ) const;

    /**
     * Returns true if a constraint check should be skipped for a specified field (e.g., if
     * the value returned by defaultValue() is trusted implicitly. An optional attribute value can be
     * passed which can help refine the skip constraint check.
     * \see fieldConstraints()
     * \since QGIS 3.0
     */
    virtual bool skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value = QVariant() ) const;

    /**
     * Changes geometries of existing features
     * \param geometry_map   A QgsGeometryMap whose index contains the feature IDs
     *                       that will have their geometries changed.
     *                       The second map parameter being the new geometries themselves
     * \returns               True in case of success and false in case of failure
     */
    virtual bool changeGeometryValues( const QgsGeometryMap &geometry_map );

    /**
     * Creates a spatial index on the datasource (if supported by the provider type).
     * \returns true in case of success
     */
    virtual bool createSpatialIndex();

    //! Create an attribute index on the datasource
    virtual bool createAttributeIndex( int field );

    /**
     * Returns flags containing the supported capabilities
        \note, some capabilities may change depending on whether
        a spatial filter is active on this provider, so it may
        be prudent to check this value per intended operation.
     */
    virtual QgsVectorDataProvider::Capabilities capabilities() const;

    /**
     *  Returns the above in friendly format.
     */
    QString capabilitiesString() const;

    /**
     * Set encoding used for accessing data from layer
     */
    virtual void setEncoding( const QString &e );

    /**
     * Gets encoding which is used for accessing data
     */
    QString encoding() const;

    /**
     * Returns the index of a field name or -1 if the field does not exist
     */
    int fieldNameIndex( const QString &fieldName ) const;

    /**
     * Returns a map where the key is the name of the field and the value is its index
     */
    QMap<QString, int> fieldNameMap() const;

    /**
     * Returns list of indexes to fetch all attributes in nextFeature()
     */
    virtual QgsAttributeList attributeIndexes() const;

    /**
     * Returns list of indexes of fields that make up the primary key
     */
    virtual QgsAttributeList pkAttributeIndexes() const;

    /**
     * Returns list of indexes to names for QgsPalLabeling fix
     */
    virtual QgsAttrPalIndexNameHash palAttributeIndexNames() const;

    /**
     * check if provider supports type of field
     */
    bool supportedType( const QgsField &field ) const;

    struct NativeType
    {
      NativeType( const QString &typeDesc, const QString &typeName, QVariant::Type type, int minLen = 0, int maxLen = 0, int minPrec = 0, int maxPrec = 0, QVariant::Type subType = QVariant::Invalid )
        : mTypeDesc( typeDesc )
        , mTypeName( typeName )
        , mType( type )
        , mMinLen( minLen )
        , mMaxLen( maxLen )
        , mMinPrec( minPrec )
        , mMaxPrec( maxPrec )
        , mSubType( subType )
      {}

      QString mTypeDesc;
      QString mTypeName;
      QVariant::Type mType;
      int mMinLen;
      int mMaxLen;
      int mMinPrec;
      int mMaxPrec;
      QVariant::Type mSubType;
    };

    /**
     * Returns the names of the supported types
     */
    QList< QgsVectorDataProvider::NativeType > nativeTypes() const;

    /**
     * Returns true if the provider is strict about the type of inserted features
     * (e.g. no multipolygon in a polygon layer)
     */
    virtual bool doesStrictFeatureTypeCheck() const { return true; }

    //! Returns a list of available encodings
    static QStringList availableEncodings();

    /**
     * Provider has errors to report
     */
    bool hasErrors() const;

    /**
     * Clear recorded errors
     */
    void clearErrors();

    /**
     * Gets recorded errors
     */
    QStringList errors() const;

    /**
     * It returns false by default.
     * Must be implemented by providers that support saving and loading styles to db returning true
     */
    virtual bool isSaveAndLoadStyleToDatabaseSupported() const;

    /**
     * It returns false by default.
     * Must be implemented by providers that support delete styles from db returning true
     */
    virtual bool isDeleteStyleFromDatabaseSupported() const;

    /**
     * Creates a new vector layer feature renderer, using provider backend specific information.
     *
     * The \a configuration map can be used to pass provider-specific configuration maps to the provider to
     * allow customization of the returned renderer. Support and format of \a configuration varies by provider.
     *
     * When called with an empty \a configuration map the provider's default renderer will be returned.
     *
     * This method returns a new renderer and the caller takes ownership of the returned object.
     *
     * Only providers which report the CreateRenderer capability will return a feature renderer. Other
     * providers will return nullptr.
     *
     * \since QGIS 3.2
     */
    virtual QgsFeatureRenderer *createRenderer( const QVariantMap &configuration = QVariantMap() ) const SIP_FACTORY;

    /**
     * Creates labeling settings, using provider backend specific information.
     *
     * The \a configuration map can be used to pass provider-specific configuration maps to the provider to
     * allow customization of the returned labeling object. Support and format of \a configuration varies by provider.
     *
     * When called with an empty \a configuration map the provider's default labeling settings will be returned.
     *
     * This method returns a new labeling settings and the caller takes ownership of the returned object.
     *
     * Only providers which report the CreateLabeling capability will return labeling settings. Other
     * providers will return nullptr.
     *
     * \since QGIS 3.6
     */
    virtual QgsAbstractVectorLayerLabeling *createLabeling( const QVariantMap &configuration = QVariantMap() ) const SIP_FACTORY;

    static QVariant convertValue( QVariant::Type type, const QString &value );

    /**
     * Returns the transaction this data provider is included in, if any.
     */
    virtual QgsTransaction *transaction() const;

    /**
     * Forces a reload of the underlying datasource if the provider implements this
     * method.
     * In particular on the OGR provider, a pooled connection will be invalidated.
     * This forces QGIS to reopen a file or connection.
     * This can be required if the underlying file is replaced.
     */
    virtual void forceReload();

    /**
     * Gets the list of layer ids on which this layer depends. This in particular determines the order of layer loading.
     */
    virtual QSet<QgsMapLayerDependency> dependencies() const;

    /**
     * Discover the available relations with the given layers.
     * \param self the layer using this data provider.
     * \param layers the other layers.
     * \returns the list of N-1 relations from this provider.
     * \since QGIS 3.0
     */
    virtual QList<QgsRelation> discoverRelations( const QgsVectorLayer *self, const QList<QgsVectorLayer *> &layers ) const;

    /**
     * Gets metadata, dependent on the provider type, that will be display in the metadata tab of the layer properties.
     * \returns The provider metadata
     */
    virtual QVariantMap metadata() const { return QVariantMap(); }

    /**
     * Gets the translated metadata key.
     * \param mdKey The metadata key
     * \returns The translated metadata value
     */
    virtual QString translateMetadataKey( const QString &mdKey ) const { return mdKey; }

    /**
     * Gets the translated metadata value.
     * \param mdKey The metadata key
     * \param value The metadata value
     * \returns The translated metadata value
     */
    virtual QString translateMetadataValue( const QString &mdKey, const QVariant &value ) const { Q_UNUSED( mdKey ); return value.toString(); }

    /**
     * Returns true if the data source has metadata, false otherwise.
     *
     * \returns true if data source has metadata, false otherwise.
     *
     * \since QGIS 3.0
     */
    virtual bool hasMetadata() const { return true; }

  signals:

    /**
     * Signals an error in this provider
     *
     * \since QGIS 3.0
     */
    void raiseError( const QString &msg ) const;

  protected:

    /**
     * Invalidates the min/max cache. This will force the provider to recalculate the
     * cache the next time it is requested.
     */
    void clearMinMaxCache();

    /**
     * Populates the cache of minimum and maximum attribute values.
     */
    void fillMinMaxCache() const;

    /**
     * Push a notification about errors that happened in this providers scope.
     * Errors should be translated strings that require the users immediate
     * attention.
     *
     * For general debug information use QgsMessageLog::logMessage() instead.
     *
     * \since QGIS 3.0
     */
    void pushError( const QString &msg ) const;

    /**
     * Converts the geometry to the provider type if possible / necessary
     * \returns the converted geometry or nullptr if no conversion was necessary or possible
     */
    QgsGeometry convertToProviderType( const QgsGeometry &geom ) const;

    /**
     * Set the list of native types supported by this provider.
     * Usually done in the constructor.
     *
     * \since QGIS 3.0
     */
    void setNativeTypes( const QList<QgsVectorDataProvider::NativeType> &nativeTypes );

    /**
     * Gets this providers encoding
     *
     * \since QGIS 3.0
     */
    QTextCodec *textEncoding() const;

  private:
    mutable bool mCacheMinMaxDirty = true;
    mutable QMap<int, QVariant> mCacheMinValues, mCacheMaxValues;

    //! Encoding
    QTextCodec *mEncoding = nullptr;

    //! List of attribute indices to fetch with nextFeature calls
    QgsAttributeList mAttributesToFetch;

    //! The names of the providers native types
    QList< NativeType > mNativeTypes;

    //! List of errors
    mutable QStringList mErrors;

    static QStringList sEncodings;

    /**
     * Includes this data provider in the specified transaction. Ownership of transaction is not transferred.
     */
    virtual void setTransaction( QgsTransaction * /*transaction*/ ) {}

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsVectorDataProvider::Capabilities )

#endif
