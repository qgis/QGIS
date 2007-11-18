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
#include <ogrsf_frmts.h>
#include <ogr_geometry.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>

#include <QFile>
#include <QProgressDialog>
#include <QString>
#include <QLabel>
#include <QTextCodec>
#include <QFileInfo>

#include "qgsdbfbase.h"
#include "cpl_error.h"
#include "qgsshapefile.h"
#include "qgis.h"
#include "qgslogger.h"

// for htonl
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif    


QgsShapeFile::QgsShapeFile(QString name, QString encoding){
  filename = name;
  features = 0;
  OGRRegisterAll();
  ogrDataSource = OGRSFDriverRegistrar::Open(QFile::encodeName(filename).constData());
  if (ogrDataSource != NULL){
    valid = true;
    ogrLayer = ogrDataSource->GetLayer(0);
    features = ogrLayer->GetFeatureCount();
  }
  else
    valid = false;
  setDefaultTable();
  // init the geometry types
  geometries << "NULL" << "POINT" << "LINESTRING" << "POLYGON" << "MULTPOINT" 
    << "MULTILINESTRING" << "MULTIPOLYGON" << "GEOMETRYCOLLECTION";
  
  codec = QTextCodec::codecForName(encoding.toLocal8Bit().data());
  if (!codec)
    codec = QTextCodec::codecForLocale();
}

QgsShapeFile::~QgsShapeFile(){
  if(ogrDataSource != 0)
  {
    // don't delete the layer if the datasource is bad -- (causes crash)
    delete ogrLayer;
  }
  delete ogrDataSource;
  delete filename;
  delete geom_type;
}

int QgsShapeFile::getFeatureCount(){
  return features;
}
bool QgsShapeFile::scanGeometries()
{
  QProgressDialog *sg = new QProgressDialog();
  sg->setMinimum(0);
  sg->setMaximum(0);
  QString label = "Scanning ";
  label += filename;
  sg->setLabel(new QLabel(label));
  sg->show();
  qApp->processEvents();

  OGRFeature *feat;
  OGRwkbGeometryType currentType = wkbUnknown;
  bool multi = false;
  while((feat = ogrLayer->GetNextFeature()))
  {
      qApp->processEvents();

    //    feat->DumpReadable(NULL);
    OGRGeometry *geom = feat->GetGeometryRef();
    if(geom)
    {
      QString gml =  geom->exportToGML();
      //      std::cerr << gml << std::endl; 
      if(gml.find("gml:Multi") > -1)
      {
        //   std::cerr << "MULTI Part Feature detected" << std::endl; 
        multi = true;
      }
      OGRFeatureDefn *fDef = feat->GetDefnRef();
      OGRwkbGeometryType gType = fDef->GetGeomType();
      //      std::cerr << fDef->GetGeomType() << std::endl; 
      if(gType > currentType)
      {
        currentType = gType;
      }
      if(gType < currentType)
      {
        std::cerr << "Encountered inconsistent geometry type " << gType << std::endl; 
      }

    }
  }
  
  // a hack to support 2.5D geometries (their wkb is equivalent to 2D variants
  // except that the highest bit is set also). For now we will ignore 3rd coordinate.
  hasMoreDimensions = false;
  if (currentType & wkb25DBit)
  {
    QgsDebugMsg("Got a shapefile with 2.5D geometry.");
    currentType = wkbFlatten(currentType);
    hasMoreDimensions = true;
  }
  
  ogrLayer->ResetReading();
  geom_type = geometries[currentType];
  if(multi && (geom_type.find("MULTI") == -1))
  {
    geom_type = "MULTI" + geom_type;
  }
  delete sg;
  
  //  std::cerr << "Geometry type is " << currentType << " (" << geometries[currentType] << ")" << std::endl; 
  return multi;
}
QString QgsShapeFile::getFeatureClass(){
  // scan the whole layer to try to determine the geometry
  // type. 
  qApp->processEvents();
  isMulti = scanGeometries();
  OGRFeature *feat;
  // skip features without geometry
  while ((feat = ogrLayer->GetNextFeature()) != NULL) {
    if (feat->GetGeometryRef())
      break;
  }
  if(feat){
    OGRGeometry *geom = feat->GetGeometryRef();
    if(geom){
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
      QgsDebugMsg("Preparing to escape " + geom_type);
      char * esc_str = new char[geom_type.length()*2+1];
      PQescapeString(esc_str, (const char *)geom_type, geom_type.length());
      geom_type = QString(esc_str);
      QgsDebugMsg("After escaping, geom_type is : " + geom_type);
      delete[] esc_str;
      
      QString file(filename);
      file.replace(file.length()-3, 3, "dbf");
      // open the dbf file
      std::ifstream dbf((const char*)file, std::ios::in | std::ios::binary);
      // read header
      DbaseHeader dbh;
      dbf.read((char *)&dbh, sizeof(dbh));
      // Check byte order
      if(htonl(1) == 1) 
      {
        /* DbaseHeader is stored in little-endian format.
         * The num_recs, size_hdr and size_rec fields must be byte-swapped when read
         * on a big-endian processor. Currently only size_hdr is used.
         */
        unsigned char *byte = reinterpret_cast<unsigned char *>(&dbh.size_hdr);
        unsigned char t = *byte; *byte = *(byte+1); *(byte+1) = t;
      }

      Fda fda;
      QString str_type = "varchar(";
      for(int field_count = 0, bytes_read = sizeof(dbh); bytes_read < dbh.size_hdr-1; field_count++, bytes_read +=sizeof(fda)){
        dbf.read((char *)&fda, sizeof(fda));
        switch(fda.field_type){
          case 'N': if((int)fda.field_decimal>0)
                      column_types.push_back("float");
                    else
                      column_types.push_back("int");          
                    break;
          case 'F': column_types.push_back("float");
                    break;                    
          case 'D': column_types.push_back("date");
                    break;
          case 'C': 
                    str_type= QString("varchar(%1)").arg(fda.field_length);
                    column_types.push_back(str_type);
                    break;
          case 'L': column_types.push_back("boolean");
                    break;
          default:
                    column_types.push_back("varchar(256)");
                    break;
        }
      }
      dbf.close();
      int numFields = feat->GetFieldCount();
      for(int n=0; n<numFields; n++)
      {
        QString s = codec->toUnicode(feat->GetFieldDefnRef(n)->GetNameRef());
        column_names.push_back(s);
      }
      
    }else valid = false;
    delete feat;
  }else valid = false;
  
  ogrLayer->ResetReading();    
  return valid?geom_type:QString::null;
}

bool QgsShapeFile::is_valid(){
  return valid;
}

QString QgsShapeFile::getName(){
  return filename;
}

QString QgsShapeFile::getTable(){
  return table_name;
}

void QgsShapeFile::setTable(QString new_table){
  new_table.replace("\'","\\'");
  new_table.replace("\\","\\\\");
  table_name = new_table;
}

void QgsShapeFile::setDefaultTable(){
  QFileInfo fi(filename);
  table_name = fi.baseName();
}

void QgsShapeFile::setColumnNames(QStringList columns)
{
  column_names.clear();
  for (QStringList::Iterator it = columns.begin(); it != columns.end(); ++it) 
  {
    column_names.push_back(*it);       
  }
}

bool QgsShapeFile::insertLayer(QString dbname, QString schema, QString geom_col, 
                               QString srid, PGconn * conn, QProgressDialog& pro, bool &fin,
                               QString& errorText)
{
  connect(&pro, SIGNAL(canceled()), this, SLOT(cancelImport()));
  import_canceled = false;
  bool result = true;
  // Mangle the table name to make it PG compliant by replacing spaces with 
  // underscores
  table_name = table_name.replace(" ","_");

  QString query = "CREATE TABLE "+schema+"."+table_name+"(gid int4 PRIMARY KEY, ";

  for(uint n=0; n<column_names.size() && result; n++){
    if(!column_names[n][0].isLetter())
      result = false;

    char * esc_str = new char[column_names[n].length()*2+1];

    PQescapeString(esc_str, (const char *)column_names[n].lower(), column_names[n].length());
    QgsDebugMsg("Escaped " + column_names[n] + " to " + QString(esc_str)); 
    query += esc_str;
    query += " " + column_types[n];

    if(n<column_names.size()-1)
    {
      query += ", ";
    }
    delete[] esc_str;
  }
  query += " )";

  QgsDebugMsg("Query string is: " + query);

  PGresult *res = PQexec(conn, (const char *)query);

  if(PQresultStatus(res)!=PGRES_COMMAND_OK){
    // flag error and send query and error message to stdout on debug
    errorText += tr("The database gave an error while executing this SQL:") + "\n";
    errorText += query + '\n';
    errorText += tr("The error was:") + "\n";
    errorText += PQresultErrorMessage(res) + '\n';
    PQclear(res);
    return false;
  }
  else {
    PQclear(res);
  }

  query = "SELECT AddGeometryColumn(\'" + schema + "\', \'" + table_name + "\', \'"+
    geom_col + "\', " + srid + ", \'" + geom_type + "\', 2)";

  res = PQexec(conn, (const char *)query);

  if(PQresultStatus(res)!=PGRES_TUPLES_OK){
    errorText += tr("The database gave an error while executing this SQL:") + "\n";

    errorText += query + '\n';
    errorText += tr("The error was:") + "\n";
    errorText += PQresultErrorMessage(res) + '\n';
    PQclear(res);
    return false;
  }
  else{
    PQclear(res);
  }

  if(isMulti)
  {
    query = QString("select constraint_name from information_schema.table_constraints where table_schema='%1' and table_name='%2' and constraint_name in ('$2','enforce_geotype_the_geom')")
            .arg( schema ).arg( table_name );

    QStringList constraints;
    res = PQexec( conn, query );
    if( PQresultStatus( res ) == PGRES_TUPLES_OK )
    {
      for(int i=0; i<PQntuples(res); i++)
	constraints.append( PQgetvalue(res, i, 0) );
    }
    PQclear(res);

    if( constraints.size()>0 ) {
      // drop the check constraint 
      // TODO This whole concept needs to be changed to either
      // convert the geometries to the same type or allow
      // multiple types in the check constraint. For now, we
      // just drop the constraint...
      query = "alter table " + table_name + " drop constraint \"" + constraints[0] + "\"";

      res = PQexec(conn, (const char*)query);
      if(PQresultStatus(res)!=PGRES_COMMAND_OK) {
        errorText += tr("The database gave an error while executing this SQL:") + "\n";
        errorText += query + '\n';
        errorText += tr("The error was:") + "\n";
        errorText += PQresultErrorMessage(res) + '\n';
        PQclear(res);
        return false;
      }

      PQclear(res);
    }

  }
      
  //adding the data into the table
  for(int m=0;m<features && result; m++){
    if(import_canceled){
      fin = true;
      break;
    }

    OGRFeature *feat = ogrLayer->GetNextFeature();
    if(feat){
      OGRGeometry *geom = feat->GetGeometryRef();
      if(geom){
        query = "INSERT INTO \"" + schema + "\".\"" + table_name + "\"" +
          QString(" VALUES( %1, ").arg(m);

        int num = geom->WkbSize();
        char * geo_temp = new char[num*3];
        // 'GeometryFromText' supports only 2D coordinates
        // TODO for proper 2.5D support we would need to use 'GeomFromEWKT'
        if (hasMoreDimensions)
          geom->setCoordinateDimension(2);
        geom->exportToWkt(&geo_temp);
        QString geometry(geo_temp);

        QString quotes;
        for(uint n=0; n<column_types.size(); n++){
          bool numericType(false);
          if(column_types[n] == "int" || column_types[n] == "float")
          {
            quotes = " ";
            numericType = true;
          }
          else
            quotes = "\'";
          query += quotes;

          // escape the string value and cope with blank data
          QString val = codec->toUnicode(feat->GetFieldAsString(n));
          if (val.isEmpty() && numericType)
          {
            val = "NULL";
          }
          val.replace("'", "''");
          //char * esc_str = new char[val.length()*2+1];
          //PQescapeString(esc_str, (const char *)val.lower().utf8(), val.length());

          // add escaped value to the query 
          query += val; //esc_str;
          query += QString(quotes + ", ");

          //delete[] esc_str;
        }
        query += QString("GeometryFromText(\'")+geometry+QString("\', ")+srid+QString("))");

        if(result)
          res = PQexec(conn, (const char *)query.utf8());

        if(PQresultStatus(res)!=PGRES_COMMAND_OK){
          // flag error and send query and error message to stdout on debug
          result = false;
          errorText += tr("The database gave an error while executing this SQL:") + "\n";
          // the query string can be quite long. Trim if necessary...
          if (query.count() > 100)
            errorText += query.left(150) + 
              tr("... (rest of SQL trimmed)", "is appended to a truncated SQL statement") + 
              "\n";
          else
            errorText += query + '\n';
          errorText += tr("The error was:") + "\n";
          errorText += PQresultErrorMessage(res);
          errorText += '\n';
        }
        else {
          PQclear(res);
        }

        pro.setValue(pro.value()+1);
        qApp->processEvents();
        delete[] geo_temp;
      }
      delete feat;
    }
  }
  // create the GIST index if the the load was successful
  if(result)
  {
    // prompt user to see if they want to build the index and warn
    // them about the potential time-cost
  }
  ogrLayer->ResetReading();
  return result;
}

void QgsShapeFile::cancelImport(){
  import_canceled = true;
}
