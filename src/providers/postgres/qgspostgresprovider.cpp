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

#include <cassert>

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

QMap<QString, QgsPostgresProvider::Conn *> QgsPostgresProvider::connections;
int QgsPostgresProvider::providerIds=0;

QgsPostgresProvider::QgsPostgresProvider(QString const & uri)
: QgsVectorDataProvider(uri),
  geomType(QGis::WKBUnknown),
  mFeatureQueueSize(200),
  gotPostgisVersion(false),
  mFetching(false)
{
  // assume this is a valid layer until we determine otherwise
  valid = true;

  providerId=providerIds++;

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

  /* Check to see if we have GEOS support and if not, warn the user about
     the problems they will see :) */
  QgsDebugMsg("Checking for GEOS support");

  if(!hasGEOS(connection))
  {
    showMessageBox(tr("No GEOS Support!"),
        tr("Your PostGIS installation has no GEOS support.\n"
          "Feature selection and identification will not "
          "work properly.\nPlease install PostGIS with " 
          "GEOS support (http://geos.refractions.net)"));
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
  if( connections.contains(conninfo) )
  {
    QgsDebugMsg( QString("Using cached connection for %1").arg(conninfo) );
    connections[conninfo]->ref++;
    return connections[conninfo]->conn;
  }

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

  QgsDebugMsg("Connection to the database was successful");
  
  Conn *conn = new Conn(pd);
  connections.insert( conninfo, conn );

  return pd;
}

void QgsPostgresProvider::disconnectDb()
{
  if(mFetching) 
  {
    PQexecNR(connection, QString("CLOSE qgisf%1").arg(providerId).toUtf8() );
    mFetching=false;
  }

  QMap<QString, Conn *>::iterator i;
  for(i=connections.begin(); i!=connections.end() && i.value()->conn!=connection; i++)
    ;

  assert( i.value()->conn==connection );
  assert( i.value()->ref>0 );

  if( --i.value()->ref==0 ) {
    PQfinish( connection );
    delete i.value();
    connections.remove( i.key() );
  }
    
  connection = 0;
}

QString QgsPostgresProvider::storageType() const
{
  return "PostgreSQL database with PostGIS extension";
}

bool QgsPostgresProvider::declareCursor(const QString &cursorName,
                                        const QgsAttributeList &fetchAttributes,
                                        bool fetchGeometry,
                                        QString whereClause)
{
  try
  {
    QString declare = QString("declare %1 binary cursor with hold for select %2")
      .arg(cursorName).arg(quotedIdentifier(primaryKey));

    if(fetchGeometry)
    {
      declare += QString(",asbinary(%1,'%2')")
        .arg( quotedIdentifier(geometryColumn) )
        .arg( endianString() ); 
    }

    for (QgsAttributeList::const_iterator it = fetchAttributes.constBegin(); it != fetchAttributes.constEnd(); ++it)
    {
      const QgsField &fld = field(*it);

      const QString &fieldname = fld.name();
      if( fieldname == primaryKey )
        continue;

      const QString &type = fld.typeName();
      if( type == "money" )
      {
        declare += QString(",cash_out(%1)").arg( quotedIdentifier(fieldname) );
      }
      else if( type.startsWith("_") )
      {
        declare += QString(",array_out(%1)").arg( quotedIdentifier(fieldname) );
      }
      else if( type == "bool" )
      {
        declare += QString(",boolout(%1)").arg( quotedIdentifier(fieldname) );
      }
      else
      {
        declare += "," + quotedIdentifier(fieldname) + "::text";
      }
    }

    declare += " from " + mSchemaTableName;

    if( !whereClause.isEmpty() )
      declare += QString(" where %1").arg(whereClause);

    QgsDebugMsg("Binary cursor: " + declare);

    return PQexecNR(connection, declare.toUtf8());
  }
  catch(PGFieldNotFound)
  {
    return false;
  }
}

bool QgsPostgresProvider::getFeature(PGresult *queryResult, int row, bool fetchGeometry,
                                     QgsFeature &feature,
                                     const QgsAttributeList &fetchAttributes)
{  
  try
  {
    int oid = *(int *)PQgetvalue(queryResult, row, 0);
    if (swapEndian)
      oid = ntohl(oid); // convert oid to opposite endian

    feature.setFeatureId(oid);

    int col;  // first attribute column after geometry

    if (fetchGeometry)
    {
      int returnedLength = PQgetlength(queryResult, row, 1); 
      if(returnedLength > 0)
      {
        unsigned char *featureGeom = new unsigned char[returnedLength + 1];
        memset(featureGeom, '\0', returnedLength + 1);
        memcpy(featureGeom, PQgetvalue(queryResult, row, 1), returnedLength); 
        feature.setGeometryAndOwnership(featureGeom, returnedLength + 1);
      }
      else
      {
        feature.setGeometryAndOwnership(0, 0);
        QgsDebugMsg("Couldn't get the feature geometry in binary form");
      }

      col = 2;
    }
    else
    {
      col = 1;
    }

    // iterate attributes
    for(QgsAttributeList::const_iterator it=fetchAttributes.constBegin(); it!=fetchAttributes.constEnd(); it++)
    {
      const QgsField &fld = field(*it);

      if( fld.name() == primaryKey )
      {
        // primary key was already processed
        feature.addAttribute(*it, QString::number(oid) );
        continue;
      }

      if( !PQgetisnull(queryResult, row, col) )
      {
        QString val = QString::fromUtf8(PQgetvalue(queryResult, row, col));

        switch (fld.type())
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
          QgsDebugMsg( QString("feature %1, field %2, value '%3': unexpected variant type %4 considered as NULL")
                         .arg( oid )
                         .arg( fld.name() )
                         .arg( val )
                         .arg( fld.type() ) );
          feature.addAttribute(*it, QVariant(QString::null));
        }
      }
      else
      {
        feature.addAttribute(*it, QVariant(QString::null));
      }

      col++;
    }

    return true;
  }
  catch(PGFieldNotFound)
  {
    return false;
  }
}

void QgsPostgresProvider::select(QgsAttributeList fetchAttributes, QgsRect rect, bool fetchGeometry, bool useIntersect)
{
  QString cursorName = QString("qgisf%1").arg(providerId);
  
  if(mFetching) 
  {
    PQexecNR(connection, QString("CLOSE %1").arg(cursorName).toUtf8() );
    mFetching=false;
  
    while(!mFeatureQueue.empty())
    {
      mFeatureQueue.pop();
    }
  }

  QString whereClause;

  if(!rect.isEmpty())
  {
    if(useIntersect)
    { 
      // Contributed by #qgis irc "creeping"
      // This version actually invokes PostGIS's use of spatial indexes
      whereClause = QString("%1 && setsrid('BOX3D(%2)'::box3d,%3) and intersects(%1,setsrid('BOX3D(%2)'::box3d,%3))")
                      .arg( quotedIdentifier(geometryColumn) )
                      .arg( rect.asWKTCoords() )
                      .arg( srid );
    }
    else
    {
      whereClause = QString("%1 && setsrid('BOX3D(%2)'::box3d,%3)")
                      .arg( quotedIdentifier(geometryColumn) )
                      .arg( rect.asWKTCoords() )
                      .arg( srid );
    }
  }

  if( !sqlWhereClause.isEmpty() )
  {
    if (!whereClause.isEmpty())
      whereClause += " and ";

    whereClause += "(" + sqlWhereClause + ")";
  }

  mFetchGeom = fetchGeometry;
  mAttributesToFetch = fetchAttributes;
  if( !declareCursor( cursorName, fetchAttributes, fetchGeometry, whereClause ) )
    return;
  
  mFetching = true;
}

bool QgsPostgresProvider::getNextFeature(QgsFeature& feature)
{
  QString cursorName = QString("qgisf%1").arg(providerId);

  if (!valid)
  {
    QgsDebugMsg("Read attempt on an invalid postgresql data source");
    return false;
  }

  if( mFeatureQueue.empty() )
  {
    QString fetch = QString("fetch forward %1 from %2").arg(mFeatureQueueSize).arg(cursorName);
    if(PQsendQuery(connection, fetch.toUtf8()) == 0) //fetch features in asynchronously
    {
      qWarning("PQsendQuery failed (1)");
    }

    PGresult *queryResult;
    while( (queryResult = PQgetResult(connection)) )
    {
      int rows = PQntuples(queryResult);
      if (rows == 0)
        continue;

      for (int row = 0; row < rows; row++)
      {
        mFeatureQueue.push(QgsFeature());
        getFeature(queryResult, row, mFetchGeom, mFeatureQueue.back(), mAttributesToFetch);
      } // for each row in queue

      PQclear(queryResult);
    }
  }

  if( mFeatureQueue.empty() )
  {
    QgsDebugMsg("End of features");
    return false;
  }
 
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

  return true;
}

bool QgsPostgresProvider::getFeatureAtId(int featureId, QgsFeature& feature, bool fetchGeometry, QgsAttributeList fetchAttributes)
{
  QString cursorName = QString("qgisfid%1").arg(providerId);
  if( !declareCursor( cursorName, fetchAttributes, fetchGeometry, QString("%2=%3").arg(quotedIdentifier(primaryKey)).arg(featureId) ) )
    return false;

  PGresult *queryResult = PQexec(connection, QString("fetch forward 1 from %1").arg(cursorName).toUtf8());
  if(queryResult==0)
    return false;

  int rows = PQntuples(queryResult);
  if (rows == 0)
  {
    QgsDebugMsg("feature " + QString::number(featureId) + " not found");
    PQclear(queryResult);
    PQexecNR(connection, QString("CLOSE %1").arg(cursorName).toUtf8());
    return false;
  }
  else if(rows != 1)
  {
    QgsDebugMsg( QString("found %1 features instead of just one.").arg(rows) );
  }

  bool gotit = getFeature(queryResult, 0, fetchGeometry, feature, fetchAttributes);

  PQclear(queryResult);

  PQexecNR(connection, QString("CLOSE %1").arg(cursorName).toUtf8());

  return gotit;
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

const QgsField &QgsPostgresProvider::field(int index) const
{
  QgsFieldMap::const_iterator it = attributeFields.find(index);

  if( it==attributeFields.constEnd() ) {
    QgsDebugMsg("Field " + QString::number(index) + " not found.");
    throw PGFieldNotFound();
  }

  return it.value();
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
  if(mFetching)
  {
     //move cursor to first record
    PQexecNR(connection, QString("move 0 in qgisf%1").arg(providerId).toUtf8());
  }
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
  QString sql = QString("SELECT regclass(%1)::oid").arg( quotedValue(mSchemaTableName) );
  PGresult *tresult= PQexec(connection, sql.toUtf8());
  QString tableoid = QString::fromUtf8(PQgetvalue(tresult, 0, 0));
  PQclear(tresult);

  // Get the table description
  sql = QString("SELECT description FROM pg_description WHERE objoid=%1 AND objsubid=0").arg( tableoid );
  tresult = PQexec(connection, sql.toUtf8());
  if (PQntuples(tresult) > 0)
    mDataComment = QString::fromUtf8(PQgetvalue(tresult, 0, 0));
  PQclear(tresult);

  // Populate the field vector for this layer. The field vector contains
  // field name, type, length, and precision (if numeric)
  sql = QString("select * from %1 limit 0").arg ( mSchemaTableName );

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

    sql = QString("SELECT typname,typlen FROM pg_type WHERE oid=%1").arg(typOid);
    // just oid; needs more work to support array type
    //      "oid = (SELECT Distinct typelem FROM pg_type WHERE "	//needs DISTINCT to guard against 2 or more rows on int2
    //      "typelem = " + typOid + " AND typlen = -1)";

    PGresult* oidResult = PQexec(connection, sql.toUtf8());
    QString fieldTypeName = QString::fromUtf8(PQgetvalue(oidResult, 0, 0));
    QString fieldSize = QString::fromUtf8(PQgetvalue(oidResult, 0, 1));
    PQclear(oidResult);

    sql = QString("SELECT attnum FROM pg_attribute WHERE attrelid=%1 AND attname=%2")
            .arg( tableoid ).arg( quotedValue(fieldName) );
      
    PGresult *tresult = PQexec(connection, sql.toUtf8());
    QString attnum = QString::fromUtf8(PQgetvalue(tresult, 0, 0));
    PQclear(tresult);

    sql = QString("SELECT description FROM pg_description WHERE objoid=%1 AND objsubid=%2")
            .arg( tableoid ).arg( attnum );

    tresult = PQexec(connection, sql.toUtf8());
    if (PQntuples(tresult) > 0)
      fieldComment = QString::fromUtf8(PQgetvalue(tresult, 0, 0));
    PQclear(tresult);

    if(fieldName!=geometryColumn)
    {
      QVariant::Type fieldType;
      bool isArray = fieldTypeName.startsWith("_");
 
      if(isArray)
        fieldTypeName = fieldTypeName.mid(1);

      if (fieldTypeName == "int8" )
        fieldType = QVariant::LongLong;
      else if (fieldTypeName.startsWith("int") || fieldTypeName=="serial" )
        fieldType = QVariant::Int;
      else if (fieldTypeName == "real" || fieldTypeName == "double precision" || fieldTypeName.startsWith("float") || fieldTypeName == "numeric" )
        fieldType = QVariant::Double;
      else if (fieldTypeName == "text" || 
               fieldTypeName == "char" ||
               fieldTypeName == "bpchar" || 
               fieldTypeName == "varchar" ||
               fieldTypeName == "bool" ||
               fieldTypeName == "money" ||
               fieldTypeName.startsWith("time") ||
               fieldTypeName.startsWith("date") )
        fieldType = QVariant::String;
      else
      {
        QgsDebugMsg( "Field " + fieldName + " ignored, because of unsupported type " + fieldTypeName);
        continue;
      }

      if(isArray)
      {
        fieldTypeName = "_" + fieldTypeName;
        fieldType = QVariant::String;
      }
        
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

  QString sql = QString("select indkey from pg_index where indisunique='t' and indrelid=regclass(%1)::oid")
                  .arg( quotedValue(mSchemaTableName) );

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

    sql = QString("SELECT relkind FROM pg_class WHERE oid=regclass(%1)::oid")
            .arg( quotedValue(mSchemaTableName) );
    PGresult* tableType = executeDbCommand(connection, sql);
    QString type = QString::fromUtf8(PQgetvalue(tableType, 0, 0));
    PQclear(tableType);

    primaryKey = "";

    if (type == "r") // the relation is a table
    {
      QgsDebugMsg("Relation is a table. Checking to see if it has an oid column.");

      // If there is an oid on the table, use that instead,
      // otherwise give up
      sql = QString("SELECT attname FROM pg_attribute WHERE attname='oid' AND attrelid=regclass(%1)")
              .arg( quotedValue(mSchemaTableName) );

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
      QStringList columns = col.split(" ", QString::SkipEmptyParts);
      if (columns.count() == 1)
      {
        // Get the column name and data type
        sql = QString("select attname,pg_type.typname from pg_attribute,pg_type where atttypid=pg_type.oid and attnum=%1 and attrelid=regclass(%2)")
                .arg( col ).arg( quotedValue(mSchemaTableName) );
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
        sql = QString("select attname from pg_attribute, pg_type where atttypid=pg_type.oid and attnum in (%1) and attrelid=regclass(%2)::oid")
                .arg( col.replace(" ", ",") )
                .arg( quotedValue(mSchemaTableName) );

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
      sql = QString("select attname from pg_attribute where attname='oid' and attrelid=regclass(%1)::oid").arg( quotedValue(mSchemaTableName) );
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
    sql = QString("select regclass(%1)::oid").arg( quotedValue( quotedIdentifier(schemaName) + "." + quotedIdentifier(tableName) ) );
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
      PQclear(result);
      continue;
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
    sql = QString("select * from pg_constraint where "
                  "conkey[1]=(select attnum from pg_attribute where attname=%1 and attrelid=%2) "
                  "and conrelid=%2 and (contype='p' or contype='u') "
                  "and array_dims(conkey)='[1:1]'").arg( quotedValue(tableCol) ).arg( rel_oid );

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
    sql = QString( "select * from pg_index where indrelid=%1 and indkey[0]=(select attnum from pg_attribute where attrelid=%1 and attname=%2)")
                    .arg( rel_oid )
                    .arg( quotedValue( i->second.column ) );
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

  QString sql = QString("select count(distinct %1)=count(%1) from %2.%3") 
                  .arg( quotedIdentifier(colName) )
                  .arg( quotedIdentifier(schemaName) )
                  .arg( quotedIdentifier(tableName) );

  PGresult* unique = PQexec(connection, sql.toUtf8());

  if (PQntuples(unique)==1 && QString::fromUtf8(PQgetvalue(unique, 0, 0)).startsWith("t"))
    isUnique = true;

  PQclear(unique);

  return isUnique;
}

int QgsPostgresProvider::SRCFromViewColumn(const QString& ns, const QString& relname, const QString& attname_table, const QString& attname_view, const QString& viewDefinition, SRC& result) const
{
  QString newViewDefSql = QString("SELECT definition FROM pg_views WHERE schemaname=%1 AND viewname=%2")
                            .arg( quotedValue(ns) ).arg( quotedValue(relname) );
  PGresult* newViewDefResult = PQexec(connection, newViewDefSql.toUtf8());
  int numEntries = PQntuples(newViewDefResult);

  if(numEntries > 0) //relation is a view
  {
    QString newViewDefinition(QString::fromUtf8(PQgetvalue(newViewDefResult, 0, 0)));

    QString newAttNameView = attname_table;
    QString newAttNameTable = attname_table;

    //find out the attribute name of the underlying table/view
    if (newViewDefinition.contains(" AS "))
    {
      QRegExp s("(\\w+)" + QString(" AS ") + QRegExp::escape(attname_table));
      if (s.indexIn(newViewDefinition) != -1)
      {
        newAttNameTable = s.cap(1);
      }
    }

    QString viewColumnSql =
      QString("SELECT "
                 "table_schema,"
                 "table_name,"
                 "column_name"
              " FROM "
                "("
                  "SELECT DISTINCT "
                    "current_database()::information_schema.sql_identifier AS view_catalog,"
                    "nv.nspname::information_schema.sql_identifier AS view_schema,"
                    "v.relname::information_schema.sql_identifier AS view_name,"
                    "current_database()::information_schema.sql_identifier AS table_catalog,"
                    "nt.nspname::information_schema.sql_identifier AS table_schema,"
                    "t.relname::information_schema.sql_identifier AS table_name,"
                    "a.attname::information_schema.sql_identifier AS column_name"
                  " FROM "
                    "pg_namespace nv,"
                    "pg_class v,"
                    "pg_depend dv,"
                    "pg_depend dt,"
                    "pg_class t,"
                    "pg_namespace nt,"
                    "pg_attribute a"
                  " WHERE "
                    "nv.oid=v.relnamespace AND "
                    "v.relkind='v'::\"char\" AND "
                    "v.oid=dv.refobjid AND "
                    "dv.refclassid='pg_class'::regclass::oid AND "
                    "dv.classid='pg_rewrite'::regclass::oid AND "
                    "dv.deptype='i'::\"char\" AND "
                    "dv.objid = dt.objid AND "
                    "dv.refobjid<>dt.refobjid AND "
                    "dt.classid='pg_rewrite'::regclass::oid AND "
                    "dt.refclassid='pg_class'::regclass::oid AND "
                    "dt.refobjid=t.oid AND "
                    "t.relnamespace = nt.oid AND "
                    "(t.relkind=ANY (ARRAY['r'::\"char\", 'v'::\"char\"])) AND "
                    "t.oid=a.attrelid AND "
                    "dt.refobjsubid=a.attnum"
                  " ORDER BY "
                    "current_database()::information_schema.sql_identifier,"
                    "nv.nspname::information_schema.sql_identifier,"
                    "v.relname::information_schema.sql_identifier,"
                    "current_database()::information_schema.sql_identifier,"
                    "nt.nspname::information_schema.sql_identifier,"
                    "t.relname::information_schema.sql_identifier,"
                    "a.attname::information_schema.sql_identifier"
                ") x"
               " WHERE "
                 "view_schema=%1 AND "
                 "view_name=%2 AND "
                 "column_name=%3")
             .arg( quotedValue(ns) )
             .arg( quotedValue(relname) )
             .arg( quotedValue(newAttNameTable) );

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
  QString typeSql = QString("SELECT "
                              "pg_type.typname"
                            " FROM "
                              "pg_attribute,"
                              "pg_class,"
                              "pg_namespace,"
                              "pg_type"
                            " WHERE "
                              "pg_class.relname=%1 AND "
                              "pg_namespace.nspname=%2 AND "
                              "pg_attribute.attname=%3 AND "
                              "pg_attribute.attrelid=pg_class.oid AND "
                              "pg_class.relnamespace=pg_namespace.oid AND "
                              "pg_attribute.atttypid=pg_type.oid")
                      .arg( quotedValue(relname ) )
                      .arg( quotedValue(ns) )
                      .arg( quotedValue(attname_table) );
  
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
  QString viewColumnSql =
    QString("SELECT "
              "table_schema,"
              "table_name,"
              "column_name"
            " FROM "
              "("
                "SELECT DISTINCT "
                  "current_database() AS view_catalog,"
                  "nv.nspname AS view_schema,"
                  "v.relname AS view_name,"
                  "current_database() AS table_catalog,"
                  "nt.nspname AS table_schema,"
                  "t.relname AS table_name,"
                  "a.attname AS column_name"
                " FROM "
                  "pg_namespace nv,"
                  "pg_class v,"
                  "pg_depend dv,"
                  "pg_depend dt,"
                  "pg_class t,"
                  "pg_namespace nt,"
                  "pg_attribute a"
                " WHERE "
                  "nv.oid=v.relnamespace AND "
                  "v.relkind='v'::\"char\" AND "
                  "v.oid=dv.refobjid AND "
                  "dv.refclassid='pg_class'::regclass::oid AND "
                  "dv.classid='pg_rewrite'::regclass::oid AND "
                  "dv.deptype='i'::\"char\" AND "
                  "dv.objid=dt.objid AND "
                  "dv.refobjid<>dt.refobjid AND "
                  "dt.classid='pg_rewrite'::regclass::oid AND "
                  "dt.refclassid='pg_class'::regclass::oid AND "
                  "dt.refobjid=t.oid AND "
                  "t.relnamespace=nt.oid AND "
                  "(t.relkind = ANY (ARRAY['r'::\"char\",'v'::\"char\"])) AND "
                  "t.oid=a.attrelid AND "
                  "dt.refobjsubid=a.attnum"
                " ORDER BY "
                  "current_database(),"
                  "nv.nspname,"
                  "v.relname,"
                  "current_database(),"
                  "nt.nspname,"
                  "t.relname,"
                  "a.attname"
                ") x"
              " WHERE "
                "view_schema=%1 AND view_name=%2")
              .arg( quotedValue(mSchemaName) )
              .arg( quotedValue(mTableName) );
  PGresult* viewColumnResult = PQexec(connection, viewColumnSql.toUtf8());

  //find out view definition
  QString viewDefSql = QString("SELECT definition FROM pg_views WHERE schemaname=%1 AND viewname=%2")
                         .arg( quotedValue( mSchemaName ) )
                         .arg( quotedValue( mTableName ) );
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
    if (viewDefinition.contains(" AS "))
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
        qWarning("original view column name was: " + attname_view.toUtf8());
      }
    }

    SRCFromViewColumn(ns, relname, attname_table, attname_view, viewDefinition, columnInformation);
    cols.insert(std::make_pair(attname_view, columnInformation));
    QgsDebugMsg("Inserting into cols (for key " + attname_view + " ): " + columnInformation.schema + "." + columnInformation.relation + "." + columnInformation.column + "." + columnInformation.type);
  }
  PQclear(viewColumnResult);

}

// Returns the minimum value of an attribute
QVariant QgsPostgresProvider::minValue(int index)
{
  try
  {
    // get the field name 
    const QgsField &fld = field(index);
    QString sql;
    if(sqlWhereClause.isEmpty())
    {
      sql = QString("select min(%1) from %2")
        .arg(quotedIdentifier(fld.name()))
        .arg(mSchemaTableName);
    }
    else
    {
      sql = QString("select min(%1) from %2 where %3")
        .arg(quotedIdentifier(fld.name()))
        .arg(mSchemaTableName)
        .arg(sqlWhereClause);
    }
    PGresult *rmin = PQexec(connection, sql.toUtf8());
    QString minValue = QString::fromUtf8(PQgetvalue(rmin,0,0));
    PQclear(rmin);
    return minValue.toDouble();
  }
  catch(PGFieldNotFound)
  {
    return QVariant(QString::null);
  }
}

// Returns the list of unique values of an attribute
void QgsPostgresProvider::getUniqueValues(int index, QStringList &uniqueValues)
{
  uniqueValues.clear();

  try
  {
    // get the field name 
    const QgsField &fld = field(index);
    QString sql;
    if(sqlWhereClause.isEmpty())
    {
      sql = QString("select distinct %1 from %2 order by %1")
        .arg(quotedIdentifier(fld.name()))
        .arg(mSchemaTableName);
    }
    else
    {
      sql = QString("select distinct %1 from %2 where %3 order by %1")
        .arg(quotedIdentifier(fld.name()))
        .arg(mSchemaTableName)
        .arg(sqlWhereClause);
    }

    PGresult *res= PQexec(connection, sql.toUtf8());
    if (PQresultStatus(res) == PGRES_TUPLES_OK)
    {
      for(int i=0; i<PQntuples(res); i++)
        uniqueValues.append( QString::fromUtf8(PQgetvalue(res,i,0)) );
    }
    PQclear(res);
  }
  catch(PGFieldNotFound)
  { 
  }
}

// Returns the maximum value of an attribute

QVariant QgsPostgresProvider::maxValue(int index)
{
  try
  {
    // get the field name 
    const QgsField &fld = field(index);
    QString sql;
    if(sqlWhereClause.isEmpty())
    {
      sql = QString("select max(%1) from %2")
        .arg(quotedIdentifier(fld.name()))
        .arg(mSchemaTableName);
    }
    else
    {
      sql = QString("select max(%1) from %2 where %3")
        .arg(quotedIdentifier(fld.name()))
        .arg(mSchemaTableName)
        .arg(sqlWhereClause);
    } 
    PGresult *rmax = PQexec(connection, sql.toUtf8());
    QString maxValue = QString::fromUtf8(PQgetvalue(rmax,0,0));
    PQclear(rmax);
    return maxValue.toDouble();
  }
  catch(PGFieldNotFound)
  {
    return QVariant(QString::null);
  }
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
  try
  {
    // Get the default column value from the Postgres information
    // schema. If there is no default we return an empty string.

    // Maintaining a cache of the results of this query would be quite
    // simple and if this query is called lots, could save some time.
    QString fieldName = field(fieldId).name();

    QString sql("SELECT column_default FROM"
      " information_schema.columns WHERE"
      " column_default IS NOT NULL"
      " AND table_schema = " + quotedValue(mSchemaName) +
      " AND table_name = " + quotedValue(mTableName) +
      " AND column_name = " + quotedValue(fieldName) );

    QVariant defaultValue(QString::null);

    PGresult* result = PQexec(connection, sql.toUtf8());

    if (PQntuples(result)==1 && !PQgetisnull(result, 0, 0) )
      defaultValue = QString::fromUtf8(PQgetvalue(result, 0, 0));

    // QgsDebugMsg( QString("defaultValue for %1 is NULL: %2").arg(fieldId).arg( defaultValue.isNull() ) );

    PQclear(result);

    return defaultValue;
  }
  catch(PGFieldNotFound)
  {
    return QVariant(QString::null);
  }
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

  QStringList postgisParts = postgisVersionInfo.split(" ", QString::SkipEmptyParts);

  // Get major and minor version
  QStringList postgisVersionParts = postgisParts[0].split(".", QString::SkipEmptyParts);

  postgisVersionMajor = postgisVersionParts[0].toInt();
  postgisVersionMinor = postgisVersionParts[1].toInt();

  // assume no capabilities
  geosAvailable = false;
  gistAvailable = false;
  projAvailable = false;

  // parse out the capabilities and store them
  QStringList geos = postgisParts.filter("GEOS");
  if(geos.size() == 1) {
    geosAvailable = (geos[0].indexOf("=1") > -1);  
  }
  QStringList gist = postgisParts.filter("STATS");
  if(gist.size() == 1) {
    gistAvailable = (geos[0].indexOf("=1") > -1);
  }
  QStringList proj = postgisParts.filter("PROJ");
  if(proj.size() == 1) {
    projAvailable = (proj[0].indexOf("=1") > -1);
  }

  useWkbHex = postgisVersionMajor < 1;

  gotPostgisVersion = true;

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
        if( *it==defVal )
        {
          if( defVal.isNull() ) {
            values += ",NULL";
          } else {
            values += "," + defVal;
          }
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

    QgsDebugMsg( QString("prepare addfeatures: %1").arg(insert) );
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
      features->setFeatureId( primaryKeyHighWater );
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
      QString sql = QString("DELETE FROM %1 WHERE %2=%3")
                      .arg(mSchemaTableName)
                      .arg(quotedIdentifier(primaryKey))
                      .arg(*it);
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
      QString sql = QString("ALTER TABLE %1 ADD COLUMN %2 %3")
                      .arg(mSchemaTableName)
                      .arg(quotedIdentifier(iter.key()))
                      .arg(iter.value());
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
      QString sql = QString("ALTER TABLE %1 DROP COLUMN %2")
                      .arg(mSchemaTableName)
                      .arg(quotedIdentifier(column));

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

      QString sql = QString("UPDATE %1 SET ").arg( mSchemaTableName );
      bool first = true;

      const QgsAttributeMap& attrs = iter.value();

      // cycle through the changed attributes of the feature
      for(QgsAttributeMap::const_iterator siter = attrs.begin(); siter != attrs.end(); ++siter)
      {
        try {
          QString fieldName = field(siter.key()).name();

          if(!first)
            sql += ",";
          else
            first=false;

          sql += QString("%1=%2")
                   .arg( quotedIdentifier(fieldName) )
                   .arg( quotedValue( siter->toString() ) );
        }
        catch(PGFieldNotFound)
        {
          // Field was missing - shouldn't happen
        }
      }

      sql += QString(" WHERE %1=%2")
               .arg( quotedIdentifier(primaryKey) )
               .arg( fid );
      
      PGresult* result=PQexec(connection, sql.toUtf8());
      if( result==0 || PQresultStatus(result)==PGRES_FATAL_ERROR )
        throw PGException(result);
         
      PQclear(result);
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

    QString update = QString("UPDATE %1 SET %2=GeomFromWKB($1%3,%4) WHERE %5=$2")
                       .arg( mSchemaTableName )
                       .arg( quotedIdentifier(geometryColumn) )
                       .arg( useWkbHex ? "" : "::bytea" )
                       .arg( srid )
                       .arg( quotedIdentifier(primaryKey) );

    PGresult *stmt = PQprepare(connection, "updatefeatures", update.toUtf8(), 2, NULL);
    if(stmt==0 || PQresultStatus(stmt)==PGRES_FATAL_ERROR)
      throw PGException(stmt);

    PQclear(stmt);

    for(QgsGeometryMap::iterator iter  = geometry_map.begin();
      iter != geometry_map.end();
      ++iter)
    {

      QgsDebugMsg("iterating over the map of changed geometries...");

      if (iter->wkbBuffer())
      {
        QgsDebugMsg("iterating over feature id " + QString::number(iter.key()));

        QString geomParam;
        appendGeomString(&*iter, geomParam);

        QList<QByteArray> qparam;
        qparam.append( geomParam.toUtf8() );
        qparam.append( QString("%1").arg( iter.key() ).toUtf8() );

        const char *param[2];
        param[0] = qparam[0];
        param[1] = qparam[1];

        PGresult *result = PQexecPrepared(connection, "updatefeatures", 2, param, NULL, NULL, 0);
        if( result==0 || PQresultStatus(result)==PGRES_FATAL_ERROR )
          throw PGException(result);

        PQclear(result);
      } // if (*iter)

    } // for each feature

    PQexecNR(connection,QString("COMMIT").toUtf8());
    PQexecNR(connection,QString("DEALLOCATE updatefeatures").toUtf8());
  } catch(PGException &e) {
    e.showErrorMessage( tr("Error while changing geometry values") );
    PQexecNR(connection,QString("ROLLBACK").toUtf8());
    PQexecNR(connection,QString("DEALLOCATE updatefeatures").toUtf8());
    returnvalue = false;
  }

  reset();

  QgsDebugMsg("exiting.");

  return returnvalue;
}

QgsAttributeList QgsPostgresProvider::allAttributesList()
{
  QgsAttributeList attributes;
  for(QgsFieldMap::const_iterator it = attributeFields.constBegin(); it != attributeFields.constEnd(); ++it)
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
  QString sql = QString("select reltuples from pg_catalog.pg_class where relname=%1").arg( quotedValue(tableName) );
  QgsDebugMsg("Running SQL: " + sql);
#else                 
  QString sql = QString("select count(*) from %1").arg( mSchemaTableName );

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
  // get the approximate extent by retrieving the bounding box
  // of the first few items with a geometry

  QString sql = QString("select box3d(%1) from %2 where ")
                  .arg( quotedIdentifier(geometryColumn) )
                  .arg( mSchemaTableName );

  if(sqlWhereClause.length() > 0)
  {
    sql += QString("(%1) and ").arg( sqlWhereClause );
  }

  sql += QString("not IsEmpty(%1) limit 5").arg( quotedIdentifier(geometryColumn) );

#if WASTE_TIME
  sql = QString("select "
                  "xmax(extent(%1)) as xmax,"
                  "xmin(extent(%1)) as xmin,"
                  "ymax(extent(%1)) as ymax,"
                  "ymin(extent(%1)) as ymin" 
                " from %2").arg( quotedIdentifier(geometryColumn).arg( mSchemaTableName );
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

  QString sql = QString("select extent(%1) from %2")
                  .arg( quotedIdentifier(geometryColumn) )
                  .arg( mSchemaTableName );

  if(sqlWhereClause.length() > 0)
  {
    sql += " where " + sqlWhereClause;
  }

#if WASTE_TIME
  sql = QString("select "
                  "xmax(extent(%1)) as xmax,"
                  "xmin(extent(%1)) as xmin,"
                  "ymax(extent(%1)) as ymax," 
                  "ymin(extent(%1)) as ymin" 
                " from %2").arg( quotedIdentifier(geometryColumn) ).arg( mSchemaTableName );
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
void QgsPostgresProvider::customEvent( QEvent * e )
{
  QgsDebugMsg("received a custom event " + QString::number(e->type()));

  switch ( e->type() )
  {
    case (QEvent::Type) QGis::ProviderExtentCalcEvent:

      QgsDebugMsg("extent has been calculated");

      // Collect the new extent from the event and set this layer's
      // extent with it.

      {
        QgsRect* r = ((QgsProviderExtentCalcEvent*) e)->layerExtent();
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

  QString firstOid = QString("select regclass(%1)::oid").arg( quotedValue(mSchemaTableName) );
  PGresult * oidResult = PQexec(connection, firstOid.toUtf8());
  // get the int value from a "normal" select
  QString oidValue = QString::fromUtf8(PQgetvalue(oidResult,0,0));
  PQclear(oidResult);

  QgsDebugMsg("Creating binary cursor");

  // get the same value using a binary cursor

  QString oidDeclare = QString("declare oidcursor binary cursor with hold for select regclass(%1)::oid").arg( quotedValue(mSchemaTableName) );
  // set up the cursor
  PQexecNR(connection, oidDeclare.toUtf8());
  QString fetch = "fetch forward 1 from oidcursor";

  QgsDebugMsg("Fetching a record and attempting to get check endian-ness");

  PGresult *fResult = PQexec(connection, fetch.toUtf8());
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
  PQexecNR(connection, QString("close oidcursor").toUtf8());
  return swapEndian;
}

bool QgsPostgresProvider::getGeometryDetails()
{
  QString fType("");
  srid = "";
  valid = false;
  QStringList log;

  QString sql = QString("select type,srid from geometry_columns"
                        " where f_table_name=%1 and f_geometry_column=%2 and f_table_schema=%3")
                  .arg( quotedValue(mTableName) )
                  .arg( quotedValue(geometryColumn) )
                  .arg( quotedValue(mSchemaName) );

  QgsDebugMsg("Getting geometry column: " + sql);

  PGresult *result = executeDbCommand(connection, sql);

  QgsDebugMsg("geometry column query returned " + QString::number(PQntuples(result)));

  if (PQntuples(result) > 0)
  {
    fType = QString::fromUtf8(PQgetvalue(result, 0, 0));
    srid = QString::fromUtf8(PQgetvalue(result, 0, 1));
    PQclear(result);
  }
  else
  {
    // Didn't find what we need in the geometry_columns table, so
    // get stuff from the relevant column instead. This may (will?) 
    // fail if there is no data in the relevant table.
    PQclear(result); // for the query just before the if() statement
    sql = QString("select srid(%1),geometrytype(%1) from %2" )
            .arg( quotedIdentifier(geometryColumn) )
            .arg( mSchemaTableName );

    //it is possible that the where clause restricts the feature type
    if(!sqlWhereClause.isEmpty())
    {
      sql += " WHERE " + sqlWhereClause;
    }

    sql += " limit 1";

    result = executeDbCommand(connection, sql);

    if (PQntuples(result) > 0)
    {
      srid = QString::fromUtf8(PQgetvalue(result, 0, 0));
      fType = QString::fromUtf8(PQgetvalue(result, 0, 1));
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
          "column ") + geometryColumn + tr(" in ") +
          mSchemaTableName + 
          tr(". The database communication log was:\n"));
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

QString QgsPostgresProvider::quotedIdentifier( QString ident ) const
{
  ident.replace('"', "\"\"");
  return ident.prepend("\"").append("\"");
}

QString QgsPostgresProvider::quotedValue( QString value ) const
{
  if( value.isNull() )
    return "NULL";
  
  // FIXME: use PQescapeStringConn
  value.replace("'", "''");
  return value.prepend("'").append("'");
}

PGresult *QgsPostgresProvider::PQexec(PGconn *conn, const char *query)
{
  PGresult *res = ::PQexec(conn, query);

#ifdef QGISDEBUG
  if(res) {
    int errorStatus = PQresultStatus(res);
    if( errorStatus!=PGRES_COMMAND_OK && errorStatus!=PGRES_TUPLES_OK ) 
    {
      QString err = QString("Errornous query: %1 returned %2 [%3]")
                          .arg(query)
                          .arg(errorStatus)
                          .arg(PQresultErrorMessage(res));
      QgsDebugMsgLevel( err, 3 );
    }
  }
#endif

  return res;
}

bool QgsPostgresProvider::PQexecNR(PGconn *conn, const char *query)
{
  PGresult *res = ::PQexec(conn, query);
  if(res)
  {
    int errorStatus = PQresultStatus(res);
#ifdef QGISDEBUG
    if( errorStatus!=PGRES_COMMAND_OK ) 
    {
      QString err = QString("Query: %1 returned %2 [%3]")
                        .arg(query)
                        .arg(errorStatus)
                        .arg(PQresultErrorMessage(res));
      QgsDebugMsgLevel( err, 3 );
    }
#endif
    PQclear(res);
    return errorStatus==PGRES_COMMAND_OK;
  }
  else
  {
    QgsDebugMsgLevel( QString("Query: %1 returned no result buffer").arg(query), 3 );
  }
  return false;
}

void QgsPostgresProvider::showMessageBox(const QString& title, const QString& text)
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
