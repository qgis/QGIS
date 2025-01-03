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
#include "qgis_sip.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsaggregatecalculator.h"
#include "qgsmaplayerdependency.h"
#include "qgsrelation.h"
#include "qgsfeaturesink.h"
#include "qgsfeaturesource.h"
#include "qgsfeaturerequest.h"
#include "qgsvectordataprovidertemporalcapabilities.h"

typedef QList<int> QgsAttributeList SIP_SKIP;
typedef QSet<int> QgsAttributeIds SIP_SKIP;
typedef QHash<int, QString> QgsAttrPalIndexNameHash;

class QgsFeatureIterator;
class QgsTransaction;
class QgsFeedback;
class QgsFeatureRenderer;
class QgsAbstractVectorLayerLabeling;


/**
 * \ingroup core
 * \brief This is the base class for vector data providers.
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

    //! Bitmask of all provider's editing capabilities
    static const int EditingCapabilities = static_cast< int >( Qgis::VectorProviderCapability::EditingCapabilities );

    /**
     * Constructor for a vector data provider.
     *
     * The \a uri argument specifies the uniform resource locator (URI) for the associated dataset.
     *
     * Additional creation options are specified within the \a options value and since QGIS 3.16 creation flags are specified within the \a flags value.
     */
    QgsVectorDataProvider( const QString &uri = QString(),
                           const QgsDataProvider::ProviderOptions &providerOptions = QgsDataProvider::ProviderOptions(),
                           Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );

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
    Qgis::WkbType wkbType() const override = 0;

    /**
     * Number of features in the layer
     * \returns number of features
     */
    long long featureCount() const override = 0;

    /**
     * Returns TRUE if the layer does not contain any feature.
     *
     * \since QGIS 3.4
     */
    virtual bool empty() const;

    /**
     * Returns TRUE if the layer is a query (SQL) layer.
     *
     * \note this is simply a shortcut to check if the SqlQuery flag
     *       is set.
     *
     *\see vectorLayerTypeFlags()
     * \since QGIS 3.24
     */
    virtual bool isSqlQuery() const;

    /**
     * Returns the vector layer type flags.
     *
     * \see isSqlQuery()
     * \since QGIS 3.24
     */
    virtual Qgis::VectorLayerTypeFlags vectorLayerTypeFlags() const;

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
    Qgis::FeatureAvailability hasFeatures() const override;

    /**
     * Returns the fields associated with this data provider.
     */
    QgsFields fields() const override = 0;

    QgsCoordinateReferenceSystem sourceCrs() const override;
    QgsRectangle sourceExtent() const override;
    QgsBox3D sourceExtent3D() const override;
    QString sourceName() const override { return QString(); }

    /**
     * Returns a short comment for the data that this provider is
     * providing access to (e.g. the comment for postgres table).
     */
    virtual QString dataComment() const override;

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
     * \param ok will be set to TRUE if calculation was successfully performed by the data provider
     * \param fids list of fids to filter, otherwise will use all fids
     * \returns calculated aggregate value
     */
    virtual QVariant aggregate( Qgis::Aggregate aggregate,
                                int index,
                                const QgsAggregateCalculator::AggregateParameters &parameters,
                                QgsExpressionContext *context,
                                bool &ok,
                                QgsFeatureIds *fids = nullptr ) const;

    /**
     * Returns the possible enum values of an attribute. Returns an empty stringlist if a provider does not support enum types
     * or if the given attribute is not an enum type.
     * \param index the index of the attribute
     * \param enumList reference to the list to fill
     */
    virtual void enumValues( int index, QStringList &enumList SIP_OUT ) const { Q_UNUSED( index ) enumList.clear(); }

    bool addFeatures( QgsFeatureList &flist SIP_INOUT, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    QString lastError() const override;

    /**
     * Deletes one or more features from the provider. This requires the DeleteFeatures capability.
     * \param id list containing feature ids to delete
     * \returns TRUE in case of success and FALSE in case of failure
     * \see truncate()
     */
    virtual bool deleteFeatures( const QgsFeatureIds &id );

    /**
     * Removes all features from the layer. This requires either the FastTruncate or DeleteFeatures capability.
     * Providers with the FastTruncate capability will use an optimised method to truncate the layer.
     * \returns TRUE in case of success and FALSE in case of failure.
     * \see deleteFeatures()
     */
    virtual bool truncate();

    /**
     * Cancels the current reloading of data.
     * \returns TRUE if the reloading has been correctly interrupted, FALSE otherwise
     * \see reloadData()
     * \since QGIS 3.2
     */
    virtual bool cancelReload();

    /**
     * Adds new \a attributes to the provider. Returns TRUE in case of success and FALSE in case of failure.
     * If attributes are added using this method then QgsVectorLayer::updateFields() must be called
     * manually to ensure that the layer's field are correctly reported.
     */
    virtual bool addAttributes( const QList<QgsField> &attributes );

    /**
     * Deletes existing \a attributes from the provider.
     * If attributes are deleted using this method then QgsVectorLayer::updateFields() must be called
     * manually to ensure that the layer's field are correctly reported.
     * \param attributes a set containing indices of attributes
     * \returns TRUE in case of success and FALSE in case of failure
     */
    virtual bool deleteAttributes( const QgsAttributeIds &attributes );

    /**
     * Renames existing attributes.
     * If attributes are renamed using this method then QgsVectorLayer::updateFields() must be called
     * manually to ensure that the layer's field are correctly reported.
     * \param renamedAttributes map of attribute index to new attribute name
     * \returns TRUE in case of success and FALSE in case of failure
     */
    virtual bool renameAttributes( const QgsFieldNameMap &renamedAttributes );

    /**
     * Changes attribute values of existing features. This should
     * succeed if the provider reports the ChangeAttributeValues capability.
     * The method returns FALSE if the provider does not have ChangeAttributeValues
     * capability or if any of the changes could not be successfully applied.
     * \param attr_map a map containing changed attributes
     * \returns TRUE in case of success and FALSE in case of failure
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
     * \returns TRUE in case of success and FALSE in case of failure
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
     */
    virtual QString defaultValueClause( int fieldIndex ) const;

    /**
     * Returns any constraints which are present at the provider for a specified
     * field index.
     * \see skipConstraintCheck()
     */
    QgsFieldConstraints::Constraints fieldConstraints( int fieldIndex ) const;

    /**
     * Returns TRUE if a constraint check should be skipped for a specified field (e.g., if
     * the value returned by defaultValue() is trusted implicitly. An optional attribute value can be
     * passed which can help refine the skip constraint check.
     * \see fieldConstraints()
     */
    virtual bool skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value = QVariant() ) const;

    /**
     * Changes geometries of existing features
     * \param geometry_map   A QgsGeometryMap whose index contains the feature IDs
     *                       that will have their geometries changed.
     *                       The second map parameter being the new geometries themselves
     * \returns               TRUE in case of success and FALSE in case of failure
     */
    virtual bool changeGeometryValues( const QgsGeometryMap &geometry_map );

    /**
     * Creates a spatial index on the datasource (if supported by the provider type).
     * \returns TRUE in case of success
     */
    virtual bool createSpatialIndex();

    //! Create an attribute index on the datasource
    virtual bool createAttributeIndex( int field );

    /**
     * Returns flags containing the supported capabilities
     * \note, some capabilities may change depending on whether
     * a spatial filter is active on this provider, so it may
     * be prudent to check this value per intended operation.
     *
     * \see attributeEditCapabilities()
     */
    Q_INVOKABLE virtual Qgis::VectorProviderCapabilities capabilities() const;

    /**
     *  Returns the above in friendly format.
     */
    QString capabilitiesString() const;

    /**
     * Returns the provider's supported attribute editing capabilities.
     *
     * \see capabilities()
     * \since QGIS 3.32
     */
    virtual Qgis::VectorDataProviderAttributeEditCapabilities attributeEditCapabilities() const;

    /**
     * Set encoding used for accessing data from layer.
     *
     * An empty encoding string indicates that the provider should automatically
     * select the most appropriate encoding for the data source.
     *
     * \warning Support for setting the provider encoding depends on the underlying data
     * provider. Check capabilities() for the QgsVectorDataProvider::SelectEncoding
     * capability in order to determine if the provider supports this ability.
     *
     * \see encoding()
     */
    virtual void setEncoding( const QString &e );

    /**
     * Returns the encoding which is used for accessing data.
     *
     * \see setEncoding()
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
     * Returns the name of the column storing geometry, if applicable.
     *
     * \since QGIS 3.42
     */
    virtual QString geometryColumnName() const;

    /**
     * Returns list of indexes to names for QgsPalLabeling fix
     *
     * \deprecated QGIS 3.32. This method is unused and will always return an empty hash.
     */
    Q_DECL_DEPRECATED QgsAttrPalIndexNameHash palAttributeIndexNames() const SIP_DEPRECATED;

    /**
     * check if provider supports type of field
     */
    bool supportedType( const QgsField &field ) const;

    struct NativeType
    {
      NativeType( const QString &typeDesc, const QString &typeName, QMetaType::Type type, int minLen = 0, int maxLen = 0, int minPrec = 0, int maxPrec = 0, QMetaType::Type subType = QMetaType::Type::UnknownType )
        : mTypeDesc( typeDesc )
        , mTypeName( typeName )
        , mType( type )
        , mMinLen( minLen )
        , mMaxLen( maxLen )
        , mMinPrec( minPrec )
        , mMaxPrec( maxPrec )
        , mSubType( subType )
      {}

      Q_DECL_DEPRECATED NativeType( const QString &typeDesc, const QString &typeName, QVariant::Type type, int minLen = 0, int maxLen = 0, int minPrec = 0, int maxPrec = 0, QVariant::Type subType = QVariant::Type::Invalid )
        : mTypeDesc( typeDesc )
        , mTypeName( typeName )
        , mType( QgsVariantUtils::variantTypeToMetaType( type ) )
        , mMinLen( minLen )
        , mMaxLen( maxLen )
        , mMinPrec( minPrec )
        , mMaxPrec( maxPrec )
        , mSubType( QgsVariantUtils::variantTypeToMetaType( subType ) ) SIP_DEPRECATED
          {}


          QString mTypeDesc;
      QString mTypeName;
      QMetaType::Type mType;
      int mMinLen;
      int mMaxLen;
      int mMinPrec;
      int mMaxPrec;
      QMetaType::Type mSubType;
    };

    /**
     * Returns the names of the supported types
     */
    QList< QgsVectorDataProvider::NativeType > nativeTypes() const;

    /**
     * Returns TRUE if the provider is strict about the type of inserted features
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
     * providers will return NULLPTR.
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
     * providers will return NULLPTR.
     *
     * \since QGIS 3.6
     */
    virtual QgsAbstractVectorLayerLabeling *createLabeling( const QVariantMap &configuration = QVariantMap() ) const SIP_FACTORY;

    /**
     * Convert \a value to \a type
     */
    static QVariant convertValue( QMetaType::Type type, const QString &value );

    /**
     * Convert \a value to \a type
     *
     * \deprecated QGIS 3.38. Use the method with a QMetaType::Type argument instead.
     */
    Q_DECL_DEPRECATED static QVariant convertValue( QVariant::Type type, const QString &value ) SIP_DEPRECATED;

    /**
     * Returns the transaction this data provider is included in, if any.
     */
    virtual QgsTransaction *transaction() const;

    /**
     * \deprecated QGIS 3.12. Will be removed in QGIS 4.0 - use reloadData() instead.
     */
    Q_DECL_DEPRECATED virtual void forceReload() SIP_DEPRECATED { reloadData(); }

    /**
     * Gets the list of layer ids on which this layer depends. This in particular determines the order of layer loading.
     */
    virtual QSet<QgsMapLayerDependency> dependencies() const;

    /**
     * Discover the available relations with the given layers.
     * \param target the layer using this data provider.
     * \param layers the other layers.
     * \returns the list of N-1 relations from this provider.
     */
    virtual QList<QgsRelation> discoverRelations( const QgsVectorLayer *target, const QList<QgsVectorLayer *> &layers ) const;

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
    virtual QString translateMetadataValue( const QString &mdKey, const QVariant &value ) const { Q_UNUSED( mdKey ) return value.toString(); }

    /**
     * Returns TRUE if the data source has metadata, FALSE otherwise.
     *
     * \returns TRUE if data source has metadata, FALSE otherwise.
     *
     */
    virtual bool hasMetadata() const { return true; }

    /**
     * Handles any post-clone operations required after this vector data provider was cloned
     * from the \a source provider.
     *
     * \since QGIS 3.8.1
     */
    virtual void handlePostCloneOperations( QgsVectorDataProvider *source );

    QgsVectorDataProviderTemporalCapabilities *temporalCapabilities() override;
    const QgsVectorDataProviderTemporalCapabilities *temporalCapabilities() const override SIP_SKIP;

    QgsDataProviderElevationProperties *elevationProperties() override;
    const QgsDataProviderElevationProperties *elevationProperties() const override SIP_SKIP;

  signals:

    /**
     * Signals an error in this provider
     *
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
     */
    void pushError( const QString &msg ) const;

    /**
     * Converts the geometry to the provider type if possible / necessary
     * \returns the converted geometry or NULLPTR if no conversion was necessary or possible
     * \note The default implementation simply calls the static version of this function.
     */
    QgsGeometry convertToProviderType( const QgsGeometry &geom ) const;

    /**
     * Set the list of native types supported by this provider.
     * Usually done in the constructor.
     *
     */
    void setNativeTypes( const QList<QgsVectorDataProvider::NativeType> &nativeTypes );

#ifdef SIP_PYQT5_RUN

    /**
     * Gets this providers encoding
     *
     */
    QTextCodec *textEncoding() const;
#endif

    /**
     * Converts the \a geometry to the provider geometry type \a providerGeometryType if possible / necessary
     * \returns the converted geometry or NULLPTR if no conversion was necessary or possible
     * \since QGIS 3.34
     */
    static QgsGeometry convertToProviderType( const QgsGeometry &geometry,  Qgis::WkbType providerGeometryType );


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

    std::unique_ptr< QgsVectorDataProviderTemporalCapabilities > mTemporalCapabilities;
    std::unique_ptr< QgsDataProviderElevationProperties > mElevationProperties;

    static QStringList sEncodings;

    /**
     * Includes this data provider in the specified transaction. Ownership of transaction is not transferred.
     */
    virtual void setTransaction( QgsTransaction * /*transaction*/ ) {}
};

#endif
