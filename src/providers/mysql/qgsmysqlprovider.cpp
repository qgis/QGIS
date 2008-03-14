/***************************************************************************
  qgsmysqlprovider.cpp -  Data provider for MySQL 5.0+
  -------------------
          begin                : 2006-01-07
          copyright            : (C) 2006 by Gary E.Sherman
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
/* $Id: */

#include "qgsmysqlprovider.h"

#include <cfloat>
#include <iostream>

#include <qfile.h>
#include <qdatastream.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qregexp.h>
#include <QUrl>
#include <qglobal.h>

#include "../../src/qgsdataprovider.h"
#include "../../src/qgsencodingfiledialog.h"
#include "../../src/qgsfeature.h"
#include "../../src/qgsfield.h"
#include "../../src/qgsrect.h"



static const QString TEXT_PROVIDER_KEY = "mysql";
static const QString TEXT_PROVIDER_DESCRIPTION = "MySQL data provider";



QgsMySQLProvider::QgsMySQLProvider(QString const &uri)
    : QgsVectorDataProvider(uri) 
{
  // assume this is a valid layer until we determine otherwise
  valid = true;
  /* OPEN LOG FILE */

  // Make connection to the data source
  // For postgres, the connection information is passed as a space delimited
  // string:
  //  host=192.168.1.5 dbname=test port=3306 user=gsherman password=xxx table=tablename
  std::cout << "Data source uri is " << uri.toLocal8Bit().data() << std::endl;

  // Strip the table and sql statement name off and store them
  int sqlStart = uri.indexOf(" sql");
  int tableStart = uri.indexOf("table=");
#ifdef QGISDEBUG
  qDebug(  "****************************************");
  qDebug(  "****   MySQL Layer Creation   *****" );
  qDebug(  "****************************************");
  qDebug(  (const char*)(QString("URI: ") + uri).toLocal8Bit().data() );
  QString msg;

  qDebug(  "tableStart: " + msg.setNum(tableStart) );
  qDebug(  "sqlStart: " + msg.setNum(sqlStart));
#endif 
  mTableName = uri.mid(tableStart + 6, sqlStart - tableStart -6);
  if(sqlStart > -1)
  { 
    sqlWhereClause = uri.mid(sqlStart + 5);
  }
  else
  {
    sqlWhereClause = QString::null;
  }
  QString connInfo = uri.left(uri.indexOf("table="));
#ifdef QGISDEBUG
  qDebug( (const char*)(QString("Table name is ") + mTableName).toLocal8Bit().data());
  qDebug( (const char*)(QString("SQL is ") + sqlWhereClause).toLocal8Bit().data() );
  qDebug( "Connection info is " + connInfo);
#endif
  // calculate the schema if specified
  mSchemaName = "";
  if (mTableName.indexOf(".") > -1) {
    mSchemaName = mTableName.left(mTableName.indexOf("."));
  }
  geometryColumn = mTableName.mid(mTableName.indexOf(" (") + 2);
  geometryColumn.truncate(geometryColumn.length() - 1);
  mTableName = mTableName.mid(mTableName.indexOf(".") + 1, mTableName.indexOf(" (") - (mTableName.indexOf(".") + 1)); 

  // Keep a schema qualified table name for convenience later on.
  if (mSchemaName.length() > 0)
    mSchemaTableName = "\"" + mSchemaName + "\".\"" + mTableName + "\"";
  else
    mSchemaTableName = "\"" + mTableName + "\"";

  /* populate the uri structure */
  mUri.schema = mSchemaName;
  mUri.table = mTableName;
  mUri.geometryColumn = geometryColumn;
  mUri.sql = sqlWhereClause;
  // parse the connection info
  QStringList conParts = connInfo.split(" ");
  QStringList parm = conParts[0].split("=");
  if(parm.size() == 2)
  {
    mUri.host = parm[1];
  }
  parm = conParts[1].split("=");
  if(parm.size() == 2)
  {
    mUri.database = parm[1];
  }
  parm = conParts[2].split("=");
  if(parm.size() == 2)
  {
    mUri.port = parm[1];
  }

  parm = conParts[3].split("=");
  if(parm.size() == 2)
  {
    mUri.username = parm[1];
  }
  parm = conParts[4].split("=");
  if(parm.size() == 2)
  {
    mUri.password = parm[1];
  }
  /* end uri structure */

#ifdef QGISDEBUG
  std::cerr << "Geometry column is: " << geometryColumn.toLocal8Bit().data() << std::endl;
  std::cerr << "Schema is: " << mSchemaName.toLocal8Bit().data() << std::endl;
  std::cerr << "Table name is: " << mTableName.toLocal8Bit().data() << std::endl;
#endif
  //QString logFile = "./pg_provider_" + mTableName + ".log";
  //pLog.open((const char *)logFile);
#ifdef QGISDEBUG
  std::cerr << "Opened log file for " << mTableName.toLocal8Bit().data() << std::endl;
#endif
  MYSQL *res = mysql_init(&mysql);
  MYSQL *con = mysql_real_connect(&mysql, mUri.host.toLocal8Bit(), mUri.username.toLocal8Bit(), mUri.password.toLocal8Bit(), mUri.table.toLocal8Bit(), 0, NULL, 0);
  // check the connection status
  if(con)
  {

#ifdef QGISDEBUG
    std::cerr << "Checking for select permission on the relation\n";
#endif
    // Check that we can read from the table (i.e., we have
    // select permission).
    QString sql = "select * from " + mSchemaTableName + " limit 1";
    PGresult* testAccess = PQexec(pd, (const char*)(sql.utf8()));
    if (PQresultStatus(testAccess) != PGRES_TUPLES_OK)
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(0, tr("Unable to access relation"),
          tr("Unable to access the ") + mSchemaTableName + 
          tr(" relation.\nThe error message from the database was:\n") +
          QString(PQresultErrorMessage(testAccess)) + ".\n" + 
          "SQL: " + sql);
      QApplication::setOverrideCursor(Qt::waitCursor);
      PQclear(testAccess);
      valid = false;
      return;
    }
    PQclear(testAccess);

    /* Check to see if we have GEOS support and if not, warn the user about
       the problems they will see :) */
#ifdef QGISDEBUG
    std::cerr << "Checking for GEOS support" << std::endl;
#endif
    if(!hasGEOS(pd))
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(0, tr("No GEOS Support!"),
          tr("Your PostGIS installation has no GEOS support.\nFeature selection and "
            "identification will not work properly.\nPlease install PostGIS with " 
            "GEOS support (http://geos.refractions.net)"));
      QApplication::setOverrideCursor(Qt::waitCursor);
    }
    //--std::cout << "Connection to the database was successful\n";

    if (getGeometryDetails()) // gets srid and geometry type
    {
      deduceEndian();
      calculateExtents();
      getFeatureCount();

      // Populate the field vector for this layer. The field vector contains
      // field name, type, length, and precision (if numeric)
      sql = "select * from " + mSchemaTableName + " limit 1";

      PGresult* result = PQexec(pd, (const char *) (sql.utf8()));
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

        sql = "select oid from pg_class where relname = '" + mTableName + "' and relnamespace = ("
	  "select oid from pg_namespace where nspname = '" + mSchemaName + "')";
        PGresult *tresult= PQexec(pd, (const char *)(sql.utf8()));
        QString tableoid = PQgetvalue(tresult, 0, 0);
        PQclear(tresult);

        sql = "select attnum from pg_attribute where attrelid = " + tableoid + " and attname = '" + fieldName + "'";
        tresult = PQexec(pd, (const char *)(sql.utf8()));
        QString attnum = PQgetvalue(tresult, 0, 0);
        PQclear(tresult);

#ifdef QGISDEBUG
        std::cerr << "Field: " << attnum.toLocal8Bit().data() << " maps to " << i << " " << fieldName.toLocal8Bit().data() << ", " 
          << fieldType.toLocal8Bit().data() << " (" << fldtyp << "),  " << fieldSize.toLocal8Bit().data() << ", "  
          << fieldModifier << std::endl;
#endif
        attributeFieldsIdMap[attnum.toInt()] = i;

        if(fieldName!=geometryColumn)
        {
          attributeFields.push_back(QgsField(fieldName, fieldType, fieldSize.toInt(), fieldModifier));
        }
      }
      PQclear(result);

      // set the primary key
      getPrimaryKey();

      // Set the postgresql message level so that we don't get the
      // 'there is no transaction in progress' warning.
#ifndef QGISDEBUG
      PQexec(connection, "set client_min_messages to error");
#endif

      // Kick off the long running threads

#ifdef POSTGRESQL_THREADS
      std::cout << "QgsPostgresProvider: About to touch mExtentThread" << std::endl;
      mExtentThread.setConnInfo( connInfo );
      mExtentThread.setTableName( mTableName );
      mExtentThread.setSqlWhereClause( sqlWhereClause );
      mExtentThread.setGeometryColumn( geometryColumn );
      mExtentThread.setCallback( this );
      std::cout << "QgsPostgresProvider: About to start mExtentThread" << std::endl;
      mExtentThread.start();
      std::cout << "QgsPostgresProvider: Main thread just dispatched mExtentThread" << std::endl;

      std::cout << "QgsPostgresProvider: About to touch mCountThread" << std::endl;
      mCountThread.setConnInfo( connInfo );
      mCountThread.setTableName( mTableName );
      mCountThread.setSqlWhereClause( sqlWhereClause );
      mCountThread.setGeometryColumn( geometryColumn );
      mCountThread.setCallback( this );
      std::cout << "QgsPostgresProvider: About to start mCountThread" << std::endl;
      mCountThread.start();
      std::cout << "QgsPostgresProvider: Main thread just dispatched mCountThread" << std::endl;
#endif
    } 
    else 
    {
      // the table is not a geometry table
      numberFeatures = 0;
      valid = false;
#ifdef QGISDEBUG
      std::cerr << "Invalid Postgres layer" << std::endl;
#endif
    }

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

QgsMySQLProvider::~QgsMySQLProvider()
{
  mFile->close();
  delete mFile;
  for (int i = 0; i < fieldCount(); i++)
  {
    delete mMinMaxCache[i];
  }
  delete[]mMinMaxCache;
}


QString QgsMySQLProvider::storageType()
{
  return "Delimited text file";
}


/**
 * Get the first feature resutling from a select operation
 * @return QgsFeature
 */
QgsFeature * QgsMySQLProvider::getFirstFeature(bool fetchAttributes)
{
    QgsFeature *f = new QgsFeature;

    reset();                    // reset back to first feature

    if ( getNextFeature_( *f, fetchAttributes ) )
    {
        return f;
    }

    delete f;

    return 0x0;
} // QgsMySQLProvider::getFirstFeature(bool fetchAttributes)

/**

  insure double value is properly translated into locate endian-ness

*/
static
double
translateDouble_( double d )
{
    union
    {
        double fpval;
        char   char_val[8];
    } from, to;

    // break double into byte sized chunks
    from.fpval = d;

    to.char_val[7] = from.char_val[0];
    to.char_val[6] = from.char_val[1];
    to.char_val[5] = from.char_val[2];
    to.char_val[4] = from.char_val[3];
    to.char_val[3] = from.char_val[4];
    to.char_val[2] = from.char_val[5];
    to.char_val[1] = from.char_val[6];
    to.char_val[0] = from.char_val[7];

    return to.fpval;

} // translateDouble_


bool
QgsMySQLProvider::getNextFeature_( QgsFeature & feature, 
                                           bool getAttributes,
                                           std::list<int> const * desiredAttributes )
{
    // before we do anything else, assume that there's something wrong with
    // the feature
    feature.setValid( false );

    QTextStream textStream( mFile );

    if ( ! textStream.atEnd() )
    {
      QString line = textStream.readLine(); // Default local 8 bit encoding

        // lex the tokens from the current data line
        QStringList tokens = QStringList::split(QRegExp(mDelimiter), line, true);

        bool xOk = false;
        bool yOk = false;

        int xFieldPos = fieldPositions[mXField];
        int yFieldPos = fieldPositions[mYField];

        double x = tokens[xFieldPos].toDouble( &xOk );
        double y = tokens[yFieldPos].toDouble( &yOk );

        if ( xOk && yOk )
        {
            // if the user has selected an area, constrain iterator to
            // features that are within that area
            if ( mSelectionRectangle && ! boundsCheck(x,y) )
            {
                bool foundFeature = false;

                while ( ! textStream.atEnd() && 
                        (xOk && yOk) )
                {
                    if ( boundsCheck(x,y) )
                    {
                        foundFeature = true;
                        break;
                    }

                    ++mFid;     // since we're skipping to next feature,
                                // increment ID

                    line = textStream.readLine();

                    tokens = QStringList::split(QRegExp(mDelimiter), line, true);

                    x = tokens[xFieldPos].toDouble( &xOk );
                    y = tokens[yFieldPos].toDouble( &yOk );
                }

                // there were no other features from the current one forward
                // that were within the selection region
                if ( ! foundFeature )
                {
                    return false;
                }
            }

            // at this point, one way or another, the current feature values
            // are valid
           feature.setValid( true );

           ++mFid;             // increment to next feature ID

           feature.setFeatureId( mFid );

           unsigned char * geometry = new unsigned char[sizeof(wkbPoint)];
           QByteArray  buffer;
           buffer.setRawData( (const char*)geometry, sizeof(wkbPoint) ); // buffer
                                                                         // points
                                                                         // to
                                                                         // geometry

           QDataStream s( &buffer, QIODevice::WriteOnly ); // open on buffers's data

           switch ( endian() )
           {
               case QgsDataProvider::NDR :
                   // we're on a little-endian platform, so tell the data
                   // stream to use that
                   s.setByteOrder( QDataStream::LittleEndian );
                   s << (Q_UINT8)1; // 1 is for little-endian
                   break;
               case QgsDataProvider::XDR :
                   // don't change byte order since QDataStream is big endian by default
                   s << (Q_UINT8)0; // 0 is for big-endian
                   break;
               default :
                   qDebug( "%s:%d unknown endian", __FILE__, __LINE__ );
                   delete [] geometry;
                   return false;
           }

           s << (Q_UINT32)1; // 1 is for WKBPoint
           s << x;
           s << y;


           feature.setGeometryAndOwnership( geometry, sizeof(wkbPoint) );

           // ensure that the buffer doesn't delete the data on us
           buffer.resetRawData( (const char*)geometry, sizeof(wkbPoint) );

           if ( getAttributes && ! desiredAttributes )
           {
               for (int fi = 0; fi < attributeFields.size(); fi++)
               {
                   feature.addAttribute(attributeFields[fi].name(), tokens[fi]);
               }
           }
           // regardless of whether getAttributes is true or not, if the
           // programmer went through the trouble of passing in such a list of
           // attribute fields, then obviously they want them
           else if ( desiredAttributes )
           {
               for ( std::list<int>::const_iterator i = desiredAttributes->begin();
                     i != desiredAttributes->end();
                     ++i )
               {
                   feature.addAttribute(attributeFields[*i].name(), tokens[*i]);
               }
           }

           return true;

        } // if able to get x and y coordinates

    } // ! textStream EOF

    return false;

} // getNextFeature_( QgsFeature & feature )



/**
  Get the next feature resulting from a select operation
  Return 0 if there are no features in the selection set
 * @return false if unable to get the next feature
 */
bool QgsMySQLProvider::getNextFeature(QgsFeature & feature,
                                              bool fetchAttributes)
{
    return getNextFeature_( feature, fetchAttributes );
} // QgsMySQLProvider::getNextFeature



QgsFeature * QgsMySQLProvider::getNextFeature(bool fetchAttributes)
{
    QgsFeature * f = new QgsFeature;

    if ( getNextFeature( *f, fetchAttributes ) )
    {
        return f;
    }
    
    delete f;

    return 0x0;
} // QgsMySQLProvider::getNextFeature(bool fetchAttributes)



QgsFeature * QgsMySQLProvider::getNextFeature(std::list<int> const & desiredAttributes, int featureQueueSize)
{
    QgsFeature * f = new QgsFeature;

    if ( getNextFeature_( *f, true, &desiredAttributes ) )
    {
        return f;
    }
    
    delete f;

    return 0x0;

} // QgsMySQLProvider::getNextFeature(std::list < int >&attlist)




/**
 * Select features based on a bounding rectangle. Features can be retrieved
 * with calls to getFirstFeature and getNextFeature.
 * @param mbr QgsRect containing the extent to use in selecting features
 */
void QgsMySQLProvider::select(QgsRect * rect, bool useIntersect)
{

  // Setting a spatial filter doesn't make much sense since we have to
  // compare each point against the rectangle.
  // We store the rect and use it in getNextFeature to determine if the
  // feature falls in the selection area
  mSelectionRectangle = new QgsRect((*rect));
  // Select implies an upcoming feature read so we reset the data source
  reset();
  // Reset the feature id to 0
  mFid = 0;

}


/**
 * Identify features within the search radius specified by rect
 * @param rect Bounding rectangle of search radius
 * @return std::vector containing QgsFeature objects that intersect rect
 */
std::vector < QgsFeature > &QgsMySQLProvider::identify(QgsRect * rect)
{
  // reset the data source since we need to be able to read through
  // all features
  reset();
  std::cerr << "Attempting to identify features falling within " << (const char *)rect->
    stringRep().toLocal8Bit().data() << std::endl;
  // select the features
  select(rect);
#ifdef WIN32
  //TODO fix this later for win32
  std::vector < QgsFeature > feat;
  return feat;
#endif

}

/*
   unsigned char * QgsMySQLProvider::getGeometryPointer(OGRFeature *fet){
   unsigned char *gPtr=0;
// get the wkb representation

//geom->exportToWkb((OGRwkbByteOrder) endian(), gPtr);
return gPtr;

}
*/


// Return the extent of the layer
QgsRect *QgsMySQLProvider::extent()
{
  return new QgsRect(mExtent->xMin(), mExtent->yMin(), mExtent->xMax(),
                     mExtent->yMax());
}

/** 
 * Return the feature type
 */
int QgsMySQLProvider::geometryType() const
{
  return 1;                     // WKBPoint
}

/** 
 * Return the feature type
 */
long QgsMySQLProvider::featureCount() const
{
  return mNumberFeatures;
}

/**
 * Return the number of fields
 */
int QgsMySQLProvider::fieldCount() const
{
  return attributeFields.size();
}

/**
 * Fetch attributes for a selected feature
 */
void QgsMySQLProvider::getFeatureAttributes(int key, QgsFeature * f)
{
  //for (int i = 0; i < ogrFet->GetFieldCount(); i++) {

  //  // add the feature attributes to the tree
  //  OGRFieldDefn *fldDef = ogrFet->GetFieldDefnRef(i);
  //  QString fld = fldDef->GetNameRef();
  //  //    OGRFieldType fldType = fldDef->GetType();
  //  QString val;

  //  val = ogrFet->GetFieldAsString(i);
  //  f->addAttribute(fld, val);
  //}
}

std::vector<QgsField> const & QgsMySQLProvider::fields() const
{
  return attributeFields;
}

void QgsMySQLProvider::reset()
{
  // Reset the file pointer to BOF
  mFile->reset();
  // Reset feature id to 0
  mFid = 0;
  // Skip ahead one line since first record is always assumed to be
  // the header record
  QTextStream stream(mFile);
  stream.readLine();
}

QString QgsMySQLProvider::minValue(int position)
{
  if (position >= fieldCount())
  {
    std::
      cerr << "Warning: access requested to invalid position " <<
      "in QgsMySQLProvider::minValue(..)" << std::endl;
  }
  if (mMinMaxCacheDirty)
  {
    fillMinMaxCash();
  }
  return QString::number(mMinMaxCache[position][0], 'f', 2);
}


QString QgsMySQLProvider::maxValue(int position)
{
  if (position >= fieldCount())
  {
    std::
      cerr << "Warning: access requested to invalid position " <<
      "in QgsMySQLProvider::maxValue(..)" << std::endl;
  }
  if (mMinMaxCacheDirty)
  {
    fillMinMaxCash();
  }
  return QString::number(mMinMaxCache[position][1], 'f', 2);
}

void QgsMySQLProvider::fillMinMaxCash()
{
  for (int i = 0; i < fieldCount(); i++)
  {
    mMinMaxCache[i][0] = DBL_MAX;
    mMinMaxCache[i][1] = -DBL_MAX;
  }

  QgsFeature f;
  reset();

  getNextFeature(f, true);
  do
  {
    for (int i = 0; i < fieldCount(); i++)
    {
      double value = (f.attributeMap())[i].fieldValue().toDouble();
      if (value < mMinMaxCache[i][0])
      {
        mMinMaxCache[i][0] = value;
      }
      if (value > mMinMaxCache[i][1])
      {
        mMinMaxCache[i][1] = value;
      }
    }
  }
  while (getNextFeature(f, true));

  mMinMaxCacheDirty = false;
}

//TODO - add sanity check for shape file layers, to include cheking to
//       see if the .shp, .dbf, .shx files are all present and the layer
//       actually has features
bool QgsMySQLProvider::isValid()
{
  return mValid;
}

/** 
 * Check to see if the point is within the selection rectangle
 */
bool QgsMySQLProvider::boundsCheck(double x, double y)
{
  bool inBounds = (((x < mSelectionRectangle->xMax()) &&
                    (x > mSelectionRectangle->xMin())) &&
                   ((y < mSelectionRectangle->yMax()) &&
                    (y > mSelectionRectangle->yMin())));
  // QString hit = inBounds?"true":"false";

  // std::cerr << "Checking if " << x << ", " << y << " is in " << 
  //mSelectionRectangle->stringRep().ascii() << ": " << hit.ascii() << std::endl; 
  return inBounds;
}

int QgsMySQLProvider::capabilities() const
{
    return QgsVectorDataProvider::SaveAsShapefile;
}


bool QgsMySQLProvider::saveAsShapefile()
{
  // OGR based save to shapefile method removed, unused?
  return false;
}



size_t QgsMySQLProvider::layerCount() const
{
    return 1;                   // XXX How to calculate the layers?
} // QgsOgrProvider::layerCount()



int *QgsMySQLProvider::getFieldLengths()
{
  // this function parses the entire data file and calculates the
  // max for each

  // Only do this if we haven't done it already (ie. the vector is
  // empty)
  int *lengths = new int[attributeFields.size()];
  // init the lengths to zero
  for (int il = 0; il < attributeFields.size(); il++)
  {
    lengths[il] = 0;
  }
  if (mValid)
  {
    reset();
    // read the line
    QTextStream stream(mFile);
    QString line;
    while (!stream.atEnd())
    {
      line = stream.readLine(); // line of text excluding '\n'
      // split the line
      QStringList parts = QStringList::split(QRegExp(mDelimiter), line, true);
      // iterate over the parts and update the max value
      for (int i = 0; i < parts.size(); i++)
      {
        if (parts[i] != QString::null)
        {
          // std::cerr << "comparing length for " << parts[i] << " against max len of " << lengths[i] << std::endl; 
          if (parts[i].length() > lengths[i])
          {
            lengths[i] = parts[i].length();
          }
        }

      }
    }
  }
  return lengths;
}





QString  QgsMySQLProvider::name() const
{
    return TEXT_PROVIDER_KEY;
} // ::name()



QString  QgsMySQLProvider::description() const
{
    return TEXT_PROVIDER_DESCRIPTION;
} //  QgsMySQLProvider::name()


/**
 * Class factory to return a pointer to a newly created 
 * QgsMySQLProvider object
 */
QGISEXTERN QgsMySQLProvider *classFactory(const QString *uri)
{
  return new QgsMySQLProvider(*uri);
}

/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
    return TEXT_PROVIDER_KEY;
}

/**
 * Required description function 
 */
QGISEXTERN QString description()
{
    return TEXT_PROVIDER_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}
