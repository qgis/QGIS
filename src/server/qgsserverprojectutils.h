/***************************************************************************
                              qgsserverprojectutils.h
                              -----------------------
  begin                : December 19, 2016
  copyright            : (C) 2016 by Paul Blottiere
  email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSERVERPROJECTUTILS_H
#define QGSSERVERPROJECTUTILS_H

#include "qgis_server.h"
#include "qgsproject.h"

/** \ingroup server
 * The QgsServerProjectUtils namespace provides a way to retrieve specific
 * entries from a QgsProject.
 * @note added in QGIS 3.0
 */
namespace QgsServerProjectUtils
{

  /** Returns if owsService capabilities are enabled.
    * @param project the QGIS project
    * @return if owsService capabilities are enabled.
    */
  SERVER_EXPORT bool owsServiceCapabilities( const QgsProject &project );

  /** Returns the owsService title defined in project.
    * @param project the QGIS project
    * @return the owsService title if defined in project.
    */
  SERVER_EXPORT QString owsServiceTitle( const QgsProject &project );

  /** Returns the owsService abstract defined in project.
    * @param project the QGIS project
    * @return the owsService abstract if defined in project.
    */
  SERVER_EXPORT QString owsServiceAbstract( const QgsProject &project );

  /** Returns the owsService keywords defined in project.
    * @param project the QGIS project
    * @return the owsService keywords if defined in project.
    */
  SERVER_EXPORT QStringList owsServiceKeywords( const QgsProject &project );

  /** Returns the owsService online resource defined in project.
    * @param project the QGIS project
    * @return the owsService online resource if defined in project.
    */
  SERVER_EXPORT QString owsServiceOnlineResource( const QgsProject &project );

  /** Returns the owsService contact organization defined in project.
    * @param project the QGIS project
    * @return the owsService contact organization if defined in project.
    */
  SERVER_EXPORT QString owsServiceContactOrganization( const QgsProject &project );

  /** Returns the owsService contact position defined in project.
    * @param project the QGIS project
    * @return the owsService contact position if defined in project.
    */
  SERVER_EXPORT QString owsServiceContactPosition( const QgsProject &project );

  /** Returns the owsService contact person defined in project.
    * @param project the QGIS project
    * @return the owsService contact person if defined in project.
    */
  SERVER_EXPORT QString owsServiceContactPerson( const QgsProject &project );

  /** Returns the owsService contact mail defined in project.
    * @param project the QGIS project
    * @return the owsService contact mail if defined in project.
    */
  SERVER_EXPORT QString owsServiceContactMail( const QgsProject &project );

  /** Returns the owsService contact phone defined in project.
    * @param project the QGIS project
    * @return the owsService contact phone if defined in project.
    */
  SERVER_EXPORT QString owsServiceContactPhone( const QgsProject &project );

  /** Returns the owsService fees defined in project.
    * @param project the QGIS project
    * @return the owsService fees if defined in project.
    */
  SERVER_EXPORT QString owsServiceFees( const QgsProject &project );

  /** Returns the owsService access constraints defined in project.
    * @param project the QGIS project
    * @return the owsService access constraints if defined in project.
    */
  SERVER_EXPORT QString owsServiceAccessConstraints( const QgsProject &project );

  /** Returns the maximum width for WMS images defined in a QGIS project.
    * @param project the QGIS project
    * @return width if defined in project, -1 otherwise.
    */
  SERVER_EXPORT int wmsMaxWidth( const QgsProject &project );

  /** Returns the maximum height for WMS images defined in a QGIS project.
    * @param project the QGIS project
    * @return height if defined in project, -1 otherwise.
    */
  SERVER_EXPORT int wmsMaxHeight( const QgsProject &project );

  /** Returns the WMS service url defined in a QGIS project.
    * @param project the QGIS project
    * @return url if defined in project, an empty string otherwise.
    */
  SERVER_EXPORT QString wmsServiceUrl( const QgsProject &project );

  /** Returns the WFS service url defined in a QGIS project.
    * @param project the QGIS project
    * @return url if defined in project, an empty string otherwise.
    */
  SERVER_EXPORT QString wfsServiceUrl( const QgsProject &project );

  /** Returns the WCS service url defined in a QGIS project.
    * @param project the QGIS project
    * @return url if defined in project, an empty string otherwise.
    */
  SERVER_EXPORT QString wcsServiceUrl( const QgsProject &project );

  /** Returns the Layer ids list defined in a QGIS project as published in WCS.
    * @param project the QGIS project
    * @return the Layer ids list.
    */
  SERVER_EXPORT QStringList wcsLayers( const QgsProject &project );
};

#endif
