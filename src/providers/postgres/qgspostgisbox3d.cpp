/***************************************************************************
      qgspostgisbox3d.cpp  -  PostgreSQL/PostGIS "box3d" representation and
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

#include "qgspostgisbox3d.h"
#include "qgslogger.h"


QgsPostGisBox3d::QgsPostGisBox3d( std::string box3d )
{

  // If given a nil box3d, give back a nominal QgsPostGisBox3d
  if ( "" == box3d )
  {
    xmin = xmax = ymin = ymax = 0;
    return;
  }

  std::string s;

  box3d = box3d.substr( box3d.find_first_of( "(" ) + 1 );
  box3d = box3d.substr( box3d.find_first_not_of( " " ) );
  s = box3d.substr( 0, box3d.find_first_of( " " ) );
  xmin = strtod( s.c_str(), NULL );

  box3d = box3d.substr( box3d.find_first_of( " " ) + 1 );
  s = box3d.substr( 0, box3d.find_first_of( " " ) );
  ymin = strtod( s.c_str(), NULL );

  box3d = box3d.substr( box3d.find_first_of( "," ) + 1 );
  box3d = box3d.substr( box3d.find_first_not_of( " " ) );
  s = box3d.substr( 0, box3d.find_first_of( " " ) );
  xmax = strtod( s.c_str(), NULL );

  box3d = box3d.substr( box3d.find_first_of( " " ) + 1 );
  s = box3d.substr( 0, box3d.find_first_of( " " ) );
  ymax = strtod( s.c_str(), NULL );

}


QgsPostGisBox3d::~QgsPostGisBox3d()
{
  // NO-OP
}


QString QgsPostGisBox3d::toStringAsBox3d()
{

  QString s;

  s = QString( "BOX3D(%f %f,%f %f)" )
      .arg( xmin )
      .arg( ymin )
      .arg( xmax )
      .arg( ymax );

  QgsDebugMsg( QString( "QgsPostGisBox3d: toStringAsBox3d is returning '%1'" ).arg( s ) );

  return s;
}

