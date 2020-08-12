/***************************************************************************
    qgsgeopackageprojectstorageguiprovider.cpp
    ---------------------
    begin                : June 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeopackageprojectstorageguiprovider.h"
///@cond PRIVATE

#include "qgsgeopackageprojectstoragedialog.h"

QString QgsGeoPackageProjectStorageGuiProvider::visibleName()
{
  return QObject::tr( "GeoPackage" );
}

QString QgsGeoPackageProjectStorageGuiProvider::showLoadGui()
{
  QgsGeoPackageProjectStorageDialog dlg( false );
  if ( !dlg.exec() )
    return QString();

  return dlg.currentProjectUri();
}

QString QgsGeoPackageProjectStorageGuiProvider::showSaveGui()
{
  QgsGeoPackageProjectStorageDialog dlg( true );
  if ( !dlg.exec() )
    return QString();

  return dlg.currentProjectUri();
}

///@endcond
