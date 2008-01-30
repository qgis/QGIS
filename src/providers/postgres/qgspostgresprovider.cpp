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

// for htonl
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include <fstream>
#include <iostream>
#include <cassert>

#include <QStringList>
#include <QApplication>
#include <QEvent>
#include <QCustomEvent>
#include <QTextOStream>


#include <qgis.h>
#include <qgsapplication.h>
#include <qgsfeature.h>
#include <qgsfield.h>
#include <qgsgeometry.h>
#include <qgsmessageoutput.h>
#include <qgsrect.h>
#include <qgsspatialrefsys.h>

#include "qgsprovidercountcalcevent.h"
#include "qgsproviderextentcalcevent.h"

#include "qgspostgresprovider.h"

#include "qgspostgrescountthread.h"
#include "qgspostgresextentthread.h"

#include "qgspostgisbox3d.h"
#include "qgslogger.h"

const QString POSTGRES_KEY = "postgres";
const QString POSTGRES_DESCRIPTION = "PostgreSQL/PostGIS data provider";
int QgsPostgresProvider::providerIds = 0;

  QgsPostgresProvider::QgsPostgresProvider(QString const & uri)
: QgsVectorDataProvider(uri),
  geomType(QGis::WKBUnknown),
  mFeatureQueueSize(200),
  gotPostgisVersion(FALSE)
{

  providerId = QString("%1").arg(providerIds++);

  // assume this is a valid layer until we determine otherwise
  valid = true;
  /* OPEN LOG FILE */

  // Make connection to the data source
  // For postgres, the connection information is passed as a space delimited
  // string:
  //  host=192.168.1.5 dbname=test port=5342 user=gsherman password=xxx table=tablename

  QgsDebugMsg("Postgresql Layer Creation");
  QgsDebugMsg("URI: " + uri);

  mUri = QgsDataSourceURI(uri);

  /* populate members from the uri structure */
  mSchemaName = mUri.schema();
  mTableName = mUri.table();
  geometryColumn = mUri.geometryColumn();
  sqlWhereClause = mUri.sql();

  // Keep a schema qualified table name for convenience later on.
  mSchemaTableName = mUri.quotedTablename();

  QgsDebugMsg("Table name is " + mTableName);
  QgsDebugMsg("SQL is " + sqlWhereClause);
  QgsDebugMsg("Connection info is " + mUri.connInfo() );

  QgsDebugMsg("Geometry column is: " + geometryColumn);
  QgsDebugMsg("Schema is: " + mSchemaName);
  QgsDebugMsg("Table name is: " + mTableName);

  //QString logFile = "./pg_provider_" + mTableName + ".log";
  //pLog.open((const char *)logFile);
  //QgsDebugMsg("Opened log file for " + mTableName);

  connection = connectDb( (const char *)mUri.connInfo() );
  if( connection==NULL ) {
    valid = false;
    return;
  }

  QgsDebugMsg("Checking for select permission on the relation\n");

  // Check that we can read from the table (i.e., we have
  // select permission).
  QString sql = "select * from " + mSchemaTableName + " limit 1";
  PGresult* testAccess = PQexec(connection, (const char*)(sql.utf8()));
  if (PQresultStatus(testAccess) != PGRES_TUPLES_OK)
  {
    showMessageBox(tr("Unable to access relation"),
        tr("Unable to access the ") + mSchemaTableName + 
        tr(" relation.\nThe error message from the database was:\n") +
        QString(PQresultErrorMessage(testAccess)) + ".\n" + 
        "SQL: " + sql);
    PQclear(testAccess);
    valid = false;
    disconnectDb();
    return;
  }
  PQclear(testAccess);

  PGresult *schema = PQexec(connection, "SELECT current_schema()");
  if (PQresultStatus(schema) == PGRES_TUPLES_OK)
  {
    mCurrentSchema = PQgetvalue(schema, 0, 0);
    if(mCurrentSchema==mSchemaName) {
      mUri.clearSchema();
      setDataSourceUri( mUri.uri() );
    }
  }
  PQclear(schema);

  if(mSchemaName=="")
    mSchemaName=mCurrentSchema;

  if (!getGeometryDetails()) // gets srid and geometry type
  {
    // the table is not a geometry table
    numberFeatures = 0;
    valid = false;

    QgsDebugMsg("Invalid Postgres layer");
    disconnectDb();
    return;
  }

  deduceEndian();
  calculateExtents();
  getFeatureCount();

  // load the field list
  loadFields();

  // set the primary key
  getPrimaryKey();

  // Set the postgresql message level so that we don't get the
  // 'there is no transaction in progress' warning.
#ifndef QGISDEBUG
  PQexec(connection, "set client_min_messages to error");
#endif

  // Kick off the long running threads

#ifdef POSTGRESQL_THREADS
  QgsDebugMsg("About to touch mExtentThread");
  mExtentThread.setConnInfo( mUri.connInfo );
  mExtentThread.setTableName( mTableName );
  mExtentThread.setSqlWhereClause( sqlWhereClause );
  mExtentThread.setGeometryColumn( geometryColumn );
  mExtentThread.setCallback( this );
  QgsDebugMsg("About to start mExtentThread");
  mExtentThread.start();
  QgsDebugMsg("Main thread just dispatched mExtentThread");

  QgsDebugMsg("About to touch mCountThread");
  mCountThread.setConnInfo( mUri.connInfo );
  mCountThread.setTableName( mTableName );
  mCountThread.setSqlWhereClause( sqlWhereClause );
  mCountThread.setGeometryColumn( geometryColumn );
  mCountThread.setCallback( this );
  QgsDebugMsg("About to start mCountThread");
  mCountThread.start();
  QgsDebugMsg("Main thread just dispatched mCountThread");
#endif

  ready = false; // not ready to read yet cuz the cursor hasn't been created

  //fill type names into sets
  mSupportedNativeTypes.insert("double precision");
  mSupportedNativeTypes.insert("int4");
  mSupportedNativeTypes.insert("int8");
  mSupportedNativeTypes.insert("text");
  mSupportedNativeTypes.insert("varchar(30)");

  if (primaryKey.isEmpty())
  {
    valid = false;
  }

  // Close the database connection if the layer isn't going to be loaded.
  if (!valid)
    disconnectDb();
}

QgsPostgresProvider::~QgsPostgresProvider()
{
#ifdef POSTGRESQL_THREADS
  QgsDebugMsg("About to wait for mExtentThread");

  mExtentThread.wait();

  QgsDebugMsg("Finished waiting for mExtentThread");

  QgsDebugMsg("About to wait for mCountThread");

  mCountThread.wait();

  QgsDebugMsg("Finished waiting for mCountThread");

  // Make sure all events from threads have been processed
  // (otherwise they will get destroyed prematurely)
  QApplication::sendPostedEvents(this, QGis::ProviderExtentCalcEvent);
  QApplication::sendPostedEvents(this, QGis::ProviderCountCalcEvent);
#endif

  disconnectDb();

  QgsDebugMsg("deconstructing.");

  //pLog.flush();
}

PGconn *QgsPostgresProvider::connectDb(const char *conninfo)
{
  if( connections.contains(conninfo) ) 
  {
    QgsDebugMsg(QString("Using cached connection for ") + conninfo);
    connections[conninfo]->ref++;
    return connections[conninfo]->conn;
  }

  QgsDebugMsg(QString("New postgres connection for ") + conninfo);

  PGconn *pd = PQconnectdb(conninfo);
  // check the connection status
  if (PQstatus(pd) != CONNECTION_OK) 
  {
    QgsDebugMsg("Connection to database failed");
    return NULL;
  }

  //set client encoding to unicode because QString uses UTF-8 anyway
  QgsDebugMsg("setting client encoding to UNICODE");

  int errcode=PQsetClientEncoding(pd, "UNICODE");

  if(errcode==0) 
  {
    QgsDebugMsg("encoding successfully set");
  } 
  else if(errcode==-1) 
  {
    QgsDebugMsg("error in setting encoding");
  } 
  else 
  {
    QgsDebugMsg("undefined return value from encoding setting");
  }

  /* Check to see if we have GEOS support and if not, warn the user about
     the problems they will see :) */
  QgsDebugMsg("Checking for GEOS support");

  if(!hasGEOS(pd))
  {
    showMessageBox(tr("No GEOS Support!"),
        tr("Your PostGIS installation has no GEOS support.\n"
          "Feature selection and identification will not "
          "work properly.\nPlease install PostGIS with " 
          "GEOS support (http://geos.refractions.net)"));
  }
  //--std::cout << "Connection to the database was successful\n";

  Conn *conn = new Conn(pd);
  connections.insert( conninfo, conn );

  return pd;
}

void QgsPostgresProvider::disconnectDb()
{
  QMapIterator <QString, Conn *> i(connections);
  while( i.hasNext() ) 
  {
    i.next();

    if( i.value()->conn == connection )
      break;
  }

  assert( i.value()->conn==connection );
  assert( i.value()->ref>0 );

  if( --i.value()->ref==0 ) 
  {
    PQfinish( i.value()->conn );
    delete (i.value());
    connections.remove( i.key() );
  }
}

QString QgsPostgresProvider::storageType()
{
  return "PostgreSQL database with PostGIS extension";
}

bool QgsPostgresProvider::getNextFeature(QgsFeature& feature)
{
  if (valid)
  {

    // Top up our queue if it is empty
    if (mFeatureQueue.empty())
    {
      QString fetch = QString("fetch forward %1 from qgisf" + providerId)
        .arg(mFeatureQueueSize);

      if(mFirstFetch)
      {
        if(PQsendQuery(connection, (const char *)fetch) == 0) //fetch features in asynchronously
        {
          qWarning("PQsendQuery failed (1)");
        }
      }
      mFirstFetch = false;
      queryResult = PQgetResult(connection);
      PQgetResult(connection); //just to get the 0 pointer...  

      int rows = PQntuples(queryResult);

      if (rows == 0)
      {
        QgsDebugMsg("End of features");

        if (ready)
          PQexec(connection, "end work");
        ready = false;
        return false;
      }

      for (int row = 0; row < rows; row++)
      {
        int oid = *(int *)PQgetvalue(queryResult, row, PQfnumber(queryResult,"\""+primaryKey+"\""));

        if (swapEndian)
          oid = ntohl(oid); // convert oid to opposite endian

        // set ID
        feature.setFeatureId(oid);

        // fetch attributes
        std::list<QString>::const_iterator name_it = mFetchAttributeNames.begin();
        QgsAttributeList::const_iterator index_it = mAttributesToFetch.constBegin();

        for(; name_it != mFetchAttributeNames.end(); ++name_it, ++index_it)
        {
          QString val;
          if( (*name_it) == primaryKey)
          {
            val = QString::number(oid);
          }
          else
          {
            char* attribute = PQgetvalue(queryResult, row, PQfnumber(queryResult,*name_it));
            val = QString::fromUtf8(attribute);
          }

          switch (attributeFields[*index_it].type())
          {
            case QVariant::Int:
              feature.addAttribute(*index_it, val.toInt());
              break;
            case QVariant::Double:
              feature.addAttribute(*index_it, val.toDouble());
              break;
            case QVariant::String:
              feature.addAttribute(*index_it, val);
              break;
            default:
              assert(0 && "unsupported field type");
          }
        }

        //fetch geometry
        if (mFetchGeom)
        {
          int returnedLength = PQgetlength(queryResult, row, PQfnumber(queryResult,"qgs_feature_geometry")); 
          if(returnedLength > 0)
          {
            unsigned char *featureGeom = new unsigned char[returnedLength + 1];
            memset(featureGeom, '\0', returnedLength + 1);
            memcpy(featureGeom, PQgetvalue(queryResult, row, PQfnumber(queryResult,"qgs_feature_geometry")), returnedLength); 
            feature.setGeometryAndOwnership(featureGeom, returnedLength + 1);
          }
          else
          {
            QgsDebugMsg("Couldn't get the feature geometry in binary form");
          }
        } 

        //don't copy the geometry. Just pass a pointer instead
        mFeatureQueue.push(QgsFeature());
        mFeatureQueue.back().setGeometry(feature.geometryAndOwnership());
        mFeatureQueue.back().setFeatureId(feature.featureId());
        mFeatureQueue.back().setAttributeMap(feature.attributeMap());

      } // for each row in queue

      PQclear(queryResult);

      if(PQsendQuery(connection, (const char *)fetch) == 0) //already fetch the next couple of features asynchronously
      {
        qWarning("PQsendQuery failed (2)");
      }

    } // if new queue is required

    // Now return the next feature from the queue
    //don't copy the geometry. Just pass a pointer instead
    feature.setGeometry(mFeatureQueue.front().geometryAndOwnership());
    feature.setFeatureId(mFeatureQueue.front().featureId());
    feature.setAttributeMap(mFeatureQueue.front().attributeMap());

    mFeatureQueue.pop();

  }
  else 
  {
    QgsDebugMsg("Read attempt on an invalid postgresql data source");
    return false;
  }

  return true;
}

void QgsPostgresProvider::select(QgsAttributeList fetchAttributes,
    QgsRect rect,
    bool fetchGeometry,
    bool useIntersect)
{
  mFetchGeom = fetchGeometry;
  mAttributesToFetch = fetchAttributes;

  mFetchAttributeNames.clear();
  QgsFieldMap attributeMap = fields();
  QgsFieldMap::const_iterator fieldIt;
  for(QgsAttributeList::const_iterator it = mAttributesToFetch.constBegin(); \
      it != mAttributesToFetch.constEnd(); ++it)
  {
    fieldIt = attributeMap.find(*it);
    if(fieldIt != attributeMap.end())
    {
      mFetchAttributeNames.push_back(fieldIt.value().name());
    }
  }

  QString declare = "declare qgisf" + providerId + " binary cursor for select \"" + primaryKey + "\"";

  if(fetchGeometry)
  {
    declare += QString(",asbinary(\"%1\",'%2') as qgs_feature_geometry").arg(geometryColumn).arg(endianString()); 
  }
  for(std::list<QString>::const_iterator it = mFetchAttributeNames.begin(); it != mFetchAttributeNames.end(); ++it)
  {
    if( (*it) != primaryKey) //no need to fetch primary key again
    {
      declare += ",\"" + *it + "\"::text";
    }
  }

  declare += " ";
  declare += QString("from %1").arg(mSchemaTableName);

  QgsDebugMsg("Binary cursor: " + declare);

  bool hasWhere = FALSE;

  if(!rect.isEmpty())
  {
    if(useIntersect)
    { 
      // Contributed by #qgis irc "creeping"
      // This version actually invokes PostGIS's use of spatial indexes
      declare += " where " + geometryColumn;
      declare += " && setsrid('BOX3D(" + rect.asWKTCoords();
      declare += ")'::box3d,";
      declare += srid;
      declare += ")";
      declare += " and intersects(" + geometryColumn;
      declare += ", setsrid('BOX3D(" + rect.asWKTCoords();
      declare += ")'::box3d,";
      declare += srid;
      declare += "))";
    }
    else
    {
      declare += " where " + geometryColumn;
      declare += " && setsrid('BOX3D(" + rect.asWKTCoords();
      declare += ")'::box3d,";
      declare += srid;
      declare += ")";
    }
    hasWhere = TRUE;
  }

  if(sqlWhereClause.length() > 0)
  {
    if (hasWhere)
      declare += " and ";
    else
      declare += " where ";
    declare += "(" + sqlWhereClause + ")";
    hasWhere = TRUE;
  }

  QgsDebugMsg("Selecting features using: " + declare);

  // set up the cursor
  if(ready){
    PQexec(connection, "end work");
  }
  PQexec(connection,"begin work");
  ready = true;
  PQexec(connection, (const char *)(declare.utf8()));

  mFeatureQueue.empty();
  mFirstFetch = true;
}

bool QgsPostgresProvider::getFeatureAtId(int featureId,
    QgsFeature& feature,
    bool fetchGeometry,
    QgsAttributeList fetchAttributes)
{

  std::list<QString> attributeNames;
  QgsFieldMap fldMap = fields();
  QgsFieldMap::const_iterator fieldIt;
  QgsAttributeList::const_iterator it;
  std::list<QString>::const_iterator namesIt;

  for (it = fetchAttributes.constBegin(); it != fetchAttributes.constEnd(); ++it)
  {
    fieldIt = fldMap.find(*it);
    if(fieldIt != fldMap.end())
    {
      attributeNames.push_back(fieldIt.value().name());
    }
  }

  QString sql = "declare qgisfid" + providerId + " binary cursor for select \"" + primaryKey + "\"";

  if(fetchGeometry)
  {
    sql += QString(",asbinary(\"%1\",'%2') as qgs_feature_geometry").arg(geometryColumn).arg(endianString()); 
  }
  for(namesIt = attributeNames.begin(); namesIt != attributeNames.end(); ++namesIt)
  {
    if( (*namesIt) != primaryKey) //no need to fetch primary key again
    {
      sql += ",\"" + *namesIt + "\"::text";
    }
  }

  sql += " " + QString("from %1").arg(mSchemaTableName);

  sql += " where " + primaryKey + " = " + QString::number(featureId);

  QgsDebugMsg("Selecting feature using: " + sql);

  PQexec(connection,"begin work");

  // execute query
  PQexec(connection, (const char *)(sql.utf8()));

  PGresult *res = PQexec(connection, "fetch forward 1 from qgisfid" + providerId);

  int rows = PQntuples(res);
  if (rows == 0)
  {
    PQexec(connection, "end work");
    QgsDebugMsg("feature " + QString::number(featureId) + " not found");
    return FALSE;
  }

  // set ID
  int oid = *(int *)PQgetvalue(res, 0, PQfnumber(res,"\""+primaryKey+"\""));
  if (swapEndian)
    oid = ntohl(oid); // convert oid to opposite endian
  feature.setFeatureId(oid);

  // fetch attributes
  it = fetchAttributes.constBegin();

  for(namesIt = attributeNames.begin(); namesIt != attributeNames.end(); ++namesIt, ++it)
  {
    QString val;
    if( (*namesIt) == primaryKey)
    {
      val = QString::number(oid);
    }
    else
    {
      char* attribute = PQgetvalue(res, 0, PQfnumber(res,*namesIt));
      val = QString::fromUtf8(attribute);
    }

    switch (attributeFields[*it].type())
    {
      case QVariant::Int:
        feature.addAttribute(*it, val.toInt());
        break;
      case QVariant::Double:
        feature.addAttribute(*it, val.toDouble());
        break;
      case QVariant::String:
        feature.addAttribute(*it, val);
        break;
      default:
        assert(0 && "unsupported field type");
    }
  }

  // fetch geometry
  if (fetchGeometry)
  {
    int returnedLength = PQgetlength(res, 0, PQfnumber(res,"qgs_feature_geometry")); 
    if(returnedLength > 0)
    {
      unsigned char *featureGeom = new unsigned char[returnedLength + 1];
      memset(featureGeom, '\0', returnedLength + 1);
      memcpy(featureGeom, PQgetvalue(res, 0, PQfnumber(res,"qgs_feature_geometry")), returnedLength); 
      feature.setGeometryAndOwnership(featureGeom, returnedLength + 1);
    }
  }

  PQexec(connection, "end work");

  return TRUE;
}

QgsDataSourceURI& QgsPostgresProvider::getURI()
{
  return mUri;
}

void QgsPostgresProvider::setExtent( QgsRect& newExtent )
{
  layerExtent.setXmax( newExtent.xMax() );
  layerExtent.setXmin( newExtent.xMin() );
  layerExtent.setYmax( newExtent.yMax() );
  layerExtent.setYmin( newExtent.yMin() );
}

// TODO - make this function return the real extent_
QgsRect QgsPostgresProvider::extent()
{
  return layerExtent;      //extent_->MinX, extent_->MinY, extent_->MaxX, extent_->MaxY);
}

/** 
 * Return the feature type
 */
QGis::WKBTYPE QgsPostgresProvider::geometryType() const
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
uint QgsPostgresProvider::fieldCount() const
{
  return attributeFields.size();
}

const QgsFieldMap & QgsPostgresProvider::fields() const
{
  return attributeFields;
}

QString QgsPostgresProvider::dataComment() const
{
  return mDataComment;
}

void QgsPostgresProvider::reset()
{
  QString move = "move 0 in qgisf" + providerId; //move cursor to first record
  PQexec(connection, (const char *)(move.utf8()));
  mFeatureQueue.empty();
  loadFields();
}

/** @todo XXX Perhaps this should be promoted to QgsDataProvider? */
QString QgsPostgresProvider::endianString()
{
  switch ( QgsApplication::endian() )
  {
    case QgsApplication::NDR : 
      return QString("NDR");
      break;
    case QgsApplication::XDR : 
      return QString("XDR");
      break;
    default :
      return QString("UNKNOWN");
  }
}

void QgsPostgresProvider::loadFields()
{
  QgsDebugMsg("Loading fields for table " + mTableName);

  // Get the relation oid for use in later queries
  QString sql = "SELECT oid FROM pg_class WHERE relname = '" + mTableName + "' AND relnamespace = ("
    "SELECT oid FROM pg_namespace WHERE nspname = '" + mSchemaName + "')";
  PGresult *tresult= PQexec(connection, (const char *)(sql.utf8()));
  QString tableoid = PQgetvalue(tresult, 0, 0);
  PQclear(tresult);

  // Get the table description
  sql = "SELECT description FROM pg_description WHERE "
    "objoid = " + tableoid + " AND objsubid = 0";
  tresult = PQexec(connection, (const char*) sql.utf8());
  if (PQntuples(tresult) > 0)
    mDataComment = PQgetvalue(tresult, 0, 0);
  PQclear(tresult);

  // Populate the field vector for this layer. The field vector contains
  // field name, type, length, and precision (if numeric)
  sql = "select * from " + mSchemaTableName + " limit 0";

  PGresult *result = PQexec(connection, (const char *) (sql.utf8()));
  //--std::cout << "Field: Name, Type, Size, Modifier:" << std::endl;

  // The queries inside this loop could possibly be combined into one
  // single query - this would make the code run faster.

  for (int i = 0; i < PQnfields(result); i++)
  {
    QString fieldName = PQfname(result, i);
    int fldtyp = PQftype(result, i);
    QString typOid = QString().setNum(fldtyp);
    int fieldModifier = PQfmod(result, i);
    QString fieldComment("");

    sql = "SELECT typname, typlen FROM pg_type WHERE "
      "oid = (SELECT typelem FROM pg_type WHERE "
      "typelem = " + typOid + " AND typlen = -1)";

    PGresult* oidResult = PQexec(connection, (const char *) sql);
    QString fieldTypeName = PQgetvalue(oidResult, 0, 0);
    QString fieldSize = PQgetvalue(oidResult, 0, 1);
    PQclear(oidResult);

    sql = "SELECT attnum FROM pg_attribute WHERE "
      "attrelid = " + tableoid + " AND attname = '" + fieldName + "'";
    PGresult *tresult = PQexec(connection, (const char *)(sql.utf8()));
    QString attnum = PQgetvalue(tresult, 0, 0);
    PQclear(tresult);

    sql = "SELECT description FROM pg_description WHERE "
      "objoid = " + tableoid + " AND objsubid = " + attnum;
    tresult = PQexec(connection, (const char*)(sql.utf8()));
    if (PQntuples(tresult) > 0)
      fieldComment = PQgetvalue(tresult, 0, 0);
    PQclear(tresult);

    QgsDebugMsg("Field: " + attnum + " maps to " + QString::number(i) + " " + fieldName + ", " 
        + fieldTypeName + " (" + QString::number(fldtyp) + "),  " + fieldSize + ", " + QString::number(fieldModifier));

    if(fieldName!=geometryColumn)
    {
      QVariant::Type fieldType;
      if (fieldTypeName.find("int") != -1 || fieldTypeName.find("serial") != -1)
        fieldType = QVariant::Int;
      else if (fieldTypeName == "real" || fieldTypeName == "double precision" || \
          fieldTypeName.find("float") != -1)
        fieldType = QVariant::Double;
      else
        fieldType = QVariant::String;
      attributeFields.insert(i, QgsField(fieldName, fieldType, fieldTypeName, fieldSize.toInt(), fieldModifier, fieldComment));
    }
  }
  PQclear(result);
}

QString QgsPostgresProvider::getPrimaryKey()
{
  // check to see if there is an unique index on the relation, which
  // can be used as a key into the table. Primary keys are always
  // unique indices, so we catch them as well.

  QString sql = "select indkey from pg_index where indisunique = 't' and "
    "indrelid = (select oid from pg_class where relname = '"
    + mTableName + "' and relnamespace = (select oid from pg_namespace where "
    "nspname = '" + mSchemaName + "'))";

  QgsDebugMsg("Getting unique index using '" + sql + "'");

  PGresult *pk = executeDbCommand(connection, sql);

  QgsDebugMsg("Got " + QString::number(PQntuples(pk)) + " rows.");

  QStringList log;

  // if we got no tuples we ain't got no unique index :)
  if (PQntuples(pk) == 0)
  {
    QgsDebugMsg("Relation has no unique index -- investigating alternatives");

    // Two options here. If the relation is a table, see if there is
    // an oid column that can be used instead.
    // If the relation is a view try to find a suitable column to use as
    // the primary key.

    sql = "select relkind from pg_class where relname = '" + mTableName + 
      "' and relnamespace = (select oid from pg_namespace where "
      "nspname = '" + mSchemaName + "')";
    PGresult* tableType = executeDbCommand(connection, sql);
    QString type = PQgetvalue(tableType, 0, 0);
    PQclear(tableType);

    primaryKey = "";

    if (type == "r") // the relation is a table
    {
      QgsDebugMsg("Relation is a table. Checking to see if it has an oid column.");

      // If there is an oid on the table, use that instead,
      // otherwise give up
      sql = "select attname from pg_attribute where attname = 'oid' and "
        "attrelid = (select oid from pg_class where relname = '" +
        mTableName + "' and relnamespace = (select oid from pg_namespace "
        "where nspname = '" + mSchemaName + "'))";
      PGresult* oidCheck = executeDbCommand(connection, sql);

      if (PQntuples(oidCheck) != 0)
      {
        // Could warn the user here that performance will suffer if
        // oid isn't indexed (and that they may want to add a
        // primary key to the table)
        primaryKey = "oid";
        primaryKeyType = "int4";
      }
      else
      {
        showMessageBox(tr("No suitable key column in table"),
            tr("The table has no column suitable for use as a key.\n\n"
              "Qgis requires that the table either has a column of type\n"
              "int4 with a unique constraint on it (which includes the\n"
              "primary key) or has a PostgreSQL oid column.\n"));
      }
      PQclear(oidCheck);
    }
    else if (type == "v") // the relation is a view
    {
      // Have a poke around the view to see if any of the columns
      // could be used as the primary key.
      tableCols cols;
      // Given a schema.view, populate the cols variable with the
      // schema.table.column's that underly the view columns.
      findColumns(cols);
      // From the view columns, choose one for which the underlying
      // column is suitable for use as a key into the view.
      primaryKey = chooseViewColumn(cols);
    }
    else
      QgsDebugMsg("Unexpected relation type of '" + type + "'.");
  }
  else // have some unique indices on the table. Now choose one...
  {
    // choose which (if more than one) unique index to use
    std::vector<std::pair<QString, QString> > suitableKeyColumns;
    for (int i = 0; i < PQntuples(pk); ++i)
    {
      QString col = PQgetvalue(pk, i, 0);
      QStringList columns = QStringList::split(" ", col);
      if (columns.count() == 1)
      {
        // Get the column name and data type
        sql = "select attname, pg_type.typname from pg_attribute, pg_type where "
          "atttypid = pg_type.oid and attnum = " +
          col + " and attrelid = (select oid from pg_class where " +
          "relname = '" + mTableName + "' and relnamespace = (select oid "
          "from pg_namespace where nspname = '" + mSchemaName + "'))";
        PGresult* types = executeDbCommand(connection, sql);

        if( PQntuples(types) > 0 )
        {
          QString columnName = PQgetvalue(types, 0, 0);
          QString columnType = PQgetvalue(types, 0, 1);

          if (columnType != "int4")
            log.append(tr("The unique index on column") + 
                " '" + columnName + "' " +
                tr("is unsuitable because Qgis does not currently support"
                  " non-int4 type columns as a key into the table.\n"));
          else
            suitableKeyColumns.push_back(std::make_pair(columnName, columnType));
        }
        else
        {
          //QgsDebugMsg( QString("name and type of %3. column of %1.%2 not found").arg(mSchemaName).arg(mTables).arg(col) );
        }

        PQclear(types);
      }
      else
      {
        sql = "select attname from pg_attribute, pg_type where "
          "atttypid = pg_type.oid and attnum in (" +
          col.replace(" ", ",") 
          + ") and attrelid = (select oid from pg_class where " +
          "relname = '" + mTableName + "' and relnamespace = (select oid "
          "from pg_namespace where nspname = '" + mSchemaName + "'))";
        PGresult* types = executeDbCommand(connection, sql);
        QString colNames;
        int numCols = PQntuples(types);
        for (int j = 0; j < numCols; ++j)
        {
          if (j == numCols-1)
            colNames += tr("and ");
          colNames += "'" + QString(PQgetvalue(types, j, 0)) 
            + (j < numCols-2 ? "', " : "' ");
        }

        log.append(tr("The unique index based on columns ") + colNames + 
            tr(" is unsuitable because Qgis does not currently support"
              " multiple columns as a key into the table.\n"));
      }
    }

    // suitableKeyColumns now contains the name of columns (and their
    // data type) that
    // are suitable for use as a key into the table. If there is
    // more than one we need to choose one. For the moment, just
    // choose the first in the list.

    if (suitableKeyColumns.size() > 0)
    {
      primaryKey = suitableKeyColumns[0].first;
      primaryKeyType = suitableKeyColumns[0].second;
    }
    else
    {
      // If there is an oid on the table, use that instead,
      // otherwise give up
      sql = "select attname from pg_attribute where attname = 'oid' and "
        "attrelid = (select oid from pg_class where relname = '" +
        mTableName + "' and relnamespace = (select oid from pg_namespace "
        "where nspname = '" + mSchemaName + "'))";
      PGresult* oidCheck = executeDbCommand(connection, sql);

      if (PQntuples(oidCheck) != 0)
      {
        primaryKey = "oid";
        primaryKeyType = "int4";
      }
      else
      {
        log.prepend("There were no columns in the table that were suitable "
            "as a qgis key into the table (either a column with a "
            "unique index and type int4 or a PostgreSQL oid column.\n");
      }
      PQclear(oidCheck);
    }

    // Either primaryKey has been set by the above code, or it
    // hasn't. If not, present some info to the user to give them some
    // idea of why not.
    if (primaryKey.isEmpty())
    {
      // Give some info to the user about why things didn't work out.
      valid = false;
      showMessageBox(tr("Unable to find a key column"), log);
    }
  }
  PQclear(pk);

  if (primaryKey.length() > 0) {
    QgsDebugMsg("Qgis row key is " + primaryKey);
  } else {
    QgsDebugMsg("Qgis row key was not set.");
  }

  return primaryKey;
}

// Given the table and column that each column in the view refers to,
// choose one. Prefers column with an index on them, but will
// otherwise choose something suitable.

QString QgsPostgresProvider::chooseViewColumn(const tableCols& cols)
{
  // For each relation name and column name need to see if it
  // has unique constraints on it, or is a primary key (if not,
  // it shouldn't be used). Should then be left with one or more
  // entries in the map which can be used as the key.

  QString sql, key;
  QStringList log;
  tableCols suitable;
  // Cache of relation oid's
  std::map<QString, QString> relOid;

  std::vector<tableCols::const_iterator> oids;
  tableCols::const_iterator iter = cols.begin();
  for (; iter != cols.end(); ++iter)
  {
    QString viewCol   = iter->first;
    QString schemaName = iter->second.schema;
    QString tableName = iter->second.relation;
    QString tableCol  = iter->second.column;
    QString colType   = iter->second.type;

    // Get the oid from pg_class for the given schema.relation for use
    // in subsequent queries.
    sql = "select oid from pg_class where relname = '" + tableName +
      "' and relnamespace = (select oid from pg_namespace where "
      " nspname = '" + schemaName + "')";
    PGresult* result = PQexec(connection, (const char*)(sql.utf8()));
    QString rel_oid;
    if (PQntuples(result) == 1)
    {
      rel_oid = PQgetvalue(result, 0, 0);
      // Keep the rel_oid for use later one.
      relOid[viewCol] = rel_oid;
    }
    else
    {
      QgsDebugMsg("Relation " + schemaName + "." + tableName +
          " doesn't exist in the pg_class table."
          "This shouldn't happen and is odd.");
      assert(0);
    }
    PQclear(result);

    // This sql returns one or more rows if the column 'tableCol' in 
    // table 'tableName' and schema 'schemaName' has one or more
    // columns that satisfy the following conditions:
    // 1) the column has data type of int4.
    // 2) the column has a unique constraint or primary key constraint
    //    on it.
    // 3) the constraint applies just to the column of interest (i.e.,
    //    it isn't a constraint over multiple columns.
    sql = "select * from pg_constraint where conkey[1] = "
      "(select attnum from pg_attribute where attname = '" + tableCol + "' "
      "and attrelid = " + rel_oid + ")"
      "and conrelid = " + rel_oid + " "
      "and (contype = 'p' or contype = 'u') "
      "and array_dims(conkey) = '[1:1]'";

    result = PQexec(connection, (const char*)(sql.utf8()));
    if (PQntuples(result) == 1 && colType == "int4")
      suitable[viewCol] = iter->second;

    QString details = "'" + viewCol + "'" + tr(" derives from ") 
      + "'" + schemaName + "." + tableName + "." + tableCol + "' ";

    if (PQntuples(result) == 1 && colType == "int4")
    {
      details += tr("and is suitable.");
    }
    else
    {
      details += tr("and is not suitable ");
      details += "(" + tr("type is ") + colType;
      if (PQntuples(result) == 1)
        details += tr(" and has a suitable constraint)");
      else
        details += tr(" and does not have a suitable constraint)");
    }

    log << details;

    PQclear(result);
    if (tableCol == "oid")
      oids.push_back(iter);
  }

  // 'oid' columns in tables don't have a constraint on them, but
  // they are useful to consider, so add them in if not already
  // here.
  for (uint i = 0; i < oids.size(); ++i)
  {
    if (suitable.find(oids[i]->first) == suitable.end())
    {
      suitable[oids[i]->first] = oids[i]->second;

      QgsDebugMsg("Adding column " + oids[i]->first + " as it may be suitable.");
    }
  }

  // Now have a map containing all of the columns in the view that
  // might be suitable for use as the key to the table. Need to choose
  // one thus:
  //
  // If there is more than one suitable column pick one that is
  // indexed, else pick one called 'oid' if it exists, else
  // pick the first one. If there are none we return an empty string. 

  // Search for one with an index
  tableCols::const_iterator i = suitable.begin();
  for (; i != suitable.end(); ++i)
  {
    // Get the relation oid from our cache.
    QString rel_oid = relOid[i->first];
    // And see if the column has an index
    sql = "select * from pg_index where indrelid = " + rel_oid +
      " and indkey[0] = (select attnum from pg_attribute where "
      "attrelid = " +	rel_oid + " and attname = '" + i->second.column + "')";
    PGresult* result = PQexec(connection, (const char*)(sql.utf8()));

    if (PQntuples(result) > 0 && uniqueData(mSchemaName, mTableName, i->first))
    { // Got one. Use it.
      key = i->first;

      QgsDebugMsg("Picked column '" + key + "' because it has an index.");
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
    if (i != suitable.end() && uniqueData(mSchemaName, mTableName, i->first))
    {
      key = i->first;

      QgsDebugMsg("Picked column " + key +
          " as it is probably the postgresql object id "
          " column (which contains unique values) and there are no"
          " columns with indices to choose from.");
    }
    // else choose the first one in the container that has unique data
    else
    {
      tableCols::const_iterator i = suitable.begin();
      for (; i != suitable.end(); ++i)
      {
        if (uniqueData(mSchemaName, mTableName, i->first))
        {
          key = i->first;

          QgsDebugMsg("Picked column " + key +
              " as it was the first suitable column found"
              " with unique data and were are no"
              " columns with indices to choose from");
          break;
        }
        else
        {
          log << QString(tr("Note: ") + "'" + i->first + "' "
              + tr("initially appeared suitable but does not "
                "contain unique data, so is not suitable.\n"));
        }
      }
    }
  }

  if (key.isEmpty())
  {
    valid = false;
    // Successive prepends means that the text appears in the dialog
    // box in the reverse order to that seen here.
    log.prepend(tr("The view you selected has the following columns, none "
          "of which satisfy the above conditions:"));
    log.prepend(tr("Qgis requires that the view has a column that can be used "
          "as a unique key. Such a column should be derived from "
          "a table column of type int4 and be a primary key, "
          "have a unique constraint on it, or be a PostgreSQL "
          "oid column. To improve "
          "performance the column should also be indexed.\n"));
    log.prepend(tr("The view ") + "'" + mSchemaName + '.' + mTableName + "' " +
        tr("has no column suitable for use as a unique key.\n"));
    showMessageBox(tr("No suitable key column in view"), log);
  }

  return key;
}

bool QgsPostgresProvider::uniqueData(QString schemaName, 
    QString tableName, QString colName)
{
  // Check to see if the given column contains unique data

  bool isUnique = false;

  QString sql = "select count(distinct \"" + colName + "\") = count(\"" +
    colName + "\") from \"" + schemaName + "\".\"" + tableName + "\"";

  PGresult* unique = PQexec(connection, (const char*) (sql.utf8()));

  if (PQntuples(unique) == 1)
    if (strncmp(PQgetvalue(unique, 0, 0),"t", 1) == 0)
      isUnique = true;

  PQclear(unique);

  return isUnique;
}

int QgsPostgresProvider::SRCFromViewColumn(const QString& ns, const QString& relname, const QString& attname_table, const QString& attname_view, const QString& viewDefinition, SRC& result) const
{
  QString newViewDefSql = "SELECT definition FROM pg_views WHERE schemaname = '" + ns + "' AND viewname = '" + relname + "'";
  PGresult* newViewDefResult = PQexec(connection, (const char*)(newViewDefSql.utf8()));
  int numEntries = PQntuples(newViewDefResult);

  if(numEntries > 0) //relation is a view
  {
    QString newViewDefinition(PQgetvalue(newViewDefResult, 0, 0));

    QString newAttNameView = attname_table;
    QString newAttNameTable = attname_table;

    //find out the attribute name of the underlying table/view
    if (newViewDefinition.contains("AS"))
    {
      QRegExp s("(\\w+) " + QString("AS ") + QRegExp::escape(attname_table));
      if (s.indexIn(newViewDefinition) != -1)
      {
        newAttNameTable = s.cap(1);
      }
    }

    QString viewColumnSql = "SELECT table_schema, table_name, column_name FROM information_schema.view_column_usage WHERE view_schema = '" + ns + "' AND view_name = '" + relname + "' AND column_name = '" + newAttNameTable +"'";
    PGresult* viewColumnResult = PQexec(connection, (const char*)(viewColumnSql.utf8()));
    if(PQntuples(viewColumnResult) > 0)
    {
      QString newTableSchema = PQgetvalue(viewColumnResult, 0, 0);
      QString newTableName = PQgetvalue(viewColumnResult, 0, 1);
      int retvalue = SRCFromViewColumn(newTableSchema, newTableName, newAttNameTable, newAttNameView, newViewDefinition, result);
      PQclear(viewColumnResult);
      return retvalue;
    }
    else
    {
      PQclear(viewColumnResult);
      return 1;
    }

  }

  PQclear(newViewDefResult);

  //relation is table, we just have to add the type
  QString typeSql = "SELECT pg_type.typname FROM pg_attribute, pg_class, pg_namespace, pg_type WHERE pg_class.relname = '" + relname + "' AND pg_namespace.nspname = '" + ns + "' AND pg_attribute.attname = '" + attname_table + "' AND pg_attribute.attrelid = pg_class.oid AND pg_class.relnamespace = pg_namespace.oid AND pg_attribute.atttypid = pg_type.oid";
  QgsDebugMsg("***********************************************************************************");
  QgsDebugMsg(typeSql);
  QgsDebugMsg("***********************************************************************************");
  PGresult* typeSqlResult = PQexec(connection, (const char*)(typeSql.utf8()));
  if(PQntuples(typeSqlResult) < 1)
  {
    return 1;
  }
  QString type = PQgetvalue(typeSqlResult, 0, 0);
  PQclear(typeSqlResult);

  result.schema=ns;
  result.relation=relname;
  result.column=attname_table;
  result.type=type;
  return 0;
}

// This function will return in the cols variable the 
// underlying view and columns for each column in
// mSchemaName.mTableName.

void QgsPostgresProvider::findColumns(tableCols& cols)
{
  QString viewColumnSql = "SELECT table_schema, table_name, column_name FROM information_schema.view_column_usage WHERE view_schema = '" + mSchemaName + "' AND view_name = '" + mTableName + "'";
  PGresult* viewColumnResult = PQexec(connection, (const char*)(viewColumnSql.utf8()));

  //find out view definition
  QString viewDefSql = "SELECT definition FROM pg_views WHERE schemaname = '" + mSchemaName + "' AND viewname = '" + mTableName + "'";
  PGresult* viewDefResult = PQexec(connection, (const char*)(viewDefSql.utf8()));
  if(PQntuples(viewDefResult) < 1)
  {
    PQclear(viewDefResult);
    return;
  }
  QString viewDefinition(PQgetvalue(viewDefResult, 0, 0));
  PQclear(viewDefResult);

  QString ns, relname, attname_table, attname_view;
  SRC columnInformation;

  for(int i = 0; i < PQntuples(viewColumnResult); ++i)
  {
    ns = PQgetvalue(viewColumnResult, i, 0);
    relname = PQgetvalue(viewColumnResult, i, 1);
    attname_table = PQgetvalue(viewColumnResult, i, 2);

    //find out original attribute name
    attname_view = attname_table;

    //examine if the column name has been renamed in the view with AS
    if (viewDefinition.contains("AS"))
    {
      // This regular expression needs more testing. Since the view
      // definition comes from postgresql and has been 'standardised', we
      // don't need to deal with everything that the user could put in a view
      // definition. Does the regexp have to deal with the schema??

      QRegExp s(".* \"?" + QRegExp::escape(relname) +
          "\"?\\.\"?" + QRegExp::escape(attname_table) +
          "\"? AS \"?(\\w+)\"?,* .*");

      QgsDebugMsg(viewDefinition + "\n" + s.pattern());

      if (s.indexIn(viewDefinition) != -1)
      {
        attname_view = s.cap(1);
        qWarning("original view column name was: " + attname_view);
      }
    }

    SRCFromViewColumn(ns, relname, attname_table, attname_view, viewDefinition, columnInformation);
    cols.insert(std::make_pair(attname_view, columnInformation));
    QgsDebugMsg("Inserting into cols (for key " + attname_view + " ): " + columnInformation.schema + "." + columnInformation.relation + "." + columnInformation.column + "." + columnInformation.type);
  }
  PQclear(viewColumnResult);

#if 0
  // This sql is derived from the one that defines the view
  // 'information_schema.view_column_usage' in PostgreSQL, with a few
  // mods to suit our purposes. 
  QString sql = ""
    "SELECT DISTINCT "
    "	nv.nspname AS view_schema, "
    "	v.relname AS view_name, "
    "	a.attname AS view_column_name, "
    "	nt.nspname AS table_schema, "
    "	t.relname AS table_name, "
    "	a.attname AS column_name, "
    "	t.relkind AS table_type, "
    "	typ.typname AS column_type, "
    "   vs.definition AS view_definition "
    "FROM "
    "	pg_namespace nv, "
    "	pg_class v, "
    "	pg_depend dv,"
    "	pg_depend dt, "
    "	pg_class t, "
    "	pg_namespace nt, "
    "	pg_attribute a,"
    "	pg_user u, "
    "	pg_type typ, "
    "   pg_views vs "
    "WHERE "
    "	nv.oid = v.relnamespace AND "
    "	v.relkind = 'v'::\"char\" AND "
    "	v.oid = dv.refobjid AND "
    "	dv.refclassid = 'pg_class'::regclass::oid AND "
    "	dv.classid = 'pg_rewrite'::regclass::oid AND "
    "	dv.deptype = 'i'::\"char\" AND "
    "	dv.objid = dt.objid AND "
    "	dv.refobjid <> dt.refobjid AND "
    "	dt.classid = 'pg_rewrite'::regclass::oid AND "
    "	dt.refclassid = 'pg_class'::regclass::oid AND "
    "	dt.refobjid = t.oid AND "
    "	t.relnamespace = nt.oid AND "
    "	(t.relkind = 'r'::\"char\" OR t.relkind = 'v'::\"char\") AND "
    "	t.oid = a.attrelid AND "
    "	dt.refobjsubid = a.attnum AND "
    "	nv.nspname NOT IN ('pg_catalog', 'information_schema' ) AND "
    "	a.atttypid = typ.oid AND "
    "   nv.nspname = vs.schemaname AND "
    "   v.relname = vs.viewname";

  // A structure to store the results of the above sql.
  typedef std::map<QString, TT> columnRelationsType;
  columnRelationsType columnRelations;

  // A structure to cache the query results that return the view 
  // definition. 
  typedef QMap<QString, QString> viewDefCache; 
  viewDefCache viewDefs; 

  PGresult* result = PQexec(connection, (const char*)(sql.utf8()));
  // Store the results of the query for convenient access 

  for (int i = 0; i < PQntuples(result); ++i)
  {
    TT temp;
    temp.view_schema      = PQgetvalue(result, i, 0);
    temp.view_name        = PQgetvalue(result, i, 1);
    temp.view_column_name = PQgetvalue(result, i, 2);
    temp.table_schema     = PQgetvalue(result, i, 3);
    temp.table_name       = PQgetvalue(result, i, 4);
    temp.column_name      = PQgetvalue(result, i, 5);
    temp.table_type       = PQgetvalue(result, i, 6);
    temp.column_type      = PQgetvalue(result, i, 7);
    QString viewDef       = PQgetvalue(result, i, 8);

    // BUT, the above SQL doesn't always give the correct value for the view 
    // column name (that's because that information isn't available directly 
    // from the database), mainly when the view column name has been renamed 
    // using 'AS'. To fix this we need to look in the view definition and 
    // adjust the view column name if necessary. 

    // Now pick the view definiton apart, looking for
    // temp.column_name to the left of an 'AS'.

    if (!viewDef.isEmpty())
    {
      // Compiling and executing the regexp for each row from the above query
      // can take quite a while - a database can easily have hundreds of
      // rows. Working on the premise that we are only doing this to catch the
      // cases where the view column has been renamed using the AS construct,
      // we'll check for that first before doing the potentially
      // time-consuming regular expression.

      if (viewDef.contains("AS"))
      {
        // This regular expression needs more testing. Since the view
        // definition comes from postgresql and has been 'standardised', we
        // don't need to deal with everything that the user could put in a view
        // definition. Does the regexp have to deal with the schema??

        QRegExp s(".* \"?" + QRegExp::escape(temp.table_name) +
            "\"?\\.\"?" + QRegExp::escape(temp.column_name) +
            "\"? AS \"?(\\w+)\"?,* .*");

        QgsDebugMsg(viewDef + "\n" + s.pattern());

        if (s.indexIn(viewDef) != -1)
        {
          temp.view_column_name = s.cap(1);
        }
      }
    }

    QgsDebugMsg(temp.view_schema + "." 
        + temp.view_name + "." 
        + temp.view_column_name + " <- " 
        + temp.table_schema + "." 
        + temp.table_name + "." 
        + temp.column_name + " is a '" 
        + temp.table_type + "' of type " 
        + temp.column_type);

    columnRelations[temp.view_schema + '.' +
      temp.view_name + '.' +
      temp.view_column_name] = temp;
  }
  PQclear(result);

  // Loop over all columns in the view in question. 

  sql = "SELECT pg_namespace.nspname || '.' || "
    "pg_class.relname || '.' || pg_attribute.attname "
    "FROM pg_attribute, pg_class, pg_namespace "
    "WHERE pg_class.relname = '" + mTableName + "' "
    "AND pg_namespace.nspname = '" + mSchemaName + "' "
    "AND pg_attribute.attrelid = pg_class.oid "
    "AND pg_class.relnamespace = pg_namespace.oid";

  result = PQexec(connection, (const char*)(sql.utf8()));

  // Loop over the columns in mSchemaName.mTableName and find out the
  // underlying schema, table, and column name.
  for (int i = 0; i < PQntuples(result); ++i)
  {
    columnRelationsType::const_iterator 
      ii = columnRelations.find(PQgetvalue(result, i, 0));
    columnRelationsType::const_iterator start_iter = ii;

    if (ii == columnRelations.end())
      continue;

    int count = 0;
    const int max_loops = 100;

    while (ii->second.table_type != "r" && count < max_loops)
    {
      QgsDebugMsg("Searching for the column that " +
          ii->second.table_schema + '.'+
          ii->second.table_name + "." +
          ii->second.column_name + " refers to.");

      columnRelationsType::const_iterator 
        jj = columnRelations.find(QString(ii->second.table_schema + '.' +
              ii->second.table_name + '.' +
              ii->second.column_name));

      if (jj == columnRelations.end())
      {
        QgsDebugMsg("WARNING: Failed to find the column that " +
            ii->second.table_schema + "." +
            ii->second.table_name + "." +
            ii->second.column_name + " refers to.");
        break;
      }

      ii = jj;
      ++count;
    }

    if (count >= max_loops)
    {
      QgsDebugMsg("Search for the underlying table.column for view column " +
          ii->second.table_schema + "." + 
          ii->second.table_name + "." +
          ii->second.column_name + " failed: exceeded maximum "
          "interation limit (" + QString::number(max_loops) + ")");

      cols[ii->second.view_column_name] = SRC("","","","");
    }
    else if (ii != columnRelations.end())
    {
      cols[start_iter->second.view_column_name] = 
        SRC(ii->second.table_schema, 
            ii->second.table_name, 
            ii->second.column_name, 
            ii->second.column_type);

      QgsDebugMsg( QString(PQgetvalue(result, i, 0)) + " derives from " +
          ii->second.table_schema + "." +
          ii->second.table_name + "." +
          ii->second.column_name);
    }
  }
  PQclear(result);
#endif //0
}

// Returns the minimum value of an attribute
QVariant QgsPostgresProvider::minValue(int index)
{
  // get the field name 
  QgsField fld = attributeFields[index];
  QString sql;
  if(sqlWhereClause.isEmpty())
  {
    sql = QString("select min(\"%1\") from %2").arg(fld.name()).arg(mSchemaTableName);
  }
  else
  {
    sql = QString("select min(\"%1\") from %2").arg(fld.name()).arg(mSchemaTableName)+" where "+sqlWhereClause;
  }
  PGresult *rmin = PQexec(connection,(const char *)(sql.utf8()));
  QString minValue = PQgetvalue(rmin,0,0);
  PQclear(rmin);
  return minValue.toDouble();
}

// Returns the maximum value of an attribute

QVariant QgsPostgresProvider::maxValue(int index)
{
  // get the field name 
  QgsField fld = attributeFields[index];
  QString sql;
  if(sqlWhereClause.isEmpty())
  {
    sql = QString("select max(\"%1\") from %2").arg(fld.name()).arg(mSchemaTableName);
  }
  else
  {
    sql = QString("select max(\"%1\") from %2").arg(fld.name()).arg(mSchemaTableName)+" where "+sqlWhereClause;
  } 
  PGresult *rmax = PQexec(connection,(const char *)(sql.utf8()));
  QString maxValue = PQgetvalue(rmax,0,0);
  PQclear(rmax);
  return maxValue.toDouble();
}


int QgsPostgresProvider::maxPrimaryKeyValue()
{
  QString sql;

  sql = QString("select max(\"%1\") from %2")
    .arg(primaryKey)
    .arg(mSchemaTableName);

  PGresult *rmax = PQexec(connection,(const char *)(sql.utf8()));
  QString maxValue = PQgetvalue(rmax,0,0);
  PQclear(rmax);

  return maxValue.toInt();
}


bool QgsPostgresProvider::isValid(){
  return valid;
}

bool QgsPostgresProvider::addFeature(QgsFeature& f, int primaryKeyHighWater)
{
  QgsDebugMsg("Entering.");

  // Determine which insertion method to use for WKB
  // PostGIS 1.0+ uses BYTEA
  // earlier versions use HEX
  bool useWkbHex(FALSE);

  if (!gotPostgisVersion)
  {
    postgisVersion(connection);
  }

  QgsDebugMsg("PostGIS version is  major: "  + QString::number(postgisVersionMajor) +
      ", minor: " + QString::number(postgisVersionMinor));

  if (postgisVersionMajor < 1)
  {
    useWkbHex = TRUE;
  }

  // Start building insert string
  QString insert("INSERT INTO ");
  insert+=mSchemaTableName;
  insert+=" (";

  // add the name of the geometry column to the insert statement
  insert += "\"" + geometryColumn;

  // add the name of the primary key column to the insert statement
  insert += "\",\"";
  insert += primaryKey + "\"";

  QgsDebugMsg("Constructing insert SQL, currently at: " + insert);

  //add the names of the other fields to the insert
  const QgsAttributeMap& attributevec = f.attributeMap();

  QgsDebugMsg("Got attribute map.");

  for(QgsAttributeMap::const_iterator it = attributevec.begin(); it != attributevec.end(); ++it)
  {
    QString fieldname;
    QgsFieldMap::const_iterator fit = attributeFields.find(it.key());
    if (fit != attributeFields.end())
      fieldname = fit->name();

    QgsDebugMsg("Checking field against: " + fieldname);

    if (
        (fieldname != "") &&
        (fieldname != geometryColumn) &&
        (fieldname != primaryKey) &&
        (!(it->isNull()))
       )
    {
      insert+=",\"";
      insert+=fieldname +"\"";
    }
  }

  insert+=") VALUES (GeomFromWKB('";

  // Add the WKB geometry to the INSERT statement
  QgsGeometry* geometry = f.geometry();
  unsigned char* geom = geometry->wkbBuffer();
  for (uint i=0; i < geometry->wkbSize(); ++i)
  {
    if (useWkbHex)
    {
      // PostGIS < 1.0 wants hex
      QString hex = QString::number((int) geom[i], 16).upper();

      if (hex.length() == 1)
      {
        hex = "0" + hex;
      }

      insert += hex;
    }
    else
    {
      // Postgis 1.0 wants bytea
      QString oct = QString::number((int) geom[i], 8);

      if(oct.length()==3)
      {
        oct="\\\\"+oct;
      }
      else if(oct.length()==1)
      {
        oct="\\\\00"+oct;
      }
      else if(oct.length()==2)
      {
        oct="\\\\0"+oct; 
      }

      insert += oct;
    }
  }

  if (useWkbHex)
  {
    insert += "',"+srid+")";
  }
  else
  {
    insert += "::bytea',"+srid+")";
  }

  //add the primary key value to the insert statement
  insert += ",";
  insert += QString::number(primaryKeyHighWater);

  //add the field values to the insert statement
  for(QgsAttributeMap::const_iterator it=attributevec.begin();it!=attributevec.end();++it)
  {
    QString fieldname;
    QgsFieldMap::const_iterator fit = attributeFields.find(it.key());
    if (fit != attributeFields.end())
      fieldname = fit->name();

    QgsDebugMsg("Checking field name " + fieldname);

    if (
        (fieldname != "") &&
        (fieldname != geometryColumn) &&
        (fieldname != primaryKey) &&
        (!(it->isNull()))
       )
    {
      QString fieldvalue = it->toString();
      bool charactertype=false;
      insert+=",";

      QgsDebugMsg("Field is in layer with value " + fieldvalue);

      //add quotes if the field is a character or date type and not
      //the postgres provided default value
      if(fieldvalue != "NULL" && fieldvalue != getDefaultValue(it.key()).toString() )
      {
        if(it->type() == QVariant::String || it->type() == QVariant::Char)
        {
          charactertype=true;
        }
      }

      // important: escape quotes in field value
      fieldvalue.replace("'", "''");

      //request default value explicitly if fieldvalue is an empty string
      if(fieldvalue.isEmpty())
      {
        insert += "DEFAULT";
      }
      else
      {
        // XXX isn't it better to always escape field value?
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
  }

  insert+=")";
  QgsDebugMsg("insert statement is: "+insert);
  qWarning("insert statement is: "+insert);

  //send INSERT statement and do error handling
  PGresult* result=PQexec(connection, (const char *)(insert.utf8()));
  if(result==0)
  {
    showMessageBox(tr("INSERT error"),tr("An error occured during feature insertion"));
    return false;
  }
  ExecStatusType message=PQresultStatus(result);
  if(message==PGRES_FATAL_ERROR)
  {
    showMessageBox(tr("INSERT error"),QString(PQresultErrorMessage(result)));
    return false;
  }

  QgsDebugMsg("Exiting with true.");
  return true;
}

QVariant QgsPostgresProvider::getDefaultValue(int fieldId)
{
  // Get the default column value from the Postgres information
  // schema. If there is no default we return an empty string.

  // Maintaining a cache of the results of this query would be quite
  // simple and if this query is called lots, could save some time.

  QString fieldName = attributeFields[fieldId].name();

  QString sql("SELECT column_default FROM "
      "information_schema.columns WHERE "
      "column_default IS NOT NULL AND "
      "table_schema = '" + mSchemaName + "' AND "
      "table_name = '" + mTableName + "' AND "
      "column_name = '" + fieldName + "'");

  QString defaultValue("");

  PGresult* result = PQexec(connection, (const char*)(sql.utf8()));

  if (PQntuples(result) == 1)
    defaultValue = PQgetvalue(result, 0, 0);

  PQclear(result);

  return defaultValue;
}

bool QgsPostgresProvider::deleteFeature(int id)
{
  QString sql("DELETE FROM "+mSchemaTableName+" WHERE \""+primaryKey+"\" = "+QString::number(id));

  QgsDebugMsg("delete sql: "+sql);

  //send DELETE statement and do error handling
  PGresult* result=PQexec(connection, (const char *)(sql.utf8()));
  if(result==0)
  {
    showMessageBox(tr("DELETE error"),tr("An error occured during deletion from disk"));
    return false;
  }
  ExecStatusType message=PQresultStatus(result);
  if(message==PGRES_FATAL_ERROR)
  {
    showMessageBox(tr("DELETE error"),QString(PQresultErrorMessage(result)));
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
QString QgsPostgresProvider::postgisVersion(PGconn *connection)
{
  PGresult *result = PQexec(connection, "select postgis_version()");
  postgisVersionInfo = PQgetvalue(result,0,0);

  QgsDebugMsg("PostGIS version info: " + postgisVersionInfo);

  PQclear(result);

  QStringList postgisParts = QStringList::split(" ", postgisVersionInfo);

  // Get major and minor version
  QStringList postgisVersionParts = QStringList::split(".", postgisParts[0]);

  postgisVersionMajor = postgisVersionParts[0].toInt();
  postgisVersionMinor = postgisVersionParts[1].toInt();

  // assume no capabilities
  geosAvailable = false;
  gistAvailable = false;
  projAvailable = false;

  // parse out the capabilities and store them
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

  gotPostgisVersion = TRUE;

  return postgisVersionInfo;
}

bool QgsPostgresProvider::addFeatures(QgsFeatureList & flist)
{
  bool returnvalue=true;
  PQexec(connection,"BEGIN");

  int primaryKeyHighWater = maxPrimaryKeyValue();

  for(QgsFeatureList::iterator it=flist.begin();it!=flist.end();++it)
  {
    primaryKeyHighWater++;
    if(!addFeature(*it, primaryKeyHighWater))
    {
      returnvalue=false;
      // TODO: exit loop here?
    }
  }
  PQexec(connection,"COMMIT");
  reset();
  return returnvalue;
}

bool QgsPostgresProvider::deleteFeatures(const QgsFeatureIds & id)
{
  bool returnvalue=true;
  PQexec(connection,"BEGIN");
  for(QgsFeatureIds::const_iterator it=id.begin();it!=id.end();++it)
  {
    if(!deleteFeature(*it))
    {
      returnvalue=false;
    }
  }
  PQexec(connection,"COMMIT");
  reset();
  return returnvalue;
}

bool QgsPostgresProvider::addAttributes(const QgsNewAttributesMap & name)
{
  bool returnvalue=true;
  PQexec(connection,"BEGIN");
  for(QgsNewAttributesMap::const_iterator iter=name.begin();iter!=name.end();++iter)
  {
    QString sql="ALTER TABLE "+mSchemaTableName+" ADD COLUMN "+iter.key()+" "+iter.value();

    QgsDebugMsg(sql);

    //send sql statement and do error handling
    PGresult* result=PQexec(connection, (const char *)(sql.utf8()));
    if(result==0)
    {
      returnvalue=false;
      ExecStatusType message=PQresultStatus(result);
      if(message==PGRES_FATAL_ERROR)
      {
        showMessageBox("ALTER TABLE error",QString(PQresultErrorMessage(result)));
      } 
    }
  }
  PQexec(connection,"COMMIT");
  reset();
  return returnvalue;
}

bool QgsPostgresProvider::deleteAttributes(const QgsAttributeIds& ids)
{
  bool returnvalue=true;
  PQexec(connection,"BEGIN");

  for(QgsAttributeIds::const_iterator iter=ids.begin();iter != ids.end();++iter)
  {
    QgsFieldMap::const_iterator field_it = attributeFields.find(*iter);
    if(field_it == attributeFields.constEnd())
    {
      continue;
    }
    QString column = field_it->name();
    QString sql="ALTER TABLE "+mSchemaTableName+" DROP COLUMN "+column;

    //send sql statement and do error handling
    PGresult* result=PQexec(connection, (const char *)(sql.utf8()));
    if(result==0)
    {
      returnvalue=false;
      ExecStatusType message=PQresultStatus(result);
      if(message==PGRES_FATAL_ERROR)
      {
        showMessageBox("ALTER TABLE error",QString(PQresultErrorMessage(result)));
      }
    }
    else
    {
      //delete the attribute from attributeFields
      attributeFields.remove(*iter);
    }
  }
  PQexec(connection,"COMMIT");
  reset();
  return returnvalue;
}

bool QgsPostgresProvider::changeAttributeValues(const QgsChangedAttributesMap & attr_map)
{
  bool returnvalue=true; 
  PQexec(connection,"BEGIN");

  // cycle through the features
  for(QgsChangedAttributesMap::const_iterator iter=attr_map.begin();iter!=attr_map.end();++iter)
  {
    int fid = iter.key();
    const QgsAttributeMap& attrs = iter.value();

    // cycle through the changed attributes of the feature
    for(QgsAttributeMap::const_iterator siter = attrs.begin(); siter != attrs.end(); ++siter)
    {
      QString val = siter->toString();
      QString fieldName = attributeFields[siter.key()].name();

      // escape quotes
      val.replace("'", "''");

      QString sql="UPDATE "+mSchemaTableName+" SET "+fieldName+"='"+val+"' WHERE \"" +primaryKey+"\"="+QString::number(fid);
      QgsDebugMsg(sql);

      // s end sql statement and do error handling
      // TODO: Make all error handling like this one
      PGresult* result=PQexec(connection, (const char *)(sql.utf8()));
      if (result==0)
      {
        showMessageBox(tr("PostGIS error"),
            tr("An error occured contacting the PostgreSQL database"));
        return false;
      }
      ExecStatusType message=PQresultStatus(result);
      if(message==PGRES_FATAL_ERROR)
      {
        showMessageBox(tr("PostGIS error"),tr("The PostgreSQL database returned: ")
            + QString(PQresultErrorMessage(result))
            + "\n" + tr("When trying: ") + sql);
        return false;
      }

    }
  }
  PQexec(connection,"COMMIT");
  reset();
  return returnvalue;
}

bool QgsPostgresProvider::changeGeometryValues(QgsGeometryMap & geometry_map)
{
  QgsDebugMsg("entering.");

  bool returnvalue = true;

  // Determine which insertion method to use for WKB
  // PostGIS 1.0+ uses BYTEA
  // earlier versions use HEX
  bool useWkbHex(FALSE);

  if (!gotPostgisVersion)
  {
    postgisVersion(connection);
  }

  QgsDebugMsg("PostGIS version is  major: " + QString::number(postgisVersionMajor) +
      ", minor: " + QString::number(postgisVersionMinor));

  if (postgisVersionMajor < 1)
  {
    useWkbHex = TRUE;
  }

  // Start the PostGIS transaction

  PQexec(connection,"BEGIN");

  for(QgsGeometryMap::iterator iter  = geometry_map.begin();
      iter != geometry_map.end();
      ++iter)
  {

    QgsDebugMsg("iterating over the map of changed geometries...");

    if (iter->wkbBuffer())
    {
      QgsDebugMsg("iterating over feature id " + QString::number(iter.key()));

      QString sql = "UPDATE "+ mSchemaTableName +" SET \"" + 
        geometryColumn + "\"=";

      sql += "GeomFromWKB('";

      // Add the WKB geometry to the UPDATE statement
      unsigned char* geom = iter->wkbBuffer();
      for (uint i=0; i < iter->wkbSize(); ++i)
      {
        if (useWkbHex)
        {
          // PostGIS < 1.0 wants hex
          QString hex = QString::number((int) geom[i], 16).upper();

          if (hex.length() == 1)
          {
            hex = "0" + hex;
          }

          sql += hex;
        }
        else
        {
          // Postgis 1.0 wants bytea
          QString oct = QString::number((int) geom[i], 8);

          if(oct.length()==3)
          {
            oct="\\\\"+oct;
          }
          else if(oct.length()==1)
          {
            oct="\\\\00"+oct;
          }
          else if(oct.length()==2)
          {
            oct="\\\\0"+oct; 
          }

          sql += oct;
        }
      }

      if (useWkbHex)
      {
        sql += "',"+srid+")";
      }
      else
      {
        sql += "::bytea',"+srid+")";
      }

      sql += " WHERE \"" +primaryKey+"\"="+QString::number(iter.key());

      QgsDebugMsg("Updating with: " + sql);

      // send sql statement and do error handling
      // TODO: Make all error handling like this one
      PGresult* result=PQexec(connection, (const char *)(sql.utf8()));
      if (result==0)
      {
        showMessageBox(tr("PostGIS error"), tr("An error occured contacting the PostgreSQL database"));
        return false;
      }
      ExecStatusType message=PQresultStatus(result);
      if(message==PGRES_FATAL_ERROR)
      {
        showMessageBox(tr("PostGIS error"), tr("The PostgreSQL database returned: ")
            + QString(PQresultErrorMessage(result))
            + "\n" + tr("When trying: ") + sql);
        return false;
      }

    } // if (*iter)

  } // for each feature

  PQexec(connection,"COMMIT");

  // TODO: Reset Geometry dirty if commit was OK

  reset();

  QgsDebugMsg("exiting.");

  return returnvalue;
}

QgsAttributeList QgsPostgresProvider::allAttributesList()
{
  QgsAttributeList attributes;
  for(QgsFieldMap::iterator it = attributeFields.begin(); it != attributeFields.end(); ++it)
  {
    attributes.push_back(it.key());
  }
  return attributes;
}


int QgsPostgresProvider::capabilities() const
{
  return (
      QgsVectorDataProvider::AddFeatures |
      QgsVectorDataProvider::DeleteFeatures |
      QgsVectorDataProvider::ChangeAttributeValues |
      QgsVectorDataProvider::AddAttributes |
      QgsVectorDataProvider::DeleteAttributes |
      QgsVectorDataProvider::ChangeGeometries |
      QgsVectorDataProvider::SelectGeometryAtId
      );
}

void QgsPostgresProvider::setSubsetString(QString theSQL)
{
  sqlWhereClause=theSQL;
  // Update datasource uri too
  mUri.setSql(theSQL);
  // Update yet another copy of the uri. Why are there 3 copies of the
  // uri? Perhaps this needs some rationalisation.....
  setDataSourceUri(mUri.uri());

  // need to recalculate the number of features...
  getFeatureCount();
  calculateExtents();
}

long QgsPostgresProvider::getFeatureCount()
{
  // get total number of features

  // First get an approximate count; then delegate to
  // a thread the task of getting the full count.

#ifdef POSTGRESQL_THREADS
  QString sql = "select reltuples from pg_catalog.pg_class where relname = '" + 
    tableName + "'";

  QgsDebugMsg("Running SQL: " + sql);
#else                 
  QString sql = "select count(*) from " + mSchemaTableName + "";

  if(sqlWhereClause.length() > 0)
  {
    sql += " where " + sqlWhereClause;
  }
#endif

  PGresult *result = PQexec(connection, (const char *) (sql.utf8()));

  QgsDebugMsg("Approximate Number of features as text: " +
      QString(PQgetvalue(result, 0, 0)));

  numberFeatures = QString(PQgetvalue(result, 0, 0)).toLong();
  PQclear(result);

  QgsDebugMsg("Approximate Number of features: " + QString::number(numberFeatures));

  return numberFeatures;
}

// TODO: use the estimateExtents procedure of PostGIS and PostgreSQL 8
// This tip thanks to #qgis irc nick "creeping"
void QgsPostgresProvider::calculateExtents()
{
#ifdef POSTGRESQL_THREADS
  // get the approximate extent by retreiving the bounding box
  // of the first few items with a geometry

  QString sql = "select box3d(" + geometryColumn + ") from " 
    + mSchemaTableName + " where ";

  if(sqlWhereClause.length() > 0)
  {
    sql += "(" + sqlWhereClause + ") and ";
  }

  sql += "not IsEmpty(" + geometryColumn + ") limit 5";


#if WASTE_TIME
  sql = "select xmax(extent(\"" + geometryColumn + "\")) as xmax,"
    "xmin(extent(\"" + geometryColumn + "\")) as xmin,"
    "ymax(extent(\"" + geometryColumn + "\")) as ymax," 
    "ymin(extent(\"" + geometryColumn + "\")) as ymin" 
    " from " + mSchemaTableName;
#endif

  QgsDebugMsg("Getting approximate extent using: '" + sql + "'");

  PGresult *result = PQexec(connection, (const char *) (sql.utf8()));

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

    QgsDebugMsg("After row " + QString::number(i) +
        ", extent is: " + layerExtent.stringRep());
  }

  // clear query result
  PQclear(result);

#else // non-postgresql threads version

  // get the extents

  QString sql = "select extent(\"" + geometryColumn + "\") from " + 
    mSchemaTableName;
  if(sqlWhereClause.length() > 0)
  {
    sql += " where " + sqlWhereClause;
  }

#if WASTE_TIME
  sql = "select xmax(extent(\"" + geometryColumn + "\")) as xmax,"
    "xmin(extent(\"" + geometryColumn + "\")) as xmin,"
    "ymax(extent(\"" + geometryColumn + "\")) as ymax," 
    "ymin(extent(\"" + geometryColumn + "\")) as ymin" 
    " from " + mSchemaTableName;
#endif

  QgsDebugMsg("Getting extents using schema.table: " + sql);

  PGresult *result = PQexec(connection, (const char *) (sql.utf8()));
  if(PQntuples(result)>0)
  {
    std::string box3d = PQgetvalue(result, 0, 0);

    if (box3d != "")
    {
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
    }
  }
  else
  {
    QgsDebugMsg("extents query failed");
  }

  // clear query result
  PQclear(result);

#endif

  // #ifdef QGISDEBUG
  //   QString xMsg;
  //   QTextOStream(&xMsg).precision(18);
  //   QTextOStream(&xMsg).width(18);
  //   QTextOStream(&xMsg) << "QgsPostgresProvider: Set extents to: " << layerExtent.
  //     xMin() << ", " << layerExtent.yMin() << " " << layerExtent.xMax() << ", " << layerExtent.yMax();
  //   std::cerr << xMsg << std::endl;
  // #endif
  QgsDebugMsg("Set extents to: " + layerExtent.stringRep());
}

/**
 * Event sink for events from threads
 */
void QgsPostgresProvider::customEvent( QCustomEvent * e )
{
  QgsDebugMsg("received a custom event " + QString::number(e->type()));

  switch ( e->type() )
  {
    case (QEvent::Type) QGis::ProviderExtentCalcEvent:

      QgsDebugMsg("extent has been calculated");

      // Collect the new extent from the event and set this layer's
      // extent with it.

      {
        QgsRect* r = (QgsRect*) e->data();
        setExtent( *r );
      }

      QgsDebugMsg("new extent has been saved");

      QgsDebugMsg("Set extent to: " + layerExtent.stringRep());

      QgsDebugMsg("emitting fullExtentCalculated()");

      emit fullExtentCalculated();

      // TODO: Only uncomment this when the overview map canvas has been subclassed
      // from the QgsMapCanvas

      //        QgsDebugMsg("emitting repaintRequested()");
      //        emit repaintRequested();

      break;

    case (QEvent::Type) QGis::ProviderCountCalcEvent:

      QgsDebugMsg("count has been calculated");

      numberFeatures = ((QgsProviderCountCalcEvent*) e)->numberFeatures();

      QgsDebugMsg("count is " + QString::number(numberFeatures));

      break;

    default:
      // do nothing
      break;
  }

  QgsDebugMsg("Finished processing custom event " + QString::number(e->type()));

}


bool QgsPostgresProvider::deduceEndian()
{
  // need to store the PostgreSQL endian format used in binary cursors
  // since it appears that starting with
  // version 7.4, binary cursors return data in XDR whereas previous versions
  // return data in the endian of the server

  QString firstOid = "select oid from pg_class where relname = '" + 
    mTableName + "' and relnamespace = (select oid from pg_namespace where nspname = '"
    + mSchemaName + "')";
  PGresult * oidResult = PQexec(connection, (const char*)(firstOid.utf8()));
  // get the int value from a "normal" select
  QString oidValue = PQgetvalue(oidResult,0,0);
  PQclear(oidResult);

  QgsDebugMsg("Creating binary cursor");

  // get the same value using a binary cursor

  PQexec(connection,"begin work");
  QString oidDeclare = QString("declare oidcursor binary cursor for select oid from pg_class where relname = '%1' and relnamespace = (select oid from pg_namespace where nspname = '%2')").arg(mTableName).arg(mSchemaName);
  // set up the cursor
  PQexec(connection, (const char *)oidDeclare);
  QString fetch = "fetch forward 1 from oidcursor";

  QgsDebugMsg("Fetching a record and attempting to get check endian-ness");

  PGresult *fResult = PQexec(connection, (const char *)fetch);
  PQexec(connection, "end work");
  swapEndian = true;
  if(PQntuples(fResult) > 0){
    // get the oid value from the binary cursor
    int oid = *(int *)PQgetvalue(fResult,0,0);

    //--std::cout << "Got oid of " << oid << " from the binary cursor" << std::endl;
    //--std::cout << "First oid is " << oidValue << std::endl;
    // compare the two oid values to determine if we need to do an endian swap
    if(oid == oidValue.toInt())
      swapEndian = false;

    PQclear(fResult);
  }
  return swapEndian;
}

bool QgsPostgresProvider::getGeometryDetails()
{
  QString fType("");
  srid = "";
  valid = false;
  QStringList log;

  QString sql = "select f_geometry_column,type,srid from geometry_columns"
    " where f_table_name='" + mTableName + "' and f_geometry_column = '" + 
    geometryColumn + "' and f_table_schema = '" + mSchemaName + "'";

  QgsDebugMsg("Getting geometry column: " + sql);

  PGresult *result = executeDbCommand(connection, sql);

  QgsDebugMsg("geometry column query returned " + QString::number(PQntuples(result)));
  QgsDebugMsg("column number of srid is " + QString::number(PQfnumber(result, "srid")));

  if (PQntuples(result) > 0)
  {
    srid = PQgetvalue(result, 0, PQfnumber(result, "srid"));
    fType = PQgetvalue(result, 0, PQfnumber(result, "type"));
    PQclear(result);
  }
  else
  {
    // Didn't find what we need in the geometry_columns table, so
    // get stuff from the relevant column instead. This may (will?) 
    // fail if there is no data in the relevant table.
    PQclear(result); // for the query just before the if() statement
    sql = "select "
      "srid(\""         + geometryColumn + "\"), "
      "geometrytype(\"" + geometryColumn + "\") from " + 
      mSchemaTableName;

    //it is possible that the where clause restricts the feature type
    if(!sqlWhereClause.isEmpty())
      {
	sql += " WHERE ";
	sql += sqlWhereClause;
      }

    sql += " limit 1";

    result = executeDbCommand(connection, sql);

    if (PQntuples(result) > 0)
    {
      srid = PQgetvalue(result, 0, PQfnumber(result, "srid"));
      fType = PQgetvalue(result, 0, PQfnumber(result, "geometrytype"));
    }
    PQclear(result);
  }

  if (!srid.isEmpty() && !fType.isEmpty())
  {
    valid = true;
    if(fType == "GEOMETRY")
    {
      // check to see if there is a unique geometry type
      sql = QString("select distinct "
          "case"
          " when geometrytype(%1) IN ('POINT','MULTIPOINT') THEN 'POINT'"
          " when geometrytype(%1) IN ('LINESTRING','MULTILINESTRING') THEN 'LINESTRING'"
          " when geometrytype(%1) IN ('POLYGON','MULTIPOLYGON') THEN 'POLYGON'"
          " end "
          "from %2").arg(geometryColumn).arg(mSchemaTableName);
      if(mUri.sql()!="")
        sql += " where " + mUri.sql();

      result = executeDbCommand(connection, sql);

      if (PQntuples(result)==1)
      {
        fType = PQgetvalue(result, 0, 0);
      }
      PQclear(result);
    }
    if (fType == "POINT")
    {
      geomType = QGis::WKBPoint;
    }
    else if(fType == "MULTIPOINT")
    {
      geomType = QGis::WKBMultiPoint;
    }
    else if(fType == "LINESTRING")
    {
      geomType = QGis::WKBLineString;
    }
    else if(fType == "MULTILINESTRING")
    {
      geomType = QGis::WKBMultiLineString;
    }
    else if (fType == "POLYGON")
    {
      geomType = QGis::WKBPolygon;
    }
    else if(fType == "MULTIPOLYGON")
    {
      geomType = QGis::WKBMultiPolygon;
    }
    else
    {
      showMessageBox(tr("Unknown geometry type"), 
          tr("Column ") + geometryColumn + tr(" in ") +
          mSchemaTableName + tr(" has a geometry type of ") +
          fType + tr(", which Qgis does not currently support."));
      valid = false;
    }
  }
  else // something went wrong...
  {
    log.prepend(tr("Qgis was unable to determine the type and srid of "
          "column " + geometryColumn + tr(" in ") +
          mSchemaTableName + 
          tr(". The database communication log was:\n")));
    showMessageBox(tr("Unable to get feature type and srid"), log);
  }

  if (valid)
  {
    QgsDebugMsg("SRID is " + srid);
    QgsDebugMsg("type is " + fType);
    QgsDebugMsg("Feature type is " + QString::number(geomType));
    QgsDebugMsg("Feature type name is " + QString(QGis::qgisFeatureTypes[geomType]));
  }
  else
  {
    QgsDebugMsg("Failed to get geometry details for Postgres layer.");
  }

  return valid;
}

PGresult* QgsPostgresProvider::executeDbCommand(PGconn* connection, 
    const QString& sql)
{
  PGresult *result = PQexec(connection, (const char *) (sql.utf8()));

  QgsDebugMsg("Executed SQL: " + sql);
  if (PQresultStatus(result) == PGRES_TUPLES_OK) {
    QgsDebugMsg("Command was successful.");
  } else {
    QgsDebugMsg("Command was unsuccessful. The error message was: "
        + QString( PQresultErrorMessage(result) ) );
  }
  return result;
}

void QgsPostgresProvider::showMessageBox(const QString& title, 
    const QString& text)
{
  QgsMessageOutput* message = QgsMessageOutput::createMessageOutput();
  message->setTitle(title);
  message->setMessage(text, QgsMessageOutput::MessageText);
  message->showMessage();
}

void QgsPostgresProvider::showMessageBox(const QString& title, 
    const QStringList& text)
{
  showMessageBox(title, text.join("\n"));
}


QgsSpatialRefSys QgsPostgresProvider::getSRS()
{
  QgsSpatialRefSys srs;
  srs.createFromSrid(srid.toInt());
  return srs;
}

QString QgsPostgresProvider::subsetString()
{
  return sqlWhereClause;
}

PGconn * QgsPostgresProvider::pgConnection()
{
  return connection;
}

QString QgsPostgresProvider::getTableName()
{
  return mTableName;
}


size_t QgsPostgresProvider::layerCount() const
{
  return 1;                   // XXX need to return actual number of layers
} // QgsPostgresProvider::layerCount()



QString  QgsPostgresProvider::name() const
{
  return POSTGRES_KEY;
} //  QgsPostgresProvider::name()



QString  QgsPostgresProvider::description() const
{
  return POSTGRES_DESCRIPTION;
} //  QgsPostgresProvider::description()




/**
 * Class factory to return a pointer to a newly created 
 * QgsPostgresProvider object
 */
QGISEXTERN QgsPostgresProvider * classFactory(const QString *uri)
{
  return new QgsPostgresProvider(*uri);
}
/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return  POSTGRES_KEY;
}
/**
 * Required description function 
 */
QGISEXTERN QString description()
{
  return POSTGRES_DESCRIPTION;
} 
/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider(){
  return true;
}

QMap<QString, QgsPostgresProvider::Conn *> QgsPostgresProvider::connections;
