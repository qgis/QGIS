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

extern "C"
{
#include <libpq-fe.h>
}
#include "qgsvectordataprovider.h"
#include "qgsdataitem.h"
#include "qgsrectangle.h"

#include <list>
#include <queue>
#include <fstream>
#include <set>

#include <QVector>

class QgsFeature;
class QgsField;
class QgsGeometry;


#include "qgsdatasourceuri.h"

/** Layer Property structure */
// TODO: Fill to Postgres/PostGIS specifications
struct QgsPostgresLayerProperty
{
  // Postgres/PostGIS layer properties
  QString       type;
  QString       schemaName;
  QString       tableName;
  QString       geometryColName;
  QStringList   pkCols;
  QString       sql;
};

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
    /**
     * Constructor for the provider. The uri must be in the following format:
     * host=localhost user=gsherman dbname=test password=xxx table=test.alaska (the_geom)
     * @param uri String containing the required parameters to connect to the database
     * and query the table.
     */
    QgsPostgresProvider( QString const & uri = "" );

    //! Destructor
    virtual ~ QgsPostgresProvider();

    /**
      *   Returns the permanent storage type for this layer as a friendly name.
      */
    virtual QString storageType() const;

    /*! Get the QgsCoordinateReferenceSystem for this layer
     * @note Must be reimplemented by each provider.
     * If the provider isn't capable of returning
     * its projection an empty srs will be return, ti will return 0
     */
    virtual QgsCoordinateReferenceSystem crs();

    /** Select features based on a bounding rectangle. Features can be retrieved with calls to nextFeature.
     *  @param fetchAttributes list of attributes which should be fetched
     *  @param rect spatial filter
     *  @param fetchGeometry true if the feature geometry should be fetched
     *  @param useIntersect true if an accurate intersection test should be used,
     *                     false if a test based on bounding box is sufficient
     */
    virtual void select( QgsAttributeList fetchAttributes = QgsAttributeList(),
                         QgsRectangle rect = QgsRectangle(),
                         bool fetchGeometry = true,
                         bool useIntersect = false );

    /**
     * Get the next feature resulting from a select operation.
     * @param feature feature which will receive data from the provider
     * @return true when there was a feature to fetch, false when end was hit
     */
    virtual bool nextFeature( QgsFeature& feature );

    /**
      * Gets the feature at the given feature ID.
      * @param featureId id of the feature
      * @param feature feature which will receive the data
      * @param fetchGeoemtry if true, geometry will be fetched from the provider
      * @param fetchAttributes a list containing the indexes of the attribute fields to copy
      * @return True when feature was found, otherwise false
      */
    virtual bool featureAtId( QgsFeatureId featureId,
                              QgsFeature& feature,
                              bool fetchGeometry = true,
                              QgsAttributeList fetchAttributes = QgsAttributeList() );

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

    @note

    Should this be subLayerCount() instead?
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
     * Get the data source URI structure used by this layer
     */
    QgsDataSourceURI& getURI();

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

    /**  * Get the name of the primary key for the layer
    */
    QString getPrimaryKey();

    /**
     * Get the field information for the layer
     * @return vector of QgsField objects
     */
    const QgsFieldMap &fields() const;

    /**
     * Return a short comment for the data that this provider is
     * providing access to (e.g. the comment for postgres table).
     */
    QString dataComment() const;

    /** Reset the layer - for a PostgreSQL layer, this means clearing the PQresult
     * pointer, setting it to 0 and reloading the field list
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

    /**Returns the possible enum values of an attribute. Returns an empty stringlist if a provider does not support enum types
      or if the given attribute is not an enum type.
     * @param index the index of the attribute
     * @param enumList reference to the list to fill
      @note: added in version 1.2*/
    virtual void enumValues( int index, QStringList& enumList );

    /**Returns true if layer is valid
    */
    bool isValid();

    QgsAttributeList attributeIndexes();

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
       @param geometry_map   A std::map containing the feature IDs to change the geometries of.
                             the second map parameter being the new geometries themselves
       @return               true in case of success and false in case of failure
     */
    bool changeGeometryValues( QgsGeometryMap & geometry_map );

    //! Get the list of supported layers
    bool supportedLayers( QVector<QgsPostgresLayerProperty> &layers,
                          bool searchGeometryColumnsOnly = true,
                          bool searchPublicOnly = true,
                          bool allowGeometrylessTables = false );

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
    int providerId; // id to append to provider specific identified (like cursors)

    bool declareCursor( const QString &cursorName,
                        const QgsAttributeList &fetchAttributes,
                        bool fetchGeometry,
                        QString whereClause );

    bool getFeature( PGresult *queryResult, int row, bool fetchGeometry,
                     QgsFeature &feature,
                     const QgsAttributeList &fetchAttributes );

    QString whereClause( QgsFeatureId featureId ) const;

    /** Gets information about the spatial tables */
    bool getTableInfo( bool searchGeometryColumnsOnly, bool searchPublicOnly, bool allowGeometrylessTables );

    /** get primary key candidates (all int4 columns) */
    QStringList pkCandidates( QString schemaName, QString viewName );

    bool hasSufficientPermsAndCapabilities();

    qint64 getBinaryInt( PGresult *queryResult, int row, int col );

    const QgsField &field( int index ) const;

    /** Double quote a PostgreSQL identifier for placement in a SQL string.
     */
    QString quotedIdentifier( QString ident ) const;

    /** Quote a value for placement in a SQL string.
     */
    QString quotedValue( QString value ) const;

    /** expression to retrieve value
     */
    QString fieldExpression( const QgsField &fld ) const;

    /** Load the field list
    */
    bool loadFields();

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

    bool mFetching; // true if a cursor was declared
    int mFetched; // number of retrieved features
    std::vector < QgsFeature > features;
    QgsFieldMap attributeFields;
    QString mDataComment;

    //! Data source URI struct for this layer
    QgsDataSourceURI mUri;

    //! List of the supported layers
    QVector<QgsPostgresLayerProperty> layersSupported;

    /**
     * Flag indicating if the layer data source is a valid PostgreSQL layer
     */
    bool valid;

    /**
     * provider references query (instead of a table)
     */
    bool isQuery;

    /**
     * geometry is geography
     */
    bool isGeography;

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
     * Name of the current schema
     */
    QString mCurrentSchema;
    /**
     * SQL statement used to limit the features retrieved
     */
    QString sqlWhereClause;

    /**
     * Primary key column for fetching features. If there is no primary key
     * the oid is used to fetch features.
     */
    QString primaryKey;
    /**
     * Primary key column is "real" primary key
     */
    bool mIsDbPrimaryKey;
    /**
     * Data type for the primary key
     */
    QString primaryKeyType;
    /**
     * Name of the geometry column in the table
     */
    QString geometryColumn;
    /**
     * Geometry type
     */
    QGis::WkbType geomType;

    /**
     * Spatial reference id of the layer
     */
    QString srid;
    /**
     * Rectangle that contains the extent (bounding box) of the layer
     */
    QgsRectangle layerExtent;

    /**
     * Number of features in the layer
     */
    mutable long featuresCounted;

    /**
     * Feature queue that GetNextFeature will retrieve from
     * before the next fetch from PostgreSQL
     */
    std::queue<QgsFeature> mFeatureQueue;

    /**
     * Maximal size of the feature queue
     */
    int mFeatureQueueSize;

    /**
     * Flag indicating whether data from binary cursors must undergo an
     * endian conversion prior to use
     @note

     XXX Umm, it'd be helpful to know what we're swapping from and to.
     XXX Presumably this means swapping from big-endian (network) byte order
     XXX to little-endian; but the inverse transaction is possible, too, and
     XXX that's not reflected in this variable
     */
    bool swapEndian;

    bool deduceEndian();
    bool getGeometryDetails();

    /* Use estimated metadata. Uses fast table counts, geometry type and extent determination */
    bool mUseEstimatedMetadata;

    // Produces a QMessageBox with the given title and text. Doesn't
    // return until the user has dismissed the dialog box.
    static void showMessageBox( const QString& title, const QString &text );
    static void showMessageBox( const QString& title, const QStringList &text );

    // A simple class to store the rows of the sql executed in the
    // findColumns() function.
    class TT
    {
      public:
        TT() {};

        QString view_schema;
        QString view_name;
        QString view_column_name;
        QString table_schema;
        QString table_name;
        QString column_name;
        QString table_type;
        QString column_type;
    };

    struct PGFieldNotFound
    {
    };

    struct PGException
    {
      PGException( PGresult *r ) : result( r )
      {
      }

      PGException( const PGException &e ) : result( e.result )
      {
      }

      ~PGException()
      {
        if ( result )
          PQclear( result );
      }

      QString errorMessage() const
      {
        return result ?
               QString::fromUtf8( PQresultErrorMessage( result ) ) :
               tr( "unexpected PostgreSQL error" );
      }

      void showErrorMessage( QString title ) const
      {
        showMessageBox( title, errorMessage() );
      }

    private:
      PGresult *result;
    };

    // A simple class to store four strings
    class SRC
    {
      public:
        SRC() {};
        SRC( QString s, QString r, QString c, QString t ) :
            schema( s ), relation( r ), column( c ), type( t ) {};
        QString schema, relation, column, type;
    };

    // A structure to store the underlying schema.table.column for
    // each column in mSchemaName.mTableName
    typedef std::map<QString, SRC> tableCols;

    // A function that chooses a view column that is suitable for use
    // a the qgis key column.
    QString chooseViewColumn( const tableCols& cols );

    // A function that determines if the given schema.table.column
    // contains unqiue entries
    bool uniqueData( QString query, QString colName );

    // Function that populates the given cols structure.
    void findColumns( tableCols& cols );

    /**Helper function that collects information about the origin and type of a view column.
       Inputs are information about the column in the underlying table
       (from information_schema.view_column_usage), the attribute name
       in the view and the view definition. For view columns that refer
       to other views, this function calls itself until a table entry is found.
    @param ns namespace of underlying table
    @param relname name of underlying relation
    @param attname attribute name in underlying table
    @param viewDefinition definition of this view
    @param result
    @return 0 in case of success*/
    int SRCFromViewColumn( const QString& ns, const QString& relname, const QString& attname_table,
                           const QString& attname_view, const QString& viewDefinition, SRC& result ) const;

    int enabledCapabilities;

    void appendGeomString( QgsGeometry *geom, QString &geomParam ) const;
    QString paramValue( QString fieldvalue, const QString &defaultValue ) const;

    class Conn
    {
      public:
        Conn( PGconn *connection )
            : ref( 1 )
            , openCursors( 0 )
            , conn( connection )
            , gotPostgisVersion( false )
        {
        }

        //! get postgis version string
        QString postgisVersion();

        //! get status of GEOS capability
        bool hasGEOS();

        //! get status of GIST capability
        bool hasGIST();

        //! get status of PROJ4 capability
        bool hasPROJ();

        //! encode wkb in hex
        bool useWkbHex() { return mUseWkbHex; }

        //! major PostgreSQL version
        int majorVersion() { return postgisVersionMajor; }

        //! PostgreSQL version
        int pgVersion() { return postgresqlVersion; }

        //! run a query and free result buffer
        bool PQexecNR( QString query );

        //! cursor handling
        bool openCursor( QString cursorName, QString declare );
        bool closeCursor( QString cursorName );

        PGconn *pgConnection() { return conn; }

        //
        // libpq wrapper
        //

        // run a query and check for errors
        PGresult *PQexec( QString query );
        void PQfinish();
        int PQsendQuery( QString query );
        PGresult *PQgetResult();
        PGresult *PQprepare( QString stmtName, QString query, int nParams, const Oid *paramTypes );
        PGresult *PQexecPrepared( QString stmtName, const QStringList &params );

        static Conn *connectDb( const QString &conninfo, bool readonly );
        static void disconnectRW( Conn *&conn );
        static void disconnectRO( Conn *&conn );
        static void disconnect( QMap<QString, Conn *> &connections, Conn *&conn );

      private:
        int ref;
        int openCursors;
        PGconn *conn;

        //! GEOS capability
        bool geosAvailable;

        //! PostGIS version string
        QString postgisVersionInfo;

        //! Are postgisVersionMajor, postgisVersionMinor, geosAvailable, gistAvailable, projAvailable valid?
        bool gotPostgisVersion;

        //! PostgreSQL version
        int postgresqlVersion;

        //! PostGIS major version
        int postgisVersionMajor;

        //! PostGIS minor version
        int postgisVersionMinor;

        //! GIST capability
        bool gistAvailable;

        //! PROJ4 capability
        bool projAvailable;

        //! encode wkb in hex
        bool mUseWkbHex;

        static QMap<QString, Conn *> connectionsRW;
        static QMap<QString, Conn *> connectionsRO;
        static QMap<QString, QString> passwordCache;
    };

    class Result
    {
      public:
        Result( PGresult *theRes = 0 ) : res( theRes ) {}
        ~Result() { if ( res ) PQclear( res ); }

        operator PGresult *() { return res; }

        Result &operator=( PGresult *theRes ) { if ( res ) PQclear( res ); res = theRes;  return *this; }

      private:
        PGresult *res;
    };

    /**
     * Connection pointers
     */
    Conn *connectionRO;
    Conn *connectionRW;

    bool connectRW()
    {
      if ( connectionRW )
        return connectionRW;

      connectionRW = Conn::connectDb( mUri.connectionInfo(), false );

      return connectionRW;
    }

    void disconnectDb();

    static int providerIds;

    QString primaryKeyDefault();
    void parseView();

    /**
     * Default value for primary key
     */
    QString mPrimaryKeyDefault;
};

class QgsPGConnectionItem : public QgsDataCollectionItem
{
  public:
    QgsPGConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsPGConnectionItem();

    QVector<QgsDataItem*> createChildren();
    virtual bool equal( const QgsDataItem *other );

    QString mConnInfo;
    QVector<QgsPostgresLayerProperty> mLayerProperties;
};

// WMS Layers may be nested, so that they may be both QgsDataCollectionItem and QgsLayerItem
// We have to use QgsDataCollectionItem and support layer methods if necessary
class QgsPGLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsPGLayerItem( QgsDataItem* parent, QString name, QString path,
                     QString connInfo, QgsLayerItem::LayerType layerType, QgsPostgresLayerProperty layerProperties );
    ~QgsPGLayerItem();

    QString createUri();

    QString mConnInfo;
    QgsPostgresLayerProperty mLayerProperty;
};

class QgsPGSchemaItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGSchemaItem( QgsDataItem* parent, QString name, QString path,
                    QString connInfo, QVector<QgsPostgresLayerProperty> layerProperties );
    ~QgsPGSchemaItem();
};

class QgsPGRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsPGRootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsPGRootItem();

    QVector<QgsDataItem*> createChildren();

    virtual QWidget * paramWidget();

  public slots:
    void connectionsChanged();
};

#endif
