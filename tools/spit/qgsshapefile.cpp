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

#include <ogrsf_frmts.h>
#include <ogr_geometry.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "qgsdbfbase.h"
#include "cpl_error.h"
#include "qgsshapefile.h"

QgsShapeFile::QgsShapeFile(QString name){
  filename = (const char*)name;
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
}

QgsShapeFile::~QgsShapeFile(){
  delete ogrLayer;
  delete ogrDataSource;
  delete filename;
  delete geom_type;
}

int QgsShapeFile::getFeatureCount(){
  return features;
}

const char * QgsShapeFile::getFeatureClass(){
  OGRFeature *feat = ogrLayer->GetNextFeature();
  if(feat){
    OGRGeometry *geom = feat->GetGeometryRef();
    if(geom){
      geom_type = geom->getGeometryName();

      QString file(filename);
      file.replace(file.length()-3, 3, "dbf");
      // open the dbf file
      std::ifstream dbf((const char*)file, std::ios::in | std::ios::binary);
      // read header
      DbaseHeader dbh;
      dbf.read((char *)&dbh, sizeof(dbh));

      Fda fda;
      for(int field_count = 0, bytes_read = sizeof(dbh); bytes_read < dbh.size_hdr-1; field_count++, bytes_read += sizeof(fda)){
      	dbf.read((char *)&fda, sizeof(fda));
        switch(fda.field_type){
          case 'N': column_types.push_back("int");
                    break;
          case 'D': column_types.push_back("date");
                    break;
          case 'C': column_types.push_back("varchar(256)");
                    break;
          case 'F': column_types.push_back("float");
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
      
    }else{
      valid = false;
      delete geom;
    }
  }else{
    valid = false;
    delete feat;
  }
  ogrLayer->ResetReading();    
  return valid?geom_type:NULL;
}

bool QgsShapeFile::is_valid(){
  return valid;
}

const char * QgsShapeFile::getName(){
  return filename;
}

bool QgsShapeFile::insertLayer(QString dbname, QString srid, PgDatabase * conn, QProgressDialog * pro, int tot=0){
  bool result = true;
  char * geo_temp;
  QString table(filename);
  table = table.section('/', -1);
  table = table.section('.', 0, 0);
  QString query = "CREATE TABLE "+table+"(gid int4, ";
  for(int n=0; n<column_names.size(); n++){
    query += QString(column_names[n]).lower() + " " + QString(column_types[n]);
    if(n < column_names.size() -1)
      query += ", ";
  }
  query += ")";  
  conn->ExecTuplesOk((const char *)query);

  query = "SELECT AddGeometryColumn(\'" + dbname + "\', \'" + table + "\', \'the_geom\', " + srid +
    ", \'" + QString(geom_type) + "\', 2)";
  conn->ExecTuplesOk((const char *)query);

  //adding the data into the table
  for(int m=0;OGRFeature *feat = ogrLayer->GetNextFeature(); m++){
    std::stringstream out;
    out << m;
    query = "INSERT INTO "+table+"values( "+out.str()+", ";
    OGRGeometry *geom = feat->GetGeometryRef();
    
    int num = geom->WkbSize();
    char * geo_temp = new char[num*3];
    geom->exportToWkt(&geo_temp);
    QString geometry(geo_temp);
    
    int numFields = feat->GetFieldCount();
    for(int n=0; n<numFields; n++)
      query += QString("\'")+QString(feat->GetFieldAsString(n))+QString("\', ");
    query += QString("GeometryFromText(\'")+QString(geometry)+QString("\', ")+srid+QString("))");

    conn->ExecTuplesOk((const char *)query);

    pro->setProgress(pro->progress()+1);  
    delete[] geo_temp;
  }   

  return result;
}
