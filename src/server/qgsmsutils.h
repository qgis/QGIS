/***************************************************************************
    qgsmsutils.h
    ---------------------
    begin                : August 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMSUTILS_H
#define QGSMSUTILS_H

#include <QString>

/** Some utility functions that may be included from everywhere in the code*/
namespace QgsMSUtils
{
  /** Creates a ramdom filename for a temporary file. This function also creates the directory to store
     the temporary files if it does not already exist. The directory is /tmp/qgis_map_serv/ under linux or
     the current working directory on windows*/
  QString createTempFilePath();
  /** Stores the specified text in a temporary file. Returns 0 in case of success*/
  int createTextFile( const QString& filePath, const QString& text );
}

#endif
