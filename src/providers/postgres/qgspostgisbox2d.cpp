/***************************************************************************
      qgspostgisbox3d.cpp  -  PostgreSQL/PostGIS "box2d" representation and
                              transformation
                             -------------------
    begin                : Feb 1, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
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

#include <fstream>
#include <cstdlib>

#include <QString>

#include "qgsrect.h"

#include "qgspostgisbox2d.h"


QgsPostGisBox2d::QgsPostGisBox2d( std::string box2d )
{

  // If given a nil box2d, give back a nominal QgsPostGisBox2d
  if ("" == box2d)
  {
    xmin = xmax = ymin = ymax = 0;
    return;
  }

  std::string s;

  box2d = box2d.substr(box2d.find_first_of("(")+1);
  box2d = box2d.substr(box2d.find_first_not_of(" "));
  s = box2d.substr(0, box2d.find_first_of(" "));
  xmin = strtod(s.c_str(), NULL);

  box2d = box2d.substr(box2d.find_first_of(" ")+1);
  s = box2d.substr(0, box2d.find_first_of(" "));
  ymin = strtod(s.c_str(), NULL);

  box2d = box2d.substr(box2d.find_first_of(",")+1);
  box2d = box2d.substr(box2d.find_first_not_of(" "));
  s = box2d.substr(0, box2d.find_first_of(" "));
  xmax = strtod(s.c_str(), NULL);

  box2d = box2d.substr(box2d.find_first_of(" ")+1);
  s = box2d.substr(0, box2d.find_first_of(" "));
  ymax = strtod(s.c_str(), NULL);
}


QgsPostGisBox2d::~QgsPostGisBox2d()
{
  // NO-OP
}


QString QgsPostGisBox2d::stringRepAsBox2d()
{

  QString s;
  
  s = QString( "BOX2D(%f %f,%f %f)" )
        .arg( xmin )
        .arg( ymin )
        .arg( xmax )
        .arg( ymax );

  std::cerr << "QgsPostGisBox2d: stringRepAsBox2d is returning '" << s.toLocal8Bit().data() << "'" << std::endl;

  return s;        
}

