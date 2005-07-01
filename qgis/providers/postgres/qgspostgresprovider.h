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
#include "../../src/qgsvectordataprovider.h"
#include "../../src/qgsrect.h"
#include <list>
#include <queue>
#include <fstream>
#include <qstring.h>
#include <qobject.h>
//#include <qmutex.h>

class OGRDataSource;
class OGRLayer;

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
    QgsPostgresProvider(QString uri = 0);

    //! Destructor
    virtual ~ QgsPostgresProvider();

    /**
      *   Returns the permanent storage type for this layer as a friendly name.
      */
    QString storageType();

    /** Used to ask the layer for its projection as a WKT string. Implements
     * virtual method of same name in QgsDataProvider. */
    QString getProjectionWKT()  {return QString("Not implemented yet");} ;

    /**
     * Get the first feature resutling from a select operation
     * @return QgsFeature
     */
    QgsFeature *getFirstFeature(bool fetchAttributes = false);

    /**
     * Get the next feature resulting from a select operation
     * @return QgsFeature
     */
    QgsFeature *getNextFeature(bool fetchAttributes = false);
    bool getNextFeature(QgsFeature &feature, bool fetchAttributes=0);

    /**Get the next feature resulting from a select operation.
     * @param attlist            a list containing the indexes of the attribute fields to copy
     * @param featureQueueSize   a hint to the provider as to how many features are likely to be retrieved in a batch
     */
//    QgsFeature* getNextFeature(std::list<int> const & attlist);
    QgsFeature* getNextFeature(std::list<int> const & attlist, int featureQueueSize = 1);
    
    /** Get the feature type. This corresponds to
     * WKBPoint,
     * WKBLineString,
     * WKBPolygon,
     * WKBMultiPoint,
     * WKBMultiLineString or
     * WKBMultiPolygon
     * as defined in qgis.h
     */

    int geometryType() const;

    /**
     * Get the number of features in the layer
     */
    long featureCount() const;

    /**
     * Get the number of fields in the layer
     */
    int fieldCount() const;

    /**
     * Select features based on a bounding rectangle. Features can be retrieved
     * with calls to getFirstFeature and getNextFeature.
     * @param mbr QgsRect containing the extent to use in selecting features
     */
    void select(QgsRect * mbr, bool useIntersect=false);

    /**
     * Get the data source URI structure used by this layer
     */
    QgsDataSourceURI * getURI();

    /**
     * Set the data source URI used by this layer
     */
    void setURI(QgsDataSourceURI &uri);

    /**
     * Set the data source specification. This must be a valid database
     * connection string:
     * host=localhost user=gsherman dbname=test password=xxx table=test.alaska (the_geom)
     * @uri data source specification
     */
    // TODO Deprecate this in favor of using the QgsDataSourceURI structure
    void setDataSourceUri(QString uri);

    /**
     * Get the data source specification.
     * @return data source specification as a string containing the host, user,
     * dbname, password, and table
     * @see setDataSourceUri
     */
    // TODO Deprecate this in favor of returning the QgsDataSourceURI structure
    QString getDataSourceUri();

    /**
     * Identify features within the search radius specified by rect
     * @param rect Bounding rectangle of search radius
     * @return std::vector containing QgsFeature objects that intersect rect
     */
    virtual std::vector<QgsFeature>& identify(QgsRect * rect);

    /**
     * Return a string representation of the endian-ness for the layer
     */
    QString endianString();

    /**
     * Changes the stored extent for this layer to the supplied extent.
     * For example, this is called when the extent worker thread has a result.
     */ 
    void setExtent( QgsRect* newExtent );

    /** Return the extent for this data layer
    */
    virtual QgsRect *extent();

    /**
     * Get the attributes associated with a feature
     */
    virtual void getFeatureAttributes(int oid, int& row, QgsFeature *f);

    /**Get the attributes with indices contained in attlist*/
    void getFeatureAttributes(int oid, int& row, QgsFeature *f, std::list<int> const& attlist);

    /**  * Get the name of the primary key for the layer
    */
    QString getPrimaryKey();
    /**
     * Get the field information for the layer
     * @return vector of QgsField objects
     */
    std::vector<QgsField> const & fields() const;

    /** Reset the layer - for a PostgreSQL layer, this means clearing the PQresult
     * pointer and setting it to 0
     */
    void reset();

    /**Returns the minimum value of an attributs
      @param position the number of the attribute*/
    QString minValue(int position);

    /**Returns the maximum value of an attributs
      @param position the number of the attribute*/
    QString maxValue(int position);

    /**Returns true if layer is valid
    */
    bool isValid();

    //! get postgis version string
    QString postgisVersion(PGconn *);

    //! get status of GEOS capability
    bool hasGEOS(PGconn *);

    //! get status of GIST capability
    bool hasGIST(PGconn *);

    //! get status of PROJ4 capability
    bool hasPROJ(PGconn *);

    /**Returns the default value for attribute @c attr for feature @c f. */
    QString getDefaultValue(const QString& attr, QgsFeature* f);

    /**Adds a list of features
      @return true in case of success and false in case of failure*/
    bool addFeatures(std::list<QgsFeature*> const flist);

    /**Deletes a list of features
      @param id list of feature ids
      @return true in case of success and false in case of failure*/
    bool deleteFeatures(std::list<int> const & id);

    /**Adds new attributes
      @param name map with attribute name as key and type as value
      @return true in case of success and false in case of failure*/
    bool addAttributes(std::map<QString,QString> const & name);

    /**Deletes existing attributes
      @param names of the attributes to delete
      @return true in case of success and false in case of failure*/
    bool deleteAttributes(std::set<QString> const & name);

    /**Changes attribute values of existing features
      @param attr_map a map containing the new attributes. The integer is the feature id,
      the first QString is the attribute name and the second one is the new attribute value
      @return true in case of success and false in case of failure*/
    bool changeAttributeValues(std::map<int,std::map<QString,QString> > const & attr_map);

    /** 
       Changes geometries of existing features
       @param geometry_map   A std::map containing the feature IDs to change the geometries of. 
                             the second map parameter being the new geometries themselves
       @return               true in case of success and false in case of failure
     */
    bool changeGeometryValues(std::map<int, QgsGeometry> & geometry_map);

    //! Flag to indicate if the provider can export to shapefile
    bool supportsSaveAsShapefile() const;

    /** Accessor for sql where clause used to limit dataset */
    QString subsetString() {return sqlWhereClause;};

    //! Get the postgres connection
    PGconn * pgConnection() {return connection;};

    //! Get the table name associated with this provider instance
    QString getTableName() {return tableName;};

    /** mutator for sql where clause used to limit dataset size */
    void setSubsetString(QString theSQL); //{sqlWhereClause = theSQL;};

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
    bool usesSrid(){return true;}
    bool usesWKT(){return false;}

    /*! Set the SRID of the target (map canvas) SRS.
     * @parm srid SRID of the map canvas SRS
     */
    void setTargetSrid(int srid);

    int getSrid();


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

    std::vector < QgsFeature > features;
    std::vector < bool > *selected;
    std::vector < QgsField > attributeFields;
    std::map < int, int > attributeFieldsIdMap;

    QString dataSourceUri;

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
    QString tableName;
    /**
     * Name of the table with schema included
     */
    QString schemaTableName;
    /** 
     * Name of the schema
     */
    QString mSchema;
    /**
     * SQL statement used to limit the features retreived
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
    int geomType;
    /**
     * SQL to select all records in this layer
     */
    QString selectSQL;
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
    std::queue<QgsFeature*> mFeatureQueue; 
        
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

    typedef std::map<QString, std::pair<QString, QString> > tableCols;

    QString chooseViewColumn(const tableCols& cols);

    void findTableColumns(QString selectCmd, tableCols& cols);
    void findColumns(QString selectCmd, tableCols& cols);
    int findRelationAndColumn(QString relation, QString column,
        QString& rRelation, QString& rColumn);

    bool ready;
    std::ofstream pLog;

    //! PostGIS version string
    QString postgisVersionInfo;

    //! GEOS capability
    bool geosAvailable;

    //! GIST capability
    bool gistAvailable;

    //! PROJ4 capability
    bool projAvailable;

    /**Writes a single feature*/
    bool addFeature(QgsFeature* f);

    /**Deletes a feature*/
    bool deleteFeature(int id);

    //! Get the feature count based on the where clause
    long getFeatureCount();

    //! Calculate the extents of the layer
    void calculateExtents();

    /**
     * Event sink for events from threads
     */
    void customEvent ( QCustomEvent * e );

};

#endif
