/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectordataprovider.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




typedef QHash<int, QString> QgsAttrPalIndexNameHash;



class QgsVectorDataProvider : QgsDataProvider, QgsFeatureSink, QgsFeatureSource
{
%Docstring
This is the base class for vector data providers.

Data providers abstract the retrieval and writing (where supported)
of feature and attribute information from a spatial datasource.
%End

%TypeHeaderCode
#include "qgsvectordataprovider.h"
%End
  public:


    enum Capability
    {
      NoCapabilities,
      AddFeatures,
      DeleteFeatures,
      ChangeAttributeValues,
      AddAttributes,
      DeleteAttributes,
      CreateSpatialIndex,
      SelectAtId,
      ChangeGeometries,
      SelectEncoding,
      CreateAttributeIndex,
      SimplifyGeometries,
      SimplifyGeometriesWithTopologicalValidation,
      TransactionSupport,
      CircularGeometries,
      ChangeFeatures,
      RenameAttributes,
      FastTruncate,
    };

    typedef QFlags<QgsVectorDataProvider::Capability> Capabilities;


    static const int EditingCapabilities;

    enum FeatureCountState
    {
      //! Feature
      Uncounted,
      //! Provider
      UnknownCount,
    };

    QgsVectorDataProvider( const QString &uri = QString() );
%Docstring
Constructor of the vector provider

:param uri:  uniform resource locator (URI) for a dataset
%End

    virtual QgsAbstractFeatureSource *featureSource() const = 0 /Factory/;
%Docstring
Return feature source object that can be used for querying provider's data. The returned feature source
is independent from provider - any changes to provider's state (e.g. change of subset string) will not be
reflected in the feature source, therefore it can be safely used for processing in background without
having to care about possible changes within provider that may happen concurrently. Also, even in the case
of provider being deleted, any feature source obtained from the provider will be kept alive and working
(they are independent and owned by the caller).

Sometimes there are cases when some data needs to be shared between vector data provider and its feature source.
In such cases, the implementation must ensure that the data is not susceptible to run condition. For example,
if it is possible that both feature source and provider may need reading/writing to some shared data at the
same time, some synchronization mechanisms must be used (e.g. mutexes) to prevent data corruption.

:return: new instance of :py:class:`QgsAbstractFeatureSource` (caller is responsible for deleting it)

.. versionadded:: 2.4
%End

    virtual QString storageType() const;
%Docstring
Returns the permanent storage type for this layer as a friendly name.
%End

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const = 0;

%Docstring
Query the provider for features specified in request.

:param request: feature request describing parameters of features to return

:return: iterator for matching features from provider
%End

    virtual QgsWkbTypes::Type wkbType() const = 0;

%Docstring
Returns the geometry type which is returned by this layer
%End

    virtual long featureCount() const = 0;

%Docstring
Number of features in the layer

:return: long containing number of features
%End

    virtual QgsFields fields() const = 0;

%Docstring
Returns the fields associated with this data provider.
%End

    virtual QgsCoordinateReferenceSystem sourceCrs() const;

    virtual QgsRectangle sourceExtent() const;

    virtual QString sourceName() const;

    virtual QString dataComment() const;
%Docstring
Return a short comment for the data that this provider is
providing access to (e.g. the comment for postgres table).
%End

    virtual QVariant minimumValue( int index ) const;

%Docstring
Returns the minimum value of an attribute

:param index: the index of the attribute

Default implementation walks all numeric attributes and caches minimal
and maximal values. If provider has facilities to retrieve minimal
value directly, override this function.
%End

    virtual QVariant maximumValue( int index ) const;

%Docstring
Returns the maximum value of an attribute

:param index: the index of the attribute

Default implementation walks all numeric attributes and caches minimal
and maximal values. If provider has facilities to retrieve maximal
value directly, override this function.
%End

    virtual QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
        QgsFeedback *feedback = 0 ) const;
%Docstring
Returns unique string values of an attribute which contain a specified subset string. Subset
matching is done in a case-insensitive manner.

:param index: the index of the attribute
:param substring: substring to match (case insensitive)
:param limit: maxmum number of the values to return, or -1 to return all unique values
:param feedback: optional feedback object for canceling request

:return: list of unique strings containing substring
%End

    virtual QVariant aggregate( QgsAggregateCalculator::Aggregate aggregate,
                                int index,
                                const QgsAggregateCalculator::AggregateParameters &parameters,
                                QgsExpressionContext *context,
                                bool &ok ) const;
%Docstring
Calculates an aggregated value from the layer's features. The base implementation does nothing,
but subclasses can override this method to handoff calculation of aggregates to the provider.

:param aggregate: aggregate to calculate
:param index: the index of the attribute to calculate aggregate over
:param parameters: parameters controlling aggregate calculation
:param context: expression context for filter
:param ok: will be set to true if calculation was successfully performed by the data provider

:return: calculated aggregate value

.. versionadded:: 2.16
%End

    virtual void enumValues( int index, QStringList &enumList /Out/ ) const;
%Docstring
Returns the possible enum values of an attribute. Returns an empty stringlist if a provider does not support enum types
or if the given attribute is not an enum type.

:param index: the index of the attribute
:param enumList: reference to the list to fill
%End

    virtual bool addFeatures( QgsFeatureList &flist /In,Out/, QgsFeatureSink::Flags flags = 0 );


    virtual bool deleteFeatures( const QgsFeatureIds &id );
%Docstring
Deletes one or more features from the provider. This requires the DeleteFeatures capability.

:param id: list containing feature ids to delete

:return: true in case of success and false in case of failure

.. seealso:: :py:func:`truncate`
%End

    virtual bool truncate();
%Docstring
Removes all features from the layer. This requires either the FastTruncate or DeleteFeatures capability.
Providers with the FastTruncate capability will use an optimised method to truncate the layer.

:return: true in case of success and false in case of failure.

.. versionadded:: 3.0

.. seealso:: :py:func:`deleteFeatures`
%End

    virtual bool addAttributes( const QList<QgsField> &attributes );
%Docstring
Adds new ``attributes`` to the provider. Returns true in case of success and false in case of failure.
If attributes are added using this method then QgsVectorLayer.updateFields() must be called
manually to ensure that the layer's field are correctly reported.
%End

    virtual bool deleteAttributes( const QgsAttributeIds &attributes );
%Docstring
Deletes existing ``attributes`` from the provider.
If attributes are deleted using this method then QgsVectorLayer.updateFields() must be called
manually to ensure that the layer's field are correctly reported.

:param attributes: a set containing indices of attributes

:return: true in case of success and false in case of failure
%End

    virtual bool renameAttributes( const QgsFieldNameMap &renamedAttributes );
%Docstring
Renames existing attributes.
If attributes are renamed using this method then QgsVectorLayer.updateFields() must be called
manually to ensure that the layer's field are correctly reported.

:param renamedAttributes: map of attribute index to new attribute name

:return: true in case of success and false in case of failure

.. versionadded:: 2.16
%End

    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map );
%Docstring
Changes attribute values of existing features. This should
succeed if the provider reports the ChangeAttributeValues capability.

:param attr_map: a map containing changed attributes

:return: true in case of success and false in case of failure
%End

    virtual bool changeFeatures( const QgsChangedAttributesMap &attr_map,
                                 const QgsGeometryMap &geometry_map );
%Docstring
Changes attribute values and geometries of existing features. This should
succeed if the provider reports both the ChangeAttributeValues and
ChangeGeometries capabilities. Providers which report the ChangeFeatures
capability implement an optimised version of this method.

:param attr_map: a map containing changed attributes
:param geometry_map:   A QgsGeometryMap whose index contains the feature IDs
that will have their geometries changed.
The second map parameter being the new geometries themselves

:return: true in case of success and false in case of failure
%End

    virtual QVariant defaultValue( int fieldIndex ) const;
%Docstring
Returns any literal default values which are present at the provider for a specified
field index. Important - this should ONLY be called when creating an attribute to insert
directly into the database. Do not call this method for non-feature creation or modification,
e.g., when validating an attribute or to compare it against an existing value, as calling it
can cause changes to the underlying data source (e.g., Postgres provider where the default value
is calculated as a result of a sequence). It is recommended that you instead use the methods
in QgsVectorLayerUtils such as :py:func:`QgsVectorLayerUtils.createFeature()`
so that default value handling and validation is automatically carried out.

.. seealso:: :py:func:`defaultValueClause`
%End

    virtual QString defaultValueClause( int fieldIndex ) const;
%Docstring
Returns any default value clauses which are present at the provider for a specified
field index. These clauses are usually SQL fragments which must be evaluated by the
provider, e.g., sequence values.

.. seealso:: :py:func:`defaultValue`

.. versionadded:: 3.0
%End

    QgsFieldConstraints::Constraints fieldConstraints( int fieldIndex ) const;
%Docstring
Returns any constraints which are present at the provider for a specified
field index.

.. versionadded:: 3.0

.. seealso:: :py:func:`skipConstraintCheck`
%End

    virtual bool skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value = QVariant() ) const;
%Docstring
Returns true if a constraint check should be skipped for a specified field (e.g., if
the value returned by defaultValue() is trusted implicitly. An optional attribute value can be
passed which can help refine the skip constraint check.

.. versionadded:: 3.0

.. seealso:: :py:func:`fieldConstraints`
%End

    virtual bool changeGeometryValues( const QgsGeometryMap &geometry_map );
%Docstring
Changes geometries of existing features

:param geometry_map:   A QgsGeometryMap whose index contains the feature IDs
that will have their geometries changed.
The second map parameter being the new geometries themselves

:return:               True in case of success and false in case of failure
%End

    virtual bool createSpatialIndex();
%Docstring
Creates a spatial index on the datasource (if supported by the provider type).

:return: true in case of success
%End

    virtual bool createAttributeIndex( int field );
%Docstring
Create an attribute index on the datasource
%End

    virtual QgsVectorDataProvider::Capabilities capabilities() const;
%Docstring
Returns flags containing the supported capabilities
\note, some capabilities may change depending on whether
a spatial filter is active on this provider, so it may
be prudent to check this value per intended operation.
%End

    QString capabilitiesString() const;
%Docstring
Returns the above in friendly format.
%End

    virtual void setEncoding( const QString &e );
%Docstring
Set encoding used for accessing data from layer
%End

    QString encoding() const;
%Docstring
Get encoding which is used for accessing data
%End

    int fieldNameIndex( const QString &fieldName ) const;
%Docstring
Returns the index of a field name or -1 if the field does not exist
%End

    QMap<QString, int> fieldNameMap() const;
%Docstring
Return a map where the key is the name of the field and the value is its index
%End

    virtual QgsAttributeList attributeIndexes() const;
%Docstring
Return list of indexes to fetch all attributes in nextFeature()
%End

    virtual QgsAttributeList pkAttributeIndexes() const;
%Docstring
Return list of indexes of fields that make up the primary key
%End

    virtual QgsAttrPalIndexNameHash palAttributeIndexNames() const;
%Docstring
Return list of indexes to names for QgsPalLabeling fix
%End

    bool supportedType( const QgsField &field ) const;
%Docstring
check if provider supports type of field
%End

    struct NativeType
    {
      NativeType( const QString &typeDesc, const QString &typeName, QVariant::Type type, int minLen = 0, int maxLen = 0, int minPrec = 0, int maxPrec = 0, QVariant::Type subType = QVariant::Invalid );

      QString mTypeDesc;
      QString mTypeName;
      QVariant::Type mType;
      int mMinLen;
      int mMaxLen;
      int mMinPrec;
      int mMaxPrec;
      QVariant::Type mSubType;
    };

    QList< QgsVectorDataProvider::NativeType > nativeTypes() const;
%Docstring
Returns the names of the supported types
%End

    virtual bool doesStrictFeatureTypeCheck() const;
%Docstring
Returns true if the provider is strict about the type of inserted features
(e.g. no multipolygon in a polygon layer)
%End

    static QStringList availableEncodings();
%Docstring
Returns a list of available encodings
%End

    bool hasErrors() const;
%Docstring
Provider has errors to report
%End

    void clearErrors();
%Docstring
Clear recorded errors
%End

    QStringList errors() const;
%Docstring
Get recorded errors
%End

    virtual bool isSaveAndLoadStyleToDatabaseSupported() const;
%Docstring
It returns false by default.
Must be implemented by providers that support saving and loading styles to db returning true
%End

    virtual bool isDeleteStyleFromDatabaseSupported() const;
%Docstring
It returns false by default.
Must be implemented by providers that support delete styles from db returning true
%End

    static QVariant convertValue( QVariant::Type type, const QString &value );

    virtual QgsTransaction *transaction() const;
%Docstring
Returns the transaction this data provider is included in, if any.
%End

    virtual void forceReload();
%Docstring
Forces a reload of the underlying datasource if the provider implements this
method.
In particular on the OGR provider, a pooled connection will be invalidated.
This forces QGIS to reopen a file or connection.
This can be required if the underlying file is replaced.
%End

    virtual QSet<QgsMapLayerDependency> dependencies() const;
%Docstring
Get the list of layer ids on which this layer depends. This in particular determines the order of layer loading.
%End

    virtual QList<QgsRelation> discoverRelations( const QgsVectorLayer *self, const QList<QgsVectorLayer *> &layers ) const;
%Docstring
Discover the available relations with the given layers.

:param self: the layer using this data provider.
:param layers: the other layers.

:return: the list of N-1 relations from this provider.

.. versionadded:: 3.0
%End

    virtual QVariantMap metadata() const;
%Docstring
Get metadata, dependent on the provider type, that will be display in the metadata tab of the layer properties.

:return: The provider metadata
%End

    virtual QString translateMetadataKey( const QString &mdKey ) const;
%Docstring
Get the translated metadata key.

:param mdKey: The metadata key

:return: The translated metadata value
%End

    virtual QString translateMetadataValue( const QString &mdKey, const QVariant &value ) const;
%Docstring
Get the translated metadata value.

:param mdKey: The metadata key
:param value: The metadata value

:return: The translated metadata value
%End

    virtual bool hasMetadata() const;
%Docstring
Returns true if the data source has metadata, false otherwise.

:return: true if data source has metadata, false otherwise.

.. versionadded:: 3.0
%End

  signals:

    void raiseError( const QString &msg ) const;
%Docstring
Signals an error in this provider

.. versionadded:: 3.0
%End

  protected:

    void clearMinMaxCache();
%Docstring
Invalidates the min/max cache. This will force the provider to recalculate the
cache the next time it is requested.
%End

    void fillMinMaxCache() const;
%Docstring
Populates the cache of minimum and maximum attribute values.
%End

    void pushError( const QString &msg ) const;
%Docstring
Push a notification about errors that happened in this providers scope.
Errors should be translated strings that require the users immediate
attention.

For general debug information use QgsMessageLog.logMessage() instead.

.. versionadded:: 3.0
%End

    QgsGeometry convertToProviderType( const QgsGeometry &geom ) const;
%Docstring
Converts the geometry to the provider type if possible / necessary

:return: the converted geometry or None if no conversion was necessary or possible
%End

    void setNativeTypes( const QList<QgsVectorDataProvider::NativeType> &nativeTypes );
%Docstring
Set the list of native types supported by this provider.
Usually done in the constructor.

.. versionadded:: 3.0
%End

    QTextCodec *textEncoding() const;
%Docstring
Get this providers encoding

.. versionadded:: 3.0
%End

};

QFlags<QgsVectorDataProvider::Capability> operator|(QgsVectorDataProvider::Capability f1, QFlags<QgsVectorDataProvider::Capability> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsvectordataprovider.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
