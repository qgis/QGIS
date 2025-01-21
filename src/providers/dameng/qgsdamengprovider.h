/***************************************************************************
    qgsdamengprovider.h  -  Data provider for Dameng/DAMENG layers
                             -------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGPROVIDER_H
#define QGSDAMENGPROVIDER_H

#include "qgsvectordataprovider.h"
#include "qgsrectangle.h"
#include "qgsdamengconn.h"
#include "qgsfields.h"
#include "qgsprovidermetadata.h"
#include "qgsreferencedgeometry.h"
#include <memory>
#include <optional>

class QgsFeature;
class QgsField;
class QgsGeometry;

class QgsDamengFeatureIterator;
class QgsDamengSharedData;
class QgsDamengTransaction;

#include "qgsdatasourceuri.h"

/**
 * \class QgsDamengProvider
 * \brief Data provider for Dameng/DAMENG layers.
 *
 * This provider implements the
 * interface defined in the QgsDataProvider class to provide access to spatial
 * data residing in a Dameng/DAMENG enabled database.
  */
class QgsDamengProvider final: public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    static const QString DAMENG_KEY;
    static const QString DAMENG_DESCRIPTION;

    enum Relkind
    {
      NotSet,
      Unknown,
      OrdinaryTable, // UTAB
      View, // VIEW
      MaterializedView, // MVIEW
    };
    Q_ENUM( Relkind )

    /**
     * Import a vector layer into the database
     * \param options options for provider, specified via a map of option name
     * to value. Valid options are lowercaseFieldNames ( set to true to convert
     * field names to lowercase ), dropStringConstraints ( set to true to remove
     * length constraints on character fields ).
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
     * host=localhost dbname=test [user=gsherman [password=xxx] | authcfg=xxx] table=test.alaska ( the_geom )
     * \param uri String containing the required parameters to connect to the database
     * and query the table.
     * \param options generic data provider options
     * \param flags generic data provider flags
     */
    explicit QgsDamengProvider( QString const &uri, const QgsDataProvider::ProviderOptions &providerOptions,
                                  Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );


    ~QgsDamengProvider() override;

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
    QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
                                       QgsFeedback *feedback = nullptr ) const override;
    bool isValid() const override;
    Qgis::ProviderStyleStorageCapabilities styleStorageCapabilities() const override;
    QgsAttributeList attributeIndexes() const override;
    QgsAttributeList pkAttributeIndexes() const override { return mPrimaryKeyAttrs; }
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

    //! Gets the table name associated with this provider instance
    QString getTableName();

    QString subsetString() const override;
    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
    bool supportsSubsetString() const override { return true; }
    Qgis::VectorProviderCapabilities capabilities() const override;
    Qgis::SpatialIndexPresence hasSpatialIndex() const override;

    /**
     * The Dameng provider does its own transforms so we return
     * true for the following three functions to indicate that transforms
     * should not be handled by the QgsCoordinateTransform object. See the
     * documentation on QgsVectorDataProvider for details on these functions.
     */
    // XXX For now we have disabled native transforms in the DM provider since
    //     it appears there are problems with some of the projection definitions
    bool supportsNativeTransform() {return false;}

    QString name() const override;
    QString description() const override;
    QgsTransaction *transaction() const override;
    static QString providerKey();

    /**
     * Convert the dameng string representation into the given QVariant type.
     * \param type the wanted type
     * \param subType if type is a collection, the wanted element type
     * \param value the value to convert
     * \returns a QVariant of the given type or a null QVariant
     */
    static QVariant convertValue( QMetaType::Type type, QMetaType::Type subType, const QString &value, const QString &typeName );

    /**
     * Returns true if the data source has metadata, false otherwise. For
     * example, if the kind of relation for the layer is a view or a
     * materialized view, then no metadata are associated with the data
     * source.
     *
     * \returns true if data source has metadata, false otherwise.
     */
    bool hasMetadata() const override;

    Qgis::VectorLayerTypeFlags vectorLayerTypeFlags() const override;
    
    void handlePostCloneOperations( QgsVectorDataProvider *source ) override;

  private:

    /** returns relation kind */
    Relkind relkind() const;

    /** Change internal query with \a query */
    void setQuery( const QString &query );

    QString geomParam( int offset ) const;

    static QString getNextString( const QString &txt, int &i, const QString &sep );
    static QVariant parseJson( const QString &txt );


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
    bool loadFields();  /** Load the field list */

    //! Convert a QgsField to work with DM
    static bool convertField( QgsField &field, const QMap<QString, QVariant> *coordinateTransformContext = nullptr );

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
    QgsDamengPrimaryKeyType pkType( const QgsField &fld ) const;

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
    
    static QgsCoordinateReferenceSystem sridToCrs( int srsId, QgsDamengConn *conn );

  private:
    //! Old-style mapping of index to name for QgsPalLabeling fix
    QgsAttrPalIndexNameHash mAttrPalIndexName;

    QgsFields mAttributeFields;
    QHash<int, char> mIdentityFields;
    QHash< int, std::tuple<int, int> > mIdentityInfos;
    QString mDataComment;

    //! Data source URI struct for this layer
    QgsDataSourceUri mUri;

    /** * Flag indicating if the layer data source is a valid Dameng layer */
    bool mValid = false;

    /** provider references query ( instead of a table ) */
    bool mIsQuery;

    /** Name of the table or subquery */
    QString mQuery;

    QString mSchemaName;  /** Name of the schema */
    QString mTableName; /** Name of the table with no schema*/

    /**
     * SQL statement used to limit the features retrieved
     */
    QString mSqlWhereClause;
    
    mutable Relkind mKind = Relkind::NotSet;  /** Kind of relation */
    QgsDamengPrimaryKeyType mPrimaryKeyType = PktUnknown; /** Data type for the primary key */
    QgsDamengGeometryColumnType mSpatialColType = SctNone;  /** Data type for the spatial column */

    /**
     * List of primary key attributes for fetching features.
     */
    QList<int> mPrimaryKeyAttrs;
    QString mPrimaryKeyDefault;

    QString mGeometryColumn;          //!< Name of the geometry column
    QString mGeometryColType;
    QString mBoundingBoxColumn;       //!< Name of the bounding box column
    mutable std::optional<QgsBox3D> mLayerExtent;        //!< Rectangle that contains the extent ( bounding box ) of the layer

    Qgis::WkbType mDetectedGeomType = Qgis::WkbType::Unknown ;  //!< Geometry type detected in the database
    Qgis::WkbType mRequestedGeomType = Qgis::WkbType::Unknown ; //!< Geometry type requested in the uri
    QString mDetectedSrid;            //!< Spatial reference detected in the database
    QString mRequestedSrid;           //!< Spatial reference requested in the uri

    std::shared_ptr<QgsDamengSharedData> mShared;  //!< Mutable data shared between provider and feature sources

    bool getGeometryDetails();

  private:
    //! @{ Only used with TopoGeometry layers
    struct TopoLayerInfo
    {
      QString topologyName;
      long    layerId;
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

    /* Use estimated metadata. Uses fast table counts, geometry type and extent determination */
    bool mUseEstimatedMetadata = false;

    bool mSelectAtIdDisabled = false; //!< Disable support for SelectAtId

    struct DMFieldNotFound {}; //! Exception to throw

    struct DMException
    {
        explicit DMException( QgsDamengResult &r )
          : mWhat( r.DMresultErrorMessage() )
        {}

        explicit DMException( QString &r )
          : mWhat( r )
        {}

        QString errorMessage() const
        {
          return mWhat;
        }

      private:
        QString mWhat;
    };

    // A function that determines if the given columns contain unique entries
    bool uniqueData( const QString &quotedColNames );

   Qgis::VectorProviderCapabilities mEnabledCapabilities;

    void appendGeomParam( const QgsGeometry &geom, QByteArray &geom_wkb ) const;
    void appendPkParams( QgsFeatureId fid, QStringList &param ) const;

    QString paramValue( const QString &fieldvalue, const QString &defaultValue ) const;

  private:
    mutable QgsDamengConn *mConnectionRO = nullptr ; //!< Read-only database connection ( initially )
    QgsDamengConn *mConnectionRW = nullptr ; //!< Read-write database connection ( on update )

    QgsDamengConn *connectionRO() const;
    QgsDamengConn *connectionRW();

    void disconnectDb();

    static QString quotedIdentifier( const QString &ident ) { return QgsDamengConn::quotedIdentifier( ident ); }
    static QString quotedValue( const QVariant &value ) { return QgsDamengConn::quotedValue( value ); }
    static QString quotedByteaValue( const QVariant &value );

  private:
    friend class QgsDamengFeatureSource;

    QgsDamengTransaction *mTransaction = nullptr;

    void setTransaction( QgsTransaction *transaction ) override;

    QHash<int, QString> mDefaultValues;

    // for handling generated columns, available in Dameng 8+
    QHash<int, QString> mGeneratedValues;

    bool mCheckPrimaryKeyUnicity = true;

    QgsLayerMetadata mLayerMetadata;

};


//! Assorted Dameng utility functions
class QgsDamengUtils
{
  public:
    static bool deleteLayer( const QString &uri, QString &errCause );
    static bool deleteSchema( const QString &schema, const QgsDataSourceUri &uri, QString &errCause, bool cascade = false );

    static QString whereClause( QgsFeatureId featureId,
                                const QgsFields &fields,
                                QgsDamengConn *conn,
                                QgsDamengPrimaryKeyType pkType,
                                const QList<int> &pkAttrs,
                                const std::shared_ptr<QgsDamengSharedData> &sharedData );

    static QString whereClause( const QgsFeatureIds &featureIds,
                                const QgsFields &fields,
                                QgsDamengConn *conn,
                                QgsDamengPrimaryKeyType pkType,
                                const QList<int> &pkAttrs,
                                const std::shared_ptr<QgsDamengSharedData> &sharedData );

    static QString andWhereClauses( const QString &c1, const QString &c2 );

    static const qint64 INT32PK_OFFSET = 4294967296; // 2^32

    // We shift negative 32bit integers to above the max 32bit
    // positive integer to support the whole range of int32 values
    // See https://github.com/qgis/QGIS/issues/22258
    static qint64 int32pk_to_fid( qint32 x )
    {
      return x >= 0 ? x : x + INT32PK_OFFSET;
    }

    static qint32 fid_to_int32pk( qint64 x )
    {
      return x <= ( ( INT32PK_OFFSET ) / 2.0 ) ? x : -( INT32PK_OFFSET - x );
    }

    //! Replaces invalid XML chars with UTF-8[<char_code>]
    static void replaceInvalidXmlChars( QString &xml );

    //! Replaces UTF-8[<char_code>] with the actual unicode char
    static void restoreInvalidXmlChars( QString &xml );
};

/**
 * Data shared between provider class and its feature sources. Ideally there should
 *  be as few members as possible because there could be simultaneous reads/writes
 *  from different threads and therefore locking has to be involved.
*/
class QgsDamengSharedData
{
  public:
    QgsDamengSharedData() = default;

    long long featuresCounted();
    void setFeaturesCounted( long long count );
    void addFeaturesCounted( long long diff );
    void ensureFeaturesCountedAtLeast( long long fetched );

    // FID lookups
    QgsFeatureId lookupFid( const QVariantList &v ); // lookup existing mapping or add a new one
    QVariantList removeFid( QgsFeatureId fid );
    void insertFid( QgsFeatureId fid, const QVariantList &k );
    QVariantList lookupKey( QgsFeatureId featureId );
    void clear();

    void clearSupportsEnumValuesCache();
    bool fieldSupportsEnumValuesIsSet( int index );
    bool fieldSupportsEnumValues( int index );
    void setFieldSupportsEnumValues( int index, bool isSupported );

  protected:
    QMutex mMutex; //!< Access to all data members is guarded by the mutex

    long long mFeaturesCounted = -1 ;    //!< Number of features in the layer

    QgsFeatureId mFidCounter = 0;                    // next feature id if map is used
    QMap<QVariantList, QgsFeatureId> mKeyToFid;      // map key values to feature id
    QMap<QgsFeatureId, QVariantList> mFidToKey;      // map feature id back to key values
    QMap<int, bool> mFieldSupportsEnumValues;        // map field index to bool flag supports enum values
};

class QgsDamengProviderMetadata final: public QgsProviderMetadata
{
  public:
    QgsDamengProviderMetadata();
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
    QMap< QString, QgsAbstractProviderConnection *> connections( bool cached = true ) override;
    QgsAbstractProviderConnection *createConnection( const QString &name ) override;
    QgsAbstractProviderConnection *createConnection( const QString &uri, const QVariantMap &configuration ) override;
    void deleteConnection( const QString &name ) override;
    void saveConnection( const QgsAbstractProviderConnection *createConnection, const QString &name ) override;
    void initProvider() override;
    void cleanupProvider() override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QList<Qgis::LayerType> supportedLayerTypes() const override;
};

// clazy:excludeall=qstring-allocations

#endif
