/***************************************************************************
      qgspostgisbox3d.h  -  PostgreSQL/PostGIS "box3d" representation and
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

#ifndef QGSPOSTGISBOX3D_H
#define QGSPOSTGISBOX3D_H

#include <string>

#include "qgsrect.h"

/*!
 * \brief   PostgreSQL/PostGIS "box3d" representation and transformation
   \author  Brendan Morley
   \date    March 2005

   
   This object is designed to represent and transform the PostGIS "box3d"
   data type.
   
   \note    Only 2 dimensions are handled at this time.
     
 */ 

class QgsPostGisBox3d : public QgsRect
{

public:

  /*!  Constructor from a string
       \param   box3d  The box3d formed as a PostGIS SQL string,
                       formed by functions such as PQgetvalue
   */
  
  QgsPostGisBox3d( std::string box3d );

  
  ~QgsPostGisBox3d();
  
  
  /*!  Returns this object as a PostGIS SQL-compatible QString
   */

  
  QString toStringAsBox3d();
  
  
protected:
  
  double zmin;
  double zmax;
    
};


#endif
