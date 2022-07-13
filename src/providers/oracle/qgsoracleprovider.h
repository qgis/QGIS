/***************************************************************************
      qgsoracleprovider.h  -  Data provider for oracle layers
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSORACLEPROVIDER_H
#define QGSORACLEPROVIDER_H

#include "qgsvectordataprovider.h"
#include "qgsrectangle.h"
#include "qgsoracletablemodel.h"
#include "qgsdatasourceuri.h"
#include "qgsfields.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#ifdef HAVE_GUI
#include "qgsproviderguimetadata.h"
#endif

#include <QVector>
#include <QQueue>
#include <QSqlQuery>
#include <QSqlError>
#include <QMutex>

class QgsFeature;
class QgsField;
class QgsGeometry;
class QgsOracleFeatureIterator;
class QgsOracleSharedData;
class QgsOracleTransaction;

enum QgsOraclePrimaryKeyType
{
  PktUnknown,
  PktInt,
  PktRowId,
  PktFidMap
};


/**
  * \class QgsOracleProvider
  * \brief Data provider for oracle layers.
  *
  * This provider implements the
  * interface defined in the QgsDataProvider class to provide access to spatial
  * data residing in a oracle enabled database.
  */
class QgsOracleProvider final: public QgsVectorDataProvider
{
    Q_OBJECT
    Q_PROPERTY( QString workspace READ getWorkspace WRITE setWorkspace )

  public:

    //! Import a vector layer into the database
    static Qgis::VectorExportResult createEmptyLayer(
      const QString &uri,
      const QgsFields &fields,
      QgsWkbTypes::Type wkbType,
      const QgsCoordinateReferenceSystem &srs,
      bool overwrite,
      QMap<int, int> &oldToNewAttrIdxMap,
      QString &errorMessage,
      const QMap<QString, QVariant> *options = nullptr
    );

    enum Relkind
    {
      NotSet,
      Unknown,
      Table,
      View,
    };
    Q_ENUM( Relkind )

    /**
     * Constructor for the provider. The uri must be in the following format:
     * host=localhost user=gsherman dbname=test password=xxx table=test.alaska (the_geom)
     * \param uri String containing the required parameters to connect to the database
     * and query the table.
     * \param options generic data provider options
     * \param flags generic data provider flags
     */
    explicit QgsOracleProvider( QString const &uri, const QgsDataProvider::ProviderOptions &options,
                                QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    //! Destructor
    ~QgsOracleProvider() override;

    QgsAbstractFeatureSource *featureSource() const override;
    QString storageType() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsWkbTypes::Type wkbType() const override;

    /**
     * Returns the number of layers for the current data source
     * \note Should this be subLayerCount() instead?
     */
    size_t layerCount() const;

    long long featureCount() const override;

    /**
     * Gets the number of fields in the layer
     */
    uint fieldCount() const;

    /**
     * Returns a string representation of the endian-ness for the layer
     */
    QString endianString();

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

    /**
     * Reset the layer
     */
    void rewind();

    QVariant minimumValue( int index ) const override;
    QVariant maximumValue( int index ) const override;
    QSet<QVariant> uniqueValues( int index, int limit = -1 ) const override;
    bool isValid() const override;
    QgsAttributeList pkAttributeIndexes() const override { return mPrimaryKeyAttrs; }
    QVariant defaultValue( QString fieldName, QString tableName = QString(), QString schemaName = QString() );
    QVariant defaultValue( int fieldId ) const override;
    QString defaultValueClause( int fieldId ) const override;
    bool skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value = QVariant() ) const override;
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool deleteFeatures( const QgsFeatureIds &id ) override;
    bool addAttributes( const QList<QgsField> &attributes ) override;
    bool deleteAttributes( const QgsAttributeIds &ids ) override;
    bool renameAttributes( const QgsFieldNameMap &renamedAttributes ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    bool createSpatialIndex() override;

    //! Gets the table name associated with this provider instance
    QString getTableName();

    QString subsetString() const override;
    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
    bool supportsSubsetString() const override { return true; }
    QgsVectorDataProvider::Capabilities capabilities() const override;
    QString name() const override;
    QString description() const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const override;

    static bool execLoggedStatic( QSqlQuery &qry, const QString &sql, const QVariantList &args, const QString &uri, const QString &originatorClass = QString(), const QString &queryOrigin = QString() );

    bool isSaveAndLoadStyleToDatabaseSupported() const override { return true; }
    void setTransaction( QgsTransaction *transaction ) override;
    QgsTransaction *transaction() const override;

    /**
     * Switch to oracle workspace
     */
    void setWorkspace( const QString &workspace );

    /**
     * Retrieve oracle workspace name
     */
    QString getWorkspace() const;

    Qgis::VectorLayerTypeFlags vectorLayerTypeFlags() const override;

    void handlePostCloneOperations( QgsVectorDataProvider *source ) override;

  private:

    /**
     * \returns relation kind
     */
    Relkind relkind() const;

    QString whereClause( QgsFeatureId featureId, QVariantList &args ) const;
    QString pkParamWhereClause() const;

    /**
     * Look up \a srsid coordinate reference system from database using \a conn connection
     * Returns the coordinate system for the data source. If the provider isn't capable of finding
     * a matching one, then an invalid QgsCoordinateReferenceSystem will be returned.
     */
    static QgsCoordinateReferenceSystem lookupCrs( QgsOracleConn *conn, int srsid );

    /**
     * Insert \a geometryColumn column from table \a tableName in Oracle geometry metadata table with given \a srs coordinate
     * reference system, using \a conn connection
     * Throws OracleException if an error occurred.
     */
    static void insertGeomMetadata( QgsOracleConn *conn, const QString &tableName, const QString &geometryColumn, const QgsCoordinateReferenceSystem &srs );

    /**
     * Evaluates the given expression string server-side and convert the result to the given type
     */
    QVariant evaluateDefaultExpression( const QString &value, const QVariant::Type &fieldType ) const;
    void appendGeomParam( const QgsGeometry &geom, QSqlQuery &qry ) const;
    void appendPkParams( QgsFeatureId fid, QSqlQuery &qry ) const;

    bool hasSufficientPermsAndCapabilities();

    QgsField field( int index ) const;

    /**
     * Load the field list
     */
    bool loadFields();

    //! Convert a QgsField to work with Oracle
    static bool convertField( QgsField &field );

    QgsFields mAttributeFields;  //!< List of fields
    QVariantList mDefaultValues; //!< List of default values
    QString mDataComment;

    //! Data source URI struct for this layer
    QgsDataSourceUri mUri;

    /**
     * Flag indicating if the layer data source is a valid oracle layer
     */
    bool mValid;

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
     * Owner of the table
     */
    QString mOwnerName;

    /**
     * SQL statement used to limit the features retrieved
     */
    QString mSqlWhereClause;

    /**
     * Kind of relation
     */
    mutable Relkind mKind = Relkind::NotSet;

    /**
     * Data type for the primary key
     */
    QgsOraclePrimaryKeyType mPrimaryKeyType;

    /**
     * List of primary key attributes for fetching features.
     */
    QList<int> mPrimaryKeyAttrs;
    QString mPrimaryKeyDefault;

    /**
     * For each attributes, true if the attribute is always generated (virtual column
     * and identity always generated columns)
     */
    QList<bool> mAlwaysGenerated;

    QString mGeometryColumn;           //!< Name of the geometry column
    mutable QgsRectangle mLayerExtent; //!< Rectangle that contains the extent (bounding box) of the layer
    mutable long long mFeaturesCounted;     //!< Number of features in the layer
    int mSrid;                         //!< Srid of column
    QgsVectorDataProvider::Capabilities mEnabledCapabilities;          //!< Capabilities of layer

    QgsWkbTypes::Type mDetectedGeomType;   //!< Geometry type detected in the database
    QgsWkbTypes::Type mRequestedGeomType;  //!< Geometry type requested in the uri

    bool getGeometryDetails();

    /* Use estimated metadata. Uses fast table counts, geometry type and extent determination */
    bool mUseEstimatedMetadata;

    /* Include additional geo attributes */
    bool mIncludeGeoAttributes;

    QgsOracleTransaction *mTransaction = nullptr;

    struct OracleFieldNotFound {}; //! Exception to throw

    struct OracleException
    {
        OracleException( QString msg, const QSqlQuery &q )
          : mWhat( tr( "Oracle error: %1\nSQL: %2\nError: %3" )
                   .arg( msg )
                   .arg( q.lastError().text() )
                   .arg( q.lastQuery() )
                 )
        {}

        OracleException( QString msg, const QSqlDatabase &q )
          : mWhat( tr( "Oracle error: %1\nError: %2" )
                   .arg( msg )
                   .arg( q.lastError().text() )
                 )
        {}

        OracleException( const OracleException &e )
          : mWhat( e.errorMessage() )
        {}

        ~OracleException()
          = default;

        QString errorMessage() const
        {
          return mWhat;
        }

      private:
        QString mWhat;

        OracleException &operator= ( const OracleException & ) = delete;
    };

    // A function that determines if the given schema.table.column
    // contains unique entries
    bool uniqueData( QString query, QString colName );

    void disconnectDb();

    static QString quotedIdentifier( QString ident ) { return QgsOracleConn::quotedIdentifier( ident ); }
    static QString quotedValue( const QVariant &value, QVariant::Type type = QVariant::Invalid ) { return QgsOracleConn::quotedValue( value, type ); }

    QMap<QVariant, QgsFeatureId> mKeyToFid;  //!< Map key values to feature id
    QMap<QgsFeatureId, QVariant> mFidToKey;  //!< Map feature back to feature id

    bool mHasSpatialIndex;                   //!< Geometry column is indexed
    QString mSpatialIndexName;               //!< Name of spatial index of geometry column
    int mOracleVersion;                      //!< Oracle database version

    std::shared_ptr<QgsOracleSharedData> mShared;

    QgsOracleConn *connectionRW();
    QgsOracleConn *connectionRO() const;

    friend class QgsOracleFeatureIterator;
    friend class QgsOracleFeatureSource;
};


//! Assorted Oracle utility functions
class QgsOracleUtils
{
  public:
    static QString whereClause( QgsFeatureId featureId,
                                const QgsFields &fields,
                                QgsOraclePrimaryKeyType primaryKeyType,
                                const QList<int> &primaryKeyAttrs,
                                std::shared_ptr<QgsOracleSharedData> sharedData,
                                QVariantList &params );

    static QString whereClause( QgsFeatureIds featureIds,
                                const QgsFields &fields,
                                QgsOraclePrimaryKeyType primaryKeyType,
                                const QList<int> &primaryKeyAttrs,
                                std::shared_ptr<QgsOracleSharedData> sharedData,
                                QVariantList &params );

    static QString andWhereClauses( const QString &c1, const QString &c2 );
};


/**
 * Data shared between provider class and its feature sources. Ideally there should
 *  be as few members as possible because there could be simultaneous reads/writes
 *  from different threads and therefore locking has to be involved.
*/
class QgsOracleSharedData
{
  public:
    QgsOracleSharedData() = default;

    // FID lookups
    QgsFeatureId lookupFid( const QVariantList &v ); // lookup existing mapping or add a new one
    QVariant removeFid( QgsFeatureId fid );
    void insertFid( QgsFeatureId fid, const QVariantList &k );
    QVariantList lookupKey( QgsFeatureId featureId );

  protected:
    QMutex mMutex; //!< Access to all data members is guarded by the mutex

    QgsFeatureId mFidCounter = 0;                    // next feature id if map is used
    QMap<QVariantList, QgsFeatureId> mKeyToFid;      // map key values to feature id
    QMap<QgsFeatureId, QVariantList> mFidToKey;      // map feature back to fea
};

class QgsOracleProviderMetadata final: public QgsProviderMetadata
{
    Q_OBJECT

  public:
    QgsOracleProviderMetadata();
    QIcon icon() const override;
    QString getStyleById( const QString &uri, const QString &styleId, QString &errCause ) override;
    int listStyles( const QString &uri, QStringList &ids, QStringList &names, QStringList &descriptions, QString &errCause ) override;
    QString loadStyle( const QString &uri, QString &errCause ) override;
    bool styleExists( const QString &uri, const QString &styleId, QString &errorCause ) override;
    bool saveStyle( const QString &uri, const QString &qmlStyle, const QString &sldStyle, const QString &styleName,
                    const QString &styleDescription, const QString &uiFileContent, bool useAsDefault, QString &errCause ) override;
    void cleanupProvider() override;
    void initProvider() override;
    Qgis::VectorExportResult createEmptyLayer( const QString &uri,
        const QgsFields &fields, QgsWkbTypes::Type wkbType,
        const QgsCoordinateReferenceSystem &srs, bool overwrite,
        QMap<int, int> &oldToNewAttrIdxMap, QString &errorMessage,
        const QMap<QString, QVariant> *options ) override;

    QgsOracleProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList<QgsDataItemProvider *> dataItemProviders() const override;

    QgsTransaction *createTransaction( const QString &connString ) override;
    QMap<QString, QgsAbstractProviderConnection *> connections( bool cached = true ) override;
    QgsAbstractProviderConnection *createConnection( const QString &name ) override;
    QgsAbstractProviderConnection *createConnection( const QString &uri, const QVariantMap &configuration ) override;
    void deleteConnection( const QString &name ) override;
    void saveConnection( const QgsAbstractProviderConnection *createConnection, const QString &name ) override;

    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QList< QgsMapLayerType > supportedLayerTypes() const override;
};

#ifdef HAVE_GUI
class QgsOracleProviderGuiMetadata final: public QgsProviderGuiMetadata
{
  public:
    QgsOracleProviderGuiMetadata();
    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
    QList<QgsProjectStorageGuiProvider *> projectStorageGuiProviders() override;
    void registerGui( QMainWindow *mainWindow ) override;
};
#endif

#endif
