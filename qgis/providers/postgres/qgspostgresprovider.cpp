/***************************************************************************
  qgspostgresprovider.cpp  -  QGIS data provider for PostgreSQL/PostGIS layers
                             -------------------
    begin                : 2004/01/07
    copyright            : (C) 2004 by Gary E.Sherman
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
#include <fstream>
#include <iostream>
#include <qtextstream.h>
#include <qstringlist.h>
#include "../../src/qgis.h"
#include "../../src/qgsfeature.h"
#include "../../src/qgsfield.h"
#include "../../src/qgsrect.h"

#include "qgspostgresprovider.h"
QgsPostgresProvider::QgsPostgresProvider(QString uri):dataSourceUri(uri)
{
/* OPEN LOG FILE */
  
    // make connection to the data source
    // for postgres, the connection information is passed as a space delimited
    // string:
    //  host=192.168.1.5 dbname=test user=gsherman password=xxx table=tablename
    //--std::cout << "Data source uri is " << uri << std::endl;
    // strip the table name off
    tableName = uri.mid(uri.find("table=") + 6);
   
    QString connInfo = uri.left(uri.find("table="));
    //--std::cout << "Table name is " << tableName << std::endl;
    //--std::cout << "Connection info is " << connInfo << std::endl;
    // calculate the schema if specified
    QString schema = "";
    if (tableName.find(".") > -1) {
        schema = tableName.left(tableName.find("."));
    }
    geometryColumn = tableName.mid(tableName.find(" (") + 2);
    geometryColumn.truncate(geometryColumn.length() - 1);
    tableName = tableName.mid(tableName.find(".") + 1, tableName.find(" (") - (tableName.find(".") + 1));

    qWarning("Geometry column is: " + geometryColumn);
    qWarning("Schema is: " + schema);
    qWarning("Table name is: " + tableName);
    QString logFile = "./pg_provider_" + tableName + ".log";
    pLog.open((const char *)logFile);
    pLog << "Opened log file for " << tableName << std::endl;
    PGconn *pd = PQconnectdb((const char *) connInfo);
    // check the connection status
    if (PQstatus(pd) == CONNECTION_OK) {
        //--std::cout << "Connection to the database was successful\n";
          // store the connection for future use
        connection = pd;
        // check the geometry column
        QString sql = "select f_geometry_column,type,srid from geometry_columns where f_table_name='"
          + tableName + "' and f_geometry_column = '" + geometryColumn + "' and f_table_schema = '" + schema + "'";
        qWarning("Getting geometry column: " + sql);
        PGresult *result = PQexec(pd, (const char *) sql);
        if (PQresultStatus(result) == PGRES_TUPLES_OK) {
            // this is a valid layer
            valid = true;
          
            //--std::cout << "geometry column query returned " << PQntuples(result) << std::endl;
            // store the srid 
            //--std::cout << "column number of srid is " << PQfnumber(result, "srid") << std::endl;
            srid = PQgetvalue(result, 0, PQfnumber(result, "srid"));
            //--std::cout << "SRID is " << srid << std::endl;
            
            // need to store the PostgreSQL endian format used in binary cursors
            // since it appears that starting with
            // version 7.4, binary cursors return data in XDR whereas previous versions
            // return data in the endian of the server
            QString firstOid = "select oid from " + tableName + " limit 1";
            PGresult * oidResult = PQexec(pd, firstOid);
            // get the int value from a "normal" select
            QString oidValue = PQgetvalue(oidResult,0,0);
            // get the same value using a binary cursor
            PQexec(pd,"begin work");
            QString oidDeclare = QString("declare oidcursor binary cursor for select oid from %1 where oid = %2").arg(tableName).arg(oidValue);
            // set up the cursor
            PQexec(pd, (const char *)oidDeclare);
             QString fetch = "fetch forward 1 from oidcursor";
            PGresult *fResult = PQexec(pd, (const char *)fetch);
            // get the oid value from the binary cursor
            int oid = *(int *)PQgetvalue(fResult,0,0);
           
            //--std::cout << "Got oid of " << oid << " from the binary cursor" << std::endl;
            //--std::cout << "First oid is " << oidValue << std::endl;
             // compare the two oid values to determine if we need to do an endian swap
            if(oid == oidValue.toInt()){
              swapEndian = false;
            }else{
              swapEndian = true;
            }
            // end the cursor transaction
            PQexec(pd, "end work");
      
            // set the type
            // set the simple type for use with symbology operations
            QString fType = PQgetvalue(result, 0, PQfnumber(result, "type"));
            if (fType == "POINT" || fType == "MULTIPOINT")
                geomType = QGis::WKBPoint;
            else if (fType == "LINESTRING" || fType == "MULTILINESTRING")
                geomType = QGis::WKBLineString;
            else if (fType == "POLYGON" || fType == "MULTIPOLYGON")
                geomType = QGis::WKBPolygon;
            //--std::cout << "Feature type is " << geomType << std::endl;
            //--std::cout << "Feature type name is " << QGis::qgisFeatureTypes[geomType] << std::endl;
            // free the result
            PQclear(result);
            // get the extents
           
            sql = "select xmax(extent(" + geometryColumn + ")) as xmax,"
              "xmin(extent(" + geometryColumn + ")) as xmin,"
              "ymax(extent(" + geometryColumn + ")) as ymax," "ymin(extent(" + geometryColumn + ")) as ymin" " from " + tableName;
               qWarning("Getting extents using schema.table: " + sql);
            result = PQexec(pd, (const char *) sql);
            layerExtent.setXmax(QString(PQgetvalue(result, 0, PQfnumber(result, "xmax"))).toDouble());
            layerExtent.setXmin(QString(PQgetvalue(result, 0, PQfnumber(result, "xmin"))).toDouble());
            layerExtent.setYmax(QString(PQgetvalue(result, 0, PQfnumber(result, "ymax"))).toDouble());
            layerExtent.setYmin(QString(PQgetvalue(result, 0, PQfnumber(result, "ymin"))).toDouble());
            QString xMsg;
            QTextOStream(&xMsg).precision(18);
            QTextOStream(&xMsg).width(18);
            QTextOStream(&xMsg) << "Set extents to: " << layerExtent.
              xMin() << ", " << layerExtent.yMin() << " " << layerExtent.xMax() << ", " << layerExtent.yMax();
            qWarning(xMsg);
            // clear query result
            PQclear(result);
            // get total number of features
            sql = "select count(*) from " + tableName;
            result = PQexec(pd, (const char *) sql);
            numberFeatures = QString(PQgetvalue(result, 0, 0)).toLong();
            //--std::cout << "Feature count is " << numberFeatures << std::endl;
            PQclear(result);
             // selectSQL stores the select sql statement. This has to include each attribute
             // plus the geometry column in binary form
            selectSQL = "select ";
            // Populate the field vector for this layer. The field vector contains
            // field name, type, length, and precision (if numeric)
            sql = "select * from " + tableName + " limit 1";
            result = PQexec(pd, (const char *) sql);
            //--std::cout << "Field: Name, Type, Size, Modifier:" << std::endl;
            for (int i = 0; i < PQnfields(result); i++) {

                QString fieldName = PQfname(result, i);
                int fldtyp = PQftype(result, i);
                QString typOid = QString().setNum(fldtyp);
                int fieldModifier = PQfmod(result, i);
                QString sql = "select typelem from pg_type where typelem = " + typOid + " and typlen = -1";
                //  //--std::cout << sql << std::endl;
                PGresult *oidResult = PQexec(pd, (const char *) sql);
                // get the oid of the "real" type
                QString poid = PQgetvalue(oidResult, 0, PQfnumber(oidResult, "typelem"));
                PQclear(oidResult);
                sql = "select typname, typlen from pg_type where oid = " + poid;
                // //--std::cout << sql << std::endl;
                oidResult = PQexec(pd, (const char *) sql);

                QString fieldType = PQgetvalue(oidResult, 0, 0);
                QString fieldSize = PQgetvalue(oidResult, 0, 1);
                PQclear(oidResult);
                //--std::cout << "Field: " << fieldName << ", " << fieldType << " (" << fldtyp << "),  " << fieldSize << ", " <<
                //  fieldModifier << std::endl;
                attributeFields.push_back(QgsField(fieldName, fieldType, fieldSize.toInt(), fieldModifier));
                // add to the select sql statement
                if(i > 0){
                  selectSQL += ", ";
                }
                if(fieldType == "geometry"){
                  selectSQL += "asbinary(" + geometryColumn + ",'" + endianString() + "') as qgs_feature_geometry";
                }else{
                  selectSQL += fieldName;
                }
            }
            // set the primary key
            getPrimaryKey();
            selectSQL += " from " + tableName;
            //--std::cout << "selectSQL: " << (const char *)selectSQL << std::endl;
            PQclear(result);
            // get the total number of features in the layer
            sql = "select count(*) from " + tableName;
            result = PQexec(pd, (const char *) sql);
            numberFeatures = QString(PQgetvalue(result, 0, 0)).toLong();
            pLog << "Number of features: " << numberFeatures << std::endl;
            //--std::cout << "Number of features in " << (const char *) tableName << ": " << numberFeatures << std::endl;
        } else {
            // the table is not a geometry table
            valid = false;
        }
//      reset tableName to include schema
        schemaTableName += schema + "." + tableName;
    
      
        ready = false; // not ready to read yet cuz the cursor hasn't been created
    } else {
        valid = false;
        //--std::cout << "Connection to database failed\n";
    }
    //create a boolean vector and set every entry to false

/* 	if (valid) {
		selected = new std::vector < bool > (ogrLayer->GetFeatureCount(), false);
	} else {
		selected = 0;
	} */
//  tabledisplay=0;
    //draw the selected features in yellow
//  selectionColor.setRgb(255,255,0);

}

QgsPostgresProvider::~QgsPostgresProvider()
{
  pLog.flush();
}

//TODO - we may not need this function - consider removing it from
//       the dataprovider.h interface
/**
	* Get the first feature resutling from a select operation
	* @return QgsFeature
	*/
  //TODO - this function is a stub and always returns 0
QgsFeature *QgsPostgresProvider::getFirstFeature(bool fetchAttributes)
{
    QgsFeature *f = 0;
    if (valid) {
        //--std::cout << "getting first feature\n";

        f = new QgsFeature();
        /*  f->setGeometry(getGeometryPointer(feat));
           if(fetchAttributes){
           getFeatureAttributes(feat, f);
           } */
    }
    return f;
}

    /**
	* Get the next feature resutling from a select operation
    * Return 0 if there are no features in the selection set
	* @return QgsFeature
	*/
QgsFeature *QgsPostgresProvider::getNextFeature(bool fetchAttributes)
{

    QgsFeature *f = 0;
    
    if (valid){
      // If the query result is not valid, then select all features
      // before proceeding
   /*    if(!ready) {
        // set up the 
        QString declare = QString("declare qgisf binary cursor for select oid," 
          "asbinary(%1,'%2') as qgs_feature_geometry from %3").arg(geometryColumn).arg(endianString()).arg(tableName);
        //--std::cout << "Selecting features using: " << declare << std::endl;
        // set up the cursor
        PQexec(connection,"begin work");
        PQexec(connection, (const char *)declare);
         //--std::cout << "Error: " << PQerrorMessage(connection) << std::endl;
           ready = true;
      } */
        QString fetch = "fetch forward 1 from qgisf";
        queryResult = PQexec(connection, (const char *)fetch);
        ////--std::cout << "Error: " << PQerrorMessage(connection) << std::endl;
        //--std::cout << "Fetched " << PQntuples(queryResult) << "rows" << std::endl;
        if(PQntuples(queryResult) == 0){
          PQexec(connection, "end work");
          ready = false;
          return 0;
        } 
     //  //--std::cout <<"Raw value of the geometry field: " << PQgetvalue(queryResult,0,PQfnumber(queryResult,"qgs_feature_geometry")) << std::endl;
       //--std::cout << "Length of oid is " << PQgetlength(queryResult,0, PQfnumber(queryResult,"oid")) << std::endl;
      // get the value of the primary key based on type
      
      int oid = *(int *)PQgetvalue(queryResult,0,PQfnumber(queryResult,primaryKey));
      // oid is in big endian
      int *noid;
     // if((endian() == NDR) && versionXDR){
       if(swapEndian){
        //--std::cout << "swapping endian for oid" << std::endl;
        // convert oid to opposite endian
        char *temp  = new char[sizeof(oid)];
        char *ptr = (char *)&oid + sizeof(oid) -1;
        int cnt = 0;
        while(cnt < sizeof(oid)){
          temp[cnt] = *ptr--;
          cnt++;
        }
        noid = (int *)temp;
        
      }else{
        noid = &oid;
      }
      // noid contains the oid to be used in fetching attributes if 
      // fetchAttributes = true
      //--std::cout << "OID: " << *noid << std::endl;
      int returnedLength = PQgetlength(queryResult,0, PQfnumber(queryResult,"qgs_feature_geometry"));
      //--std::cout << "Returned length is " << returnedLength << std::endl;
      if(returnedLength > 0){
      unsigned char *feature = new unsigned char[returnedLength + 1];
      memset(feature, '\0', returnedLength + 1);
      memcpy(feature, PQgetvalue(queryResult,0,PQfnumber(queryResult,"qgs_feature_geometry")), returnedLength);
      int wkbType = *((int *) (feature + 1));
      //--std::cout << "WKBtype is: " << wkbType << std::endl;
      f = new QgsFeature(*noid);
      f->setGeometry(feature);
        if (fetchAttributes) {
          getFeatureAttributes(*noid, f);
        }
      }else{
        //--std::cout <<"Couldn't get the feature geometry in binary form" << std::endl;
    }
    }
    else {
        //--std::cout << "Read attempt on an invalid postgresql data source\n";
    }
    return f;
}

    /**
	* Select features based on a bounding rectangle. Features can be retrieved
	* with calls to getFirstFeature and getNextFeature.
	* @param mbr QgsRect containing the extent to use in selecting features
	*/
void QgsPostgresProvider::select(QgsRect * rect)
{
    // spatial query to select features
    //--std::cout << "Selection rectangle is " << *rect << std::endl;
    QString declare = QString("declare qgisf binary cursor for select "
      + primaryKey  
      + ",asbinary(%1,'%2') as qgs_feature_geometry from %3").arg(geometryColumn).arg(endianString()).arg(tableName);
    declare += " where " + geometryColumn;
    declare += " && GeometryFromText('BOX3D(" + rect->stringRep();
    declare += ")'::box3d,";
    declare += srid;
    declare += ")";          
    //--std::cout << "Selecting features using: " << declare << std::endl;
    // set up the cursor
    if(ready){
      PQexec(connection, "end work");
    }
    PQexec(connection,"begin work");
    PQexec(connection, (const char *)declare);
}

    /**
		* Set the data source specification. This may be a path or database
	* connection string
	* @uri data source specification
	*/
void QgsPostgresProvider::setDataSourceUri(QString uri)
{
    dataSourceUri = uri;
}

        /**
	* Get the data source specification. This may be a path or database
	* connection string
	* @return data source specification
	*/
QString QgsPostgresProvider::getDataSourceUri()
{
    return dataSourceUri;
}

    /**
	* Identify features within the search radius specified by rect
	* @param rect Bounding rectangle of search radius
	* @return std::vector containing QgsFeature objects that intersect rect
	*/
std::vector<QgsFeature>& QgsPostgresProvider::identify(QgsRect * rect)
{
    features.clear();
    // select the features
    select(rect);

    return features;
}

/* unsigned char * QgsPostgresProvider::getGeometryPointer(OGRFeature *fet){
//	OGRGeometry *geom = fet->GetGeometryRef();
	unsigned char *gPtr=0;
		// get the wkb representation
	 	gPtr = new unsigned char[geom->WkbSize()];
      
		geom->exportToWkb((OGRwkbByteOrder) endian(), gPtr);
	return gPtr;

} */

int QgsPostgresProvider::endian()
{
    char *chkEndian = new char[4];
    memset(chkEndian, '\0', 4);
    chkEndian[0] = 0xE8;

    int *ce = (int *) chkEndian;
    int retVal;
    if (232 == *ce)
        retVal = NDR;
    else
        retVal = XDR;
    delete[]chkEndian;
    return retVal;
}

// TODO - make this function return the real extent_
QgsRect *QgsPostgresProvider::extent()
{
    return &layerExtent;      //extent_->MinX, extent_->MinY, extent_->MaxX, extent_->MaxY);
}

/** 
* Return the feature type
*/
int QgsPostgresProvider::geometryType()
{
    return geomType;
}

/** 
* Return the feature type
*/
long QgsPostgresProvider::featureCount()
{
    return numberFeatures;
}

/**
* Return the number of fields
*/
int QgsPostgresProvider::fieldCount()
{
    return attributeFields.size();
}

/**
* Fetch attributes for a selected feature
*/
 void QgsPostgresProvider::getFeatureAttributes(int key, QgsFeature *f){
  QString sql = QString("select * from %1 where %2 = %3").arg(tableName).arg(primaryKey).arg(key);
  PGresult *attr = PQexec(connection, (const char *)sql);
  
   for (int i = 0; i < fieldCount(); i++) {
     QString fld = PQfname(attr, i);
     // Dont add the WKT representation of the geometry column to the identify
     // results
     if(fld != geometryColumn){
       // Add the attribute to the feature
       QString val = PQgetvalue(attr,0, i);
       f->addAttribute(fld, val);
     }
   }
} 

std::vector<QgsField>& QgsPostgresProvider::fields()
{
    return attributeFields;
}

void QgsPostgresProvider::reset()
{
  // reset the cursor to the first record
  //--std::cout << "Resetting the cursor to the first record " << std::endl;
   QString declare = QString("declare qgisf binary cursor for select " +
    primaryKey + 
    ",asbinary(%1,'%2') as qgs_feature_geometry from %3").arg(geometryColumn).arg(endianString()).arg(tableName);
        //--std::cout << "Selecting features using: " << declare << std::endl;
        pLog << "Setting up binary cursor: " << declare << std::endl;
        // set up the cursor
        PQexec(connection,"end work");
        PQexec(connection,"begin work");
        PQexec(connection, (const char *)declare);
         //--std::cout << "Error: " << PQerrorMessage(connection) << std::endl;
           ready = true;
}
/* QString QgsPostgresProvider::getFieldTypeName(PGconn * pd, int oid)
{
    QString typOid = QString().setNum(oid);
    QString sql = "select typelem from pg_type where typelem = " + typOid + " and typlen = -1";
    ////--std::cout << sql << std::endl;
    PGresult *result = PQexec(pd, (const char *) sql);
    // get the oid of the "real" type
    QString poid = PQgetvalue(result, 0, PQfnumber(result, "typelem"));
    PQclear(result);
    sql = "select typname, typlen from pg_type where oid = " + poid;
    // //--std::cout << sql << std::endl;
    result = PQexec(pd, (const char *) sql);

    QString typeName = PQgetvalue(result, 0, 0);
    QString typeLen = PQgetvalue(result, 0, 1);
    PQclear(result);
    typeName += "(" + typeLen + ")";
    return typeName;
} */
QString QgsPostgresProvider::endianString()
{
	char *chkEndian = new char[4];
	memset(chkEndian, '\0', 4);
	chkEndian[0] = 0xE8;
	int *ce = (int *) chkEndian;
	if (232 == *ce)
		return QString("NDR");
	else
		return QString("XDR");
}
QString QgsPostgresProvider::getPrimaryKey(){
  QString sql = "select oid from pg_class where relname = '" + tableName + "'";
  pLog << "Getting primary key" << std::endl;
  pLog << sql << std::endl;
  PGresult *pk = PQexec(connection,(const char *)sql);
  pLog << "Got " << PQntuples(pk) << " rows " << std::endl;
  // get the oid for the table
  QString oid = PQgetvalue(pk,0,0);
  // check to see if there is a primary key
  sql = "select indkey from pg_index where indrelid = " +
    oid + " and indisprimary = 't'";
  pLog << sql << std::endl;
  PQclear(pk);
  pk = PQexec(connection,(const char *)sql);
  // if we got no tuples we ain't go no pk :)
  if(PQntuples(pk) == 0){
    // no key - should we warn the user that performance will suffer
    pLog << "Table has no primary key -- using oid to fetch records" << std::endl;
    primaryKey = "oid";
  }else{
    // store the key column
    QString keyString = PQgetvalue(pk,0,0);
    QStringList columns = QStringList::split(" ", keyString);
    if(columns.count() > 1){
      //TODO concatenated key -- can we use this?
      pLog << "Table has a concatenated primary key" << std::endl;
    }
    primaryKeyIndex = columns[0].toInt()-1;
    QgsField fld = attributeFields[primaryKeyIndex];
     primaryKey = fld.getName();
     pLog << "Primary key is " << primaryKey << std::endl;//);// +<< " at column " << primaryKeyIndex << std::endl;
     pLog.flush();
  }
  PQclear(pk);
  return primaryKey;
  /*
Process to determine the fields used in a primary key:
  test=# select oid from pg_class where relname = 'earthquakes';
    oid
  -------
   24865
  (1 row)
 
  test=# select indkey from pg_index where indrelid = 24865 and indisprimary = 't';
   indkey
  --------
   1 5
  (1 row)

Primary key is composed of fields 1 and 5
*/
}

// Returns the minimum value of an attribute
QString QgsPostgresProvider::minValue(int position){
    // get the field name 
    QgsField fld = attributeFields[position];
    QString sql = QString("select min(%1) from %2").arg(fld.getName()).arg(tableName);
    PGresult *rmin = PQexec(connection,(const char *)sql);
    QString minValue = PQgetvalue(rmin,0,0);
    PQclear(rmin);
    return minValue;
  }

// Returns the maximum value of an attribute
    
QString QgsPostgresProvider::maxValue(int position){
     // get the field name 
    QgsField fld = attributeFields[position];
    QString sql = QString("select max(%1) from %2").arg(fld.getName()).arg(tableName);
    PGresult *rmax = PQexec(connection,(const char *)sql);
    QString maxValue = PQgetvalue(rmax,0,0);
    PQclear(rmax);
    return maxValue;
  }
  

/**
* Class factory to return a pointer to a newly created 
* QgsPostgresProvider object
*/
extern "C" QgsPostgresProvider * classFactory(const char *uri)
{
    return new QgsPostgresProvider(uri);
}
/** Required key function (used to map the plugin to a data store type)
*/
extern "C" QString providerKey(){
	return QString("postgres");
}
/**
* Required description function 
*/
extern "C" QString description(){
	return QString("PostgreSQL/PostGIS data provider");
} 
/**
* Required isProvider function. Used to determine if this shared library
* is a data provider plugin
*/
extern "C" bool isProvider(){
  return true;
}
