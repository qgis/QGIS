/***************************************************************************
               qgsfile.cpp  - Basic file class, QFile add-in.
                             -------------------
    begin                : 2005-10-08
    copyright            : (C) 2005 Mateusz Łoskot
    email                : mateusz at loskot dot net
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
#include "qgsfile.h"
#include <qfileinfo.h>
#include <fstream>

/**************************************************************************
     Constructors/Destructor
**************************************************************************/

QgsFile::QgsFile()
{
  /* construction */
}

QgsFile::QgsFile(const QString& name) : QFile(name)
{
  /* construction */
}

QgsFile::~QgsFile()
{
  /* destruction */
}

/**************************************************************************
     Public Methods
**************************************************************************/

bool QgsFile::copy( const QString& newName )
{
  // Just be sure file is closed
  close();

  // Copy file to new location
  if ( !copy( name(), newName ) )
  {
    return false;
  }

  // Success
  return true;
}

bool QgsFile::rename (const QString & newName)
{
  return false;
}

/**************************************************************************
     Public Static Methods
**************************************************************************/

bool QgsFile::copy(const QString& fileName, const QString& newName)
{
  // Check if given paths exist
  QFileInfo srcFi(fileName);
  if (!srcFi.exists())
  {
    return false;
  }

  // Check required permissions
  if (!srcFi.isReadable())
    return false;

  // XXX - check if we can write to the destination
  /*
  if (!dstFi.isWritable())
  return file;
  */

  // Open source file to read
  std::ifstream in(fileName.latin1());
  if (!in.is_open())
  {
    return false;
  }

  // Open new file to write
  std::ofstream out(newName.latin1());
  if (!out.is_open())
  {
    return false;
  }

  // Copy byte-by-byte - safe for text and binary files
  char c;
  while (in.get(c))
  {
    out.put(c);
  }

  // Check copy operation results
  if (!in.eof() || !out.good())
  {
    return false; // undefined error
  }

  in.close();
  out.close();

  return true;
}



