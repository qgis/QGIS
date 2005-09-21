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

#include <qapplication.h>
#include <ogrsf_frmts.h>
#include <ogr_geometry.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "qgsdbfbase.h"
#include "cpl_error.h"
#include "qgsshapefile.h"
#include "qgsscangeometries.h"
#include "../../src/qgis.h"

// for htonl
#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif    


QgsShapeFile::QgsShapeFile(QString name){
  filename = name;
  features = 0;
  OGRRegisterAll();
  ogrDataSource = OGRSFDriverRegistrar::Open((const char *) filename);
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
  int progressThreshold = 5;
  int progressCount = 0;
  QgsScanGeometries *sg = new QgsScanGeometries();
  sg->setFileInfo("Scanning " + filename);
  sg->show();
  qApp->processEvents();

  OGRFeature *feat;
  int currentType = 0;
  bool multi = false;
  while((feat = ogrLayer->GetNextFeature()))
  {
// update the progress counter
//    if(++progressCount == progressThreshold)
//    {
//      progressCount = 0;
//      sg->setStatus(0);
      qApp->processEvents();
//    }

 

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
      std::cerr << "Preparing to escape " << geom_type.local8Bit() << std::endl; 
      char * esc_str = new char[geom_type.length()*2+1];
      PQescapeString(esc_str, (const char *)geom_type, geom_type.length());
      geom_type = QString(esc_str);
      std::cerr << "After escaping, geom_type is : " << geom_type.local8Bit() << std::endl;  
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
        column_names.push_back(feat->GetFieldDefnRef(n)->GetNameRef());
      
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
  QString name(filename);
  name = name.section('/', -1);
  table_name = name.section('.', 0, 0);
}

void QgsShapeFile::setColumnNames(QStringList columns)
{
  column_names.clear();
  for (QStringList::Iterator it = columns.begin(); it != columns.end(); ++it) 
  {
    column_names.push_back(*it);       
  }
}

bool QgsShapeFile::insertLayer(QString dbname, QString schema, QString geom_col, QString srid, PGconn * conn, QProgressDialog * pro, bool &fin){
  connect(pro, SIGNAL(cancelled()), this, SLOT(cancelImport()));
  import_cancelled = false;
  bool result = true;
  // Mangle the table name to make it PG compliant by replacing spaces with 
  // underscores
  table_name = table_name.replace(" ","_");
  QString query = "CREATE TABLE "+schema+"."+table_name+"(gid int4 PRIMARY KEY, ";
  for(int n=0; n<column_names.size() && result; n++){
    if(!column_names[n][0].isLetter())
      result = false;
    char * esc_str = new char[column_names[n].length()*2+1];
    std::cerr << "Escaping " << column_names[n].local8Bit() << " to ";
    PQescapeString(esc_str, (const char *)column_names[n].lower(), column_names[n].length());
    std::cerr << esc_str << std::endl; 
    query += esc_str;
    std::cerr << query.local8Bit() << std::endl; 
    query += " ";
    std::cerr << query.local8Bit() << std::endl; 
    query += column_types[n];
    std::cerr << query.local8Bit() << std::endl; 
    if(n<column_names.size()-1)
    {
      query += ", ";
      std::cerr << query.local8Bit() << std::endl; 
    }
    delete[] esc_str;
  }
  query += " )";
      std::cerr << query.local8Bit() << std::endl; 

  PGresult *res = PQexec(conn, (const char *)query);
  qWarning(query);
  if(PQresultStatus(res)!=PGRES_COMMAND_OK){
    // flag error and send query and error message to stdout on debug
    result = false;
    qWarning(PQresultErrorMessage(res));
  }
  else {
    PQclear(res);
  }

  query = "SELECT AddGeometryColumn(\'" + dbname + "\', \'" + table_name + "\', \'"+geom_col+"\', " + srid +
    ", \'" + geom_type + "\', 2)";            
  if(result) res = PQexec(conn, (const char *)query);
  if(PQresultStatus(res)!=PGRES_TUPLES_OK){
    result = false;    
  }
  else{
    qWarning(query);
    qWarning(PQresultErrorMessage(res));
    PQclear(res);
  }
  if(isMulti)
  {
    // drop the check constraint 
    // TODO This whole concept needs to be changed to either
    // convert the geometries to the same type or allow
    // multiple types in the check constraint. For now, we
    // just drop the constraint...
    query = "alter table " + table_name + " drop constraint \"$2\"";
    // XXX tacky - we don't even check the result...
    PQexec(conn, (const char*)query);
  }
      
  //adding the data into the table
  for(int m=0;m<features && result; m++){
    if(import_cancelled){
      fin = true;
      break;
    }

    OGRFeature *feat = ogrLayer->GetNextFeature();
    if(feat){
      OGRGeometry *geom = feat->GetGeometryRef();
      if(geom){
        query = "INSERT INTO "+schema+"."+table_name+QString(" VALUES( %1, ").arg(m);

        int num = geom->WkbSize();
        char * geo_temp = new char[num*3];
        geom->exportToWkt(&geo_temp);
        QString geometry(geo_temp);

        QString quotes;
        for(int n=0; n<column_types.size(); n++){
          if(column_types[n] == "int" || column_types[n] == "float")
            quotes = " ";
          else
            quotes = "\'";
          query += quotes;

          // escape the string value
          QString val = feat->GetFieldAsString(n);
          char * esc_str = new char[val.length()*2+1];
          PQescapeString(esc_str, (const char *)val.lower(), val.length());

          // add escaped value to the query 
          query += esc_str;
          query += QString(quotes + ", ");

          delete[] esc_str;
        }
        query += QString("GeometryFromText(\'")+geometry+QString("\', ")+srid+QString("))");
    //    std::cerr << query << std::endl; 

        if(result)
          res = PQexec(conn, (const char *)query);
        if(PQresultStatus(res)!=PGRES_COMMAND_OK){
          // flag error and send query and error message to stdout on debug
          result = false;
          qWarning(PQresultErrorMessage(res));
        }
        else {
          PQclear(res);
        }

        pro->setProgress(pro->progress()+1);
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
  import_cancelled = true;
}
