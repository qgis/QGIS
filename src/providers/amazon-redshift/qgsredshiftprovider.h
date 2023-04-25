/***************************************************************************
   qgsredshiftprovider.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSREDSHIFTPROVIDER_H
#define QGSREDSHIFTPROVIDER_H

#include <memory>

#include "qgsfields.h"
#include "qgsprovidermetadata.h"
#include "qgsrectangle.h"
#include "qgsredshiftconn.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayerexporter.h"

class QgsFeature;
class QgsField;
class QgsGeometry;

class QgsRedshiftFeatureIterator;
class QgsRedshiftSharedData;

#include "qgsdatasourceuri.h"

/**
 * \class QgsRedshiftProvider
 * \brief Data provider for Redshift layers.
 *
 * This provider implements the
 * interface defined in the QgsDataProvider class to provide access to spatial
 * data residing in a Redshift enabled database.
 */
class QgsRedshiftProvider final: public QgsVectorDataProvider
{
    Q_OBJECT

  public:
    static const QString REDSHIFT_KEY;
    static const QString REDSHIFT_DESCRIPTION;

    enum Relkind
    {
      Unknown,
      OrdinaryTable,    // r
      View,             // v
      MaterializedView, // m
    };
    Q_ENUM( Relkind )

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
     * host=localhost dbname=test [user=gsherman [password=xxx] | authcfg=xxx]
     * table=test.alaska (the_geom) \param uri String containing the required
     * parameters to connect to the database and query the table. \param options
     * generic data provider options \param flags generic data provider flags
     */
    explicit QgsRedshiftProvider( QString const &uri, const QgsDataProvider::ProviderOptions &providerOptions,
                                  QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsRedshiftProvider() override;

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
     * Returns a list of unquoted column names from an uri key
     */
    static QStringList parseUriKey( const QString &key );

    /**
     * Changes the stored extent for this layer to the supplied extent.
     * For example, this is called when the extent worker thread has a result.
     */
    void setExtent( QgsRectangle &newExtent );

    QgsRectangle extent() const override;
    void updateExtents() override;

    /**
     * Determine the fields making up the primary key
     */
    bool determinePrimaryKeyLocalDatabase();

    /**
     * Determine the fields making up the primary key from the uri attribute
     * keyColumn
     *
     * Fills mPrimaryKeyAttrs
     * from mUri
     */
    void determinePrimaryKeyFromUriKeyColumn();

    QgsFields fields() const override;
    QString dataComment() const override;
    QVariant extremeValue( int index, const QString &func ) const;
    QVariant minimumValue( int index ) const override;
    QVariant maximumValue( int index ) const override;
    QSet<QVariant> uniqueValues( int index, int limit = -1 ) const override;
    QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
                                       QgsFeedback *feedback = nullptr ) const override;

    bool isValid() const override;
    bool isSaveAndLoadStyleToDatabaseSupported() const override
    {
      return true;
    }
    bool isDeleteStyleFromDatabaseSupported() const override
    {
      return true;
    }
    QgsAttributeList attributeIndexes() const override;
    QgsAttributeList pkAttributeIndexes() const override
    {
      return mPrimaryKeyAttrs;
    }
    QString defaultValueClause( int fieldId ) const override;
    QVariant defaultValue( int fieldId ) const override;
    bool skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint,
                              const QVariant &value = QVariant() ) const override;
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool deleteFeatures( const QgsFeatureIds &id ) override;
    bool truncate() override;
    bool addAttributes( const QList<QgsField> &attributes ) override;
    bool deleteAttributes( const QgsAttributeIds &name ) override;
    bool renameAttributes( const QgsFieldNameMap &renamedAttributes ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    bool changeFeatures( const QgsChangedAttributesMap &attr_map, const QgsGeometryMap &geometry_map ) override;

    //! Gets the Redshift connection
    PGconn *pgConnection();

    //! Gets the table name associated with this provider instance
    QString getTableName();

    QString subsetString() const override;
    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
    bool supportsSubsetString() const override
    {
      return true;
    }
    QgsVectorDataProvider::Capabilities capabilities() const override;
    SpatialIndexPresence hasSpatialIndex() const override;

    /**
     * The Redshift provider does its own transforms so we return
     * true for the following three functions to indicate that transforms
     * should not be handled by the QgsCoordinateTransform object. See the
     * documentation on QgsVectorDataProvider for details on these functions.
     */
    // XXX For now we have disabled native transforms in the PG provider since
    //     it appears there are problems with some of the projection definitions
    bool supportsNativeTransform()
    {
      return false;
    }

    QString name() const override;
    QString description() const override;
    static QString providerKey();

    /**
     * Convert the redshift string representation into the given QVariant type.
     * \param type the wanted type
     * \param subType if type is a collection, the wanted element type
     * \param value the value to convert
     * \returns a QVariant of the given type or a null QVariant
     */
    static QVariant convertValue( QVariant::Type type, const QString &value, const QString &typeName );

    /**
     * Redshift maximum VARCHAR size(65535) and default size(256)
     */
    static int redshiftMaxVarcharLength()
    {
      return 65535;
    }
    static int redshiftDefaultVarcharLength()
    {
      return 256;
    }

    /**
     * Redshift maximum CHAR size(4096) and default size(1)
     */
    static int redshiftMaxCharLength()
    {
      return 4096;
    }
    static int redshiftDefaultCharLength()
    {
      return 1;
    }

    /**
     * Returns true if the data source has metadata, false otherwise. For
     * example, if the kind of relation for the layer is a view or a
     * materialized view, then no metadata are associated with the data
     * source.
     *
     * \returns true if data source has metadata, false otherwise.
     *
     * \since QGIS 3.0
     */
    bool hasMetadata() const override;

  private:
    Relkind relkind() const;

    bool declareCursor( const QString &cursorName, const QgsAttributeList &fetchAttributes, bool fetchGeometry,
                        QString whereClause );

    bool getFeature( QgsRedshiftResult &queryResult, int row, bool fetchGeometry, QgsFeature &feature,
                     const QgsAttributeList &fetchAttributes );

    QString geomParam( int offset ) const;

    /**
     * Gets parametrized primary key clause
     * \param offset specifies offset to use for the pk value parameter
     * \param alias specifies an optional alias given to the subject table
     */
    QString pkParamWhereClause( int offset, const char *alias = nullptr ) const;
    QString whereClause( QgsFeatureId featureId ) const;
    QString whereClause( QgsFeatureIds featureIds ) const;
    QString filterWhereClause() const;

    bool setPermsAndCapabilitiesForQuery();
    bool setPermsAndCapabilitiesForDatashare();
    bool setPermsAndCapabilitiesForLocalDatabase();
    bool setPermsAndCapabilities();

    QgsField field( int index ) const;

    static Oid getFldType( Oid typeoid );

    /**
     * Load the field list
     */
    bool loadFields();

    /**
     * Set the default widget type for the fields
     */
    void setEditorWidgets();

    //! Convert a QgsField to work with Redshift
    static bool convertField( QgsField &field, const QMap<QString, QVariant> *options = nullptr );

    /**
     * Search all the layers using the given table.
     */
    static QList<QgsVectorLayer *> searchLayers( const QList<QgsVectorLayer *> &layers, const QString &connectionInfo,
        const QString &schema, const QString &tableName );

    /**
     * Effect a reload including resetting the feature count
     * and setting the layer extent to minimal
     *
     * \since QGIS 3.12
     */
    void reloadProviderData() override;

    //! Old-style mapping of index to name for QgsPalLabeling fix
    QgsAttrPalIndexNameHash mAttrPalIndexName;

    QgsFields mAttributeFields;
    QHash<int, char> mIdentityFields;
    QString mDataComment;

    //! Data source URI struct for this layer
    QgsDataSourceUri mUri;

    /**
     * Flag indicating if the layer data source is a valid Redshift layer
     */
    bool mValid = false;

    /**
     * provider references query (instead of a table)
     */
    bool mIsQuery;

    /**
     * Name of the external database
     */
    QString mExternalDatabaseName;

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
     * Data type for the spatial column
     */
    QgsRedshiftGeometryColumnType mSpatialColType = SctNone;

    /**
     * List of primary key attributes for fetching features.
     */
    QList<int> mPrimaryKeyAttrs;
    QString mPrimaryKeyDefault;

    QString mGeometryColumn;           //!< Name of the geometry column
    QString mBoundingBoxColumn;        //!< Name of the bounding box column
    mutable QgsRectangle mLayerExtent; //!< Rectangle that contains the extent
    //!< (bounding box) of the layer

    Qgis::WkbType mDetectedGeomType = Qgis::WkbType::Unknown;  //!< Geometry type detected in the database
    Qgis::WkbType mRequestedGeomType = Qgis::WkbType::Unknown; //!< Geometry type requested in the uri
    QString mDetectedSrid;                                       //!< Spatial reference detected in the database
    QString mRequestedSrid;                                      //!< Spatial reference requested in the uri

    std::shared_ptr<QgsRedshiftSharedData> mShared; //!< Mutable data shared between provider and feature sources

    bool getGeometryDetails();

    /* Use estimated metadata. Uses fast table counts, geometry type and extent
     * determination */
    bool mUseEstimatedMetadata = false;

    bool mSelectAtIdDisabled = false; //!< Disable support for SelectAtId

    struct PGFieldNotFound
    {
    }; //! Exception to throw

    struct PGException
    {
        explicit PGException( QgsRedshiftResult &r ) : mWhat( r.PQresultErrorMessage() )
        {
        }

        QString errorMessage() const
        {
          return mWhat;
        }

      private:
        QString mWhat;
    };

    // A function that determines if the given columns contain unique entries
    bool uniqueData( const QString &quotedColNames );

    QgsVectorDataProvider::Capabilities mEnabledCapabilities;

    void appendGeomParam( const QgsGeometry &geom, QStringList &param ) const;
    void appendPkParams( QgsFeatureId fid, QStringList &param ) const;

    QString paramValue( const QString &fieldvalue, const QString &defaultValue ) const;

    QgsRedshiftConn *mConnectionRO = nullptr; //!< Read-only database connection (initially)
    QgsRedshiftConn *mConnectionRW = nullptr; //!< Read-write database connection (on update)

    QgsRedshiftConn *connectionRO() const;
    QgsRedshiftConn *connectionRW();

    void disconnectDb();

    static QString quotedIdentifier( const QString &ident )
    {
      return QgsRedshiftConn::quotedIdentifier( ident );
    }
    static QString quotedValue( const QVariant &value )
    {
      return QgsRedshiftConn::quotedValue( value );
    }

    friend class QgsRedshiftFeatureSource;

    QHash<int, QString> mDefaultValues;

    bool mCheckPrimaryKeyUnicity = true;

    QgsLayerMetadata mLayerMetadata;

    QStringList getValuesRowForInsert( QgsFeature &f, const QHash<int, QVariant> &defVals, const QVector<int> &insertCols );

    bool isIdentityField( int idx ) const;
};

//! Assorted Redshift utility functions
class QgsRedshiftUtils
{
  public:
    static bool deleteLayer( const QString &uri, QString &errCause );
    static bool deleteSchema( const QString &schema, const QgsDataSourceUri &uri, QString &errCause,
                              bool cascade = false );

    static QString whereClause( QgsFeatureId featureId, const QgsFields &fields, QgsRedshiftConn *conn,
                                const QList<int> &pkAttrs, const std::shared_ptr<QgsRedshiftSharedData> &sharedData );

    static QString whereClause( const QgsFeatureIds &featureIds, const QgsFields &fields, QgsRedshiftConn *conn,
                                const QList<int> &pkAttrs, const std::shared_ptr<QgsRedshiftSharedData> &sharedData );

    static QString andWhereClauses( const QString &c1, const QString &c2 );

    //! Replaces invalid XML chars with UTF-8[<char_code>]
    static void replaceInvalidXmlChars( QString &xml );

    //! Replaces UTF-8[<char_code>] with the actual unicode char
    static void restoreInvalidXmlChars( QString &xml );
};

/**
 * Data shared between provider class and its feature sources. Ideally there
 * should be as few members as possible because there could be simultaneous
 * reads/writes from different threads and therefore locking has to be involved.
 */
class QgsRedshiftSharedData
{
  public:
    QgsRedshiftSharedData() = default;

    long featuresCounted();
    void setFeaturesCounted( long count );
    void addFeaturesCounted( long diff );
    void ensureFeaturesCountedAtLeast( long fetched );

    // FID lookups
    QgsFeatureId lookupFid( const QVariantList &v ); // lookup existing mapping or add a new one
    QVariantList removeFid( QgsFeatureId fid );
    void insertFid( QgsFeatureId fid, const QVariantList &k );
    QVariantList lookupKey( QgsFeatureId featureId );
    void clear();

  protected:
    QMutex mMutex; //!< Access to all data members is guarded by the mutex

    long mFeaturesCounted = -1; //!< Number of features in the layer

    QgsFeatureId mFidCounter = 0;               // next feature id if map is used
    QMap<QVariantList, QgsFeatureId> mKeyToFid; // map key values to feature id
    QMap<QgsFeatureId, QVariantList> mFidToKey; // map feature id back to key values
};

class QgsRedshiftProviderMetadata final : public QgsProviderMetadata
{
  public:
    QgsRedshiftProviderMetadata();
    QgsDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options,
                                     QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList<QgsDataItemProvider *> dataItemProviders() const override;
    Qgis::VectorExportResult createEmptyLayer( const QString &uri, const QgsFields &fields,
        Qgis::WkbType wkbType,
        const QgsCoordinateReferenceSystem &srs, bool overwrite,
        QMap<int, int> &oldToNewAttrIdxMap, QString &errorMessage,
        const QMap<QString, QVariant> *options ) override;
    bool saveStyle( const QString &uri, const QString &qmlStyle, const QString &sldStyle, const QString &styleName,
                    const QString &styleDescription, const QString &uiFileContent, bool useAsDefault,
                    QString &errCause ) override;
    QString loadStyle( const QString &uri, QString &errCause ) override;
    int listStyles( const QString &uri, QStringList &ids, QStringList &names, QStringList &descriptions,
                    QString &errCause ) override;
    bool deleteStyleById( const QString &uri, const QString &styleId, QString &errCause ) override;
    QString getStyleById( const QString &uri, const QString &styleId, QString &errCause ) override;
    QMap<QString, QgsAbstractProviderConnection *> connections( bool cached = true ) override;
    QgsAbstractProviderConnection *createConnection( const QString &name ) override;
    QgsAbstractProviderConnection *createConnection( const QString &uri, const QVariantMap &configuration ) override;
    void deleteConnection( const QString &name ) override;
    void saveConnection( const QgsAbstractProviderConnection *createConnection, const QString &name ) override;
    void initProvider() override;
    void cleanupProvider() override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
};

// clazy:excludeall=qstring-allocations

#endif
