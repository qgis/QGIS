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
#include <qobject.h>
#include <qregexp.h> 

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

#include "../../src/qgsprovidercountcalcevent.h"
#include "../../src/qgsproviderextentcalcevent.h"

#include "qgspostgresprovider.h"

#include "qgspostgrescountthread.h"
#include "qgspostgresextentthread.h"

#include "qgspostgisbox3d.h"

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
  std::cout << "Data source uri is " << uri << std::endl;
  
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
  mSchema = "";
  if (tableName.find(".") > -1) {
    mSchema = tableName.left(tableName.find("."));
  }
  geometryColumn = tableName.mid(tableName.find(" (") + 2);
  geometryColumn.truncate(geometryColumn.length() - 1);
  tableName = tableName.mid(tableName.find(".") + 1, tableName.find(" (") - (tableName.find(".") + 1)); 
 
  /* populate the uri structure */
  mUri.schema = mSchema;
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
  std::cerr << "Schema is: " + mSchema << std::endl;
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

    PQexec(pd,(const char *)QString("set search_path = '%1','public'").arg(mSchema));
    // store the connection for future use
    connection = pd;

    if (getGeometryDetails()) // gets srid and geometry type
    {
      deduceEndian();
      calculateExtents();
      getFeatureCount();

      // selectSQL stores the select sql statement. This has to include 
      // each attribute plus the geometry column in binary form
      selectSQL = "select ";
      // Populate the field vector for this layer. The field vector contains
      // field name, type, length, and precision (if numeric)
      QString sql = "select * from " + tableName + " limit 1";
      PGresult* result = PQexec(pd, (const char *) sql);
      //--std::cout << "Field: Name, Type, Size, Modifier:" << std::endl;
      for (int i = 0; i < PQnfields(result); i++)
      {
	QString fieldName = PQfname(result, i);
	int fldtyp = PQftype(result, i);
	QString typOid = QString().setNum(fldtyp);
	int fieldModifier = PQfmod(result, i);

	sql = "select typelem from pg_type where typelem = " + typOid + " and typlen = -1";
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
	if(i > 0)
	  selectSQL += ", ";

	if (fieldType == "geometry")
	  selectSQL += "asbinary(" + geometryColumn + ",'" + endianString() + "') as qgs_feature_geometry";
	else
	  selectSQL += fieldName;
      }
      PQclear(result);

      // set the primary key
      getPrimaryKey();
      selectSQL += " from " + tableName;
      //--std::cout << "selectSQL: " << (const char *)selectSQL << std::endl;
      
      // Kick off the long running threads

      std::cout << "QgsPostgresProvider: About to touch mExtentThread" << std::endl;
      mExtentThread.setConnInfo( connInfo );
      mExtentThread.setTableName( tableName );
      mExtentThread.setSqlWhereClause( sqlWhereClause );
      mExtentThread.setGeometryColumn( geometryColumn );
      mExtentThread.setCallback( this );
      std::cout << "QgsPostgresProvider: About to start mExtentThread" << std::endl;
      mExtentThread.start();
      std::cout << "QgsPostgresProvider: Main thread just dispatched mExtentThread" << std::endl;

      std::cout << "QgsPostgresProvider: About to touch mCountThread" << std::endl;
      mCountThread.setConnInfo( connInfo );
      mCountThread.setTableName( tableName );
      mCountThread.setSqlWhereClause( sqlWhereClause );
      mCountThread.setGeometryColumn( geometryColumn );
      mCountThread.setCallback( this );
      std::cout << "QgsPostgresProvider: About to start mCountThread" << std::endl;
      mCountThread.start();
      std::cout << "QgsPostgresProvider: Main thread just dispatched mCountThread" << std::endl;
      
    } else {
      // the table is not a geometry table
      numberFeatures = 0;
      valid = false;
#ifdef QGISDEBUG
      std::cerr << "Invalid Postgres layer" << std::endl;
#endif
    }

    // reset tableName to include schema
    schemaTableName += mSchema + "." + tableName;

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
  
  //fill type names into lists
  mNumericalTypes.push_back("double precision");
  mNumericalTypes.push_back("int4");
  mNumericalTypes.push_back("int8");
  mNonNumericalTypes.push_back("text");
  mNonNumericalTypes.push_back("varchar(30)");

}

QgsPostgresProvider::~QgsPostgresProvider()
{

  std::cout << "QgsPostgresProvider: About to wait for mExtentThread" << std::endl;

  mExtentThread.wait();

  std::cout << "QgsPostgresProvider: Finished waiting for mExtentThread" << std::endl;

  std::cout << "QgsPostgresProvider: About to wait for mCountThread" << std::endl;

  mCountThread.wait();

  std::cout << "QgsPostgresProvider: Finished waiting for mCountThread" << std::endl;

  // Make sure all events from threads have been processed
  // (otherwise they will get destroyed prematurely)
  QApplication::sendPostedEvents(this, QGis::ProviderExtentCalcEvent);
  QApplication::sendPostedEvents(this, QGis::ProviderCountCalcEvent);

  PQfinish(connection);
  
  std::cout << "QgsPostgresProvider: deconstructing." << std::endl;

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

    f = new QgsFeature(*noid);
    if (fetchAttributes)
      getFeatureAttributes(*noid, f);
     
    int returnedLength = PQgetlength(queryResult,0, PQfnumber(queryResult,"qgs_feature_geometry"));
    //--std::cout << "Returned length is " << returnedLength << std::endl;
    if(returnedLength > 0){
      unsigned char *feature = new unsigned char[returnedLength + 1];
      memset(feature, '\0', returnedLength + 1);
      memcpy(feature, PQgetvalue(queryResult,0,PQfnumber(queryResult,"qgs_feature_geometry")), returnedLength);
#ifdef QGISDEBUG
      // a bit verbose
      //int wkbType = *((int *) (feature + 1));
      //std::cout << "WKBtype is: " << wkbType << std::endl;
#endif

      f->setGeometry(feature, returnedLength + 1);
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
        // XXX I'm assuming swapping from big-endian, or network, byte order to little endian
#ifdef QGISDEBUG
//XXX TOO MUCH OUTPUT!!!	  qWarning("swapping endian for oid");
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
  
    
    f = new QgsFeature(*noid);    
    if(!attlist.empty())
    {
      getFeatureAttributes(*noid, f, attlist);
    } 

    int returnedLength = PQgetlength(queryResult,0, PQfnumber(queryResult,"qgs_feature_geometry")); 
    if(returnedLength > 0)
    {
      unsigned char *feature = new unsigned char[returnedLength + 1];
      memset(feature, '\0', returnedLength + 1);
      memcpy(feature, PQgetvalue(queryResult,0,PQfnumber(queryResult,"qgs_feature_geometry")), returnedLength); 
#ifdef QGISDEBUG
      // Too verbose
      //int wkbType = *((int *) (feature + 1));
      //std::cout << "WKBtype is: " << wkbType << std::endl;
#endif

      f->setGeometry(feature, returnedLength + 1);

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
//    declare += " where intersects(" + geometryColumn;
//    declare += ", GeometryFromText('BOX3D(" + rect->stringRep();
//    declare += ")'::box3d,";
//    declare += srid;
//    declare += "))";
    
    // Contributed by #qgis irc "creeping"
    // This version actually invokes PostGIS's use of spatial indexes
    declare += " where " + geometryColumn;
    declare += " && GeometryFromText('BOX3D(" + rect->stringRep();
    declare += ")'::box3d,";
    declare += srid;
    declare += ")";
    declare += " and intersects(" + geometryColumn;
    declare += ", GeometryFromText('BOX3D(" + rect->stringRep();
    declare += ")'::box3d,";
    declare += srid;
    declare += "))";
  }else{
    declare += " where " + geometryColumn;
    declare += " && setsrid('BOX3D(" + rect->stringRep();
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

void QgsPostgresProvider::setExtent( QgsRect* newExtent )
{
  layerExtent.setXmax( newExtent->xMax() );
  layerExtent.setXmin( newExtent->xMin() );
  layerExtent.setYmax( newExtent->yMax() );
  layerExtent.setYmin( newExtent->yMin() );
}

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

QString QgsPostgresProvider::getPrimaryKey()
{
  // check to see if there is a primary key on the relation
  /*
     Process to determine the fields used in a primary key for a table:

     test=# select indkey from pg_index where indisprimary = 't' 
            and indrelid = 
            (select oid from pg_class where relname = 'earthquakes';
     indkey
     --------
     1 5
     (1 row)

     Primary key is composed of fields 1 and 5
  */
  QString sql = "select indkey from pg_index where indisprimary = 't' and "
    "indrelid = (select oid from pg_class where relname = '"
    + tableName + "')";

#ifdef QGISDEBUG
  std::cerr << "Getting primary key using '" << sql << "'" << std::endl;
#endif

  PGresult *pk = PQexec(connection,(const char *)sql);

#ifdef QGISDEBUG
  std::cout << "Got " << PQntuples(pk) << " rows." << std::endl;
#endif

  // if we got no tuples we ain't go no pk :)
  if( PQntuples(pk) == 0 )
  {
#ifdef QGISDEBUG
    std::cout << "Relation has no primary key -- "
	      << "investigating alternatives" << std::endl;
#endif

    // Two options here. If the relation is a table, see if there is
    // an oid column that can be used instead.
    // If the relation is a view try to find a suitable column to use as
    // the primary key.

    primaryKey = "";

    sql = "select relkind from pg_class where relname = '" 
      + tableName + "'";
    PGresult* tableType = PQexec(connection, (const char*) sql);
    QString type = PQgetvalue(tableType, 0, 0);
    PQclear(tableType);

    if (type == "r") // the relation is a table
    {
#ifdef QGISDEBUG
      std::cerr << "Relation is a table. Checking to see if it has an "
		<< "oid column.\n";
#endif
      sql = "select oid from " + tableName + " limit 1";
      PGresult* oidPresent = PQexec(connection, (const char*)sql);

      if (PQntuples(oidPresent) == 0)
      {
	valid = false;
	QApplication::restoreOverrideCursor();
	QMessageBox::warning(0, QObject::tr("No oid column in table"),
	    QObject::tr("The table has no primary key nor oid column. \n"
		  "Qgis requires that the table either has a primary key \n"
		  "or has a column containing the PostgreSQL oid.\n"
		  "For better performance the column should be indexed\n"));
	QApplication::setOverrideCursor(Qt::waitCursor);
      }
      else
      {
	// Could warn the user here that performance will suffer if
	// oid isn't indexed (and that they may want to add a
	// primary key to the table)
	primaryKey = "oid";
      }
      PQclear(oidPresent);
    }
    else if (type = "v") // the relation is a view
    {
      // Have a poke around the view to see if any of the columns
      // could be used as the primary key.

      // Get the select statement that defines the view
      sql = "select definition from pg_views where viewname = '" 
	+ tableName + "'";
      PGresult* def = PQexec(connection, (const char*)sql);
      if (PQntuples(def) == 0)
	qDebug("View " + tableName + " is not a view!");
      QString viewDef = PQgetvalue(def, 0, 0);
      PQclear(def);

      table_cols cols;
      // Find columns in the view that have unique constraints on the
      // underlying table columns 
      findTableColumns(viewDef, cols);
      // and choose one of them
      primaryKey = chooseViewColumn(cols);

      if (primaryKey.isEmpty())
      {
	valid = false;
	QApplication::restoreOverrideCursor();
	QMessageBox::warning(0, QObject::tr("No 'primary key' column in view"),
	    QObject::tr("The view has no column suitable for use as a "
	       "primary key.\n"
	       "Qgis requires that the view have a column that can be\n"
	       "used as a primary key. It should be of type int4 and contain\n"
	       "a unique value for each row of the table.\n"
               "For better performance the column should be derived\n"
	       "from an indexed column"));
	QApplication::setOverrideCursor(Qt::waitCursor);
      }
    }
    else
      qWarning("Unexpected relation type of '" + type + "'.");
  }
  else
  {
    // store the key column
    QString keyString = PQgetvalue(pk,0,0);
    QStringList columns = QStringList::split(" ", keyString);
    if (columns.count() > 1)
    {
      valid = false;
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(0, QObject::tr("No primary key column in table"),
	     QObject::tr("The table has a primary key that is composed of \n"
			 "more than one column. Qgis does not currently \n"
			 "support this."));
	QApplication::setOverrideCursor(Qt::waitCursor);
      //TODO concatenated key -- can we use this?
#ifdef QGISDEBUG
      std::cerr << "Table has a concatenated primary key" << std::endl;
#endif
    }
    int primaryKeyIndex = attributeFieldsIdMap[columns[0].toInt()];
    QgsField fld = attributeFields[primaryKeyIndex];
    // if the primary key is 4-byte integer we use it
    if (fld.type() == "int4")
    {
      primaryKey = fld.name();
      primaryKeyType = fld.type();
    }
    else
    {
      // key is not a 4-byte int -- use the oid instead
      primaryKey = "oid";
    }
  }
  PQclear(pk);

#ifdef QGISDEBUG
  std::cout << "Primary key is " << primaryKey << std::endl;
#endif

  return primaryKey;
}

QString QgsPostgresProvider::chooseViewColumn(const table_cols& cols)
{
  // For each relation name and column name need to see if it
  // has unique constraints on it, or is a primary key (if not,
  // it shouldn't be used). Should then be left with one or more
  // entries in the map which can be used as the key.

  QString sql, key;
  table_cols suitable;

  std::vector<table_cols::const_iterator> oids;
  table_cols::const_iterator iter = cols.begin();
  for (; iter != cols.end(); ++iter)
  {
    QString view_col   = iter->first;
    QString table_name = iter->second.first;
    QString table_col  = iter->second.second;
    // This sql returns one or more rows if the column 'table_col' in 
    // table 'table_name' has one or more columns that satisfy the
    // following conditions:
    // 1) the column has data type of int4.
    // 2) the column has a unique constraint or primary key constraint
    //    on it.
    // 3) the constraint applies just to the column of interest (i.e.,
    //    it isn't a constraint over multiple columns.
    sql = "select * from pg_constraint where conkey[1] = "
      "(select attnum from pg_attribute where attname = '"
      + table_col + "' and attrelid = (select oid from "
      "pg_class where relname = '" + table_name + "') and "
      "atttypid = (select oid from pg_type where typname = 'int4')) and "
      "conrelid = (select oid from pg_class where relname = '" +
      table_name + "') and (contype = 'p' or contype = 'u') "
      " and array_dims(conkey) = '[1:1]'";

#ifdef QGISDEBUG
    std::cout << "Column " << view_col << " from " 
	      << table_name << "." << table_col;
#endif
    PGresult* result = PQexec(connection, (const char*)sql);
    if (PQntuples(result) == 1)
      suitable[view_col] = iter->second;

#ifdef QGISDEBUG
    if (PQntuples(result) == 1)
      std::cout << " is suitable.\n";
    else
      std::cout << " is not suitable.\n";
#endif
    PQclear(result);
    if (table_col == "oid")
      oids.push_back(iter);
  }

  // 'oid' columns in tables don't have a constraint on them, but
  // they are useful to consider, so add them in if not already
  // here.
  for (int i = 0; i < oids.size(); ++i)
  {
    if (suitable.find(oids[i]->first) == suitable.end())
    {
      suitable[oids[i]->first] = oids[i]->second;
#ifdef QGISDEBUG
      std::cout << "Adding column " << oids[i]->first 
		<< " as it may be suitable." << std::endl;
#endif
    }
  }

  // If there is more than one suitable column pick one that is
  // indexed, else pick one called 'oid' if it exists, else
  // pick the first one. If there are none we return an empty string. 
  if (suitable.size() == 1)
  {
    key = suitable.begin()->first;
#ifdef QGISDEBUG
    std::cout << "Picked column " << key
	      << " as it is the only one that was suitable.\n";
#endif
  }
  else if (suitable.size() > 1)
  {
    // Search for one with an index
    table_cols::const_iterator i = suitable.begin();
    for (; i != suitable.end(); ++i)
    {
      sql = "select * from pg_index where indrelid = (select oid "
	"from pg_class where relname = '" + i->second.first + 
	"') and indkey[0] = "
	"(select attnum from pg_attribute where attrelid = "
	"(select oid from pg_class where relname = '" +
	i->second.first + "') and attname = '" + 
	i->second.second + "')";
      PGresult* result = PQexec(connection, (const char*)sql);

      if (PQntuples(result) > 0)
      { // Got one. Use it.
	key = i->first;
#ifdef QGISDEBUG
	std::cout << "Picked column '" << key
		  << "' because it has an index.\n";
#endif
	break;
      }
      PQclear(result);
    }

    if (key.isEmpty())
    {
      // If none have indices, choose one that is called 'oid' (if it
      // exists). This is legacy support and could be removed in
      // future. 
      i = suitable.find("oid");
      if (i != suitable.end())
      {
	key = i->first;
#ifdef QGISDEBUG
	std::cout << "Picked column " << key
		  << " as it is probably the postgresql object id "
		  << " column (which contains unique values) and there are no"
		  << " columns with indices to choose from\n.";
#endif
      }
      else // else choose the first one in the container
      {
	key = suitable.begin()->first;
#ifdef QGISDEBUG
	std::cout << "Picked column " << key
		  << " as it was the first suitable column found"
		  << " and there are no"
		  << " columns with indices to choose from\n.";
#endif
      }
    }
  }
  // XXX Remove when findTableColumns() is enhanced.
  //
  // Temporary hack to choose a column called 'oid' to
  // allow for none of the above working due to the
  // findTableColumns() function currently not detecting
  // underlying tables if a view refers to a second view.

  if (key == "" && cols.find("oid") != cols.end())
    key = "oid";

  return key;
}


void QgsPostgresProvider::findTableColumns(QString select_cmd, 
					   table_cols& cols)
{
  //table_cols& temp_cols;
  findColumns(select_cmd, cols);

  // Loop over the columns

  // Now see if the found relations are views. If so, call
  // ourselves until there are no views left.

}
// Finds out the underlying tables and columns for each column in the
// view defined by the given select statement.

// XXX need to make this recursive to deal with views that refer to
// views, etc, to get the table at the bottom of each view column.

void QgsPostgresProvider::findColumns(QString select_cmd, 
					   table_cols& cols)
{
  QRegExp col_regexp("SELECT *(.+) *FROM");
  col_regexp.setCaseSensitive(false);

  if (col_regexp.search(select_cmd) > -1)
  {
#ifdef QGISDEBUG
    std::cout << "Column definitions are: " << col_regexp.cap(1) << std::endl;
#endif
    // Now split the columns definitions on commas to get at actual
    // column definitions
    QStringList fields = QStringList::split(QRegExp(" *, *"),
					    col_regexp.cap(1));

    for (QStringList::iterator i = fields.begin(); i != fields.end(); ++i)
    {
      QString f = (*i).simplifyWhiteSpace();

      QString view_col_name, table_col_name, table_name;
      // if the view column name is renamed, there will be the
      // 'AS' command, so split on that if present.
      QRegExp rename_regexp("(.+) AS (.+)");
      if (rename_regexp.search(f) > -1)
      {
	view_col_name = rename_regexp.cap(2);
	table_col_name = rename_regexp.cap(1);
      }
      else
	table_col_name = f;

      // Now extract the table name from the column
      // name. It appears that the sql that we get from
      // postgresql always has the full form of 'relation.column'
      if (table_col_name.contains('.'))
      {
	QStringList tt = QStringList::split('.', table_col_name);
	table_name = tt[0];
	table_col_name = tt[1];
      }
      else
	qWarning("The view column definition '" + table_col_name +
		 "' is not in the full form.");

      // If there was no 'AS', the view column name is the same as
      // the column name from the underlying table.
      if (view_col_name == "")
	view_col_name = table_col_name;

#ifdef QGISDEBUG
      std::cout << "View column '" << view_col_name << "' comes from " 
		<< table_name << "." << table_col_name << std::endl;
#endif
      // If there are braces () in the table_col_name this probably
      // indicates that some sql function is being used to transform
      // the underlying column. This is probably not
      // suitable so exclude such columns from the result.
      if (!view_col_name.contains('('))
	cols[view_col_name] = 
	  std::make_pair<QString, QString>(table_name, table_col_name);
      else
      {
#ifdef QGISDEBUG
	std::cout << "View column '" << view_col_name << "' contains "
	  "an open bracket in the definition which probably means that "
	  "a postgreSQL function is being used to derive the column and "
	  "hence that it is unsuitable for use as a key into the "
	  "table." << std::endl;
#endif
      }
    }
  }
  else
    std::cerr << "Couldn't extract view column definitions from '"
	      << select_cmd << "'." << std::endl;
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

bool QgsPostgresProvider::addAttributes(std::map<QString,QString> const & name)
{
    bool returnvalue=true;
    PQexec(connection,"BEGIN");
    for(std::map<QString,QString>::const_iterator iter=name.begin();iter!=name.end();++iter)
    {
	QString sql="ALTER TABLE "+tableName+" ADD COLUMN "+(*iter).first+" "+(*iter).second;
#ifdef QGISDEBUG
	qWarning(sql);
#endif
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
    PQexec(connection,"COMMIT");
    reset();
    return returnvalue;
}

bool QgsPostgresProvider::deleteAttributes(std::set<QString> const & name)
{
    bool returnvalue=true;
    PQexec(connection,"BEGIN");
    for(std::set<QString>::const_iterator iter=name.begin();iter!=name.end();++iter)
    {
	QString sql="ALTER TABLE "+tableName+" DROP COLUMN "+(*iter);
#ifdef QGISDEBUG
	qWarning(sql);
#endif
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
	else
	{
	    //delete the attribute from attributeFields
	    for(std::vector<QgsField>::iterator it=attributeFields.begin();it!=attributeFields.end();++it)
	    {
		if((*it).name()==(*iter))
		{
		    attributeFields.erase(it);
		    break;
		}
	    }
	}
    }
    PQexec(connection,"COMMIT");
    reset();
    return returnvalue;
}

bool QgsPostgresProvider::changeAttributeValues(std::map<int,std::map<QString,QString> > const & attr_map)
{
    bool returnvalue=true; 
    PQexec(connection,"BEGIN");

    for(std::map<int,std::map<QString,QString> >::const_iterator iter=attr_map.begin();iter!=attr_map.end();++iter)
    {
	for(std::map<QString,QString>::const_iterator siter=(*iter).second.begin();siter!=(*iter).second.end();++siter)
	{
	    QString value=(*siter).second;
	    
            //find out, if value contains letters and quote if yes
	    bool text=false;
	    for(int i=0;i<value.length();++i)
	    {
		if(value[i].isLetter())
		{
		    text=true;
		}
	    }
	    if(text)
	    {
		value.prepend("'");
		value.append("'");
	    }

	    QString sql="UPDATE "+tableName+" SET "+(*siter).first+"="+value+" WHERE " +primaryKey+"="+QString::number((*iter).first);
#ifdef QGISDEBUG
	    qWarning(sql);
#endif

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
    PQexec(connection,"COMMIT");
    reset();
    return returnvalue;
}

bool QgsPostgresProvider::supportsSaveAsShapefile() const
{
  return false;
}

int QgsPostgresProvider::capabilities() const
{
    return ( QgsVectorDataProvider::AddFeatures | 
	     QgsVectorDataProvider::DeleteFeatures |
	     QgsVectorDataProvider::ChangeAttributeValues |
	     QgsVectorDataProvider::AddAttributes |
	     QgsVectorDataProvider::DeleteAttributes );
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
  
  // First get an approximate count; then delegate to
  // a thread the task of getting the full count.
  
  QString sql = "select reltuples from pg_catalog.pg_class where relname = '" + 
                 tableName + "'";
  
  std::cerr << "QgsPostgresProvider: Running SQL: " << 
        sql << std::endl;        
                 
  //QString sql = "select count(*) from " + tableName;
  
  //if(sqlWhereClause.length() > 0)
  //{
  //  sql += " where " + sqlWhereClause;
  //}
  
  PGresult *result = PQexec(connection, (const char *) sql);
  
#ifdef QGISDEBUG
      std::cerr << "QgsPostgresProvider: Approximate Number of features as text: " << 
        PQgetvalue(result, 0, 0) << std::endl;
#endif

  numberFeatures = QString(PQgetvalue(result, 0, 0)).toLong();
  PQclear(result);

#ifdef QGISDEBUG
      std::cerr << "QgsPostgresProvider: Approximate Number of features: " << 
        numberFeatures << std::endl;
#endif

  return numberFeatures;
}

// TODO: use the estimateExtents procedure of PostGIS and PostgreSQL 8
// This tip thanks to #qgis irc nick "creeping"
void QgsPostgresProvider::calculateExtents()
{
  // get the approximate extent by retreiving the bounding box
  // of the first few items with a geometry

  QString sql = "select box3d(" + geometryColumn + ") from " + tableName + 
                " where ";
                
  if(sqlWhereClause.length() > 0)
  {
    sql += "(" + sqlWhereClause + ") and ";
  }

  sql += "not IsEmpty(" + geometryColumn + ") limit 5";

#if WASTE_TIME
  sql = "select xmax(extent(" + geometryColumn + ")) as xmax,"
    "xmin(extent(" + geometryColumn + ")) as xmin,"
    "ymax(extent(" + geometryColumn + ")) as ymax," "ymin(extent(" + geometryColumn + ")) as ymin" " from " + tableName;
#endif

#ifdef QGISDEBUG 
  qDebug("QgsPostgresProvider::calculateExtents - Getting approximate extent using: '" + sql + "'");
#endif
  PGresult *result = PQexec(connection, (const char *) sql);
  
  // TODO: Guard against the result having no rows
  
  for (int i = 0; i < PQntuples(result); i++)
  {
    std::string box3d = PQgetvalue(result, i, 0);
  
    if (0 == i)
    {
      // create the initial extent
      layerExtent = QgsPostGisBox3d(box3d);
    }
    else
    {
      // extend the initial extent
      QgsPostGisBox3d b = QgsPostGisBox3d(box3d);
      
      layerExtent.combineExtentWith( &b );
    }
  
    std::cout << "QgsPostgresProvider: After row " << i << ", extent is: " 
          << layerExtent.xMin() << ", " << layerExtent.yMin() <<
      " " << layerExtent.xMax() << ", " << layerExtent.yMax() << std::endl;
  
  }

#ifdef QGISDEBUG
  QString xMsg;
  QTextOStream(&xMsg).precision(18);
  QTextOStream(&xMsg).width(18);
  QTextOStream(&xMsg) << "QgsPostgresProvider: Set extents to: " << layerExtent.
    xMin() << ", " << layerExtent.yMin() << " " << layerExtent.xMax() << ", " << layerExtent.yMax();
  std::cerr << xMsg << std::endl;
#endif

  std::cout << "QgsPostgresProvider: Set limit 5 extents to: " 
        << layerExtent.xMin() << ", " << layerExtent.yMin() <<
    " " << layerExtent.xMax() << ", " << layerExtent.yMax() << std::endl;

  // clear query result
  PQclear(result);

}

/**
 * Event sink for events from threads
 */
void QgsPostgresProvider::customEvent( QCustomEvent * e )
{
  std::cout << "QgsPostgresProvider: received a custom event " << e->type() << std::endl;
    
  switch ( e->type() )
  {
    case (QEvent::Type) QGis::ProviderExtentCalcEvent:
        
        std::cout << "QgsPostgresProvider: extent has been calculated" << std::endl;
        
        // Collect the new extent from the event and set this layer's
        // extent with it.
        
        setExtent( (QgsRect*) e->data() );
        
       
        std::cout << "QgsPostgresProvider: new extent has been saved" << std::endl;
          
        std::cout << "QgsPostgresProvider: Set extent to: " 
              << layerExtent.xMin() << ", " << layerExtent.yMin() <<
          " " << layerExtent.xMax() << ", " << layerExtent.yMax() << std::endl;

        std::cout << "QgsPostgresProvider: emitting fullExtentCalculated()" << std::endl;
        
        emit fullExtentCalculated();

        // TODO: Only uncomment this when the overview map canvas has been subclassed
        // from the QgsMapCanvas
        
//        std::cout << "QgsPostgresProvider: emitting repaintRequested()" << std::endl;
//        emit repaintRequested();
        
        break;
  
    case (QEvent::Type) QGis::ProviderCountCalcEvent:
  
        std::cout << "QgsPostgresProvider: count has been calculated" << std::endl;
        
        QgsProviderCountCalcEvent* e1 = (QgsProviderCountCalcEvent*) e;
        
        long numberFeatures = e1->numberFeatures();
        
        std::cout << "QgsPostgresProvider: count is " << numberFeatures << std::endl;
        
        break;
  }
    
  std::cout << "QgsPostgresProvider: Finished processing custom event " << e->type() << std::endl;
    
}


bool QgsPostgresProvider::deduceEndian()
{
  // need to store the PostgreSQL endian format used in binary cursors
  // since it appears that starting with
  // version 7.4, binary cursors return data in XDR whereas previous versions
  // return data in the endian of the server

  QString firstOid = "select oid from " + tableName + " limit 1";
  PGresult * oidResult = PQexec(connection, firstOid);
  // get the int value from a "normal" select
  QString oidValue = PQgetvalue(oidResult,0,0);

#ifdef QGISDEBUG
  std::cerr << "Creating binary cursor" << std::endl;
#endif

  // get the same value using a binary cursor

  PQexec(connection,"begin work");
  QString oidDeclare = QString("declare oidcursor binary cursor for select oid from %1 where oid = %2").arg(tableName).arg(oidValue);
  // set up the cursor
  PQexec(connection, (const char *)oidDeclare);
  QString fetch = "fetch forward 1 from oidcursor";

#ifdef QGISDEBUG
  std::cerr << "Fetching a record and attempting to get check endian-ness" << std::endl;
#endif

  PGresult *fResult = PQexec(connection, (const char *)fetch);
  PQexec(connection, "end work");
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
  }
  return swapEndian;
}

bool QgsPostgresProvider::getGeometryDetails()
{
  QString sql = "select f_geometry_column,type,srid from geometry_columns"
    " where f_table_name='" + tableName + "' and f_geometry_column = '" + 
    geometryColumn + "' and f_table_schema = '" + mSchema + "'";

#ifdef QGISDEBUG
  std::cerr << "Getting geometry column: " + sql << std::endl;
#endif

  QString fType;

  valid = false;

  PGresult *result = PQexec(connection, (const char *) sql);

  if (PQntuples(result) > 0)
  {
    valid = true;
#ifdef QGISDEBUG
    std::cout << "geometry column query returned " 
	      << PQntuples(result) << std::endl;
    std::cout << "column number of srid is " 
	      << PQfnumber(result, "srid") << std::endl;
#endif
    srid = PQgetvalue(result, 0, PQfnumber(result, "srid"));

    fType = PQgetvalue(result, 0, PQfnumber(result, "type"));
    if (fType == "POINT" || fType == "MULTIPOINT")
      geomType = QGis::WKBPoint;
    else if (fType == "LINESTRING" || fType == "MULTILINESTRING")
      geomType = QGis::WKBLineString;
    else if (fType == "POLYGON" || fType == "MULTIPOLYGON")
      geomType = QGis::WKBPolygon;
    PQclear(result);
  }
  else
  {
    // Didn't find what we need in the geometry_columns table, so
    // get stuff from the relevant column instead. This may (will?) 
    // fail if there is no data in the relevant table.
    PQclear(result);

    sql = "select "
      "srid("         + geometryColumn + "), "
      "geometrytype(" + geometryColumn + ") from " + 
      tableName + " limit 1";

    result = PQexec(connection, (const char*) sql);

    if (PQntuples(result) > 0)
    {
      valid = true;
      srid = PQgetvalue(result, 0, PQfnumber(result, "srid"));
      fType = PQgetvalue(result, 0, PQfnumber(result, "geometrytype"));
      if (fType == "POINT" || fType == "MULTIPOINT")
	geomType = QGis::WKBPoint;
      else if (fType == "LINESTRING" || fType == "MULTILINESTRING")
	geomType = QGis::WKBLineString;
      else if (fType == "POLYGON" || fType == "MULTIPOLYGON")
	geomType = QGis::WKBPolygon;
    }
    PQclear(result);
  }

#ifdef QGISDEBUG
  std::cout << "SRID is " << srid << '\n'
	    << "type is " << fType << '\n'
	    << "Feature type is " << geomType << '\n'
	    << "Feature type name is " 
	    << QGis::qgisFeatureTypes[geomType] << std::endl;
#endif
  return valid;
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

