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

#include "qgsshapefile.h"

QgsShapeFile::QgsShapeFile(QString filename){
  /*
  ogrDataSource = OGRSFDriverRegistrar::Open((const char *) filename);
  if (ogrDataSource != NULL) qWarning("Valid!");
  else qWarning("Invalid!");
  */
}
QgsShapeFile::~QgsShapeFile(){

}
