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
#include "qgsvectorlayerexporter.h"
#include "qgsfields.h"

#include <QStringList>
#include <QFile>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class QgsFeature;
class QgsField;
class QFile;
class QTextStream;

class QgsMssqlFeatureIterator;

#include "qgsdatasourceuri.h"
#include "qgsgeometry.h"
#include "qgsmssqlgeometryparser.h"

/**
\class QgsMssqlProvider
\brief Data provider for mssql server.
*
*/
class QgsMssqlProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:
    explicit QgsMssqlProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options );

    ~QgsMssqlProvider() override;

    static QSqlDatabase GetDatabase( const QString &service, const QString &host, const QString &database, const QString &username, const QString &password );

    QgsAbstractFeatureSource *featureSource() const override;

    static bool OpenDatabase( QSqlDatabase db );

    /* Implementation of functions from QgsVectorDataProvider */

    QString storageType() const override;
    QStringList subLayers() const override;
    QVariant minimumValue( int index ) const override;
    QVariant maximumValue( int index ) const override;
    QSet<QVariant> uniqueValues( int index, int limit = -1 ) const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;

    QgsWkbTypes::Type wkbType() const override;

    long featureCount() const override;

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

    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = nullptr ) override;

    bool deleteFeatures( const QgsFeatureIds &id ) override;

    bool addAttributes( const QList<QgsField> &attributes ) override;

    bool deleteAttributes( const QgsAttributeIds &attributes ) override;

    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;

    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;

    bool createSpatialIndex() override;

    bool createAttributeIndex( int field ) override;

    //! Convert a QgsField to work with MSSQL
    static bool convertField( QgsField &field );

    //! Convert values to quoted values for database work *
    static QString quotedValue( const QVariant &value );

    QString defaultValueClause( int fieldId ) const override;

    //! Import a vector layer into the database
    static QgsVectorLayerExporter::ExportError createEmptyLayer(
      const QString &uri,
      const QgsFields &fields,
      QgsWkbTypes::Type wkbType,
      const QgsCoordinateReferenceSystem &srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = nullptr,
      const QMap<QString, QVariant> *options = nullptr
    );

    QgsCoordinateReferenceSystem crs() const override;

    /**
     * Returns a thread-safe connection name for use with QSqlDatabase
     */
    static QString dbConnectionName( const QString &name );

  protected:
    //! Loads fields from input file to member attributeFields
    QVariant::Type DecodeSqlType( const QString &sqlTypeName );
    void loadFields();
    void loadMetadata();

  private:

    //! Fields
    QgsFields mAttributeFields;
    QMap<int, QString> mDefaultValues;
    QList<QString> mComputedColumns;

    mutable QgsMssqlGeometryParser mParser;

    //! Layer extent
    mutable QgsRectangle mExtent;

    bool mValid;

    bool mUseWkb;
    bool mUseEstimatedMetadata;
    bool mSkipFailures;

    long mNumberFeatures = 0;
    QString mFidColName;
    int mFidColIdx = -1;
    mutable long mSRId;
    QString mGeometryColName;
    QString mGeometryColType;

    // QString containing the last reported error message
    QString mLastError;

    // Coordinate reference system
    mutable QgsCoordinateReferenceSystem mCrs;

    mutable QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;

    // The database object
    QSqlDatabase mDatabase;

    // The current sql query
    QSqlQuery mQuery;

    // The current sql statement
    QString mStatement;

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

    // Sets the error messages
    void setLastError( const QString &error );

    static void mssqlWkbTypeAndDimension( QgsWkbTypes::Type wkbType, QString &geometryType, int &dim );
    static QgsWkbTypes::Type getWkbType( const QString &wkbType );

    friend class QgsMssqlFeatureSource;

    static int sConnectionId;
};

#endif // QGSMSSQLPROVIDER_H
