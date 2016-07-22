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
#include "qgsvectorlayerimport.h"
#include "qgsfield.h"

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
    explicit QgsMssqlProvider( const QString& uri = QString() );

    virtual ~QgsMssqlProvider();

    static QSqlDatabase GetDatabase( const QString& service, const QString& host, const QString& database, const QString& username, const QString& password );

    virtual QgsAbstractFeatureSource* featureSource() const override;

    static bool OpenDatabase( QSqlDatabase db );

    /* Implementation of functions from QgsVectorDataProvider */

    /**
     * Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const override;

    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used when the provider's source can combine layers
     * it knows about in some way before it hands them off to the provider.
     */
    virtual QStringList subLayers() const override;
    virtual QVariant minimumValue( int index ) const override;
    virtual QVariant maximumValue( int index ) const override;
    virtual void uniqueValues( int index, QList<QVariant> &uniqueValues, int limit = -1 ) const override;
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) const override;

    /**
     * Get feature type.
     * @return int representing the feature type
     */
    virtual Qgis::WkbType geometryType() const override;

    /**
     * Number of features in the layer
     * @return long containing number of features
     */
    virtual long featureCount() const override;

    /** Update the extent, feature count, wkb type and srid for this layer */
    void UpdateStatistics( bool estimate ) const;

    virtual QgsFields fields() const override;

    QString subsetString() const override;

    /** Mutator for sql where clause used to limit dataset size */
    bool setSubsetString( const QString& theSQL, bool updateFeatureCount = true ) override;

    virtual bool supportsSubsetString() const override { return true; }

    /** Returns a bitmask containing the supported capabilities
        Note, some capabilities may change depending on whether
        a spatial filter is active on this provider, so it may
        be prudent to check this value per intended operation.
     */
    virtual int capabilities() const override;


    /* Implementation of functions from QgsDataProvider */

    /** Return a provider name

        Essentially just returns the provider key.  Should be used to build file
        dialogs so that providers can be shown with their supported types. Thus
        if more than one provider supports a given format, the user is able to
        select a specific provider to open that file.

        @note

        Instead of being pure virtual, might be better to generalize this
        behavior and presume that none of the sub-classes are going to do
        anything strange with regards to their name or description?
     */
    QString name() const override;

    /** Return description

        Return a terse string describing what the provider is.

        @note

        Instead of being pure virtual, might be better to generalize this
        behavior and presume that none of the sub-classes are going to do
        anything strange with regards to their name or description?
     */
    QString description() const override;

    virtual QgsRectangle extent() const override;

    bool isValid() const override;

    virtual bool isSaveAndLoadStyleToDBSupported() const override { return true; }

    /** Writes a list of features to the database*/
    virtual bool addFeatures( QgsFeatureList & flist ) override;

    /** Deletes a feature*/
    virtual bool deleteFeatures( const QgsFeatureIds & id ) override;

    /**
     * Adds new attributes
     * @param attributes list of new attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool addAttributes( const QList<QgsField> &attributes ) override;

    /**
     * Deletes existing attributes
     * @param attributes a set containing names of attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool deleteAttributes( const QgsAttributeIds &attributes ) override;

    /** Changes attribute values of existing features */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;

    /** Changes existing geometries*/
    virtual bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;

    /**
     * Create a spatial index for the current layer
     */
    virtual bool createSpatialIndex() override;

    /** Create an attribute index on the datasource*/
    virtual bool createAttributeIndex( int field ) override;

    /** Convert a QgsField to work with MSSQL */
    static bool convertField( QgsField &field );

    /** Convert values to quoted values for database work **/
    static QString quotedValue( const QVariant& value );

    QVariant defaultValue( int fieldId ) const override;

    /** Import a vector layer into the database */
    static QgsVectorLayerImport::ImportError createEmptyLayer(
      const QString& uri,
      const QgsFields &fields,
      Qgis::WkbType wkbType,
      const QgsCoordinateReferenceSystem &srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = nullptr,
      const QMap<QString, QVariant> *options = nullptr
    );

    virtual QgsCoordinateReferenceSystem crs() const override;

  protected:
    /** Loads fields from input file to member attributeFields */
    QVariant::Type DecodeSqlType( const QString& sqlTypeName );
    void loadFields();
    void loadMetadata();

  private:

    //! Fields
    QgsFields mAttributeFields;
    QMap<int, QVariant> mDefaultValues;

    mutable QgsMssqlGeometryParser mParser;

    //! Layer extent
    mutable QgsRectangle mExtent;

    bool mValid;

    bool mUseWkb;
    bool mUseEstimatedMetadata;
    bool mSkipFailures;

    long mNumberFeatures;
    QString mFidColName;
    mutable long mSRId;
    QString mGeometryColName;
    QString mGeometryColType;

    // QString containing the last reported error message
    QString mLastError;

    // Coordinate reference sytem
    mutable QgsCoordinateReferenceSystem mCrs;

    mutable Qgis::WkbType mWkbType;

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
    void setLastError( const QString& error )
    {
      mLastError = error;
    }

    static void mssqlWkbTypeAndDimension( Qgis::WkbType wkbType, QString &geometryType, int &dim );
    static Qgis::WkbType getWkbType( const QString& geometryType, int dim );

    friend class QgsMssqlFeatureSource;

    static int sConnectionId;
};

#endif // QGSMSSQLPROVIDER_H
