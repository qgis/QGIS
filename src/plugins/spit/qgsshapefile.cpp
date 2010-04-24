/***************************************************************************
                          qgsshapefile.cpp  -  description
                             -------------------
    begin                : Fri Dec 19 2003
    copyright            : (C) 2003 by Denis Antipov
    email                :
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

#include <QApplication>
#include <ogr_api.h>
#include <cpl_conv.h>
#include <string>
#include <fstream>
#include <cstdio>

#include <QFile>
#include <QProgressDialog>
#include <QString>
#include <QLabel>
#include <QTextCodec>
#include <QFileInfo>

#include "qgsapplication.h"
#include "qgsdbfbase.h"
#include "cpl_error.h"
#include "qgsshapefile.h"
#include "qgis.h"
#include "qgslogger.h"

#include "qgspgutil.h"
#include "qgslogger.h"

// for htonl
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif


QgsShapeFile::QgsShapeFile( QString name, QString encoding )
{
  fileName = name;
  features = 0;
  QgsApplication::registerOgrDrivers();
  ogrDataSource = OGROpen( QFile::encodeName( fileName ).constData(), FALSE, NULL );
  if ( ogrDataSource != NULL )
  {
    valid = true;
    ogrLayer = OGR_DS_GetLayer( ogrDataSource, 0 );
    features = OGR_L_GetFeatureCount( ogrLayer, TRUE );
  }
  else
    valid = false;
  setDefaultTable();
  // init the geometry types
  geometries << "NULL" << "POINT" << "LINESTRING" << "POLYGON" << "MULTIPOINT"
  << "MULTILINESTRING" << "MULTIPOLYGON" << "GEOMETRYCOLLECTION";

  codec = QTextCodec::codecForName( encoding.toLocal8Bit().data() );
  if ( !codec )
    codec = QTextCodec::codecForLocale();
}

QgsShapeFile::~QgsShapeFile()
{
  OGR_DS_Destroy( ogrDataSource );
}

int QgsShapeFile::getFeatureCount()
{
  return features;
}
bool QgsShapeFile::scanGeometries()
{
  QProgressDialog *sg = new QProgressDialog();
  sg->setMinimum( 0 );
  sg->setMaximum( 0 );
  QString label = tr( "Scanning " );
  label += fileName;
  sg->setLabel( new QLabel( label ) );
  sg->show();
  qApp->processEvents();

  OGRFeatureH feat;
  OGRwkbGeometryType currentType = wkbUnknown;
  bool multi = false;
  while (( feat = OGR_L_GetNextFeature( ogrLayer ) ) )
  {
    qApp->processEvents();

    //    feat->DumpReadable(NULL);
    OGRGeometryH geom = OGR_F_GetGeometryRef( feat );
    if ( geom )
    {
      QString gml =  OGR_G_ExportToGML( geom );
      // QgsDebugMsg(gml);
      if ( gml.indexOf( "gml:Multi" ) > -1 )
      {
        // QgsDebugMsg("MULTI Part Feature detected");
        multi = true;
      }
      OGRFeatureDefnH fDef = OGR_F_GetDefnRef( feat );
      OGRwkbGeometryType gType = OGR_FD_GetGeomType( fDef );
      // QgsDebugMsg(gType);
      if ( gType > currentType )
      {
        currentType = gType;
      }
      if ( gType < currentType )
      {
        QgsDebugMsg( QString( "Encountered inconsistent geometry type %1" ).arg( gType ) );
      }

    }
  }

  // a hack to support 2.5D geometries (their wkb is equivalent to 2D variants
  // except that the highest bit is set also). For now we will ignore 3rd coordinate.
  hasMoreDimensions = false;
  if ( currentType & wkb25DBit )
  {
    QgsDebugMsg( "Got a shapefile with 2.5D geometry." );
    currentType = wkbFlatten( currentType );
    hasMoreDimensions = true;
  }

  OGR_L_ResetReading( ogrLayer );
  geom_type = geometries[currentType];
  if ( multi && ( geom_type.indexOf( "MULTI" ) == -1 ) )
  {
    geom_type = "MULTI" + geom_type;
  }
  delete sg;

  // QgsDebugMsg(QString("Geometry type is %1 (%2)").arg(currentType).arg(geometries[currentType]));
  return multi;
}
QString QgsShapeFile::getFeatureClass()
{
  // scan the whole layer to try to determine the geometry
  // type.
  qApp->processEvents();
  isMulti = scanGeometries();
  OGRFeatureH feat;
  // skip features without geometry
  while (( feat = OGR_L_GetNextFeature( ogrLayer ) ) != NULL )
  {
    if ( OGR_F_GetGeometryRef( feat ) )
      break;
  }
  if ( feat )
  {
    OGRGeometryH geom = OGR_F_GetGeometryRef( feat );
    if ( geom )
    {
      /* OGR doesn't appear to report geometry type properly
       * for a layer containing both polygon and multipolygon
       * entities
       *
      // get the feature type from the layer
      OGRFeatureDefn * gDef = ogrLayer->GetLayerDefn();
      OGRwkbGeometryType gType = gDef->GetGeomType();
      geom_type = QGis::qgisFeatureTypes[gType];
      */
      //geom_type = QString(geom->getGeometryName());
      //geom_type = "GEOMETRY";
      QgsDebugMsg( "Preparing to escape " + geom_type );
      char * esc_str = new char[geom_type.length()*2+1];
      PQescapeString( esc_str, geom_type.toUtf8(), geom_type.length() );
      geom_type = QString( esc_str );
      QgsDebugMsg( "After escaping, geom_type is : " + geom_type );
      delete[] esc_str;

      QString file( fileName );
      file.replace( file.length() - 3, 3, "dbf" );
      // open the dbf file
      std::ifstream dbf( file.toUtf8(), std::ios::in | std::ios::binary );
      // read header
      DbaseHeader dbh;
      dbf.read(( char * )&dbh, sizeof( dbh ) );
      // Check byte order
      if ( htonl( 1 ) == 1 )
      {
        /* DbaseHeader is stored in little-endian format.
         * The num_recs, size_hdr and size_rec fields must be byte-swapped when read
         * on a big-endian processor. Currently only size_hdr is used.
         */
        unsigned char *byte = reinterpret_cast<unsigned char *>( &dbh.size_hdr );
        unsigned char t = *byte; *byte = *( byte + 1 ); *( byte + 1 ) = t;
      }

      Fda fda;
      QString str_type = "varchar(";
      for ( int field_count = 0, bytes_read = sizeof( dbh ); bytes_read < dbh.size_hdr - 1; field_count++, bytes_read += sizeof( fda ) )
      {
        dbf.read(( char * )&fda, sizeof( fda ) );
        switch ( fda.field_type )
        {
          case 'N':
            if (( int )fda.field_decimal > 0 )
              column_types.push_back( "float" );
            else
              column_types.push_back( "int" );
            break;
          case 'F': column_types.push_back( "float" );
            break;
          case 'D': column_types.push_back( "date" );
            break;
          case 'C':
            str_type = QString( "varchar(%1)" ).arg( fda.field_length );
            column_types.push_back( str_type );
            break;
          case 'L': column_types.push_back( "boolean" );
            break;
          default:
            column_types.push_back( "varchar(256)" );
            break;
        }
      }
      dbf.close();
      int numFields = OGR_F_GetFieldCount( feat );
      for ( int n = 0; n < numFields; n++ )
      {
        QString s = codec->toUnicode( OGR_Fld_GetNameRef( OGR_F_GetFieldDefnRef( feat, n ) ) );
        column_names.push_back( s );
      }

    }
    else valid = false;
    OGR_F_Destroy( feat );
  }
  else valid = false;

  OGR_L_ResetReading( ogrLayer );
  return valid ? geom_type : QString::null;
}

bool QgsShapeFile::is_valid()
{
  return valid;
}

QString QgsShapeFile::getName()
{
  return fileName;
}

QString QgsShapeFile::getTable()
{
  return table_name;
}

void QgsShapeFile::setTable( QString new_table )
{
  new_table.replace( "\'", "\\'" );
  new_table.replace( "\\", "\\\\" );
  table_name = new_table;
}

void QgsShapeFile::setDefaultTable()
{
  QFileInfo fi( fileName );
  table_name = fi.baseName();
}

void QgsShapeFile::setColumnNames( QStringList columns )
{
  column_names.clear();
  for ( QStringList::Iterator it = columns.begin(); it != columns.end(); ++it )
  {
    column_names.push_back( *it );
  }
}

bool QgsShapeFile::insertLayer( QString dbname, QString schema, QString primary_key, QString geom_col,
                                QString srid, PGconn * conn, QProgressDialog& pro, bool &fin,
                                QString& errorText )
{
  connect( &pro, SIGNAL( canceled() ), this, SLOT( cancelImport() ) );
  import_canceled = false;
  bool result = true;

  QString query = QString( "CREATE TABLE %1.%2(%3 SERIAL PRIMARY KEY" )
                  .arg( QgsPgUtil::quotedIdentifier( schema ) )
                  .arg( QgsPgUtil::quotedIdentifier( table_name ) )
                  .arg( QgsPgUtil::quotedIdentifier( primary_key ) );

  for ( uint n = 0; n < column_names.size() && result; n++ )
  {
    query += QString( ",%1 %2" )
             .arg( QgsPgUtil::quotedIdentifier( column_names[n] ) )
             .arg( column_types[n] );
  }
  query += " )";

  QgsDebugMsg( "Query string is: " + query );

  PGresult *res = PQexec( conn, query.toUtf8() );

  if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
  {
    // flag error and send query and error message to stdout on debug
    errorText += tr( "The database gave an error while executing this SQL:\n%1\nThe error was:\n%2\n" )
                 .arg( query ).arg( PQresultErrorMessage( res ) );
    PQclear( res );
    return false;
  }
  else
  {
    PQclear( res );
  }

  query = QString( "SELECT AddGeometryColumn(%1,%2,%3,%4,%5,2)" )
          .arg( QgsPgUtil::quotedValue( schema ) )
          .arg( QgsPgUtil::quotedValue( table_name ) )
          .arg( QgsPgUtil::quotedValue( geom_col ) )
          .arg( srid )
          .arg( QgsPgUtil::quotedValue( geom_type ) );

  res = PQexec( conn, query.toUtf8() );

  if ( PQresultStatus( res ) != PGRES_TUPLES_OK )
  {
    errorText += tr( "The database gave an error while executing this SQL:\n%1\nThe error was:\n%2\n" )
                 .arg( query ).arg( PQresultErrorMessage( res ) );
    PQclear( res );
    return false;
  }
  else
  {
    PQclear( res );
  }

  if ( isMulti )
  {
    query = QString( "select constraint_name from information_schema.table_constraints where table_schema=%1 and table_name=%2 and constraint_name in ('$2','enforce_geotype_the_geom')" )
            .arg( QgsPgUtil::quotedValue( schema ) )
            .arg( QgsPgUtil::quotedValue( table_name ) );

    QStringList constraints;
    res = PQexec( conn, query.toUtf8() );
    if ( PQresultStatus( res ) == PGRES_TUPLES_OK )
    {
      for ( int i = 0; i < PQntuples( res ); i++ )
        constraints.append( PQgetvalue( res, i, 0 ) );
    }
    PQclear( res );

    if ( constraints.size() > 0 )
    {
      // drop the check constraint
      // TODO This whole concept needs to be changed to either
      // convert the geometries to the same type or allow
      // multiple types in the check constraint. For now, we
      // just drop the constraint...
      query = QString( "alter table %1 drop constraint %2" )
              .arg( QgsPgUtil::quotedIdentifier( table_name ) )
              .arg( QgsPgUtil::quotedIdentifier( constraints[0] ) );

      res = PQexec( conn, query.toUtf8() );
      if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
      {
        errorText += tr( "The database gave an error while executing this SQL:\n%1\nThe error was:\n%2\n" )
                     .arg( query ).arg( PQresultErrorMessage( res ) );
        PQclear( res );
        return false;
      }

      PQclear( res );
    }

  }

  //adding the data into the table
  for ( int m = 0; m < features && result; m++ )
  {
    if ( import_canceled )
    {
      fin = true;
      break;
    }

    OGRFeatureH feat = OGR_L_GetNextFeature( ogrLayer );
    if ( feat )
    {
      OGRGeometryH geom = OGR_F_GetGeometryRef( feat );
      if ( geom )
      {
        query = QString( "INSERT INTO %1.%2(" )
                .arg( QgsPgUtil::quotedIdentifier( schema ) )
                .arg( QgsPgUtil::quotedIdentifier( table_name ) );
        QString values = " VALUES (";

        char *geo_temp;
        // 'GeometryFromText' supports only 2D coordinates
        // TODO for proper 2.5D support we would need to use 'GeomFromEWkt'
        if ( hasMoreDimensions )
          OGR_G_SetCoordinateDimension( geom, 2 );
        OGR_G_ExportToWkt( geom, &geo_temp );
        QString geometry( geo_temp );
        CPLFree( geo_temp );

        for ( uint n = 0; n < column_types.size(); n++ )
        {
          QString val;

          // FIXME: OGR_F_GetFieldAsString returns junk when called with a 8.255 float field
          if ( column_types[n] == "float" )
            val = QString::number( OGR_F_GetFieldAsDouble( feat, n ) );
          else
            val = codec->toUnicode( OGR_F_GetFieldAsString( feat, n ) );

          if ( val.isEmpty() )
            val = "NULL";
          else
            val = QgsPgUtil::quotedValue( val );

          if ( n > 0 )
          {
            query += ",";
            values += ",";
          }
          query += QgsPgUtil::quotedIdentifier( column_names[n] );
          values += val;
        }
        query += "," + QgsPgUtil::quotedIdentifier( geom_col );
        values += QString( ",GeometryFromText(%1,%2)" )
                  .arg( QgsPgUtil::quotedValue( geometry ) )
                  .arg( srid );

        query += ")" + values + ")";

        if ( result )
          res = PQexec( conn, query.toUtf8() );

        if ( PQresultStatus( res ) != PGRES_COMMAND_OK )
        {
          // flag error and send query and error message to stdout on debug
          result = false;
          errorText += tr( "The database gave an error while executing this SQL:" ) + "\n";
          // the query string can be quite long. Trim if necessary...
          if ( query.count() > 100 )
            errorText += query.left( 150 ) +
                         tr( "... (rest of SQL trimmed)", "is appended to a truncated SQL statement" ) +
                         "\n";
          else
            errorText += query + '\n';
          errorText += tr( "The error was:\n%1\n" ).arg( PQresultErrorMessage( res ) );
          errorText += '\n';
        }
        else
        {
          PQclear( res );
        }

        pro.setValue( pro.value() + 1 );
        qApp->processEvents();
      }
      OGR_F_Destroy( feat );
    }
  }
  // create the GIST index if the the load was successful
  if ( result )
  {
    // prompt user to see if they want to build the index and warn
    // them about the potential time-cost
  }
  OGR_L_ResetReading( ogrLayer );
  return result;
}

void QgsShapeFile::cancelImport()
{
  import_canceled = true;
}
