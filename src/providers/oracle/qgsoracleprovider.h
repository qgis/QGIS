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
#include "qgsvectorlayerimport.h"
#include "qgsoracletablemodel.h"
#include "qgsdatasourceuri.h"

#include <QVector>
#include <QQueue>
#include <QSqlQuery>
#include <QSqlError>

class QgsFeature;
class QgsField;
class QgsGeometry;
class QgsOracleFeatureIterator;

/**
  \class QgsOracleProvider
  \brief Data provider for oracle layers.

  This provider implements the
  interface defined in the QgsDataProvider class to provide access to spatial
  data residing in a oracle enabled database.
  */
class QgsOracleProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    /** Import a vector layer into the database */
    static QgsVectorLayerImport::ImportError createEmptyLayer(
      const QString& uri,
      const QgsFields &fields,
      QGis::WkbType wkbType,
      const QgsCoordinateReferenceSystem *srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = 0,
      const QMap<QString, QVariant> *options = 0
    );

    /**
     * Constructor for the provider. The uri must be in the following format:
     * host=localhost user=gsherman dbname=test password=xxx table=test.alaska (the_geom)
     * @param uri String containing the required parameters to connect to the database
     * and query the table.
     */
    QgsOracleProvider( QString const &uri = "" );

    //! Destructor
    virtual ~QgsOracleProvider();

    /**
      *   Returns the permanent storage type for this layer as a friendly name.
      */
    virtual QString storageType() const;

    /*! Get the QgsCoordinateReferenceSystem for this layer
     * @note Must be reimplemented by each provider.
     * If the provider isn't capable of returning
     * its projection an empty srs will be returned
     */
    virtual QgsCoordinateReferenceSystem crs();

    /** Get the feature type. This corresponds to
     * WKBPoint,
     * WKBLineString,
     * WKBPolygon,
     * WKBMultiPoint,
     * WKBMultiLineString or
     * WKBMultiPolygon
     * as defined in qgis.h
     */
    QGis::WkbType geometryType() const;

    /** return the number of layers for the current data source
     * @note Should this be subLayerCount() instead?
    */
    size_t layerCount() const;

    /**
     * Get the number of features in the layer
     */
    long featureCount() const;

    /**
     * Get the number of fields in the layer
     */
    uint fieldCount() const;

    /**
     * Return a string representation of the endian-ness for the layer
     */
    QString endianString();

    /**
     * Changes the stored extent for this layer to the supplied extent.
     * For example, this is called when the extent worker thread has a result.
     */
    void setExtent( QgsRectangle& newExtent );

    /** Return the extent for this data layer
    */
    virtual QgsRectangle extent();

    /** Update the extent
    */
    virtual void updateExtents();

    /** Determine the fields making up the primary key
    */
    bool determinePrimaryKey();

    /**
     * Get the field information for the layer
     * @return vector of QgsField objects
     */
    const QgsFields &fields() const;

    /**
     * Return a short comment for the data that this provider is
     * providing access to (e.g. the comment for oracle table).
     */
    QString dataComment() const;

    /** Reset the layer
     */
    void rewind();

    /** Returns the minimum value of an attribute
     *  @param index the index of the attribute */
    QVariant minimumValue( int index );

    /** Returns the maximum value of an attribute
     *  @param index the index of the attribute */
    QVariant maximumValue( int index );

    /** Return the unique values of an attribute
     *  @param index the index of the attribute
     *  @param values reference to the list of unique values */
    virtual void uniqueValues( int index, QList<QVariant> &uniqueValues, int limit = -1 );

    /**Returns true if layer is valid
    */
    bool isValid();

    QgsAttributeList pkAttributeIndexes() { return mPrimaryKeyAttrs; }

    /**Returns the default value for field specified by @c fieldName */
    QVariant defaultValue( QString fieldName, QString tableName = QString::null, QString schemaName = QString::null );

    /**Returns the default value for field specified by @c fieldId */
    QVariant defaultValue( int fieldId );

    /**Adds a list of features
      @return true in case of success and false in case of failure*/
    bool addFeatures( QgsFeatureList & flist );

    /**Deletes a list of features
      @param id list of feature ids
      @return true in case of success and false in case of failure*/
    bool deleteFeatures( const QgsFeatureIds & id );

    /**Adds new attributes
      @param name map with attribute name as key and type as value
      @return true in case of success and false in case of failure*/
    bool addAttributes( const QList<QgsField> &attributes );

    /**Deletes existing attributes
      @param names of the attributes to delete
      @return true in case of success and false in case of failure*/
    bool deleteAttributes( const QgsAttributeIds & name );

    /**Changes attribute values of existing features
      @param attr_map a map containing the new attributes. The integer is the feature id,
      the first QString is the attribute name and the second one is the new attribute value
      @return true in case of success and false in case of failure*/
    bool changeAttributeValues( const QgsChangedAttributesMap & attr_map );

    /**
       Changes geometries of existing features
       @param geometry_map   A QMap containing the feature IDs to change the geometries of.
                             the second map parameter being the new geometries themselves
       @return               true in case of success and false in case of failure
     */
    bool changeGeometryValues( QgsGeometryMap & geometry_map );

    /**Tries to create an spatial index file for faster access if only a subset of the features is required
     @return true in case of success*/
    bool createSpatialIndex();

    //! Get the table name associated with this provider instance
    QString getTableName();

    /** Accessor for sql where clause used to limit dataset */
    QString subsetString();

    /** mutator for sql where clause used to limit dataset size */
    bool setSubsetString( QString theSQL, bool updateFeatureCount = true );

    virtual bool supportsSubsetString() { return true; }

    /**Returns a bitmask containing the supported capabilities*/
    int capabilities() const;

    /** return a provider name

    Essentially just returns the provider key.  Should be used to build file
    dialogs so that providers can be shown with their supported types. Thus
    if more than one provider supports a given format, the user is able to
    select a specific provider to open that file.

    @note

    Instead of being pure virtual, might be better to generalize this
    behavior and presume that none of the sub-classes are going to do
    anything strange with regards to their name or description?

    */
    QString name() const;

    /** return description

    Return a terse string describing what the provider is.

    @note

    Instead of being pure virtual, might be better to generalize this
    behavior and presume that none of the sub-classes are going to do
    anything strange with regards to their name or description?

    */
    QString description() const;

    /**
     * Query the provider for features specified in request.
     */
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request = QgsFeatureRequest() );

    static bool exec( QSqlQuery &qry, QString sql );

  private:
    QString whereClause( QgsFeatureId featureId ) const;
    QString pkParamWhereClause() const;
    QString paramValue( QString fieldvalue, const QString &defaultValue ) const;
    void appendGeomParam( QgsGeometry *geom, QSqlQuery &qry ) const;
    void appendPkParams( QgsFeatureId fid, QSqlQuery &qry ) const;

    bool hasSufficientPermsAndCapabilities();

    const QgsField &field( int index ) const;

    /** Load the field list
    */
    bool loadFields();

    /** convert a QgsField to work with Oracle */
    static bool convertField( QgsField &field );

    QgsFields mAttributeFields;  //! List of fields
    QVariantList mDefaultValues; //! List of default values
    QString mDataComment;

    //! Data source URI struct for this layer
    QgsDataSourceURI mUri;

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
     * Data type for the primary key
     */
    enum { pktUnknown, pktInt, pktRowId, pktFidMap } mPrimaryKeyType;

    /**
     * List of primary key attributes for fetching features.
     */
    QList<int> mPrimaryKeyAttrs;
    QString mPrimaryKeyDefault;

    QString mGeometryColumn;          //! name of the geometry column
    QgsRectangle mLayerExtent;        //! Rectangle that contains the extent (bounding box) of the layer
    mutable long mFeaturesCounted;    //! Number of features in the layer
    int mSrid;                        //! srid of column
    int mEnabledCapabilities;         //! capabilities of layer

    QGis::WkbType mDetectedGeomType;  //! geometry type detected in the database
    QGis::WkbType mRequestedGeomType; //! geometry type requested in the uri

    bool getGeometryDetails();

    /* Use estimated metadata. Uses fast table counts, geometry type and extent determination */
    bool mUseEstimatedMetadata;

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
      {}

      QString errorMessage() const
      {
        return mWhat;
      }

    private:
      QString mWhat;
    };

    // A function that determines if the given schema.table.column
    // contains unqiue entries
    bool uniqueData( QString query, QString colName );

    void disconnectDb();

    static QString quotedIdentifier( QString ident ) { return QgsOracleConn::quotedIdentifier( ident ); }
    static QString quotedValue( QVariant value ) { return QgsOracleConn::quotedValue( value ); }

    QgsFeatureId lookupFid( const QVariant &v ); //! lookup existing mapping or add a new one

    QMap<QVariant, QgsFeatureId> mKeyToFid;  //! map key values to feature id
    QMap<QgsFeatureId, QVariant> mFidToKey;  //! map feature back to fea
    QgsFeatureId mFidCounter;                //! next feature id if map is used
    QgsOracleConn *mConnection;

    QString mSpatialIndex;                   //! name of spatial index of geometry column
    bool mHasSpatial;                        //! Oracle Spatial is installed

    friend QgsOracleFeatureIterator;
};

#endif
