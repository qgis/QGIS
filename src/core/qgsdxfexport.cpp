/***************************************************************************
                         qgsdxfexport.cpp
                         ----------------
    begin                : September 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdxfexport.h"
#include <QIODevice>
#include <QTextStream>

QgsDxfExport::QgsDxfExport()
{
}

QgsDxfExport::~QgsDxfExport()
{

}

int QgsDxfExport::writeToFile( QIODevice* d )
{
  if ( !d )
  {
    return 1;
  }

  if ( !d->open( QIODevice::WriteOnly ) )
  {
    return 2;
  }

  QTextStream outStream( d );
  writeHeader( outStream );
  return 0;
}

void QgsDxfExport::writeHeader( QTextStream& stream )
{
  stream << "999\n";
  stream << "DXF created from QGIS\n";
  stream << "  0\n";
  stream << "SECTION\n";
  stream << "  2\n";
  stream << "HEADER\n";
  stream << "  9\n";
  stream << "$LTSCALE\n";
  stream << " 40\n";
  stream << "1\n";
  stream << "  0\n";
  stream << "ENDSEC\n";
}

void QgsDxfExport::writeEndFile( QTextStream& stream )
{
  stream << "  0\n";
  stream << "ENDSEC\n";
}
