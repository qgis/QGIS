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
#include "qgsvectorlayerimport.h"
#include "qgspostgresconn.h"


class QgsFeature;
class QgsField;
class QgsGeometry;

class QgsPostgresFeatureIterator;

#include "qgsdatasourceuri.h"

/**
  \class QgsPostgresProvider
  \brief Data provider for PostgreSQL/PostGIS layers.

  This provider implements the
  interface defined in the QgsDataProvider class to provide access to spatial
  data residing in a PostgreSQL/PostGIS enabled database.
  */
class QgsPostgresProvider : public QgsVectorDataProvider
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
    QgsPostgresProvider( QString const &uri = "" );

    //! Destructor
    virtual ~QgsPostgresProvider();

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

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request );

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
     * providing access to (e.g. the comment for postgres table).
     */
    QString dataComment() const;

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

    /**Returns the possible enum values of an attribute. Returns an empty stringlist if a provider does not support enum types
      or if the given attribute is not an enum type.
     * @param index the index of the attribute
     * @param enumList reference to the list to fill
      @note: added in version 1.2*/
    virtual void enumValues( int index, QStringList& enumList );

    /**Returns true if layer is valid
    */
    bool isValid();


    /**
     * It returns true. Saving style to db is supported by this provider
     */
    virtual bool isSaveAndLoadStyleToDBSupported() { return true; }

    QgsAttributeList attributeIndexes();

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

    //! Get the postgres connection
    PGconn * pgConnection();

    //! Get the table name associated with this provider instance
    QString getTableName();

    /** Accessor for sql where clause used to limit dataset */
    QString subsetString();

    /** mutator for sql where clause used to limit dataset size */
    bool setSubsetString( QString theSQL, bool updateFeatureCount = true );

    virtual bool supportsSubsetString() { return true; }

    /**Returns a bitmask containing the supported capabilities*/
    int capabilities() const;

    /** The Postgres provider does its own transforms so we return
     * true for the following three functions to indicate that transforms
     * should not be handled by the QgsCoordinateTransform object. See the
     * documentation on QgsVectorDataProvider for details on these functions.
     */
    // XXX For now we have disabled native transforms in the PG provider since
    //     it appears there are problems with some of the projection definitions
    bool supportsNativeTransform() {return false;}


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

  signals:
    /**
     *   This is emitted whenever the worker thread has fully calculated the
     *   PostGIS extents for this layer, and its event has been received by this
     *   provider.
     */
    void fullExtentCalculated();

    /**
     *   This is emitted when this provider is satisfied that all objects
     *   have had a chance to adjust themselves after they'd been notified that
     *   the full extent is available.
     *
     *   \note  It currently isn't being emitted because we don't have an easy way
     *          for the overview canvas to only be repainted.  In the meantime
     *          we are satisfied for the overview to reflect the new extent
     *          when the user adjusts the extent of the main map canvas.
     */
    void repaintRequested();

  private:
    int mProviderId; // id to append to provider specific identified (like cursors)

    bool declareCursor( const QString &cursorName,
                        const QgsAttributeList &fetchAttributes,
                        bool fetchGeometry,
                        QString whereClause );

    bool getFeature( QgsPostgresResult &queryResult,
                     int row,
                     bool fetchGeometry,
                     QgsFeature &feature,
                     const QgsAttributeList &fetchAttributes );

    QString geomParam( int offset ) const;
    /** Get parametrized primary key clause
     * @param offset specifies offset to use for the pk value parameter
     * @param alias specifies an optional alias given to the subject table
     */
    QString pkParamWhereClause( int offset, const char* alias = 0 ) const;
    QString whereClause( QgsFeatureId featureId ) const;
    QString filterWhereClause() const;

    bool hasSufficientPermsAndCapabilities();

    const QgsField &field( int index ) const;

    /** Load the field list
    */
    bool loadFields();

    /** convert a QgsField to work with PG */
    static bool convertField( QgsField &field );

    /**Parses the enum_range of an attribute and inserts the possible values into a stringlist
    @param enumValues the stringlist where the values are appended
    @param attributeName the name of the enum attribute
    @return true in case of success and fals in case of error (e.g. if the type is not an enum type)*/
    bool parseEnumRange( QStringList& enumValues, const QString& attributeName ) const;

    /** Parses the possible enum values of a domain type (given in the check constraint of the domain type)
    @param enumValues Reference to list that receives enum values
    @param attributeName Name of the domain type attribute
    @return true in case of success and false in case of error (e.g. if the attribute is not a domain type or does not have a check constraint)
    */
    bool parseDomainCheckConstraint( QStringList& enumValues, const QString& attributeName ) const;

    QgsFields mAttributeFields;
    QString mDataComment;

    //! Data source URI struct for this layer
    QgsDataSourceURI mUri;

    /**
     * Flag indicating if the layer data source is a valid PostgreSQL layer
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
     * Name of the schema
     */
    QString mSchemaName;
    /**
     * SQL statement used to limit the features retrieved
     */
    QString mSqlWhereClause;

    /**
     * Data type for the primary key
     */
    enum { pktUnknown, pktInt, pktTid, pktOid, pktFidMap } mPrimaryKeyType;

    /**
     * Data type for the spatial column
     */
    QgsPostgresGeometryColumnType mSpatialColType;

    /**
     * List of primary key attributes for fetching features.
     */
    QList<int> mPrimaryKeyAttrs;
    QString mPrimaryKeyDefault;

    QString mGeometryColumn;          //! name of the geometry column
    QgsRectangle mLayerExtent;        //! Rectangle that contains the extent (bounding box) of the layer
    mutable long mFeaturesCounted;    //! Number of features in the layer

    QGis::WkbType mDetectedGeomType;  //! geometry type detected in the database
    QGis::WkbType mRequestedGeomType; //! geometry type requested in the uri
    QString mDetectedSrid;            //! Spatial reference detected in the database
    QString mRequestedSrid;           //! Spatial reference requested in the uri

    bool getGeometryDetails();

    //! @{ Only used with TopoGeometry layers

    struct TopoLayerInfo
    {
      QString topologyName;
      long    layerId;
    };

    TopoLayerInfo mTopoLayerInfo;

    bool getTopoLayerInfo();

    void dropOrphanedTopoGeoms();

    //! @}

    /* Use estimated metadata. Uses fast table counts, geometry type and extent determination */
    bool mUseEstimatedMetadata;

    bool mSelectAtIdDisabled; //! Disable support for SelectAtId

    struct PGFieldNotFound {}; //! Exception to throw

    struct PGException
    {
      PGException( QgsPostgresResult &r )
          : mWhat( r.PQresultErrorMessage() )
      {}

      PGException( const PGException &e )
          : mWhat( e.errorMessage() )
      {}

      ~PGException()
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

    int mEnabledCapabilities;

    void appendGeomParam( QgsGeometry *geom, QStringList &param ) const;
    void appendPkParams( QgsFeatureId fid, QStringList &param ) const;

    QString paramValue( QString fieldvalue, const QString &defaultValue ) const;

    QgsPostgresConn *mConnectionRO; //! read-only database connection (initially)
    QgsPostgresConn *mConnectionRW; //! read-write database connection (on update)

    //! establish read-write connection
    bool connectRW()
    {
      if ( mConnectionRW )
        return mConnectionRW;

      mConnectionRW = QgsPostgresConn::connectDb( mUri.connectionInfo(), false );

      return mConnectionRW;
    }

    void disconnectDb();

    static QString quotedIdentifier( QString ident ) { return QgsPostgresConn::quotedIdentifier( ident ); }
    static QString quotedValue( QVariant value ) { return QgsPostgresConn::quotedValue( value ); }

    static int sProviderIds;

    QMap<QVariant, QgsFeatureId> mKeyToFid;  // map key values to feature id
    QMap<QgsFeatureId, QVariant> mFidToKey;  // map feature back to fea
    QgsFeatureId mFidCounter;       // next feature id if map is used
    QgsFeatureId lookupFid( const QVariant &v ); // lookup existing mapping or add a new one

    friend class QgsPostgresFeatureIterator;
    QgsPostgresFeatureIterator* mActiveIterator; //!< pointer to currently active iterator (0 if none)
};

#endif
