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

#include<qstring.h>
 
//#include "qva	OGRDataSource *ogrDataSource;luevector.h"
 
class QgsShapeFile
{
  public:
  
  QgsShapeFile(QString filename);
  ~QgsShapeFile();


  private:
  //OGRDataSource *ogrDataSource;

};
