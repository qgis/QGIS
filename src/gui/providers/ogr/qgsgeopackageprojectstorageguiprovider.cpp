/***************************************************************************
    qgsgeopackageprojectstorageguiprovider.cpp
    ---------------------
    begin                : June 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
