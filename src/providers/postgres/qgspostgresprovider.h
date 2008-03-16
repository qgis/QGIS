//Added by qt3to4:
#include <QCustomEvent>
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
/* $Id$ */

#ifndef QGSPOSTGRESPROVIDER_H
#define QGSPOSTGRESPROVIDER_H

extern "C"
{
#include <libpq-fe.h>
}
#include "qgsvectordataprovider.h"
#include "qgsrect.h"
#include <list>
#include <queue>
#include <fstream>
#include <set>

class QgsFeature;
class QgsField;
class QgsGeometry;

#include "qgsdatasourceuri.h"

#include "qgspostgrescountthread.h"
#include "qgspostgresextentthread.h"

/**
  \class QgsPostgresProvider
  \brief Data provider for PostgreSQL/PostGIS layers.

  This provider implements the
  interface defined in the QgsDataProvider class to provide access to spatial
  data residing in a PostgreSQL/PostGIS enabled database.
  */
class QgsPostgresProvider:public QgsVectorDataProvider
{

  Q_OBJECT

  public:
    /**
     * Constructor for the provider. The uri must be in the following format:
     * host=localhost user=gsherman dbname=test password=xxx table=test.alaska (the_geom)
     * @param uri String containing the required parameters to connect to the database
     * and query the table.
     */
    QgsPostgresProvider(QString const & uri = "");

    //! Destructor
    virtual ~ QgsPostgresProvider();

    /**
      *   Returns the permanent storage type for this layer as a friendly name.
      */
    virtual QString storageType() const;

    /*! Get the QgsSpatialRefSys for this layer
     * @note Must be reimplemented by each provider. 
     * If the provider isn't capable of returning
     * its projection an empty srs will be return, ti will return 0
     */
    virtual QgsSpatialRefSys getSRS();

    /** Select features based on a bounding rectangle. Features can be retrieved with calls to getNextFeature.
     *  @param fetchAttributes list of attributes which should be fetched
     *  @param rect spatial filter
     *  @param fetchGeometry true if the feature geometry should be fetched
     *  @param useIntersect true if an accurate intersection test should be used,
     *                     false if a test based on bounding box is sufficient
     */
    virtual void select(QgsAttributeList fetchAttributes = QgsAttributeList(),
                        QgsRect rect = QgsRect(),
                        bool fetchGeometry = true,
                        bool useIntersect = false);
  
    /**
     * Get the next feature resulting from a select operation.
     * @param feature feature which will receive data from the provider
     * @return true when there was a feature to fetch, false when end was hit
     */
    virtual bool getNextFeature(QgsFeature& feature);
    
    /** 
      * Gets the feature at the given feature ID.
      * @param featureId id of the feature
      * @param feature feature which will receive the data
      * @param fetchGeoemtry if true, geometry will be fetched from the provider
      * @param fetchAttributes a list containing the indexes of the attribute fields to copy
      * @return True when feature was found, otherwise false
      */
    virtual bool getFeatureAtId(int featureId,
                                QgsFeature& feature,
                                bool fetchGeometry = true,
                                QgsAttributeList fetchAttributes = QgsAttributeList());

    
    /** Get the feature type. This corresponds to
     * WKBPoint,
     * WKBLineString,
     * WKBPolygon,
     * WKBMultiPoint,
     * WKBMultiLineString or
     * WKBMultiPolygon
     * as defined in qgis.h
     */
    QGis::WKBTYPE geometryType() const;


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
    void setExtent( QgsRect& newExtent );

    /** Return the extent for this data layer
    */
    virtual QgsRect extent();

    /**  * Get the name of the primary key for the layer
    */
    QString getPrimaryKey();

    /**
     * Get the field information for the layer
     * @return vector of QgsField objects
     */
    const QgsFieldMap & fields() const;

    /**
     * Return a short comment for the data that this provider is
     * providing access to (e.g. the comment for postgres table).
     */
    QString dataComment() const;

    /** Reset the layer - for a PostgreSQL layer, this means clearing the PQresult
     * pointer, setting it to 0 and reloading the field list
     */
    void reset();

    /** Returns the minimum value of an attribute
     *  @param index the index of the attribute */
    QVariant minValue(int index);

    /** Returns the maximum value of an attribute
     *  @param index the index of the attribute */
    QVariant maxValue(int index);

    /** Return the unique values of an attribute
     *  @param index the index of the attribute
     *  @param values reference to the list of unique values */
    virtual void getUniqueValues(int index, QStringList &uniqueValues);

    /**Returns true if layer is valid
    */
    bool isValid();
    
    QgsAttributeList allAttributesList();

    //! get postgis version string
    QString postgisVersion(PGconn *);

    //! get status of GEOS capability
    bool hasGEOS(PGconn *);

    //! get status of GIST capability
    bool hasGIST(PGconn *);

    //! get status of PROJ4 capability
    bool hasPROJ(PGconn *);

    /**Returns the default value for field specified by @c fieldId */
    QVariant getDefaultValue(int fieldId);

    /**Adds a list of features
      @return true in case of success and false in case of failure*/
    bool addFeatures(QgsFeatureList & flist);

    /**Deletes a list of features
      @param id list of feature ids
      @return true in case of success and false in case of failure*/
    bool deleteFeatures(const QgsFeatureIds & id);

    /**Adds new attributes
      @param name map with attribute name as key and type as value
      @return true in case of success and false in case of failure*/
    bool addAttributes(const QgsNewAttributesMap & name);

    /**Deletes existing attributes
      @param names of the attributes to delete
      @return true in case of success and false in case of failure*/
    bool deleteAttributes(const QgsAttributeIds & name);

    /**Changes attribute values of existing features
      @param attr_map a map containing the new attributes. The integer is the feature id,
      the first QString is the attribute name and the second one is the new attribute value
      @return true in case of success and false in case of failure*/
    bool changeAttributeValues(const QgsChangedAttributesMap & attr_map);

    /** 
       Changes geometries of existing features
       @param geometry_map   A std::map containing the feature IDs to change the geometries of. 
                             the second map parameter being the new geometries themselves
       @return               true in case of success and false in case of failure
     */
    bool changeGeometryValues(QgsGeometryMap & geometry_map);

    //! Get the postgres connection
    PGconn * pgConnection();

    //! Get the table name associated with this provider instance
    QString getTableName();

    /** Accessor for sql where clause used to limit dataset */
    QString subsetString();

    /** mutator for sql where clause used to limit dataset size */
    void setSubsetString(QString theSQL);

    /**Returns a bitmask containing the supported capabilities*/
    int capabilities() const;

    /** The Postgres provider does its own transforms so we return
     * true for the following three functions to indicate that transforms
     * should not be handled by the QgsCoordinateTransform object. See the
     * documentation on QgsVectorDataProvider for details on these functions.
     */
    // XXX For now we have disabled native transforms in the PG provider since
    //     it appears there are problems with some of the projection definitions
    bool supportsNativeTransform(){return false;}


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

    /** Double quote a PostgreSQL identifier for placement in a SQL string.
     */
    QString quotedIdentifier( QString ident ) const;

    /** Quote a value for placement in a SQL string.
     */
    QString quotedValue( QString value ) const;

    /** Load the field list
    */
    void loadFields();

    bool mFirstFetch; //true if fetch forward is called the first time after select
    std::vector < QgsFeature > features;
    QgsFieldMap attributeFields;
    QString mDataComment;

    //! Data source URI struct for this layer
    QgsDataSourceURI mUri;


    //! Child thread for calculating extents.
    QgsPostgresExtentThread mExtentThread;

    //! Child thread for calculating count.
    QgsPostgresCountThread mCountThread;


    /**
     * Pointer to the PostgreSQL query result object. If this pointer is 0,
     * there is no current selection set. Any future getNextFeature requests
     * will require execution of the select query to recreate the result set.
     */
    PGresult *queryResult;
    /**
     * Flag indicating if the layer data source is a valid PostgreSQL layer
     */
    bool valid;
    /**
     * Name of the table with no schema
     */
    QString mTableName;
    /**
     * Name of the table with schema included
     */
    QString mSchemaTableName;
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
    QGis::WKBTYPE geomType;
    /**
     * Connection pointer
     */
    PGconn *connection;
    /**
     * Spatial reference id of the layer
     */
    QString srid;
    /**
     * Rectangle that contains the extent (bounding box) of the layer
     */
    QgsRect layerExtent;

    /**
     * Number of features in the layer
     */
    long numberFeatures;

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

    /**Stores the names of the attributes to fetch*/
    std::list<QString> mFetchAttributeNames;

    bool deduceEndian();
    bool getGeometryDetails();

    PGresult* executeDbCommand(PGconn* connection, const QString& sql);

    // Produces a QMessageBox with the given title and text. Doesn't
    // return until the user has dismissed the dialog box.
    static void showMessageBox(const QString& title, const QString& text);
    static void showMessageBox(const QString& title, const QStringList& text);

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

    struct PGException {
      PGException(PGresult *r) : result(r)
      {
      }

      PGException(const PGException &e) : result(e.result) 
      {
      }

      ~PGException()
      {
        if(result)
          PQclear(result);
      }

      QString errorMessage() const
      {
        return result ?
          QString::fromUtf8(PQresultErrorMessage(result)) :
          tr("unexpected PostgreSQL error");
      }

      void showErrorMessage(QString title) const
      {
        showMessageBox(title, errorMessage() );
      }

    private:
      PGresult *result;
    };

    // A simple class to store four strings
    class SRC 
    { 
    public:
      SRC() {};
      SRC(QString s, QString r, QString c, QString t) :
        schema(s), relation(r), column(c), type(t) {};
      QString schema, relation, column, type; 
    };

    // A structure to store the underlying schema.table.column for
    // each column in mSchemaName.mTableName
    typedef std::map<QString, SRC> tableCols;

    // A function that chooses a view column that is suitable for use
    // a the qgis key column.
    QString chooseViewColumn(const tableCols& cols);

    // A function that determines if the given schema.table.column
    // contains unqiue entries
    bool uniqueData(QString schemaName, QString tableName, QString colName);

    // Function that populates the given cols structure.
    void findColumns(tableCols& cols);

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
    int SRCFromViewColumn(const QString& ns, const QString& relname, const QString& attname_table, 
			  const QString& attname_view, const QString& viewDefinition, SRC& result) const;

    bool ready;
    std::ofstream pLog;

    //! PostGIS version string
    QString postgisVersionInfo;

    //! Are postgisVersionMajor, postgisVersionMinor, geosAvailable, gistAvailable, projAvailable valid?
    bool gotPostgisVersion;

    //! PostGIS major version
    int postgisVersionMajor;

    //! PostGIS minor version
    int postgisVersionMinor;

    //! GEOS capability
    bool geosAvailable;

    //! GIST capability
    bool gistAvailable;

    //! PROJ4 capability
    bool projAvailable;

    int enabledCapabilities;

    /**Returns the maximum value of the primary key attribute
       @note  You should run this inside of a PostgreSQL transaction
              so that you can safely increment the value returned for
              use in newly added features.
      */
    int maxPrimaryKeyValue();

    //! Get the feature count based on the where clause
    long getFeatureCount();

    //! Calculate the extents of the layer
    void calculateExtents();

    /**
     * Event sink for events from threads
     */
    void customEvent ( QCustomEvent *e );

    PGconn *connectDb(const QString &conninfo);
    void disconnectDb();

    bool useWkbHex;
   
    void appendGeomString(QgsGeometry *geom, QString &geomParam) const;
    QByteArray paramValue(QString fieldvalue, const QString &defaultValue) const;

    // run a query and free result buffer
    static void PQexecNR(PGconn *conn, const char *query);
};

#endif
