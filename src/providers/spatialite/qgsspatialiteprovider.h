/***************************************************************************
            qgsspatialiteprovider.h Data provider for SpatiaLite DBMS
begin                : Dec 2008
copyright            : (C) 2008 Sandro Furieri
email                : a.furieri@lqt.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSPATIALITEPROVIDER_H
#define QGSSPATIALITEPROVIDER_H

extern "C"
{
#include <sys/types.h>
#include <sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>
}

#include "qgsvectordataprovider.h"
#include "qgsrectangle.h"
#include "qgsfields.h"
#include "qgswkbtypes.h"

#include <list>
#include <queue>
#include <fstream>
#include <set>

#include "qgsprovidermetadata.h"

class QgsFeature;
class QgsField;

class QgsSqliteHandle;
class QgsSpatiaLiteFeatureIterator;
class QgsSpatiaLiteTransaction;
class QgsTransaction;

#include "qgsdatasourceuri.h"

/**
 * \class QgsSpatiaLiteProvider
 * \brief Data provider for SQLite/SpatiaLite layers.
 *
 * This provider implements the
 * interface defined in the QgsDataProvider class to provide access to spatial
 * data residing in a SQLite/SpatiaLite enabled database.
  */
class QgsSpatiaLiteProvider final: public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    static const QString SPATIALITE_KEY;
    static const QString SPATIALITE_DESCRIPTION;

    //! Import a vector layer into the database
    static Qgis::VectorExportResult createEmptyLayer(
      const QString &uri,
      const QgsFields &fields,
      QgsWkbTypes::Type wkbType,
      const QgsCoordinateReferenceSystem &srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = nullptr,
      const QMap<QString, QVariant> *coordinateTransformContext = nullptr
    );

    /**
     * Constructor of the vector provider
     * \param uri uniform resource locator (URI) for a dataset
     * \param options generic data provider options
     */
    explicit QgsSpatiaLiteProvider( QString const &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~ QgsSpatiaLiteProvider() override;

    QgsAbstractFeatureSource *featureSource() const override;
    QString storageType() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    QString subsetString() const override;
    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
    bool supportsSubsetString() const override { return true; }
    QgsWkbTypes::Type wkbType() const override;
    //! Return the table schema condition
    static QString tableSchemaCondition( const QgsDataSourceUri &dsUri );

    /**
     * Returns the number of layers for the current data source
     *
     * \note Should this be subLayerCount() instead?
     */
    size_t layerCount() const;

    long long featureCount() const override;
    QgsRectangle extent() const override;
    void updateExtents() override;
    QgsFields fields() const override;
    QVariant minimumValue( int index ) const override;
    QVariant maximumValue( int index ) const override;
    QSet<QVariant> uniqueValues( int index, int limit = -1 ) const override;
    QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
                                       QgsFeedback *feedback = nullptr ) const override;

    bool isValid() const override;
    bool isSaveAndLoadStyleToDatabaseSupported() const override;
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool deleteFeatures( const QgsFeatureIds &id ) override;
    bool truncate() override;
    bool addAttributes( const QList<QgsField> &attributes ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    QgsVectorDataProvider::Capabilities capabilities() const override;
    QVariant defaultValue( int fieldId ) const override;
    bool skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value = QVariant() ) const override;
    bool createAttributeIndex( int field ) override;
    SpatialIndexPresence hasSpatialIndex() const override;

    /**
     * The SpatiaLite provider does its own transforms so we return
     * true for the following three functions to indicate that transforms
     * should not be handled by the QgsCoordinateTransform object. See the
     * documentation on QgsVectorDataProvider for details on these functions.
     */
    // XXX For now we have disabled native transforms in the SpatiaLite
    //   (following the PostgreSQL provider example)
    bool supportsNativeTransform()
    {
      return false;
    }

    QString name() const override;
    QString description() const override;
    QgsAttributeList pkAttributeIndexes() const override;
    void invalidateConnections( const QString &connection ) override;
    QList<QgsRelation> discoverRelations( const QgsVectorLayer *target, const QList<QgsVectorLayer *> &layers ) const override;

    static QString providerKey();

    // static functions
    static void convertToGeosWKB( const unsigned char *blob, int blob_size,
                                  unsigned char **wkb, int *geom_size );
    static int computeMultiWKB3Dsize( const unsigned char *p_in, int little_endian,
                                      int endian_arch );

    struct SLFieldNotFound {}; //! Exception to throw

    struct SLException
    {
        explicit SLException( char *msg ) : errMsg( msg )
        {
        }

        SLException( const SLException &e ) : errMsg( e.errMsg )
        {
        }

        ~SLException()
        {
          if ( errMsg )
            sqlite3_free( errMsg );
        }

        SLException &operator=( const SLException &other ) = delete;

        QString errorMessage() const
        {
          return errMsg ? QString::fromUtf8( errMsg ) : QStringLiteral( "unknown cause" );
        }

      private:
        char *errMsg = nullptr;

    };

    //! Check if version is above major and minor
    static bool versionIsAbove( sqlite3 *sqlite_handle, int major, int minor = 0 );


    /**
     * sqlite3 handles pointer
     */
    QgsSqliteHandle *mHandle = nullptr;

    /**
     * Sqlite exec sql wrapper for SQL logging
     */
    static int exec_sql( sqlite3 *handle, const QString &sql, const QString &uri, char *errMsg = nullptr, const QString &origin = QString() );

  private:

    //! Loads fields from input file to member mAttributeFields
    void loadFields();

    //! For views, try to get primary key from a dedicated meta table
    void determineViewPrimaryKey();

    //! Returns integer primary key(s) from a table name
    QStringList tablePrimaryKeys( const QString &tableName ) const;

    //! Check if a table/view has any triggers.  Triggers can be used on views to make them editable.
    bool hasTriggers();

    //! Check if a table has a row id (internal primary key)
    bool hasRowid();

    //! Convert a QgsField to work with SL
    static bool convertField( QgsField &field );

    QString geomParam() const;

    //! Gets SpatiaLite version string
    QString spatialiteVersion();

    /**
     * Search all the layers using the given table.
     */
    static QList<QgsVectorLayer *> searchLayers( const QList<QgsVectorLayer *> &layers, const QString &connectionInfo, const QString &tableName );

    QgsSpatiaLiteTransaction *mTransaction = nullptr;

    QgsTransaction *transaction() const override;

    void setTransaction( QgsTransaction *transaction ) override;

    QgsFields mAttributeFields;

    //! Map of field index to default value SQL fragments
    QMap<int, QString> mDefaultValueClause;

    //! Flag indicating if the layer data source is a valid SpatiaLite layer
    bool mValid = false;

    //! Flag indicating if the layer data source is based on a query
    bool mIsQuery = false;

    //! Flag indicating if ROWID has been injected in the query
    bool mRowidInjectedInQuery = false;

    //! Flag indicating if the layer data source is based on a plain Table
    bool mTableBased = false;

    //! Flag indicating if the layer data source is based on a View
    bool mViewBased = false;

    //! Flag indicating if the layer data source is based on a VirtualShape
    bool mVShapeBased = false;

    //! Flag indicating if the layer data source has ReadOnly restrictions
    bool mReadOnly = false;

    //! DB full path
    QString mSqlitePath;

    //! Name of the table with no schema
    QString mTableName;

    //! Name of the table or subquery
    QString mQuery;

    //! Name of the primary key column in the table
    QString mPrimaryKey;

    //! Flag indicating whether the primary key is auto-generated
    bool mPrimaryKeyAutoIncrement = false;

    //! List of primary key columns in the table
    QgsAttributeList mPrimaryKeyAttrs;

    //! Name of the geometry column in the table
    QString mGeometryColumn;

    //! Map of field index to default value
    QMap<int, QString> mDefaultValues;

    //! Name of the SpatialIndex table
    QString mIndexTable;

    //! Name of the SpatialIndex geometry column
    QString mIndexGeometry;

    //! Geometry type
    QgsWkbTypes::Type mGeomType = QgsWkbTypes::Unknown;

    //! SQLite handle
    sqlite3 *mSqliteHandle = nullptr;

    //! String used to define a subset of the layer
    QString mSubsetString;

    //! CoordDimensions of the layer
    int nDims;

    //! Spatial reference id of the layer
    int mSrid = -1;

    //! auth id
    QString mAuthId;

    //! proj4text
    QString mProj4text;

    //! Rectangle that contains the extent (bounding box) of the layer
    QgsRectangle mLayerExtent;

    //! Number of features in the layer
    long long mNumberFeatures = 0;

    //! this Geometry is supported by an R*Tree spatial index
    bool mSpatialIndexRTree = false;

    //! this Geometry is supported by an MBR cache spatial index
    bool mSpatialIndexMbrCache = false;

    QgsVectorDataProvider::Capabilities mEnabledCapabilities = QgsVectorDataProvider::Capabilities();

    QgsField field( int index ) const;

    //! SpatiaLite version string
    QString mSpatialiteVersionInfo;

    //! Are mSpatialiteVersionMajor, mSpatialiteVersionMinor valid?
    bool mGotSpatialiteVersion = false;

    //! SpatiaLite major version
    int mSpatialiteVersionMajor = 0;

    //! SpatiaLite minor version
    int mSpatialiteVersionMinor = 0;

    //! Internal transaction handling (for addFeatures etc.)
    int mSavepointId;
    static QAtomicInt sSavepointId;

    /**
     * internal utility functions used to handle common SQLite tasks
     */
    //void sqliteOpen();
    void closeDb();
    bool checkLayerType();
    bool getGeometryDetails();
    bool getTableGeometryDetails();
    bool getViewGeometryDetails();
    bool getVShapeGeometryDetails();
    bool getQueryGeometryDetails();
    bool getSridDetails();
    bool getTableSummary();
    bool checkLayerTypeAbstractInterface( gaiaVectorLayerPtr lyr );
    bool getGeometryDetailsAbstractInterface( gaiaVectorLayerPtr lyr );
    bool getTableSummaryAbstractInterface( gaiaVectorLayerPtr lyr );
    void loadFieldsAbstractInterface( gaiaVectorLayerPtr lyr );
    void getViewSpatialIndexName();
    bool prepareStatement( sqlite3_stmt *&stmt,
                           const QgsAttributeList &fetchAttributes,
                           bool fetchGeometry,
                           QString whereClause );
    bool getFeature( sqlite3_stmt *stmt, bool fetchGeometry,
                     QgsFeature &feature,
                     const QgsAttributeList &fetchAttributes );

    void updatePrimaryKeyCapabilities();

    int computeSizeFromMultiWKB2D( const unsigned char *p_in, int nDims,
                                   int little_endian,
                                   int endian_arch );
    int computeSizeFromMultiWKB3D( const unsigned char *p_in, int nDims,
                                   int little_endian,
                                   int endian_arch );
    void convertFromGeosWKB2D( const unsigned char *blob, int blob_size,
                               unsigned char *wkb, int geom_size,
                               int nDims, int little_endian, int endian_arch );
    void convertFromGeosWKB3D( const unsigned char *blob, int blob_size,
                               unsigned char *wkb, int geom_size,
                               int nDims, int little_endian, int endian_arch );
    void convertFromGeosWKB( const unsigned char *blob, int blob_size,
                             unsigned char **wkb, int *geom_size,
                             int dims );
    int computeSizeFromGeosWKB3D( const unsigned char *blob, int size,
                                  QgsWkbTypes::Type type, int nDims, int little_endian,
                                  int endian_arch );
    int computeSizeFromGeosWKB2D( const unsigned char *blob, int size,
                                  QgsWkbTypes::Type type, int nDims, int little_endian,
                                  int endian_arch );

    void fetchConstraints();

    void insertDefaultValue( int fieldIndex, QString defaultVal );

    /**
     * Handles an error encountered while executing an sql statement.
     */
    void handleError( const QString &sql, char *errorMessage, const QString &savepointId );

    /**
     * Returns the sqlite handle to be used, if we are inside a transaction it will be the transaction's handle
     */
    sqlite3 *sqliteHandle( ) const;

    static QString createIndexName( QString tableName, QString field );

    friend class QgsSpatiaLiteFeatureSource;

    // QgsVectorDataProvider interface
  public:
    virtual QString defaultValueClause( int fieldIndex ) const override;

    Qgis::VectorLayerTypeFlags vectorLayerTypeFlags() const override;

};

class QgsSpatiaLiteProviderMetadata final: public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsSpatiaLiteProviderMetadata();
    QIcon icon() const override;

    void cleanupProvider() override;
    QString getStyleById( const QString &uri, const QString &styleId, QString &errCause ) override;
    bool styleExists( const QString &uri, const QString &styleId, QString &errorCause ) override;
    bool saveStyle( const QString &uri, const QString &qmlStyle, const QString &sldStyle,
                    const QString &styleName, const QString &styleDescription,
                    const QString &uiFileContent, bool useAsDefault, QString &errCause ) override;
    QString loadStyle( const QString &uri, QString &errCause ) override;
    int listStyles( const QString &uri, QStringList &ids, QStringList &names,
                    QStringList &descriptions, QString &errCause ) override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    ProviderCapabilities providerCapabilities() const override;
    QgsSpatiaLiteProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList< QgsMapLayerType > supportedLayerTypes() const override;

    Qgis::VectorExportResult createEmptyLayer( const QString &uri, const QgsFields &fields,
        QgsWkbTypes::Type wkbType, const QgsCoordinateReferenceSystem &srs,
        bool overwrite, QMap<int, int> &oldToNewAttrIdxMap, QString &errorMessage,
        const QMap<QString, QVariant> *options ) override;
    bool createDb( const QString &dbPath, QString &errCause ) override;
    QList< QgsDataItemProvider * > dataItemProviders() const override;

    // QgsProviderMetadata interface
  public:
    QMap<QString, QgsAbstractProviderConnection *> connections( bool cached ) override;
    QgsAbstractProviderConnection *createConnection( const QString &name ) override;
    void deleteConnection( const QString &name ) override;
    void saveConnection( const QgsAbstractProviderConnection *connection, const QString &name ) override;
    QgsTransaction *createTransaction( const QString &connString ) override;

  protected:

    QgsAbstractProviderConnection *createConnection( const QString &uri, const QVariantMap &configuration ) override;
};


// clazy:excludeall=qstring-allocations

#endif
