/***************************************************************************
      qgspostgresprovider.h  -  Data provider for PostgrSQL/PostGIS layers
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
extern "C"
{
#include <libpq-fe.h>
}
#include "../../src/qgsvectordataprovider.h"
#include <list>

class QgsFeature;
class QgsField;
class OGRDataSource;
class OGRLayer;
/**
\class QgsPostgresProvider
\brief Data provider for PostgrSQL/PostGIS layers. 

This provider implements the
interface defined in the QgsDataProvider class to provide access to spatial
data residing in a PostgreSQL/PostGIS enabled database. 
*/
class QgsPostgresProvider:public QgsVectorDataProvider
{
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
     *@param attlist a list containing the indexes of the attribute fields to copy
     */
    QgsFeature* getNextFeature(std::list<int>& attlist);
    /** Get the feature type. This corresponds to 
    * WKBPoint,
    * WKBLineString,
    * WKBPolygon,
    * WKBMultiPoint,
    * WKBMultiLineString or
    * WKBMultiPolygon
  * as defined in qgis.h
  */
    int geometryType();
    /** 
    * Get the number of features in the layer
    */
    long featureCount();
    /** 
    * Get the number of fields in the layer
    */
    int fieldCount();
    /**
  * Select features based on a bounding rectangle. Features can be retrieved 
  * with calls to getFirstFeature and getNextFeature.
  * @param mbr QgsRect containing the extent to use in selecting features
  */
    void select(QgsRect * mbr, bool useIntersect=false);
    /** 
    * Set the data source specification. This must be a valid database
  * connection string:
  * host=localhost user=gsherman dbname=test password=xxx table=test.alaska (the_geom)
  * @uri data source specification
  */
    void setDataSourceUri(QString uri);

   /** 
  * Get the data source specification. 
  * @return data source specification as a string containing the host, user, 
  * dbname, password, and table
  * @see setDataSourceUri
  */
    QString getDataSourceUri();

    /**
  * Identify features within the search radius specified by rect
  * @param rect Bounding rectangle of search radius
  * @return std::vector containing QgsFeature objects that intersect rect
  */
    virtual std::vector<QgsFeature>& identify(QgsRect * rect);

  /** 
  * Return endian-ness for this layer
  * @see ENDIAN
  */
    int endian();
  /** 
  * Return a string representation of the endian-ness for the layer
  */
   QString endianString();
   
  /** Return the extent for this data layer
  */
    virtual QgsRect *extent();
  /**
  * Get the attributes associated with a feature
  */
    void getFeatureAttributes(int oid, QgsFeature *f); 
    /**Get the attributes with indices contained in attlist*/
    void getFeatureAttributes(int oid, QgsFeature *f, std::list<int>& attlist);
  /**
  * Get the name of the primary key for the layer
*/  
    QString getPrimaryKey();
 /**
 * Get the field information for the layer
 * @return vector of QgsField objects
 */
      std::vector<QgsField>& fields();

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

  /**
     Enables editing capabilities of the provider (if supported)
     @return false in case of error or if the provider does not support editing
  */
  virtual bool startEditing();

  /**
     Disables the editing capabilities of the provider
  */
  virtual void stopEditing();

  /**
     Commits changes
     @return false in case of problems
  */
  virtual bool commitChanges();

  /**
     Discards changes
     @return false in case of problems
  */
  virtual bool rollBack();

  /**Returns true if the provider is in editing mode*/
  virtual bool isEditable() const {return false;}

  /**Returns true if the provider has been modified since the last commit*/
  virtual bool isModified() const {return false;}

  /**Adds a feature
     @return true in case of success and false in case of failure*/
  bool addFeature(QgsFeature* f);

  /**Deletes a feature
     @param id the number of the feature
     @return true in case of success and false in case of failure*/
  bool deleteFeature(int id);



  //! get postgis version string
  QString postgisVersion(PGconn *);
  //! get status of GEOS capability
  bool hasGEOS(PGconn *);
  //! get status of GIST capability
  bool hasGIST(PGconn *);
  //! get status of PROJ4 capability
  bool hasPROJ(PGconn *);
  
private:
      std::vector < QgsFeature > features;
      std::vector < bool > *selected;
      std::vector < QgsField > attributeFields;
      std::map < int, int > attributeFieldsIdMap;
    QString dataSourceUri;
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
    * SQL statement used to limit the features retreived
    */
    QString sqlWhereClause;
    /**
    * Primary key column for fetching features. If there is no primary key
    * the oid is used to fetch features. 
    */
    QString primaryKey;
    /** 
    * Index (column number) of the primary key
    */
    int primaryKeyIndex;
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
    * Enum defining endian types:
    *
    * NDR = little endian
    *
    * XDR = big endian
    * @see endian
    */
    enum ENDIAN
    {
        NDR = 1,
        XDR = 0
    };
    /**
    * Flag indicating whether data from binary cursors must undergo an
    * endian conversion prior to use
    */
    bool swapEndian;
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
};
