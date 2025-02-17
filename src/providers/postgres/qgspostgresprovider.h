/***************************************************************************
      qgspostgresprovider.h  -  Data provider for PostgreSQL/PostGIS layers
                             -------------------
    begin                : Jan 2, 2004
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOSTGRESPROVIDER_H
#define QGSPOSTGRESPROVIDER_H

#include "qgsvectordataprovider.h"
#include "qgsrectangle.h"
#include "qgspostgresconn.h"
#include "qgsfields.h"
#include "qgsprovidermetadata.h"
#include "qgsreferencedgeometry.h"
#include <memory>
#include <optional>
#include "qgspostgresutils.h"

class QgsFeature;
class QgsField;
class QgsGeometry;

class QgsPostgresFeatureIterator;
class QgsPostgresSharedData;
class QgsPostgresTransaction;
class QgsPostgresListener;

#include "qgsdatasourceuri.h"

/**
 * \class QgsPostgresProvider
 * \brief Data provider for PostgreSQL/PostGIS layers.
 *
 * This provider implements the
 * interface defined in the QgsDataProvider class to provide access to spatial
 * data residing in a PostgreSQL/PostGIS enabled database.
  */
class QgsPostgresProvider final : public QgsVectorDataProvider
{
    Q_OBJECT

    friend class TestQgsPostgresProvider;

  public:
    static const QString POSTGRES_KEY;
    static const QString POSTGRES_DESCRIPTION;

    /**
     * Import a vector layer into the database
     * \param options options for provider, specified via a map of option name
     * to value. Valid options are lowercaseFieldNames (set to true to convert
     * field names to lowercase), dropStringConstraints (set to true to remove
     * length constraints on character fields).
     */
    static Qgis::VectorExportResult createEmptyLayer(
      const QString &uri,
      const QgsFields &fields,
      Qgis::WkbType wkbType,
      const QgsCoordinateReferenceSystem &srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = nullptr,
      const QMap<QString, QVariant> *options = nullptr
    );

    /**
     * Constructor for the provider. The uri must be in the following format:
     * host=localhost dbname=test [user=gsherman [password=xxx] | authcfg=xxx] table=test.alaska (the_geom)
     * \param uri String containing the required parameters to connect to the database
     * and query the table.
     * \param options generic data provider options
     * \param flags generic data provider flags
     */
    explicit QgsPostgresProvider( QString const &uri, const QgsDataProvider::ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );


    ~QgsPostgresProvider() override;

    QgsAbstractFeatureSource *featureSource() const override;
    QString storageType() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    Qgis::WkbType wkbType() const override;
    QgsLayerMetadata layerMetadata() const override;

    /**
     * Returns the number of layers for the current data source
     * \note Should this be subLayerCount() instead?
     */
    size_t layerCount() const;

    long long featureCount() const override;

    /**
     * Determines if there is at least one feature available on this table.
     *
     * \note In contrast to the featureCount() method, this method is not
     *       affected by estimated metadata.
     *
     * \since QGIS 3.4
     */
    bool empty() const override;

    /**
     * Returns a string representation of the endian-ness for the layer
     */
    static QString endianString();

    /**
     * Returns a list of unquoted column names from an uri key
     */
    static QStringList parseUriKey( const QString &key );

    /**
     * Changes the stored extent for this layer to the supplied extent.
     * For example, this is called when the extent worker thread has a result.
     */
    void setExtent( const QgsRectangle &newExtent );

    QgsRectangle extent() const override;
    QgsBox3D extent3D() const override;
    void updateExtents() override;

    /**
     * Determine the fields making up the primary key
     */
    bool determinePrimaryKey();

    /**
     * Determine the fields making up the primary key from the uri attribute keyColumn
     *
     * Fills mPrimaryKeyType and mPrimaryKeyAttrs
     * from mUri
     */
    void determinePrimaryKeyFromUriKeyColumn();

    QgsFields fields() const override;
    QString dataComment() const override;
    QVariant minimumValue( int index ) const override;
    QVariant maximumValue( int index ) const override;
    QSet<QVariant> uniqueValues( int index, int limit = -1 ) const override;
    QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1, QgsFeedback *feedback = nullptr ) const override;
    void enumValues( int index, QStringList &enumList ) const override;
    bool isValid() const override;
    Qgis::ProviderStyleStorageCapabilities styleStorageCapabilities() const override;
    QgsAttributeList attributeIndexes() const override;
    QgsAttributeList pkAttributeIndexes() const override { return mPrimaryKeyAttrs; }
    QString geometryColumnName() const override;
    QString defaultValueClause( int fieldId ) const override;
    QVariant defaultValue( int fieldId ) const override;
    bool skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value = QVariant() ) const override;
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool deleteFeatures( const QgsFeatureIds &id ) override;
    bool truncate() override;
    bool addAttributes( const QList<QgsField> &attributes ) override;
    bool deleteAttributes( const QgsAttributeIds &name ) override;
    bool renameAttributes( const QgsFieldNameMap &renamedAttributes ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    bool changeFeatures( const QgsChangedAttributesMap &attr_map, const QgsGeometryMap &geometry_map ) override;

    //! Gets the postgres connection
    PGconn *pgConnection();

    //! Gets the table name associated with this provider instance
    QString getTableName();

    QString subsetString() const override;
    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
    bool supportsSubsetString() const override;
    QString subsetStringDialect() const override;
    QString subsetStringHelpUrl() const override;
    Qgis::VectorProviderCapabilities capabilities() const override;
    Qgis::VectorDataProviderAttributeEditCapabilities attributeEditCapabilities() const override;
    Qgis::SpatialIndexPresence hasSpatialIndex() const override;

    /**
     * The Postgres provider does its own transforms so we return
     * true for the following three functions to indicate that transforms
     * should not be handled by the QgsCoordinateTransform object. See the
     * documentation on QgsVectorDataProvider for details on these functions.
     */
    // XXX For now we have disabled native transforms in the PG provider since
    //     it appears there are problems with some of the projection definitions
    bool supportsNativeTransform() { return false; }

    QString name() const override;
    QString description() const override;
    QgsTransaction *transaction() const override;
    static QString providerKey();

    /**
     * Convert the postgres string representation into the given QVariant type.
     * \param type the wanted type
     * \param subType if type is a collection, the wanted element type
     * \param value the value to convert
     * \returns a QVariant of the given type or a null QVariant
     */
    QVariant convertValue( QMetaType::Type type, QMetaType::Type subType, const QString &value, const QString &typeName ) const;
    static QVariant convertValue( QMetaType::Type type, QMetaType::Type subType, const QString &value, const QString &typeName, QgsPostgresConn *conn );

    QList<QgsRelation> discoverRelations( const QgsVectorLayer *target, const QList<QgsVectorLayer *> &layers ) const override;

    /**
     * Returns true if the data source has metadata, false otherwise. For
     * example, if the kind of relation for the layer is a view or a
     * materialized view, then no metadata are associated with the data
     * source.
     *
     * \returns true if data source has metadata, false otherwise.
     *
     */
    bool hasMetadata() const override;

    /**
     * Launch a listening thead to listen to postgres NOTIFY on "qgis" channel
     * the notification is transformed into a Qt signal.
     *
     */
    void setListening( bool isListening ) override;

    Qgis::VectorLayerTypeFlags vectorLayerTypeFlags() const override;

    void handlePostCloneOperations( QgsVectorDataProvider *source ) override;

  private:
    /**
     * \returns relation kind
     */
    Qgis::PostgresRelKind relkind() const;

    /**
     * Change internal query with \a query
     */
    void setQuery( const QString &query );

    bool declareCursor( const QString &cursorName, const QgsAttributeList &fetchAttributes, bool fetchGeometry, QString whereClause );

    bool getFeature( QgsPostgresResult &queryResult, int row, bool fetchGeometry, QgsFeature &feature, const QgsAttributeList &fetchAttributes );

    QString geomParam( int offset ) const;


    static QString getNextString( const QString &txt, int &i, const QString &sep );
    static QVariant parseHstore( const QString &txt );
    static QVariant parseJson( const QString &txt );
    static QVariant parseOtherArray( const QString &txt, QMetaType::Type subType, const QString &typeName, QgsPostgresConn *conn );
    static QVariant parseStringArray( const QString &txt );
    static QVariant parseMultidimensionalArray( const QString &txt );
    static QVariant parseArray( const QString &txt, QMetaType::Type type, QMetaType::Type subType, const QString &typeName, QgsPostgresConn *conn );


    /**
     * Gets parametrized primary key clause
     * \param offset specifies offset to use for the pk value parameter
     * \param alias specifies an optional alias given to the subject table
     */
    QString pkParamWhereClause( int offset, const char *alias = nullptr ) const;
    QString whereClause( QgsFeatureId featureId ) const;
    QString whereClause( QgsFeatureIds featureIds ) const;
    QString filterWhereClause() const;

    bool hasSufficientPermsAndCapabilities();

    QgsField field( int index ) const;

    /**
     * Load the field list
     */
    bool loadFields();

    /**
     * Set the default widget type for the fields
     */
    void setEditorWidgets();

    //! Convert a QgsField to work with PG
    static bool convertField( QgsField &field, const QMap<QString, QVariant> *coordinateTransformContext = nullptr );

    /**
     * Parses the enum_range of an attribute and inserts the possible values into a stringlist
     * \param enumValues the stringlist where the values are appended
     * \param attributeName the name of the enum attribute
     * \returns true in case of success and fals in case of error (e.g. if the type is not an enum type)
    */
    bool parseEnumRange( QStringList &enumValues, const QString &attributeName ) const;

    /**
     * Parses the possible enum values of a domain type (given in the check constraint of the domain type)
     * \param enumValues Reference to list that receives enum values
     * \param attributeName Name of the domain type attribute
     * \returns true in case of success and false in case of error (e.g. if the attribute is not a domain type or does not have a check constraint)
     */
    bool parseDomainCheckConstraint( QStringList &enumValues, const QString &attributeName ) const;

    /**
     * Returns the type of primary key for a PK field
     *
     * \param fld the field to determine PK type of
     * \returns the PrimaryKeyType
     *
     * \note that this only makes sense for single-field primary keys,
     *       whereas multi-field keys always need the PktFidMap
     *       primary key type.
     */
    QgsPostgresPrimaryKeyType pkType( const QgsField &fld ) const;

    /**
     * Search all the layers using the given table.
     */
    static QList<QgsVectorLayer *> searchLayers( const QList<QgsVectorLayer *> &layers, const QString &connectionInfo, const QString &schema, const QString &tableName );

    /**
     * Effect a reload including resetting the feature count
     * and setting the layer extent to minimal
     *
     * \since QGIS 3.12
    */
    void reloadProviderData() override;

    QgsFields mAttributeFields;
    QHash<int, char> mIdentityFields;
    QString mDataComment;

    //! Data source URI struct for this layer
    QgsDataSourceUri mUri;

    /**
     * Flag indicating if the layer data source is a valid PostgreSQL layer
     */
    bool mValid = false;

    /**
     * provider references query (instead of a table)
     */
    bool mIsQuery;

    /**
     * Name of the table with no schema
     */
    QString mTableName;

    /**
     * Name of the table or subquery
     */
    QString mQuery;

    /**
     * Name of the schema
     */
    QString mSchemaName;

    /**
     * SQL statement used to limit the features retrieved
     */
    QString mSqlWhereClause;

    /**
     * Kind of relation
     */
    mutable Qgis::PostgresRelKind mKind = Qgis::PostgresRelKind::NotSet;

    /**
     * Data type for the primary key
     */
    QgsPostgresPrimaryKeyType mPrimaryKeyType = PktUnknown;

    /**
     * Data type for the spatial column
     */
    QgsPostgresGeometryColumnType mSpatialColType = SctNone;

    /**
     * List of primary key attributes for fetching features.
     */
    QList<int> mPrimaryKeyAttrs;
    QString mPrimaryKeyDefault;

    QString mGeometryColumn;                      //!< Name of the geometry column
    QString mBoundingBoxColumn;                   //!< Name of the bounding box column
    mutable std::optional<QgsBox3D> mLayerExtent; //!< QgsBox3D that contains the extent (bounding box) of the layer

    Qgis::WkbType mDetectedGeomType = Qgis::WkbType::Unknown;  //!< Geometry type detected in the database
    Qgis::WkbType mRequestedGeomType = Qgis::WkbType::Unknown; //!< Geometry type requested in the uri
    QString mDetectedSrid;                                     //!< Spatial reference detected in the database
    QString mRequestedSrid;                                    //!< Spatial reference requested in the uri

    std::shared_ptr<QgsPostgresSharedData> mShared; //!< Mutable data shared between provider and feature sources

    bool getGeometryDetails();

    //! @{ Only used with TopoGeometry layers

    struct TopoLayerInfo
    {
        QString topologyName;
        long layerId;
        int layerLevel;
        enum TopoFeatureType
        {
          Puntal = 1,
          Lineal = 2,
          Polygonal = 3,
          Mixed = 4
        } featureType;
    };

    TopoLayerInfo mTopoLayerInfo;

    bool getTopoLayerInfo();

    void dropOrphanedTopoGeoms();

    //! @}

    /* Use estimated metadata. Uses fast table counts, geometry type and extent determination */
    bool mUseEstimatedMetadata = false;

    bool mSelectAtIdDisabled = false; //!< Disable support for SelectAtId

    struct PGFieldNotFound
    {}; //! Exception to throw

    // A function that determines if the given columns contain unique entries
    bool uniqueData( const QString &quotedColNames );

    Qgis::VectorProviderCapabilities mEnabledCapabilities;

    void appendGeomParam( const QgsGeometry &geom, QStringList &param ) const;
    void appendPkParams( QgsFeatureId fid, QStringList &param ) const;

    QString paramValue( const QString &fieldvalue, const QString &defaultValue ) const;

    mutable QgsPostgresConn *mConnectionRO = nullptr; //!< Read-only database connection (initially)
    QgsPostgresConn *mConnectionRW = nullptr;         //!< Read-write database connection (on update)

    QgsPostgresConn *connectionRO() const;
    QgsPostgresConn *connectionRW();

    void disconnectDb();

    static QString quotedIdentifier( const QString &ident ) { return QgsPostgresConn::quotedIdentifier( ident ); }
    static QString quotedValue( const QVariant &value ) { return QgsPostgresConn::quotedValue( value ); }
    static QString quotedJsonValue( const QVariant &value ) { return QgsPostgresConn::quotedJsonValue( value ); }
    static QString quotedByteaValue( const QVariant &value );

    friend class QgsPostgresFeatureSource;

    QgsPostgresTransaction *mTransaction = nullptr;

    void setTransaction( QgsTransaction *transaction ) override;

    QHash<int, QString> mDefaultValues;

    // for handling generated columns, available in PostgreSQL 12+
    // See https://www.postgresql.org/docs/12/ddl-generated-columns.html
    QHash<int, QString> mGeneratedValues;

    bool mCheckPrimaryKeyUnicity = true;

    QgsLayerMetadata mLayerMetadata;

    std::unique_ptr<QgsPostgresListener> mListener;

    static QgsReferencedGeometry fromEwkt( const QString &ewkt, QgsPostgresConn *conn );
    static QString toEwkt( const QgsReferencedGeometry &geom, QgsPostgresConn *conn );
    static QString geomAttrToString( const QVariant &attr, QgsPostgresConn *conn );
    static int crsToSrid( const QgsCoordinateReferenceSystem &crs, QgsPostgresConn *conn );
    static QgsCoordinateReferenceSystem sridToCrs( int srsId, QgsPostgresConn *conn );

    /**
     * Set mLayerExtent by estimation, if possible
     *
     * \returns whether it was possible to estimate extent.
     * If false is returned, mLayerExtent is left untouched.
     */
    bool estimateExtent() const;

    /**
     * Set mLayerExtent by 3d computation, if possible
     *
     * \returns whether it was possible to estimate extent.
     * If false is returned, mLayerExtent is left untouched.
     */
    bool computeExtent3D() const;
};

class QgsPostgresProviderMetadata final : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsPostgresProviderMetadata();
    QIcon icon() const override;
    QgsDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() ) override;
    QList<QgsDataItemProvider *> dataItemProviders() const override;
    Qgis::VectorExportResult createEmptyLayer( const QString &uri, const QgsFields &fields, Qgis::WkbType wkbType, const QgsCoordinateReferenceSystem &srs, bool overwrite, QMap<int, int> &oldToNewAttrIdxMap, QString &errorMessage, const QMap<QString, QVariant> *options ) override;

    bool styleExists( const QString &uri, const QString &styleId, QString &errorCause ) override;
    bool saveStyle( const QString &uri, const QString &qmlStyle, const QString &sldStyle, const QString &styleName, const QString &styleDescription, const QString &uiFileContent, bool useAsDefault, QString &errCause ) override;
    QString loadStyle( const QString &uri, QString &errCause ) override;
    virtual QString loadStoredStyle( const QString &uri, QString &styleName, QString &errCause ) override;
    int listStyles( const QString &uri, QStringList &ids, QStringList &names, QStringList &descriptions, QString &errCause ) override;
    bool deleteStyleById( const QString &uri, const QString &styleId, QString &errCause ) override;
    QString getStyleById( const QString &uri, const QString &styleId, QString &errCause ) override;
    QgsTransaction *createTransaction( const QString &connString ) override;
    QMap<QString, QgsAbstractProviderConnection *> connections( bool cached = true ) override;
    QgsAbstractProviderConnection *createConnection( const QString &name ) override;
    QgsAbstractProviderConnection *createConnection( const QString &uri, const QVariantMap &configuration ) override;
    void deleteConnection( const QString &name ) override;
    void saveConnection( const QgsAbstractProviderConnection *createConnection, const QString &name ) override;
    void initProvider() override;
    void cleanupProvider() override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QList<Qgis::LayerType> supportedLayerTypes() const override;
    bool saveLayerMetadata( const QString &uri, const QgsLayerMetadata &metadata, QString &errorMessage ) override;
    QgsProviderMetadata::ProviderCapabilities providerCapabilities() const override;
};

// clazy:excludeall=qstring-allocations

#endif
