/***************************************************************************
                             qgsgdalutils.cpp
                             ----------------
    begin                : September 2018
    copyright            : (C) 2018 Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalutils.h"

#define CPL_SUPRESS_CPLUSPLUS  //#spellok
#include "gdal.h"
#include "cpl_string.h"

#include <QString>

bool QgsGdalUtils::supportsRasterCreate( GDALDriverH driver )
{
  QString driverShortName = GDALGetDriverShortName( driver );
  if ( driverShortName == QLatin1String( "SQLite" ) )
  {
    // it supports Create() but only for vector side
    return false;
  }
  char **driverMetadata = GDALGetMetadata( driver, nullptr );
  return  CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) &&
          CSLFetchBoolean( driverMetadata, GDAL_DCAP_RASTER, false );
}
