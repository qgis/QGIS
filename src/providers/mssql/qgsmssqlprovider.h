/***************************************************************************
      qgsmssqlprovider.h  -  Data provider for mssql server
                             -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMSSQLPROVIDER_H
#define QGSMSSQLPROVIDER_H

#include "qgsvectordataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfields.h"

#include <QStringList>
#include <QFile>
#include <QVariantMap>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "qgsprovidermetadata.h"

class QgsFeature;
class QgsField;
class QFile;
class QTextStream;

class QgsMssqlFeatureIterator;
class QgsMssqlSharedData;
class QgsMssqlTransaction;
class QgsMssqlDatabase;

#include "qgsdatasourceuri.h"
#include "qgsgeometry.h"
#include "qgsmssqlgeometryparser.h"

enum QgsMssqlPrimaryKeyType
{
  PktUnknown,
  PktInt,
  PktFidMap
};

/**
 * \class QgsMssqlProvider
 * \brief Data provider for mssql server.
*/
class QgsMssqlProvider final: public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    static const QString MSSQL_PROVIDER_KEY;
    static const QString MSSQL_PROVIDER_DESCRIPTION;

    explicit QgsMssqlProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions,
                               QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsMssqlProvider() override;

    QgsAbstractFeatureSource *featureSource() const override;

    /* Implementation of functions from QgsVectorDataProvider */

    void updateExtents() override;
    QString storageType() const override;
    QStringList subLayers() const override;
    QVariant minimumValue( int index ) const override;
    QVariant maximumValue( int index ) const override;
    QSet<QVariant> uniqueValues( int index, int limit = -1 ) const override;
    QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
                                       QgsFeedback *feedback = nullptr ) const override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;

    QgsWkbTypes::Type wkbType() const override;

    long long featureCount() const override;

    //! Update the extent, feature count, wkb type and srid for this layer
    void UpdateStatistics( bool estimate ) const;

    QgsFields fields() const override;

    QString subsetString() const override;

    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;

    bool supportsSubsetString() const override { return true; }

    QgsVectorDataProvider::Capabilities capabilities() const override;


    /* Implementation of functions from QgsDataProvider */

    QString name() const override;

    QString description() const override;

    QgsAttributeList pkAttributeIndexes() const override;

    QgsRectangle extent() const override;

    bool isValid() const override;

    bool isSaveAndLoadStyleToDatabaseSupported() const override { return true; }

    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;

    bool deleteFeatures( const QgsFeatureIds &id ) override;

    bool addAttributes( const QList<QgsField> &attributes ) override;

    bool deleteAttributes( const QgsAttributeIds &attributes ) override;

    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;

    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;

    bool createSpatialIndex() override;

    bool createAttributeIndex( int field ) override;

    //! Convert a QgsField to work with MSSQL
    static bool convertField( QgsField &field );

    //! Convert values to quoted values for database work
    static QString quotedValue( const QVariant &value );
    static QString quotedIdentifier( const QString &value );

    QString defaultValueClause( int fieldId ) const override;
    QVariant defaultValue( int fieldId ) const override;

    //! Convert time value
    static QVariant convertTimeValue( const QVariant &value );


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

    QgsCoordinateReferenceSystem crs() const override;

    void setTransaction( QgsTransaction *transaction ) override;
    QgsTransaction *transaction() const override;

    std::shared_ptr<QgsMssqlDatabase> connection() const;

    void handlePostCloneOperations( QgsVectorDataProvider *source ) override;

  protected:
    //! Loads fields from input file to member attributeFields
    QVariant::Type DecodeSqlType( const QString &sqlTypeName );
    void loadFields();
    void loadMetadata();

  private:

    bool execLogged( QSqlQuery &qry, const QString &sql, const QString &queryOrigin = QString() ) const;

    //! Fields
    QgsFields mAttributeFields;
    QMap<int, QString> mDefaultValues;
    QList<QString> mComputedColumns;

    mutable QgsMssqlGeometryParser mParser;

    //! Layer extent
    mutable QgsRectangle mExtent;

    bool mValid = false;

    bool mUseWkb = false;
    bool mUseEstimatedMetadata = false;
    bool mSkipFailures = false;
    bool mUseGeometryColumnsTableForExtent = false;

    long long mNumberFeatures = 0;

    /**
      *
      * Data type for the primary key
      */
    QgsMssqlPrimaryKeyType mPrimaryKeyType = PktUnknown;

    /**
     * List of primary key attributes for fetching features.
     */
    QList<int> mPrimaryKeyAttrs;

    mutable long mSRId;
    QString mGeometryColName;
    QString mGeometryColType;

    // QString containing the last reported error message
    QString mLastError;

    // Coordinate reference system
    mutable QgsCoordinateReferenceSystem mCrs;

    mutable QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;

    // current layer name
    QString mSchemaName;
    QString mTableName;

    // login
    QString mUserName;
    QString mPassword;

    // server access
    QString mService;
    QString mDatabaseName;
    QString mHost;

    // available tables
    QStringList mTables;

    // SQL statement used to limit the features retrieved
    QString mSqlWhereClause;

    bool mDisableInvalidGeometryHandling = false;

    // this makes sure that we keep the DB connection open while the provider is alive
    std::shared_ptr<QgsMssqlDatabase> mConn;

    QgsMssqlTransaction *mTransaction = nullptr;

    // Sets the error messages
    void setLastError( const QString &error );

    QSqlQuery createQuery() const;

    static void mssqlWkbTypeAndDimension( QgsWkbTypes::Type wkbType, QString &geometryType, int &dim );
    static QgsWkbTypes::Type getWkbType( const QString &wkbType );

    QString whereClauseFid( QgsFeatureId fid );

    static QStringList parseUriKey( const QString &key );

    //! Extract the extent from the geometry_columns table, returns false if fails
    bool getExtentFromGeometryColumns( QgsRectangle &extent ) const;
    //! Extract primary key(s) from the geometry_columns table, returns false if fails
    bool getPrimaryKeyFromGeometryColumns( QStringList &primaryKeys );

    std::shared_ptr<QgsMssqlSharedData> mShared;

    friend class QgsMssqlFeatureSource;

    static int sConnectionId;
};

/**
 * Data shared between provider class and its feature sources. Ideally there should
 * be as few members as possible because there could be simultaneous reads/writes
 * from different threads and therefore locking has to be involved.
*/
class QgsMssqlSharedData
{
  public:
    QgsMssqlSharedData() = default;

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

class QgsMssqlProviderMetadata final: public QgsProviderMetadata
{
  public:
    QgsMssqlProviderMetadata();
    QString getStyleById( const QString &uri, const QString &styleId, QString &errCause ) override;
    int listStyles( const QString &uri, QStringList &ids, QStringList &names, QStringList &descriptions, QString &errCause ) override;
    QString loadStyle( const QString &uri, QString &errCause ) override;
    bool styleExists( const QString &uri, const QString &styleId, QString &errorCause ) override;
    bool saveStyle( const QString &uri, const QString &qmlStyle, const QString &sldStyle,
                    const QString &styleName, const QString &styleDescription,
                    const QString &uiFileContent, bool useAsDefault, QString &errCause ) override;

    Qgis::VectorExportResult createEmptyLayer(
      const QString &uri,
      const QgsFields &fields,
      QgsWkbTypes::Type wkbType,
      const QgsCoordinateReferenceSystem &srs,
      bool overwrite,
      QMap<int, int> &oldToNewAttrIdxMap,
      QString &errorMessage,
      const QMap<QString, QVariant> *options ) override;
    QgsMssqlProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    virtual QList< QgsDataItemProvider * > dataItemProviders() const override;
    QgsTransaction *createTransaction( const QString &connString ) override;

    // Connections API
    QMap<QString, QgsAbstractProviderConnection *> connections( bool cached = true ) override;
    QgsAbstractProviderConnection *createConnection( const QString &name ) override;
    QgsAbstractProviderConnection *createConnection( const QString &uri, const QVariantMap &configuration ) override;
    void deleteConnection( const QString &name ) override;
    void saveConnection( const QgsAbstractProviderConnection *createConnection, const QString &name ) override;

    // Data source URI API
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;

  private:

    bool execLogged( QSqlQuery &qry, const QString &sql, const QString &uri, const QString &queryOrigin = QString() ) const;

};

#endif // QGSMSSQLPROVIDER_H
