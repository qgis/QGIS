/***************************************************************************
                          qgsshapefile.h  -  description
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

#ifndef QGSSHAPEFILE_H
#define QGSSHAPEFILE_H

#include<qstring.h>
#include <ogrsf_frmts.h>

class OGRLayer;
class OGRDataSource;
 
class QgsShapeFile
{
  public:  
  QgsShapeFile(QString filename);
  ~QgsShapeFile();

  const char * getFeatureCount();
  const char * getFeatureClass();
  bool is_valid();
  const char * getName();


  private:
  OGRDataSource *ogrDataSource;
  OGRLayer * ogrLayer;
  bool valid;
  const char * filename;
  //const char *featureType = {"null", "Point", "Line", "Polygon", "MultiPoint", "MultiLine", "MultiPolygon"};

};

#endif
