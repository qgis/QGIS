/***************************************************************************
      qgspostgisbox3d.h  -  PostgreSQL/PostGIS "box2d" representation and
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

#ifndef QGSPOSTGISBOX2D_H
#define QGSPOSTGISBOX2D_H

#include <string>

#include <qstring.h>

#include "../../src/qgsrect.h"

/*!
 * \brief   PostgreSQL/PostGIS "box3d" representation and transformation
   \author  Brendan Morley
   \date    March 2005

   
   This object is designed to represent and transform the PostGIS "box2d"
   data type.
   
   \note    This class only has meaning with PostGIS 1.0 installations.
            pre-1.0 only understands the box3d version
            
   TODO     Enforce the PostGIS 1.0 prerequisite         
     
 */ 

class QgsPostGisBox2d : public QgsRect
{

public:

  /*!  Constructor from a string
       \param   box2d  The box2d formed as a PostGIS SQL string,
                       formed by functions such as PQgetvalue
   */
  
  QgsPostGisBox2d( std::string box2d );

  
  ~QgsPostGisBox2d();
  
  
  /*!  Returns this object as a PostGIS SQL-compatible QString
   */

  
  QString stringRepAsBox2d();
  
  
private:
  
    
};


#endif
