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

QgsPostgresProvider::QgsPostgresProvider(QString const & uri)
: QgsVectorDataProvider(uri),
  geomType(QGis::WKBUnknown),
  mFeatureQueueSize(200),
  gotPostgisVersion(FALSE)
{
  // assume this is a valid layer until we determine otherwise
  valid = true;

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

  connection = connectDb( mUri.connInfo() );
  if( connection==NULL ) {
    valid = false;
    return;
  }

  QgsDebugMsg("Checking for permissions on the relation");

  // Check that we can read from the table (i.e., we have
  // select permission).
  QString sql = QString("select * from %1 limit 1").arg(mSchemaTableName);
  PGresult* testAccess = PQexec(connection, sql.toUtf8());
  if (PQresultStatus(testAccess) != PGRES_TUPLES_OK)
  {
    showMessageBox(tr("Unable to access relation"),
        tr("Unable to access the ") + mSchemaTableName + 
        tr(" relation.\nThe error message from the database was:\n") +
        QString::fromUtf8(PQresultErrorMessage(testAccess)) + ".\n" + 
        "SQL: " + sql);
    PQclear(testAccess);
    valid = false;
    disconnectDb();
    return;
  }
  PQclear(testAccess);

  sql = QString("SELECT "
                           "has_table_privilege(%1,'DELETE'),"
                           "has_table_privilege(%1,'UPDATE'),"
                           "has_table_privilege(%1,'INSERT'),"
                           "current_schema()")
                           .arg( quotedValue(mSchemaTableName) );

  testAccess = PQexec( connection, sql.toUtf8() );
  if( PQresultStatus(testAccess) != PGRES_TUPLES_OK ) {
    showMessageBox(tr("Unable to access relation"),
        tr("Unable to determine table access privileges for the ") + mSchemaTableName + 
        tr(" relation.\nThe error message from the database was:\n") +
        QString::fromUtf8(PQresultErrorMessage(testAccess)) + ".\n" + 
        "SQL: " + sql);
    PQclear(testAccess);
    valid = false;
    disconnectDb();
    return;
  }

  enabledCapabilities = QgsVectorDataProvider::SelectGeometryAtId;
  
  if( QString::fromUtf8( PQgetvalue(testAccess, 0, 0) )=="t" ) {
    // DELETE
    enabledCapabilities |= QgsVectorDataProvider::DeleteFeatures;
  }
      
  if( QString::fromUtf8( PQgetvalue(testAccess, 0, 1) )=="t" ) {
    // UPDATE
    enabledCapabilities |= QgsVectorDataProvider::ChangeGeometries | QgsVectorDataProvider::ChangeAttributeValues;
  }

  if( QString::fromUtf8( PQgetvalue(testAccess, 0, 2) )=="t" ) {
    // INSERT
    enabledCapabilities |= QgsVectorDataProvider::AddFeatures;
  }

  mCurrentSchema = QString::fromUtf8( PQgetvalue(testAccess, 0, 3) );
  if(mCurrentSchema==mSchemaName) {
    mUri.clearSchema();
    setDataSourceUri( mUri.uri() );
  }
  if(mSchemaName=="")
    mSchemaName=mCurrentSchema;
  PQclear(testAccess);

  sql = QString("SELECT 1 FROM pg_class,pg_namespace WHERE "
                   "pg_class.relnamespace=pg_namespace.oid AND "
                   "pg_get_userbyid(relowner)=current_user AND "
                   "relname=%1 AND nspname=%2")
          .arg( quotedValue(mTableName) )
          .arg( quotedValue(mSchemaName) );
  testAccess = PQexec(connection, sql.toUtf8());
  if (PQresultStatus(testAccess) == PGRES_TUPLES_OK && PQntuples(testAccess)==1)
  {
    enabledCapabilities |= QgsVectorDataProvider::AddAttributes | QgsVectorDataProvider::DeleteAttributes;
  }
  PQclear(testAccess);

  if ( !getGeometryDetails() ) // gets srid and geometry type
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
  PQexecNR(connection, QString("set client_min_messages to error").toUtf8());
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

PGconn *QgsPostgresProvider::connectDb(const QString & conninfo)
{
  QgsDebugMsg(QString("New postgres connection for ") + conninfo);

  PGconn *pd = PQconnectdb(conninfo.toLocal8Bit());	// use what is set based on locale; after connecting, use Utf8
  // check the connection status
  if (PQstatus(pd) != CONNECTION_OK) 
  {
    QgsDebugMsg("Connection to database failed");
    return NULL;
  }

  //set client encoding to unicode because QString uses UTF-8 anyway
  QgsDebugMsg("setting client encoding to UNICODE");

  int errcode=PQsetClientEncoding(pd, QString("UNICODE").toLocal8Bit());

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

  return pd;
}

void QgsPostgresProvider::disconnectDb()
{
  PQfinish( connection );
  connection = 0;
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
      QString fetch = QString("fetch forward %1 from qgisf")
        .arg(mFeatureQueueSize);

      if(mFirstFetch)
      {
        if(PQsendQuery(connection, fetch.toUtf8()) == 0) //fetch features in asynchronously
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

        PQclear(queryResult);
        if (ready)
          PQexecNR(connection, QString("end work").toUtf8());
        ready = false;
        return false;
      }

      for (int row = 0; row < rows; row++)
      {
        int oid = *(int *)PQgetvalue(queryResult, row, PQfnumber(queryResult,quotedIdentifier(primaryKey).toUtf8()));

        if (swapEndian)
          oid = ntohl(oid); // convert oid to opposite endian
 
        mFeatureQueue.push(QgsFeature());

        // set ID
        mFeatureQueue.back().setFeatureId(oid);

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
            int fn = PQfnumber(queryResult,quotedIdentifier(*name_it).toUtf8());

            if( !PQgetisnull(queryResult, row, fn) )
              val = QString::fromUtf8(PQgetvalue(queryResult, row, fn));
            else
              val = QString::null;
          }

          if( val.isNull() )
          {
            mFeatureQueue.back().addAttribute(*index_it, val);
          }
          else
          {
            switch (attributeFields[*index_it].type())
            {
            case QVariant::LongLong:
              mFeatureQueue.back().addAttribute(*index_it, val.toLongLong());
              break;
            case QVariant::Int:
              mFeatureQueue.back().addAttribute(*index_it, val.toInt());
              break;
            case QVariant::Double:
              mFeatureQueue.back().addAttribute(*index_it, val.toDouble());
              break;
            case QVariant::String:
              mFeatureQueue.back().addAttribute(*index_it, val);
              break;
            default:
              assert(0 && "unsupported field type");
            }
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
            memcpy(featureGeom, PQgetvalue(queryResult, row, PQfnumber(queryResult,QString("qgs_feature_geometry").toUtf8())), returnedLength); 
            mFeatureQueue.back().setGeometryAndOwnership(featureGeom, returnedLength + 1);
          }
          else
          {
            mFeatureQueue.back().setGeometryAndOwnership(0, 0);
            QgsDebugMsg("Couldn't get the feature geometry in binary form");
          }
        } 
      } // for each row in queue

      PQclear(queryResult);

      if(PQsendQuery(connection, fetch.toUtf8()) == 0) //already fetch the next couple of features asynchronously
      {
        qWarning("PQsendQuery failed (2)");
      }

    } // if new queue is required

    // Now return the next feature from the queue
    if(mFetchGeom)
    {
      QgsGeometry* featureGeom = mFeatureQueue.front().geometryAndOwnership();
	    feature.setGeometry(featureGeom);
    }
    else
    {
	    feature.setGeometryAndOwnership(0, 0);
    }
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
  for(QgsAttributeList::const_iterator it = mAttributesToFetch.constBegin();
      it != mAttributesToFetch.constEnd(); ++it)
  {
    fieldIt = attributeMap.find(*it);
    if(fieldIt != attributeMap.end())
    {
      mFetchAttributeNames.push_back(fieldIt.value().name());
    }
  }

  QString declare = "declare qgisf binary cursor for select " + quotedIdentifier(primaryKey);

  if(fetchGeometry)
  {
    declare += QString(",asbinary(%1,'%2') as qgs_feature_geometry")
                  .arg( quotedIdentifier(geometryColumn) )
                  .arg( endianString() ); 
  }
  for(std::list<QString>::const_iterator it = mFetchAttributeNames.begin(); it != mFetchAttributeNames.end(); ++it)
  {
    if( (*it) != primaryKey) //no need to fetch primary key again
    {
      declare += "," + quotedIdentifier(*it) + "::text";
    }
  }

  declare += QString(" from %1").arg(mSchemaTableName);

  QgsDebugMsg("Binary cursor: " + declare);

  bool hasWhere = FALSE;

  if(!rect.isEmpty())
  {
    if(useIntersect)
    { 
      // Contributed by #qgis irc "creeping"
      // This version actually invokes PostGIS's use of spatial indexes
      declare += " where " + quotedIdentifier(geometryColumn);
      declare += " && setsrid('BOX3D(" + rect.asWKTCoords();
      declare += ")'::box3d,";
      declare += srid;
      declare += ")";
      declare += " and intersects(" + quotedIdentifier(geometryColumn);
      declare += ", setsrid('BOX3D(" + rect.asWKTCoords();
      declare += ")'::box3d,";
      declare += srid;
      declare += "))";
    }
    else
    {
      declare += " where " + quotedIdentifier(geometryColumn);
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
    PQexecNR(connection, QString("end work").toUtf8());
  }
  PQexecNR(connection,QString("begin work").toUtf8());
  ready = true;
  PQexecNR(connection, declare.toUtf8());

  while(!mFeatureQueue.empty())
    {
      mFeatureQueue.pop();
    }
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

  QString sql = "declare qgisfid binary cursor for select " + quotedIdentifier(primaryKey);

  if(fetchGeometry)
  {
    sql += QString(",asbinary(%1,'%2') as qgs_feature_geometry")
             .arg( quotedIdentifier(geometryColumn) )
             .arg( endianString() ); 
  }
  for(namesIt = attributeNames.begin(); namesIt != attributeNames.end(); ++namesIt)
  {
    if( (*namesIt) != primaryKey) //no need to fetch primary key again
    {
      sql += "," + quotedIdentifier(*namesIt) + "::text";
    }
  }

  sql += " " + QString("from %1").arg(mSchemaTableName);

  sql += " where " + quotedIdentifier(primaryKey) + "=" + QString::number(featureId);

  QgsDebugMsg("Selecting feature using: " + sql);

  PQexecNR(connection,QString("begin work").toUtf8());

  // execute query
  PQexecNR(connection, sql.toUtf8());

  PGresult *res = PQexec(connection, QString("fetch forward 1 from qgisfid").toUtf8());

  int rows = PQntuples(res);
  if (rows == 0)
  {
    PQclear(res);
    PQexecNR(connection, QString("end work").toUtf8());
    QgsDebugMsg("feature " + QString::number(featureId) + " not found");
    return FALSE;
  }

  // set ID
  int oid = *(int *)PQgetvalue(res, 0, PQfnumber(res,quotedIdentifier(primaryKey).toUtf8()));
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
      int fn = PQfnumber(res,quotedIdentifier(*namesIt).toUtf8());

      if( PQgetisnull(res, 0, fn) )
        val = QString::fromUtf8(PQgetvalue(res, 0, fn));
      else
        val = QString::null;
    }

    if( val.isNull() )
    {
      feature.addAttribute(*it, val);
    }
    else
    {
      switch (attributeFields[*it].type())
      {
      case QVariant::LongLong:
        feature.addAttribute(*it, val.toLongLong());
        break;
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
  }

  // fetch geometry
  if (fetchGeometry)
  {
    int returnedLength = PQgetlength(res, 0, PQfnumber(res,"qgs_feature_geometry")); 
    if(returnedLength > 0)
    {
      unsigned char *featureGeom = new unsigned char[returnedLength + 1];
      memset(featureGeom, '\0', returnedLength + 1);
      memcpy(featureGeom, PQgetvalue(res, 0, PQfnumber(res,QString("qgs_feature_geometry").toUtf8())), returnedLength); 
      feature.setGeometryAndOwnership(featureGeom, returnedLength + 1);
    }
  }

  PQclear(res);
  PQexecNR(connection, QString("end work").toUtf8());

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
  QString move = "move 0 in qgisf"; //move cursor to first record
  PQexecNR(connection, move.toUtf8());
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
  QString sql = "SELECT regclass(" + quotedValue(mSchemaTableName) + ")::oid";
  PGresult *tresult= PQexec(connection, sql.toUtf8());
  QString tableoid = QString::fromUtf8(PQgetvalue(tresult, 0, 0));
  PQclear(tresult);

  // Get the table description
  sql = "SELECT description FROM pg_description WHERE "
    "objoid = " + tableoid + " AND objsubid = 0";
  tresult = PQexec(connection, sql.toUtf8());
  if (PQntuples(tresult) > 0)
    mDataComment = QString::fromUtf8(PQgetvalue(tresult, 0, 0));
  PQclear(tresult);

  // Populate the field vector for this layer. The field vector contains
  // field name, type, length, and precision (if numeric)
  sql = "select * from " + mSchemaTableName + " limit 0";

  PGresult *result = PQexec(connection, sql.toUtf8());
  //--std::cout << "Field: Name, Type, Size, Modifier:" << std::endl;

  // The queries inside this loop could possibly be combined into one
  // single query - this would make the code run faster.

  attributeFields.clear();
  for (int i = 0; i < PQnfields(result); i++)
  {
    QString fieldName = QString::fromUtf8(PQfname(result, i));
    int fldtyp = PQftype(result, i);
    QString typOid = QString().setNum(fldtyp);
    int fieldModifier = PQfmod(result, i);
    QString fieldComment("");

    sql = "SELECT typname, typlen FROM pg_type WHERE " 
      "oid="+typOid;	// just oid; needs more work to support array type
//      "oid = (SELECT Distinct typelem FROM pg_type WHERE "	//needs DISTINCT to guard against 2 or more rows on int2
//      "typelem = " + typOid + " AND typlen = -1)";

    PGresult* oidResult = PQexec(connection, sql.toUtf8());
    QString fieldTypeName = QString::fromUtf8(PQgetvalue(oidResult, 0, 0));
    QString fieldSize = QString::fromUtf8(PQgetvalue(oidResult, 0, 1));
    PQclear(oidResult);

    sql = "SELECT attnum FROM pg_attribute WHERE "
      "attrelid = " + tableoid + " AND attname = " + quotedValue(fieldName);
    PGresult *tresult = PQexec(connection, sql.toUtf8());
    QString attnum = QString::fromUtf8(PQgetvalue(tresult, 0, 0));
    PQclear(tresult);

    sql = "SELECT description FROM pg_description WHERE "
      "objoid = " + tableoid + " AND objsubid = " + attnum;
    tresult = PQexec(connection, sql.toUtf8());
    if (PQntuples(tresult) > 0)
      fieldComment = QString::fromUtf8(PQgetvalue(tresult, 0, 0));
    PQclear(tresult);

    QgsDebugMsg("Field: " + attnum + " maps to " + QString::number(i) + " " + fieldName + ", " 
        + fieldTypeName + " (" + QString::number(fldtyp) + "),  " + fieldSize + ", " + QString::number(fieldModifier));

    if(fieldName!=geometryColumn)
    {
      QVariant::Type fieldType;
      if (fieldTypeName.find("int8") != -1)
        fieldType = QVariant::LongLong;
      else if (fieldTypeName.find("int") != -1 || fieldTypeName.find("serial") != -1)
        fieldType = QVariant::Int;
      else if (fieldTypeName == "real" || fieldTypeName == "double precision" || fieldTypeName.find("float") != -1)
        fieldType = QVariant::Double;
      else if (fieldTypeName == "bytea")
        continue;
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

  QString sql ="select indkey from pg_index where indisunique = 't' and "
    "indrelid = regclass(" + quotedValue(mSchemaTableName) + ")::oid";

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

    sql = "SELECT relkind FROM pg_class WHERE oid = regclass(" + quotedValue(mSchemaTableName) + ")::oid";
    PGresult* tableType = executeDbCommand(connection, sql);
    QString type = QString::fromUtf8(PQgetvalue(tableType, 0, 0));
    PQclear(tableType);

    primaryKey = "";

    if (type == "r") // the relation is a table
    {
      QgsDebugMsg("Relation is a table. Checking to see if it has an oid column.");

      // If there is an oid on the table, use that instead,
      // otherwise give up
      sql = "SELECT attname FROM pg_attribute WHERE attname = 'oid' AND "
        "attrelid = regclass(" + quotedValue(mSchemaTableName) + ")";

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
      QString col = QString::fromUtf8(PQgetvalue(pk, i, 0));
      QStringList columns = QStringList::split(" ", col);
      if (columns.count() == 1)
      {
        // Get the column name and data type
        sql = "select attname, pg_type.typname from pg_attribute, pg_type where "
          "atttypid = pg_type.oid and attnum = " +
          col + " and attrelid = regclass(" + quotedValue(mSchemaTableName) + ")";
        PGresult* types = executeDbCommand(connection, sql);

        if( PQntuples(types) > 0 )
        {
          QString columnName = QString::fromUtf8(PQgetvalue(types, 0, 0));
          QString columnType = QString::fromUtf8(PQgetvalue(types, 0, 1));

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
          + ") and attrelid = regclass(" + quotedValue(mSchemaTableName) + ")::oid";
        PGresult* types = executeDbCommand(connection, sql);
        QString colNames;
        int numCols = PQntuples(types);
        for (int j = 0; j < numCols; ++j)
        {
          if (j == numCols-1)
            colNames += tr("and ");
          colNames += quotedValue( QString::fromUtf8(PQgetvalue(types, j, 0)) );
          if ( j < numCols-2 )
            colNames+= ",";
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
        "attrelid = regclass('" + mSchemaTableName + "')::oid";
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
    sql = "select regclass('" + quotedIdentifier(schemaName) + "." + quotedIdentifier(tableName) + "')::oid";
    PGresult* result = PQexec(connection, sql.toUtf8());
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

    result = PQexec(connection, sql.toUtf8());
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
    PGresult* result = PQexec(connection, sql.toUtf8());

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

  QString sql = "select count(distinct " + quotedIdentifier(colName) + ") = count(" +  quotedIdentifier(colName) + ") from " + quotedIdentifier(schemaName) + "." + quotedIdentifier(tableName);

  PGresult* unique = PQexec(connection, sql.toUtf8());

  if (PQntuples(unique) == 1)
//    if (strncmp(PQgetvalue(unique, 0, 0),"t", 1) == 0)
    if (QString::fromUtf8(PQgetvalue(unique, 0, 0)).compare("t") == 0)	//really should compare just first character as original did
      isUnique = true;

  PQclear(unique);

  return isUnique;
}

int QgsPostgresProvider::SRCFromViewColumn(const QString& ns, const QString& relname, const QString& attname_table, const QString& attname_view, const QString& viewDefinition, SRC& result) const
{
  QString newViewDefSql = "SELECT definition FROM pg_views WHERE schemaname = '" + ns + "' AND viewname = '" + relname + "'";
  PGresult* newViewDefResult = PQexec(connection, newViewDefSql.toUtf8());
  int numEntries = PQntuples(newViewDefResult);

  if(numEntries > 0) //relation is a view
  {
    QString newViewDefinition(QString::fromUtf8(PQgetvalue(newViewDefResult, 0, 0)));

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

    QString viewColumnSql = "SELECT table_schema, table_name, column_name FROM (SELECT DISTINCT current_database()::information_schema.sql_identifier AS view_catalog, nv.nspname::information_schema.sql_identifier AS view_schema, v.relname::information_schema.sql_identifier AS view_name, current_database()::information_schema.sql_identifier AS table_catalog, nt.nspname::information_schema.sql_identifier AS table_schema, t.relname::information_schema.sql_identifier AS table_name, a.attname::information_schema.sql_identifier AS column_name "
" FROM pg_namespace nv, pg_class v, pg_depend dv, pg_depend dt, pg_class t, pg_namespace nt, pg_attribute a "
" WHERE nv.oid = v.relnamespace AND v.relkind = 'v'::\"char\" AND v.oid = dv.refobjid AND dv.refclassid = 'pg_class'::regclass::oid AND dv.classid = 'pg_rewrite'::regclass::oid AND dv.deptype = 'i'::\"char\" AND dv.objid = dt.objid AND dv.refobjid <> dt.refobjid AND dt.classid = 'pg_rewrite'::regclass::oid AND dt.refclassid = 'pg_class'::regclass::oid AND dt.refobjid = t.oid AND t.relnamespace = nt.oid AND (t.relkind = ANY (ARRAY['r'::\"char\", 'v'::\"char\"])) AND t.oid = a.attrelid AND dt.refobjsubid = a.attnum "
"ORDER BY current_database()::information_schema.sql_identifier, nv.nspname::information_schema.sql_identifier, v.relname::information_schema.sql_identifier, current_database()::information_schema.sql_identifier, nt.nspname::information_schema.sql_identifier, t.relname::information_schema.sql_identifier, a.attname::information_schema.sql_identifier) x WHERE view_schema = '" + ns + "' AND view_name = '" + relname + "' AND column_name = '" + newAttNameTable +"'";
    PGresult* viewColumnResult = PQexec(connection, viewColumnSql.toUtf8());
    if(PQntuples(viewColumnResult) > 0)
    {
      QString newTableSchema = QString::fromUtf8(PQgetvalue(viewColumnResult, 0, 0));
      QString newTableName = QString::fromUtf8(PQgetvalue(viewColumnResult, 0, 1));
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
  PGresult* typeSqlResult = PQexec(connection, typeSql.toUtf8());
  if(PQntuples(typeSqlResult) < 1)
  {
    return 1;
  }
  QString type = QString::fromUtf8(PQgetvalue(typeSqlResult, 0, 0));
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
  QString viewColumnSql = "SELECT table_schema, table_name, column_name FROM (SELECT DISTINCT current_database() AS view_catalog, nv.nspname AS view_schema, v.relname AS view_name, current_database() AS table_catalog, nt.nspname AS table_schema, t.relname AS table_name, a.attname AS column_name "
" FROM pg_namespace nv, pg_class v, pg_depend dv, pg_depend dt, pg_class t, pg_namespace nt, pg_attribute a "
" WHERE nv.oid = v.relnamespace AND v.relkind = 'v'::\"char\" AND v.oid = dv.refobjid AND dv.refclassid = 'pg_class'::regclass::oid AND dv.classid = 'pg_rewrite'::regclass::oid AND dv.deptype = 'i'::\"char\" AND dv.objid = dt.objid AND dv.refobjid <> dt.refobjid AND dt.classid = 'pg_rewrite'::regclass::oid AND dt.refclassid = 'pg_class'::regclass::oid AND dt.refobjid = t.oid AND t.relnamespace = nt.oid AND (t.relkind = ANY (ARRAY['r'::\"char\", 'v'::\"char\"])) AND t.oid = a.attrelid AND dt.refobjsubid = a.attnum "
"ORDER BY current_database(), nv.nspname, v.relname, current_database(), nt.nspname, t.relname, a.attname) x WHERE view_schema = '" + mSchemaName + "' AND view_name = '" + mTableName + "'";
  PGresult* viewColumnResult = PQexec(connection, viewColumnSql.toUtf8());

  //find out view definition
  QString viewDefSql = "SELECT definition FROM pg_views WHERE schemaname = '" + mSchemaName + "' AND viewname = '" + mTableName + "'";
  PGresult* viewDefResult = PQexec(connection, viewDefSql.toUtf8());
  if(PQntuples(viewDefResult) < 1)
  {
    PQclear(viewDefResult);
    return;
  }
  QString viewDefinition(QString::fromUtf8(PQgetvalue(viewDefResult, 0, 0)));
  PQclear(viewDefResult);

  QString ns, relname, attname_table, attname_view;
  SRC columnInformation;

  for(int i = 0; i < PQntuples(viewColumnResult); ++i)
  {
    ns = QString::fromUtf8(PQgetvalue(viewColumnResult, i, 0));
    relname = QString::fromUtf8(PQgetvalue(viewColumnResult, i, 1));
    attname_table = QString::fromUtf8(PQgetvalue(viewColumnResult, i, 2));

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
    sql = QString("select min(%1) from %2").arg(quotedIdentifier(fld.name())).arg(mSchemaTableName);
  }
  else
  {
    sql = QString("select min(%1) from %2").arg(quotedIdentifier(fld.name())).arg(mSchemaTableName)+" where "+sqlWhereClause;
  }
  PGresult *rmin = PQexec(connection, sql.toUtf8());
  QString minValue = QString::fromUtf8(PQgetvalue(rmin,0,0));
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
    sql = QString("select max(%1) from %2").arg(quotedIdentifier(fld.name())).arg(mSchemaTableName);
  }
  else
  {
    sql = QString("select max(%1) from %2").arg(quotedIdentifier(fld.name())).arg(mSchemaTableName)+" where "+sqlWhereClause;
  } 
  PGresult *rmax = PQexec(connection, sql.toUtf8());
  QString maxValue = QString::fromUtf8(PQgetvalue(rmax,0,0));
  PQclear(rmax);
  return maxValue.toDouble();
}


int QgsPostgresProvider::maxPrimaryKeyValue()
{
  QString sql;

  sql = QString("select max(%1) from %2")
    .arg(quotedIdentifier(primaryKey))
    .arg(mSchemaTableName);

  PGresult *rmax = PQexec(connection, sql.toUtf8());
  QString maxValue = QString::fromUtf8(PQgetvalue(rmax,0,0));
  PQclear(rmax);

  return maxValue.toInt();
}


bool QgsPostgresProvider::isValid(){
  return valid;
}

QVariant QgsPostgresProvider::getDefaultValue(int fieldId)
{
  // Get the default column value from the Postgres information
  // schema. If there is no default we return an empty string.

  // Maintaining a cache of the results of this query would be quite
  // simple and if this query is called lots, could save some time.

  QString fieldName = attributeFields[fieldId].name();

  QString sql("SELECT column_default FROM"
      " information_schema.columns WHERE"
      " column_default IS NOT NULL"
      " AND table_schema = " + quotedValue(mSchemaName) +
      " AND table_name = " + quotedValue(mTableName) +
      " AND column_name = " + quotedValue(fieldName) );

  QVariant defaultValue = QString::null;

  PGresult* result = PQexec(connection, sql.toUtf8());

  if (PQntuples(result)==1 && !PQgetisnull(result, 0, 0) )
    defaultValue = QString::fromUtf8(PQgetvalue(result, 0, 0));

  QgsDebugMsg( QString("defaultValue for %1 is NULL: %2").arg(fieldId).arg( defaultValue.isNull() ) );

  PQclear(result);

  return defaultValue;
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
  PGresult *result = PQexec(connection, QString("select postgis_version()").toUtf8());
  postgisVersionInfo = QString::fromUtf8(PQgetvalue(result,0,0));

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
  if(geos.size() == 1) {
    geosAvailable = (geos[0].find("=1") > -1);  
  }
  QStringList gist = postgisParts.grep("STATS");
  if(gist.size() == 1) {
    gistAvailable = (geos[0].find("=1") > -1);
  }
  QStringList proj = postgisParts.grep("PROJ");
  if(proj.size() == 1) {
    projAvailable = (proj[0].find("=1") > -1);
  }

  useWkbHex = postgisVersionMajor < 1;

  gotPostgisVersion = TRUE;

  return postgisVersionInfo;
}

QByteArray QgsPostgresProvider::paramValue(QString fieldValue, const QString &defaultValue) const
{
  if( fieldValue.isNull() )
    return QByteArray(0);  // QByteArray(0).isNull() is true

  if( fieldValue==defaultValue && !defaultValue.isNull() )
  {
    PGresult *result = PQexec( connection, QString("select %1").arg(defaultValue).toUtf8() );
    if( PQgetisnull(result, 0, 0) ) {
      PQclear(result);
      return QByteArray(0);  // QByteArray(0).isNull() is true
    } else {
      QString val = QString::fromUtf8(PQgetvalue(result,0,0));
      PQclear(result);
      return val.toUtf8();
    }
  }

  return fieldValue.toUtf8();
}

bool QgsPostgresProvider::addFeatures(QgsFeatureList & flist)
{
  if( flist.size() == 0 )
    return true;

  bool returnvalue=true;

  try {
    PQexecNR(connection,QString("BEGIN").toUtf8());

    // Prepare the INSERT statement
    QString insert = QString("INSERT INTO %1(%2,%3")
      .arg( mSchemaTableName )
      .arg( quotedIdentifier(geometryColumn) )
      .arg( quotedIdentifier(primaryKey) ),
      values = QString(") VALUES (GeomFromWKB($1%1,%2),$2")
      .arg( useWkbHex ? "" : "::bytea" )
      .arg( srid );

    const QgsAttributeMap &attributevec = flist[0].attributeMap();

    QStringList defaultValue;
    QList<int> fieldId;

    // look for unique attribute values to place in statement instead of passing as parameter
    // e.g. for defaults
    for(QgsAttributeMap::const_iterator it = attributevec.begin(); it != attributevec.end(); it++)
    {
      QgsFieldMap::const_iterator fit = attributeFields.find( it.key() );     
      if (fit == attributeFields.end() )
        continue;

      QString fieldname = fit->name();

      QgsDebugMsg("Checking field against: " + fieldname);

      if( fieldname.isEmpty() || fieldname==geometryColumn || fieldname==primaryKey )
        continue;

      int i;
      for(i=1; i<flist.size(); i++)
      {
        const QgsAttributeMap &attributevec = flist[i].attributeMap();

        QgsAttributeMap::const_iterator thisit = attributevec.find( it.key() );
        if( thisit == attributevec.end() )
          break;

        if( *thisit!=*it )
          break;
      }

      insert += "," + quotedIdentifier(fieldname);

      QString defVal = getDefaultValue( it.key() ).toString();

      if( i==flist.size() )
      {
        if( !defVal.isNull() && *it==defVal)
        {
          values += "," + defVal;
        }
        else
        {
          values += "," + quotedValue( it->toString() );
        }
      } 
      else
      {
        // value is not unique => add parameter
        values += QString(",$%1").arg( defaultValue.size()+3 );
        defaultValue.append( defVal );
        fieldId.append( it.key() );
      }
    }

    insert += values + ")";

    PGresult *stmt = PQprepare(connection, "addfeatures", insert.toUtf8(), fieldId.size()+2, NULL);
    if(stmt==0 || PQresultStatus(stmt)==PGRES_FATAL_ERROR)
      throw PGException(stmt);

    PQclear(stmt);

    int primaryKeyHighWater = maxPrimaryKeyValue();
    const char **param = new const char *[ fieldId.size()+2 ];

    for(QgsFeatureList::iterator features=flist.begin(); features!=flist.end(); features++)
    {
      const QgsAttributeMap &attributevec = features->attributeMap();

      QString geomParam;
      appendGeomString( features->geometry(), geomParam);

      QList<QByteArray> qparam;

      qparam.append( geomParam.toUtf8() );
      qparam.append( QString("%1").arg( ++primaryKeyHighWater ).toUtf8() );

      param[0] = qparam[0];
      param[1] = qparam[1];

      for(int i=0; i<fieldId.size(); i++)
      {
        qparam.append( paramValue( attributevec[ fieldId[i] ].toString(), defaultValue[i] ) );
        if( qparam[i+2].isNull() )
          param[i+2] = 0;
        else
          param[i+2] = qparam[i+2];
      }

      PGresult *result = PQexecPrepared(connection, "addfeatures", fieldId.size()+2, param, NULL, NULL, 0);
      if( result==0 || PQresultStatus(result)==PGRES_FATAL_ERROR )
      {
        delete param;
        throw PGException(result);
      }

      PQclear(result);
    }

    PQexecNR(connection,QString("COMMIT").toUtf8());
    PQexecNR(connection,QString("DEALLOCATE addfeatures").toUtf8());
    delete param;
  } catch(PGException &e) {
    e.showErrorMessage( tr("Error while adding features") );
    PQexecNR(connection,QString("ROLLBACK").toUtf8());

    PQexecNR(connection,QString("DEALLOCATE addfeatures").toUtf8());
    returnvalue = false;
  }

  reset();
  return returnvalue;
}

bool QgsPostgresProvider::deleteFeatures(const QgsFeatureIds & id)
{
  bool returnvalue=true;

  try {
    PQexecNR(connection,QString("BEGIN").toUtf8());

    for(QgsFeatureIds::const_iterator it=id.begin();it!=id.end();++it) {
      QString sql("DELETE FROM "+mSchemaTableName+" WHERE "+quotedIdentifier(primaryKey)+"="+QString::number(*it));

      QgsDebugMsg("delete sql: "+sql);

      //send DELETE statement and do error handling
      PGresult* result=PQexec(connection, sql.toUtf8());
      if( result==0 || PQresultStatus(result)==PGRES_FATAL_ERROR )
        throw PGException(result);
      PQclear(result);
    }
    
    PQexecNR(connection,QString("COMMIT").toUtf8());
  } catch(PGException &e) {
    e.showErrorMessage( tr("Error while deleting features") );
    PQexecNR(connection,QString("ROLLBACK").toUtf8());
    returnvalue = false;
  }
  reset();
  return returnvalue;
}

bool QgsPostgresProvider::addAttributes(const QgsNewAttributesMap & name)
{
  bool returnvalue=true;

  try {
    PQexecNR(connection,QString("BEGIN").toUtf8());

    for(QgsNewAttributesMap::const_iterator iter=name.begin();iter!=name.end();++iter)
    {
      QString sql="ALTER TABLE "+mSchemaTableName+" ADD COLUMN "+quotedIdentifier(iter.key())+" " +iter.value();

      QgsDebugMsg(sql);

      //send sql statement and do error handling
      PGresult* result=PQexec(connection, sql.toUtf8());
      if( result==0 || PQresultStatus(result)==PGRES_FATAL_ERROR )
        throw PGException(result);
      PQclear(result);
    }

    PQexecNR(connection,QString("COMMIT").toUtf8());
  } catch(PGException &e) {
    e.showErrorMessage( tr("Error while adding attributes") );
    PQexecNR(connection,QString("ROLLBACK").toUtf8());
    returnvalue = false;
  }

  reset();
  return returnvalue;
}

bool QgsPostgresProvider::deleteAttributes(const QgsAttributeIds& ids)
{
  bool returnvalue=true;

  try {
    PQexecNR(connection,QString("BEGIN").toUtf8());

    for(QgsAttributeIds::const_iterator iter=ids.begin();iter != ids.end();++iter)
    {
      QgsFieldMap::const_iterator field_it = attributeFields.find(*iter);
      if(field_it == attributeFields.constEnd())
        continue;
    
      QString column = field_it->name();
      QString sql="ALTER TABLE "+mSchemaTableName+" DROP COLUMN "+quotedIdentifier(column);

      //send sql statement and do error handling
      PGresult* result=PQexec(connection, sql.toUtf8());
      if( result==0 || PQresultStatus(result)==PGRES_FATAL_ERROR )
        throw PGException(result);
      PQclear(result);

      //delete the attribute from attributeFields
      attributeFields.remove(*iter);
    }

    PQexecNR(connection,QString("COMMIT").toUtf8());
  } catch(PGException &e) {
    e.showErrorMessage( tr("Error while deleting attributes") );
    PQexecNR(connection,QString("ROLLBACK").toUtf8());
    returnvalue = false;
  }

  reset();
  return returnvalue;
}

bool QgsPostgresProvider::changeAttributeValues(const QgsChangedAttributesMap & attr_map)
{
  bool returnvalue=true;

  try {
    PQexecNR(connection,QString("BEGIN").toUtf8());

    // cycle through the features
    for(QgsChangedAttributesMap::const_iterator iter=attr_map.begin();iter!=attr_map.end();++iter)
    {
      int fid = iter.key();
      
      // skip added features
      if(fid<0)
        continue;

      const QgsAttributeMap& attrs = iter.value();

      // cycle through the changed attributes of the feature
      for(QgsAttributeMap::const_iterator siter = attrs.begin(); siter != attrs.end(); ++siter)
      {
        QString fieldName = attributeFields[siter.key()].name();

        QString sql = QString("UPDATE %1 SET %2=%3 WHERE %4=%5")
                        .arg( mSchemaTableName )
                        .arg( quotedIdentifier(fieldName) )
                        .arg( quotedValue( siter->toString() ) )
                        .arg( quotedIdentifier(primaryKey) )
                        .arg( fid );
        QgsDebugMsg(sql);

        PGresult* result=PQexec(connection, sql.toUtf8());
        if( result==0 || PQresultStatus(result)==PGRES_FATAL_ERROR )
          throw PGException(result);
        PQclear(result);
      }
    }

    PQexecNR(connection,QString("COMMIT").toUtf8());
  } catch(PGException &e) {
    e.showErrorMessage( tr("Error while changing attributes") );
    PQexecNR(connection,QString("ROLLBACK").toUtf8());
    returnvalue = false;
  }

  reset();

  return returnvalue;
}

void QgsPostgresProvider::appendGeomString(QgsGeometry *geom, QString &geomString) const
{
  unsigned char *buf = geom->wkbBuffer();
  for (uint i=0; i < geom->wkbSize(); ++i)
  {
    if (useWkbHex)
      geomString += QString("%1").arg( (int) buf[i], 2, 16, QChar('0'));
    else
      geomString += QString("\\%1").arg( (int) buf[i], 3, 8, QChar('0'));
  }
}

bool QgsPostgresProvider::changeGeometryValues(QgsGeometryMap & geometry_map)
{
  QgsDebugMsg("entering.");

  bool returnvalue = true;

  try {
    // Start the PostGIS transaction
    PQexecNR(connection,QString("BEGIN").toUtf8());

    for(QgsGeometryMap::iterator iter  = geometry_map.begin();
      iter != geometry_map.end();
      ++iter)
    {

      QgsDebugMsg("iterating over the map of changed geometries...");

      if (iter->wkbBuffer())
      {
        QgsDebugMsg("iterating over feature id " + QString::number(iter.key()));

        QString sql = QString("UPDATE %1 SET %2=GeomFromWKB('")
                        .arg( mSchemaTableName )
                        .arg( quotedIdentifier(geometryColumn) );
        appendGeomString(&*iter, sql);
        sql += QString("'%1,%2) WHERE %3=%4")
                 .arg( useWkbHex ? "::bytea" : "" )
                 .arg( srid )
                 .arg( quotedIdentifier(primaryKey) )
                 .arg( iter.key() );

        QgsDebugMsg("Updating with: " + sql);

        PGresult* result=PQexec(connection, sql.toUtf8());
        if( result==0 || PQresultStatus(result)==PGRES_FATAL_ERROR )
          throw PGException(result);
        PQclear(result);
      } // if (*iter)
    } // for each feature

    PQexecNR(connection,QString("COMMIT").toUtf8());
  } catch(PGException &e) {
    e.showErrorMessage( tr("Error while changing attributes") );
    PQexecNR(connection,QString("ROLLBACK").toUtf8());
    returnvalue = false;
  }

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
  return enabledCapabilities;
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

  PGresult *result = PQexec(connection, sql.toUtf8());

  QgsDebugMsg("Approximate Number of features as text: " +
      QString::fromUtf8(PQgetvalue(result, 0, 0)));

  numberFeatures = QString::fromUtf8(PQgetvalue(result, 0, 0)).toLong();
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

  QString sql = "select box3d(" + quotedIdentifier(geometryColumn) + ") from " 
    + mSchemaTableName + " where ";

  if(sqlWhereClause.length() > 0)
  {
    sql += "(" + sqlWhereClause + ") and ";
  }

  sql += "not IsEmpty(" + quotedIdentifier(geometryColumn) + ") limit 5";


#if WASTE_TIME
  sql = "select xmax(extent(" + quotedIdentifier(geometryColumn) + ")) as xmax,"
    "xmin(extent(" + quotedIdentifier(geometryColumn) + ")) as xmin,"
    "ymax(extent(" + quotedIdentifier(geometryColumn) + ")) as ymax," 
    "ymin(extent(" + quotedIdentifier(geometryColumn) + ")) as ymin" 
    " from " + mSchemaTableName;
#endif

  QgsDebugMsg("Getting approximate extent using: '" + sql + "'");

  PGresult *result = PQexec(connection, sql.toUtf8());

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

  QString sql = "select extent(" + quotedIdentifier(geometryColumn) + ") from " + 
    mSchemaTableName;
  if(sqlWhereClause.length() > 0)
  {
    sql += " where " + sqlWhereClause;
  }

#if WASTE_TIME
  sql = "select xmax(extent(" + quotedIdentifier(geometryColumn) + ")) as xmax,"
    "xmin(extent(" + quotedIdentifier(geometryColumn) + ")) as xmin,"
    "ymax(extent(" + quotedIdentifier(geometryColumn) + ")) as ymax," 
    "ymin(extent(" + quotedIdentifier(geometryColumn) + ")) as ymin" 
    " from " + mSchemaTableName;
#endif

  QgsDebugMsg("Getting extents using schema.table: " + sql);

  PGresult *result = PQexec(connection, sql.toUtf8());
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

  QString firstOid = "select regclass('" + mSchemaTableName + "')::oid";
  PGresult * oidResult = PQexec(connection, firstOid.toUtf8());
  // get the int value from a "normal" select
  QString oidValue = QString::fromUtf8(PQgetvalue(oidResult,0,0));
  PQclear(oidResult);

  QgsDebugMsg("Creating binary cursor");

  // get the same value using a binary cursor

  PQexecNR(connection,QString("begin work").toUtf8());
  QString oidDeclare = "declare oidcursor binary cursor for select regclass('" + mSchemaTableName + "')::oid";
  // set up the cursor
  PQexecNR(connection, oidDeclare.toUtf8());
  QString fetch = "fetch forward 1 from oidcursor";

  QgsDebugMsg("Fetching a record and attempting to get check endian-ness");

  PGresult *fResult = PQexec(connection, fetch.toUtf8());
  PQexecNR(connection, QString("end work").toUtf8());
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
    srid = QString::fromUtf8(PQgetvalue(result, 0, PQfnumber(result, QString("srid").toUtf8())));
    fType = QString::fromUtf8(PQgetvalue(result, 0, PQfnumber(result, QString("type").toUtf8())));
    PQclear(result);
  }
  else
  {
    // Didn't find what we need in the geometry_columns table, so
    // get stuff from the relevant column instead. This may (will?) 
    // fail if there is no data in the relevant table.
    PQclear(result); // for the query just before the if() statement
    sql = "select "
      "srid(" + quotedIdentifier(geometryColumn) + "), "
      "geometrytype(" + quotedIdentifier(geometryColumn) + ") from " + 
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
      srid = QString::fromUtf8(PQgetvalue(result, 0, PQfnumber(result, QString("srid").toUtf8())));
      fType = QString::fromUtf8(PQgetvalue(result, 0, PQfnumber(result, QString("geometrytype").toUtf8())));
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
          "from %2").arg(quotedIdentifier(geometryColumn)).arg(mSchemaTableName);
      if(mUri.sql()!="")
        sql += " where " + mUri.sql();

      result = executeDbCommand(connection, sql);

      if (PQntuples(result)==1)
      {
        fType = QString::fromUtf8(PQgetvalue(result, 0, 0));
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
  PGresult *result = PQexec(connection, sql.toUtf8());

  QgsDebugMsg("Executed SQL: " + sql);
  if (PQresultStatus(result) == PGRES_TUPLES_OK) {
    QgsDebugMsg("Command was successful.");
  } else {
    QgsDebugMsg("Command was unsuccessful. The error message was: "
        + QString::fromUtf8( PQresultErrorMessage(result) ) );
  }
  return result;
}

QString QgsPostgresProvider::quotedIdentifier( QString ident )
{
  ident.replace('"', "\"\"");
  return ident.prepend("\"").append("\"");
}

QString QgsPostgresProvider::quotedValue( QString value )
{
  if( value.isNull() )
    return "NULL";
  
  // FIXME: use PQescapeStringConn
  value.replace("'", "''");
  return value.prepend("'").append("'");
}

void QgsPostgresProvider::PQexecNR(PGconn *conn, const char *query)
{
  PGresult *res = PQexec(conn, query);
  if(res)
  {
    QgsDebugMsg( QString("Query: %1 returned %2 [%3]")
                   .arg(query)
                   .arg(PQresStatus(PQresultStatus(res)))
                   .arg(PQresultErrorMessage(res))
      );
    PQclear(res);
  }
  else
  {
    QgsDebugMsg( QString("Query: %1 returned no result buffer").arg(query) );
  }
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
QGISEXTERN bool isProvider()
{
  return true;
}
