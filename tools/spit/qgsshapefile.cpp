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

#include "cpl_error.h"
#include "qgsshapefile.h"

QgsShapeFile::QgsShapeFile(QString filename){

  valid = false;
  OGRRegisterAll();
  ogrDataSource = OGRSFDriverRegistrar::Open((const char *) filename);
  if (ogrDataSource != NULL){
    valid = true;
    ogrLayer = ogrDataSource->GetLayer(0);

  }
  else 
    qWarning(filename + " Invalid!" + CPLGetLastErrorMsg());
  
}

QgsShapeFile::~QgsShapeFile(){
  delete ogrLayer;
  delete ogrDataSource;
}

const char * QgsShapeFile::getFeatureCount(){
  std::ostringstream res;
  res << ogrLayer->GetFeatureCount();
  return res.str().c_str();
}

const char * QgsShapeFile::getFeatureClass(){
  const char * res;
  OGRFeature *feat = ogrLayer->GetNextFeature();
  if(feat){
    OGRGeometry *geom = feat->GetGeometryRef();
    if(geom){
      res = geom->getGeometryName();
    }else{
      valid = false;
      delete geom;
    }
  }else{
    valid = false;
    delete feat;
  }
  ogrLayer->ResetReading();    
  return valid?res:NULL;
}
