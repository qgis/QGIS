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
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "qgsdbfbase.h"
#include "cpl_error.h"
#include "qgsshapefile.h"

QgsShapeFile::QgsShapeFile(QString name){
  filename = (const char*)name;
  OGRRegisterAll();
  ogrDataSource = OGRSFDriverRegistrar::Open((const char *) filename);
  if (ogrDataSource != NULL){
    valid = true;
    ogrLayer = ogrDataSource->GetLayer(0);
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

const char * QgsShapeFile::getFeatureCount(){
  std::ostringstream res;
  res << ogrLayer->GetFeatureCount();
  return res.str().c_str();
}

const char * QgsShapeFile::getFeatureClass(){
  std::ostringstream out;
  OGRFeature *feat = ogrLayer->GetNextFeature();
  if(feat){
    OGRGeometry *geom = feat->GetGeometryRef();
    if(geom){

      geom_type = geom->getGeometryName();
      int num = geom->WkbSize();
      char * geo_temp = new char[num*3];
      geom->exportToWkt(&geo_temp);

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
        column_types.push_back(fda.field_type);
      }
      dbf.close();
      
      int numFields = feat->GetFieldCount();
      for(int n=0; n<numFields; n++){
        column_names.push_back(feat->GetFieldDefnRef(n)->GetNameRef());
        std::cout << column_names[n] << "of type: " << column_types[n] << std::endl;
      }

      QString geometry(geo_temp);
      delete[] geo_temp;
      
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
