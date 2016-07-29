/***************************************************************************
                    qgsgrassrasterprovidermodule.cpp
                     -------------------
    begin                : October, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QString>

#include "qgsgrassrasterprovider.h"

QGISEXTERN QgsGrassRasterProvider * classFactory( const QString *uri )
{
  return new QgsGrassRasterProvider( *uri );
}

QGISEXTERN QString providerKey()
{
  return QString( "grassraster" );
}

QGISEXTERN QString description()
{
  return QString( "GRASS %1 raster provider" ).arg( GRASS_VERSION_MAJOR );
}

QGISEXTERN bool isProvider()
{
  return true;
}
