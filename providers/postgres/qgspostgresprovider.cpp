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
#include <qapplication.h>
#include <qmessagebox.h>
#include <qcursor.h>

// for ntohl
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include "../../src/qgis.h"
#include "../../src/qgsfeature.h"
#include "../../src/qgsfield.h"
#include "../../src/qgsrect.h"

#include "qgspostgresprovider.h"
#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif
QgsPostgresProvider::QgsPostgresProvider(QString uri):dataSourceUri(uri)
{
  // assume this is a valid layer until we determine otherwise
  valid = true;
  /* OPEN LOG FILE */

  // Make connection to the data source
  // For postgres, the connection information is passed as a space delimited
  // string:
  //  host=192.168.1.5 dbname=test port=5342 user=gsherman password=xxx table=tablename
  //--std::cout << "Data source uri is " << uri << std::endl;
  
  // Strip the table and sql statement name off and store them
  int sqlStart = uri.find(" sql");
  int tableStart = uri.find("table=");
#ifdef QGISDEBUG
  qDebug( "****************************************");
  qDebug(  "****   Postgresql Layer Creation   *****" );
  qDebug(  "****************************************");
  qDebug(  "URI: " + uri );
  QString msg;
  
  qDebug(  "tableStart: " + msg.setNum(tableStart) );
  qDebug(  "sqlStart: " + msg.setNum(sqlStart));
#endif 
  tableName = uri.mid(tableStart + 6, sqlStart - tableStart -6);
 if(sqlStart > -1)
 { 
    sqlWhereClause = uri.mid(sqlStart + 5);
 }
 else
 {
   sqlWhereClause = QString::null;
 }
  QString connInfo = uri.left(uri.find("table="));
#ifdef QGISDEBUG
  qDebug( "Table name is " + tableName);
  qDebug( "SQL is " + sqlWhereClause );
  qDebug( "Connection info is " + connInfo);
#endif
  // calculate the schema if specified
  QString schema = "";
  if (tableName.find(".") > -1) {
    schema = tableName.left(tableName.find("."));
  }
  geometryColumn = tableName.mid(tableName.find(" (") + 2);
  geometryColumn.truncate(geometryColumn.length() - 1);
  tableName = tableName.mid(tableName.find(".") + 1, tableName.find(" (") - (tableName.find(".") + 1));
  
  /* populate the uri structure */
  mUri.schema = schema;
  mUri.table = tableName;
  mUri.geometryColumn = geometryColumn;
  mUri.sql = sqlWhereClause;
  // parse the connection info
  QStringList conParts = QStringList::split(" ", connInfo);
  QStringList parm = QStringList::split("=", conParts[0]);
  if(parm.size() == 2)
  {
    mUri.host = parm[1];
  }
  parm = QStringList::split("=", conParts[1]);
  if(parm.size() == 2)
  {
    mUri.database = parm[1];
  }
  parm = QStringList::split("=", conParts[2]);
  if(parm.size() == 2)
  {
    mUri.port = parm[1];
  }

  parm = QStringList::split("=", conParts[3]);
  if(parm.size() == 2)
  {
    mUri.username = parm[1];
  }
  parm = QStringList::split("=", conParts[4]);
  if(parm.size() == 2)
  {
    mUri.password = parm[1];
  }
  /* end uri structure */

#ifdef QGISDEBUG
  std::cerr << "Geometry column is: " << geometryColumn << std::endl;
  std::cerr << "Schema is: " + schema << std::endl;
  std::cerr << "Table name is: " + tableName << std::endl;
#endif
  //QString logFile = "./pg_provider_" + tableName + ".log";
  //pLog.open((const char *)logFile);
#ifdef QGISDEBUG
  std::cerr << "Opened log file for " << tableName << std::endl;
#endif
  PGconn *pd = PQconnectdb((const char *) connInfo);
  // check the connection status
  if (PQstatus(pd) == CONNECTION_OK) {
    /* Check to see if we have GEOS support and if not, warn the user about
       the problems they will see :) */
#ifdef QGISDEBUG
    std::cerr << "Checking for GEOS support" << std::endl;
#endif
    if(!hasGEOS(pd)){
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(0, "No GEOS Support!",
          "Your PostGIS installation has no GEOS support.\nFeature selection and "
          "identification will not work properly.\nPlease install PostGIS with " 
          "GEOS support (http://geos.refractions.net)");
      QApplication::setOverrideCursor(Qt::waitCursor);
    }
    //--std::cout << "Connection to the database was successful\n";
    // set the schema

    PQexec(pd,(const char *)QString("set search_path = '%1','public'").arg(schema));
    // store the connection for future use
    connection = pd;
    // check the geometry column
    QString sql = "select f_geometry_column,type,srid from geometry_columns where f_table_name='"
      + tableName + "' and f_geometry_column = '" + geometryColumn + "' and f_table_schema = '" + schema + "'";
#ifdef QGISDEBUG
    std::cerr << "Getting geometry column: " + sql << std::endl;
#endif
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
#ifdef QGISDEBUG
      std::cerr << "Creating binary cursor" << std::endl;
#endif
      // get the same value using a binary cursor
      PQexec(pd,"begin work");
      QString oidDeclare = QString("declare oidcursor binary cursor for select oid from %1 where oid = %2").arg(tableName).arg(oidValue);
      // set up the cursor
      PQexec(pd, (const char *)oidDeclare);
      QString fetch = "fetch forward 1 from oidcursor";
#ifdef QGISDEBUG
      std::cerr << "Fetching a record and attempting to get check endian-ness" << std::endl;
#endif
      PGresult *fResult = PQexec(pd, (const char *)fetch);
      if(PQntuples(fResult) > 0){
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
        PQclear(fResult);

        // end the cursor transaction
        PQexec(pd, "end work");
#ifdef QGISDEBUG
        std::cerr << "Setting layer type" << std::endl;
#endif
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

        sql = "select extent(" + geometryColumn + ") from " + tableName;
        if(sqlWhereClause.length() > 0)
        {
          sql += " where " + sqlWhereClause;
        }

#if WASTE_TIME
        sql = "select xmax(extent(" + geometryColumn + ")) as xmax,"
          "xmin(extent(" + geometryColumn + ")) as xmin,"
          "ymax(extent(" + geometryColumn + ")) as ymax," "ymin(extent(" + geometryColumn + ")) as ymin" " from " + tableName;
#endif

#ifdef QGISDEBUG 
        std::cerr << "Getting extents using schema.table: " + sql << std::endl;
#endif
        result = PQexec(pd, (const char *) sql);
        std::string box3d = PQgetvalue(result, 0, 0);
        std::string s;

        box3d = box3d.substr(box3d.find_first_of("(")+1);
        box3d = box3d.substr(box3d.find_first_not_of(" "));
        s = box3d.substr(0, box3d.find_first_of(" "));
        double minx = strtod(s.c_str(), NULL);

        box3d = box3d.substr(box3d.find_first_of(" ")+1);
        s = box3d.substr(0, box3d.find_first_of(" "));
        double miny = strtod(s.c_str(), NULL);

        box3d = box3d.substr(box3d.find_first_of(",")+1);
        box3d = box3d.substr(box3d.find_first_not_of(" "));
        s = box3d.substr(0, box3d.find_first_of(" "));
        double maxx = strtod(s.c_str(), NULL);

        box3d = box3d.substr(box3d.find_first_of(" ")+1);
        s = box3d.substr(0, box3d.find_first_of(" "));
        double maxy = strtod(s.c_str(), NULL);

        layerExtent.setXmax(maxx);
        layerExtent.setXmin(minx);
        layerExtent.setYmax(maxy);
        layerExtent.setYmin(miny);
        QString xMsg;
        QTextOStream(&xMsg).precision(18);
        QTextOStream(&xMsg).width(18);
        QTextOStream(&xMsg) << "Set extents to: " << layerExtent.
          xMin() << ", " << layerExtent.yMin() << " " << layerExtent.xMax() << ", " << layerExtent.yMax();
#ifdef QGISDEBUG
        std::cerr << xMsg << std::endl;
#endif
        // clear query result
        PQclear(result);
        // get total number of features
        sql = "select count(*) from " + tableName;
        if(sqlWhereClause.length() > 0)
        {
          sql += " where " + sqlWhereClause;
        }
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
        sql = "select oid from pg_class where relname = '" + tableName + "'";
        PGresult *tresult= PQexec(pd, (const char *)sql);
        QString tableoid = PQgetvalue(tresult, 0, 0);
        PQclear(tresult);
        sql = "select attnum from pg_attribute where attrelid = " + tableoid + " and attname = '" + fieldName + "'";
        tresult = PQexec(pd, (const char *)sql);
        QString attnum = PQgetvalue(tresult, 0, 0);
        PQclear(tresult);
#ifdef QGISDEBUG
          std::cerr << "Field: " << attnum << " maps to " << i << " " << fieldName << ", " 
                  << fieldType << " (" << fldtyp << "),  " << fieldSize << ", "  
                  << fieldModifier << std::endl;
#endif
        attributeFieldsIdMap[attnum.toInt()] = i;
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
        if(sqlWhereClause.length() > 0)
        {
          sql += " where " + sqlWhereClause;
        }
        result = PQexec(pd, (const char *) sql);
        numberFeatures = QString(PQgetvalue(result, 0, 0)).toLong();
#ifdef QGISDEBUG
        std::cerr << "Number of features: " << numberFeatures << std::endl;
#endif
        PQclear(result);
      }else{
        numberFeatures = 0;
        valid = false;
      }//--std::cout << "Number of features in " << (const char *) tableName << ": " << numberFeatures << std::endl;
    } else {
      // the table is not a geometry table
      valid = false;
#ifdef QGISDEBUG
      std::cerr << "Invalid Postgres layer" << std::endl;
#endif
    }
    //      reset tableName to include schema
    schemaTableName += schema + "." + tableName;


    ready = false; // not ready to read yet cuz the cursor hasn't been created
  } else {
    valid = false;
    //--std::cout << "Connection to database failed\n";
  }
  //create a boolean vector and set every entry to false

  /*  if (valid) {
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
  PQfinish(connection);
  //pLog.flush();
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

bool QgsPostgresProvider::getNextFeature(QgsFeature &feature, bool fetchAttributes)
{
  return true;
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
    QString fetch = "fetch forward 1 from qgisf";
    queryResult = PQexec(connection, (const char *)fetch);
//    std::cerr << "Error: " << PQerrorMessage(connection) << std::endl;
//   std::cerr << "Fetched " << PQntuples(queryResult) << "rows" << std::endl;
    if(PQntuples(queryResult) == 0){
      PQexec(connection, "end work");
      ready = false;
      return 0;
    } 
    //  //--std::cout <<"Raw value of the geometry field: " << PQgetvalue(queryResult,0,PQfnumber(queryResult,"qgs_feature_geometry")) << std::endl;
    //--std::cout << "Length of oid is " << PQgetlength(queryResult,0, PQfnumber(queryResult,"oid")) << std::endl;
    // get the value of the primary key based on type

    int oid = *(int *)PQgetvalue(queryResult,0,PQfnumber(queryResult,primaryKey));
#ifdef QGISDEBUG
   // std::cerr << "OID from database: " << oid << std::endl; 
#endif
    // oid is in big endian
    // XXX If you're so sure about that, then why do you have to check to swap?
    int *noid;

    // We don't support primary keys that are not int4 so if
    // the key is int8 we use the oid as the id instead.
    // TODO Throw a warning to let the user know that the layer
    // is not using a primary key and that performance will suffer

    // XXX noid = &oid could probably be moved out of if statements since all
    // XXX valid execution paths do that
    if(primaryKeyType == "int8")
    {
      noid = &oid;
    }
    else
    {
      if(swapEndian)
      {
        // XXX I'm assuming swapping from big-endian, or network, byte order to little endian
#ifdef QGISDEBUG
//        std::cerr << "swapping endian for oid" << std::endl;
#endif 
        // convert oid to opposite endian
        // XXX "Opposite?"  Umm, that's not enough information.
        oid = ntohl(oid);
        noid = &oid;
      }
      else
      {
        noid = &oid;
      }
    }
    // noid contains the oid to be used in fetching attributes if 
    // fetchAttributes = true
#ifdef QGISDEBUG
//    std::cerr << "Using OID: " << *noid << std::endl;
#endif
    int returnedLength = PQgetlength(queryResult,0, PQfnumber(queryResult,"qgs_feature_geometry"));
    //--std::cout << "Returned length is " << returnedLength << std::endl;
    if(returnedLength > 0){
      unsigned char *feature = new unsigned char[returnedLength + 1];
      memset(feature, '\0', returnedLength + 1);
      memcpy(feature, PQgetvalue(queryResult,0,PQfnumber(queryResult,"qgs_feature_geometry")), returnedLength);
      int wkbType = *((int *) (feature + 1));
      //--std::cout << "WKBtype is: " << wkbType << std::endl;
      f = new QgsFeature(*noid);
      f->setGeometry(feature, returnedLength + 1);
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

QgsFeature* QgsPostgresProvider::getNextFeature(std::list<int> const & attlist)
{
  QgsFeature *f = 0;
  if (valid)
  {
    QString fetch = "fetch forward 1 from qgisf";
    queryResult = PQexec(connection, (const char *)fetch);
    if(PQntuples(queryResult) == 0)
    {
#ifdef QGISDEBUG
  std::cerr << "Feature is null\n";
#endif  
  PQexec(connection, "end work");
  ready = false;
  return 0;
    }

    int *noid;
    int oid = *(int *)PQgetvalue(queryResult,0,PQfnumber(queryResult,primaryKey));
#ifdef QGISDEBUG 
//  std::cerr << "Primary key type is " << primaryKeyType << std::endl; 
#endif  
  // We don't support primary keys that are not int4 so if
  // the key is int8 we use the oid as the id instead.
  // TODO Throw a warning to let the user know that the layer
  // is not using a primary key and that performance will suffer
  if(primaryKeyType == "int8")
  {
      noid = &oid;
  }
  else
  {
      if(swapEndian)
      {
    char *temp  = new char[sizeof(oid)];
    char *ptr = (char *)&oid + sizeof(oid) -1;
    int cnt = 0;
    while(cnt < sizeof(oid))
    {
        temp[cnt] = *ptr--;
        cnt++;
    }  
    noid = (int *)temp;
      }
      else
      {
    noid = &oid;
      }
  }
  
    
    
    int returnedLength = PQgetlength(queryResult,0, PQfnumber(queryResult,"qgs_feature_geometry")); 
    if(returnedLength > 0)
    {
      unsigned char *feature = new unsigned char[returnedLength + 1];
      memset(feature, '\0', returnedLength + 1);
      memcpy(feature, PQgetvalue(queryResult,0,PQfnumber(queryResult,"qgs_feature_geometry")), returnedLength); 
      int wkbType = *((int *) (feature + 1));
      f = new QgsFeature(*noid);
      f->setGeometry(feature, returnedLength + 1);
      if(!attlist.empty())
      {
        getFeatureAttributes(*noid, f, attlist);
      } 

    }
    else
    {
      //--std::cout <<"Couldn't get the feature geometry in binary form" << std::endl;
    }
  }
  else 
  {
    //--std::cout << "Read attempt on an invalid postgresql data source\n";
  }
  return f;   
}

/**
 * Select features based on a bounding rectangle. Features can be retrieved
 * with calls to getFirstFeature and getNextFeature.
 * @param mbr QgsRect containing the extent to use in selecting features
 */
void QgsPostgresProvider::select(QgsRect * rect, bool useIntersect)
{
  // spatial query to select features

#ifdef QGISDEBUG
  std::cerr << "Selection rectangle is " << *rect << std::endl;
  std::cerr << "Selection polygon is " << rect->asPolygon() << std::endl;
#endif
  QString declare = QString("declare qgisf binary cursor for select "
      + primaryKey  
      + ",asbinary(%1,'%2') as qgs_feature_geometry from %3").arg(geometryColumn).arg(endianString()).arg(tableName);
#ifdef QGISDEBUG
  std::cout << "Binary cursor: " << declare << std::endl; 
#endif
  if(useIntersect){
    declare += " where intersects(" + geometryColumn;
    declare += ", GeometryFromText('BOX3D(" + rect->stringRep();
    declare += ")'::box3d,";
    declare += srid;
    declare += "))";
  }else{
    declare += " where " + geometryColumn;
    declare += " && GeometryFromText('BOX3D(" + rect->stringRep();
    declare += ")'::box3d,";
    declare += srid;
    declare += ")";
  }
  if(sqlWhereClause.length() > 0)
  {
    declare += " and " + sqlWhereClause;
  }

#ifdef QGISDEBUG
  std::cerr << "Selecting features using: " << declare << std::endl;
#endif
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

QgsDataSourceURI * QgsPostgresProvider::getURI()
{
  return &mUri;
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
//  OGRGeometry *geom = fet->GetGeometryRef();
unsigned char *gPtr=0;
// get the wkb representation
gPtr = new unsigned char[geom->WkbSize()];

geom->exportToWkb((OGRwkbByteOrder) endian(), gPtr);
return gPtr;

} */


// TODO - make this function return the real extent_
QgsRect *QgsPostgresProvider::extent()
{
  return &layerExtent;      //extent_->MinX, extent_->MinY, extent_->MaxX, extent_->MaxY);
}

/** 
 * Return the feature type
 */
int QgsPostgresProvider::geometryType() const
{
  return geomType;
}

/** 
 * Return the feature type
 */
long QgsPostgresProvider::featureCount() const
{
  return numberFeatures;
}

/**
 * Return the number of fields
 */
int QgsPostgresProvider::fieldCount() const
{
  return attributeFields.size();
}

/**
 * Fetch attributes for a selected feature
 */
void QgsPostgresProvider::getFeatureAttributes(int key, QgsFeature *f){
  QString sql = QString("select * from %1 where %2 = %3").arg(tableName).arg(primaryKey).arg(key);
#ifdef QGISDEBUG
//  std::cerr << "getFeatureAttributes using: " << sql << std::endl; 
#endif
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

/**Fetch attributes with indices contained in attlist*/
void QgsPostgresProvider::getFeatureAttributes(int key, 
                                               QgsFeature *f, 
                                               std::list<int> const & attlist)
{
  std::list<int>::const_iterator iter;
  int i=-1;
  for(iter=attlist.begin();iter!=attlist.end();++iter)
  {
    ++i;
    QString sql = QString("select %1 from %2 where %3 = %4")
      .arg(fields()[*iter].name())
      .arg(tableName)
      .arg(primaryKey)
      .arg(key);//todo: only query one attribute
    PGresult *attr = PQexec(connection, (const char *)sql);
    QString fld = PQfname(attr, 0);
    // Dont add the WKT representation of the geometry column to the identify
    // results
    if(fld != geometryColumn)
    {
      // Add the attribute to the feature
      QString val = PQgetvalue(attr,0, i);
      //qWarning(val);
      f->addAttribute(fld, val);
    }
  }
}

std::vector<QgsField> const & QgsPostgresProvider::fields() const
{
  return attributeFields;
}

void QgsPostgresProvider::reset()
{
  // reset the cursor to the first record
  //--std::cout << "Resetting the cursor to the first record " << std::endl;
  QString declare = QString("declare qgisf binary cursor for select " +
      primaryKey + 
      ",asbinary(%1,'%2') as qgs_feature_geometry from %3").arg(geometryColumn)
      .arg(endianString()).arg(tableName);
  if(sqlWhereClause.length() > 0)
  {
    declare += " where " + sqlWhereClause;
  }
  //--std::cout << "Selecting features using: " << declare << std::endl;
#ifdef QGISDEBUG
  std::cerr << "Setting up binary cursor: " << declare << std::endl;
#endif
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

/** @todo XXX Perhaps this should be promoted to QgsDataProvider? */
QString QgsPostgresProvider::endianString()
{
  switch ( endian() )
  {
    case QgsDataProvider::NDR : 
      return QString("NDR");
      break;
    case QgsDataProvider::XDR : 
      return QString("XDR");
      break;
    default :
      return QString("UNKNOWN");
  }
}

QString QgsPostgresProvider::getPrimaryKey(){
  QString sql = "select oid from pg_class where relname = '" + tableName + "'";
#ifdef QGISDEBUG
  std::cerr << "Getting primary key" << std::endl;
  std::cerr << sql << std::endl;
#endif
  PGresult *pk = PQexec(connection,(const char *)sql);
#ifdef QGISDEBUG
  std::cerr << "Got " << PQntuples(pk) << " rows " << std::endl;
#endif
  // get the oid for the table
  QString oid = PQgetvalue(pk,0,0);
  // check to see if there is a primary key
  sql = "select indkey from pg_index where indrelid = " +
    oid + " and indisprimary = 't'";
#ifdef QGISDEBUG
  std::cerr << sql << std::endl;
#endif
  PQclear(pk);
  pk = PQexec(connection,(const char *)sql);
  // if we got no tuples we ain't go no pk :)
  if(PQntuples(pk) == 0){
    // no key - should we warn the user that performance will suffer
#ifdef QGISDEBUG
    std::cerr << "Table has no primary key -- using oid to fetch records" << std::endl;
#endif
    primaryKey = "oid";
  }else{
    // store the key column
    QString keyString = PQgetvalue(pk,0,0);
    QStringList columns = QStringList::split(" ", keyString);
    if(columns.count() > 1){
      //TODO concatenated key -- can we use this?
#ifdef QGISDEBUG
      std::cerr << "Table has a concatenated primary key" << std::endl;
#endif
    }
    primaryKeyIndex = attributeFieldsIdMap[columns[0].toInt()];
    QgsField fld = attributeFields[primaryKeyIndex];
    // if the primary key is 4-byte integer we use it
    if(fld.type() == "int4")
    {
      primaryKey = fld.name();
      primaryKeyType = fld.type();
    }
    else
    {
      // key is not a 4-byte int -- use the oid instead
      primaryKey = "oid";
    }
#ifdef QGISDEBUG
    std::cerr << "Primary key is " << primaryKey << std::endl;//);// +<< " at column " << primaryKeyIndex << std::endl;
#endif
    //  pLog.flush();
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
  QString sql;
  if(sqlWhereClause.isEmpty())
  {
      sql = QString("select min(%1) from %2").arg(fld.name()).arg(tableName);
  }
  else
  {
      sql = QString("select min(%1) from %2").arg(fld.name()).arg(tableName)+" where "+sqlWhereClause;
  }
  PGresult *rmin = PQexec(connection,(const char *)sql);
  QString minValue = PQgetvalue(rmin,0,0);
  PQclear(rmin);
  return minValue;
}

// Returns the maximum value of an attribute

QString QgsPostgresProvider::maxValue(int position){
  // get the field name 
  QgsField fld = attributeFields[position];
  QString sql;
  if(sqlWhereClause.isEmpty())
  {
      sql = QString("select max(%1) from %2").arg(fld.name()).arg(tableName);
  }
  else
  {
      sql = QString("select max(%1) from %2").arg(fld.name()).arg(tableName)+" where "+sqlWhereClause;
  } 
  PGresult *rmax = PQexec(connection,(const char *)sql);
  QString maxValue = PQgetvalue(rmax,0,0);
  PQclear(rmax);
  return maxValue;
}

bool QgsPostgresProvider::isValid(){
  return valid;
}

bool QgsPostgresProvider::addFeature(QgsFeature* f)
{
    if(f)
    {
  QString insert("INSERT INTO ");
  insert+=tableName;
  insert+="(";

  //add the name of the geometry column to the insert statement
  insert+=geometryColumn;//first the geometry

  //add the names of the other fields to the insert
  std::vector<QgsFeatureAttribute> attributevec=f->attributeMap();
  for(std::vector<QgsFeatureAttribute>::iterator it=attributevec.begin();it!=attributevec.end();++it)
  {
      QString fieldname=it->fieldName();
      if(fieldname!=geometryColumn)
      {
    insert+=",";
    insert+=fieldname;
      }
  }

  insert+=") VALUES (GeomFromWKB('";

  //add the wkb geometry to the insert statement
  unsigned char* geom=f->getGeometry();
  for(int i=0;i<f->getGeometrySize();++i)
  {
      QString hex=QString::number((int)geom[i],16).upper();
      if(hex.length()==1)
      {
    hex="0"+hex;
      }
      insert+=hex;
  }
  insert+="',-1)";

  //add the field values to the insert statement
  for(std::vector<QgsFeatureAttribute>::iterator it=attributevec.begin();it!=attributevec.end();++it)
  {
      if(it->fieldName()!=geometryColumn)
      {
    QString fieldvalue=it->fieldValue();
    bool charactertype=false;
    insert+=",";

    //add quotes if the field is a characted type
    if(fieldvalue!="NULL")
    {
        for(std::vector<QgsField>::iterator iter=attributeFields.begin();iter!=attributeFields.end();++iter)
        {
      if(iter->name()==it->fieldName())
      {
          if(iter->type().contains("char",false)>0||iter->type()=="text")
          {
        charactertype=true;
          }
      }
        }
    }

    if(charactertype)
    {
        insert+="'";
    }
    insert+=fieldvalue;
    if(charactertype)
    {
        insert+="'";
    }
      }
  }

  insert+=")";
#ifdef QGISDEBUG
  qWarning("insert statement is: "+insert);
#endif

  //send INSERT statement and do error handling
  PGresult* result=PQexec(connection, (const char *)insert);
  if(result==0)
  {
      QMessageBox::information(0,"INSERT error","An error occured during feature insertion",QMessageBox::Ok);
      return false;
  }
  ExecStatusType message=PQresultStatus(result);
  if(message==PGRES_FATAL_ERROR)
  {
      QMessageBox::information(0,"INSERT error",QString(PQresultErrorMessage(result)),QMessageBox::Ok);
      return false;
  }
  
  return true;
    }
    return false;
}

QString QgsPostgresProvider::getDefaultValue(const QString& attr, QgsFeature* f)
{
    return "NULL";
}

bool QgsPostgresProvider::deleteFeature(int id)
{
    QString sql("DELETE FROM "+tableName+" WHERE "+primaryKey+" = "+QString::number(id));
#ifdef QGISDEBUG
    qWarning("delete sql: "+sql);
#endif

    //send DELETE statement and do error handling
    PGresult* result=PQexec(connection, (const char *)sql);
    if(result==0)
    {
  QMessageBox::information(0,"DELETE error","An error occured during deletion from disk",QMessageBox::Ok);
  return false;
    }
    ExecStatusType message=PQresultStatus(result);
    if(message==PGRES_FATAL_ERROR)
    {
  QMessageBox::information(0,"DELETE error",QString(PQresultErrorMessage(result)),QMessageBox::Ok);
  return false;
    }
  
    return true;
}

/**
 * Check to see if GEOS is available
 */
bool QgsPostgresProvider::hasGEOS(PGconn *connection){
  // make sure info is up to date for the current connection
  postgisVersion(connection);
  // get geos capability
  return geosAvailable;
}
/* Functions for determining available features in postGIS */
QString QgsPostgresProvider::postgisVersion(PGconn *connection){
  PGresult *result = PQexec(connection, "select postgis_version()");
  postgisVersionInfo = PQgetvalue(result,0,0);
#ifdef QGISDEBUG
  std::cerr << "PostGIS version info: " << postgisVersionInfo << std::endl;
#endif
  // assume no capabilities
  geosAvailable = false;
  gistAvailable = false;
  projAvailable = false;
  // parse out the capabilities and store them
  QStringList postgisParts = QStringList::split(" ", postgisVersionInfo);
  QStringList geos = postgisParts.grep("GEOS");
  if(geos.size() == 1){
    geosAvailable = (geos[0].find("=1") > -1);  
  }
  QStringList gist = postgisParts.grep("STATS");
  if(gist.size() == 1){
    gistAvailable = (geos[0].find("=1") > -1);
  }
  QStringList proj = postgisParts.grep("PROJ");
  if(proj.size() == 1){
    projAvailable = (proj[0].find("=1") > -1);
  }
  return postgisVersionInfo;
}

bool QgsPostgresProvider::addFeatures(std::list<QgsFeature*> const flist)
{
    bool returnvalue=true;
    for(std::list<QgsFeature*>::const_iterator it=flist.begin();it!=flist.end();++it)
    {
  if(!addFeature(*it))
  {
      returnvalue=false;
  }
    }
    return returnvalue;
}

bool QgsPostgresProvider::deleteFeatures(std::list<int> const & id)
{
    bool returnvalue=true; 
    for(std::list<int>::const_iterator it=id.begin();it!=id.end();++it)
    {
  if(!deleteFeature(*it))
  {
      returnvalue=false;
  }
    }
    return returnvalue;
}

bool QgsPostgresProvider::addAttributes(std::list<QString> const & name, std::list<QString> const & type)
{
    bool returnvalue=true;
    std::list<QString>::const_iterator itert=type.begin();
    for(std::list<QString>::const_iterator itern=name.begin();itern!=name.end();++itern)
    {
	QString sql="ALTER TABLE "+tableName+" ADD COLUMN "+(*itern)+" "+(*itert);
	//send sql statement and do error handling
	PGresult* result=PQexec(connection, (const char *)sql);
	if(result==0)
	{
	    returnvalue=false;
	    ExecStatusType message=PQresultStatus(result);
	    if(message==PGRES_FATAL_ERROR)
	    {
		QMessageBox::information(0,"ALTER TABLE error",QString(PQresultErrorMessage(result)),QMessageBox::Ok);
	    } 
	}
	++itert;
    }
    return returnvalue;
}

bool QgsPostgresProvider::deleteAttributes(std::list<QString> const & name)
{
    bool returnvalue=true;
    for(std::list<QString>::const_iterator iter=name.begin();iter!=name.end();++iter)
    {
	QString sql="ALTER TABLE "+tableName+" DROP COLUMN "+(*iter);
	//send sql statement and do error handling
	PGresult* result=PQexec(connection, (const char *)sql);
	if(result==0)
	{
	    returnvalue=false;
	    ExecStatusType message=PQresultStatus(result);
	    if(message==PGRES_FATAL_ERROR)
	    {
		QMessageBox::information(0,"ALTER TABLE error",QString(PQresultErrorMessage(result)),QMessageBox::Ok);
	    }
	}
    }
    return returnvalue;
}

bool QgsPostgresProvider::changeAttributeValues(std::map<int,std::map<QString,QString> > const & attr_map)
{
    bool returnvalue=true;
    for(std::map<int,std::map<QString,QString> >::const_iterator iter=attr_map.begin();iter!=attr_map.end();++iter)
    {
	for(std::map<QString,QString>::const_iterator siter=(*iter).second.begin();siter!=(*iter).second.end();++siter)
	{
	    //TODO: collect several attribute changes for a feature in one statement (possibly more efficient)
	    QString sql="UPDATE "+tableName+" SET "+(*siter).first+" = "+(*siter).second+" WHERE " +primaryKey+" = "+QString::number((*iter).first);
            //send sql statement and do error handling
	    PGresult* result=PQexec(connection, (const char *)sql);
	    if(result==0)
	    {
		returnvalue=false;
		ExecStatusType message=PQresultStatus(result);
		if(message==PGRES_FATAL_ERROR)
		{
		    QMessageBox::information(0,"UPDATE error",QString(PQresultErrorMessage(result)),QMessageBox::Ok);
		}
	    }
	}
    }
    return returnvalue;
}

bool QgsPostgresProvider::supportsSaveAsShapefile() const
{
  return false;
}
void QgsPostgresProvider::setSubsetString(QString theSQL)
{
  sqlWhereClause=theSQL;
  // need to recalculate the number of features...
  getFeatureCount();
  calculateExtents();
  
}
long QgsPostgresProvider::getFeatureCount()
{
    // get total number of features
  QString sql = "select count(*) from " + tableName;
  if(sqlWhereClause.length() > 0)
  {
    sql += " where " + sqlWhereClause;
  }
  PGresult *result = PQexec(connection, (const char *) sql);
  numberFeatures = QString(PQgetvalue(result, 0, 0)).toLong();
        //--std::cout << "Feature count is " << numberFeatures << std::endl;
  PQclear(result);
  return numberFeatures;
  }
  void QgsPostgresProvider::calculateExtents()
  {
           // get the extents

    QString sql = "select extent(" + geometryColumn + ") from " + tableName;
    if(sqlWhereClause.length() > 0)
    {
      sql += " where " + sqlWhereClause;
    }

#if WASTE_TIME
        sql = "select xmax(extent(" + geometryColumn + ")) as xmax,"
          "xmin(extent(" + geometryColumn + ")) as xmin,"
          "ymax(extent(" + geometryColumn + ")) as ymax," "ymin(extent(" + geometryColumn + ")) as ymin" " from " + tableName;
#endif

#ifdef QGISDEBUG 
        qDebug("+++++++++QgsPostgresProvider::calculateExtents -  Getting extents using schema.table: " + sql);
#endif
        PGresult *result = PQexec(connection, (const char *) sql);
        std::string box3d = PQgetvalue(result, 0, 0);
        std::string s;

        box3d = box3d.substr(box3d.find_first_of("(")+1);
        box3d = box3d.substr(box3d.find_first_not_of(" "));
        s = box3d.substr(0, box3d.find_first_of(" "));
        double minx = strtod(s.c_str(), NULL);

        box3d = box3d.substr(box3d.find_first_of(" ")+1);
        s = box3d.substr(0, box3d.find_first_of(" "));
        double miny = strtod(s.c_str(), NULL);

        box3d = box3d.substr(box3d.find_first_of(",")+1);
        box3d = box3d.substr(box3d.find_first_not_of(" "));
        s = box3d.substr(0, box3d.find_first_of(" "));
        double maxx = strtod(s.c_str(), NULL);

        box3d = box3d.substr(box3d.find_first_of(" ")+1);
        s = box3d.substr(0, box3d.find_first_of(" "));
        double maxy = strtod(s.c_str(), NULL);

        layerExtent.setXmax(maxx);
        layerExtent.setXmin(minx);
        layerExtent.setYmax(maxy);
        layerExtent.setYmin(miny);
        QString xMsg;
        QTextOStream(&xMsg).precision(18);
        QTextOStream(&xMsg).width(18);
        QTextOStream(&xMsg) << "Set extents to: " << layerExtent.
            xMin() << ", " << layerExtent.yMin() << " " << layerExtent.xMax() << ", " << layerExtent.yMax();
#ifdef QGISDEBUG
        std::cerr << xMsg << std::endl;
#endif
        // clear query result
        PQclear(result);
      
    }
    
/**
 * Class factory to return a pointer to a newly created 
 * QgsPostgresProvider object
 */
QGISEXTERN QgsPostgresProvider * classFactory(const char *uri)
{
  return new QgsPostgresProvider(uri);
}
/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey(){
  return QString("postgres");
}
/**
 * Required description function 
 */
QGISEXTERN QString description(){
  return QString("PostgreSQL/PostGIS data provider");
} 
/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider(){
  return true;
}

