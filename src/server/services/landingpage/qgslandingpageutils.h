/***************************************************************************
  qgslandingpageutils.h - QgsLandingPageUtils

 ---------------------
 begin                : 3.8.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLANDINGPAGEUTILS_H
#define QGSLANDINGPAGEUTILS_H

#include <QMap>
#include <QStringList>
#include <QRegularExpression>

#include "nlohmann/json_fwd.hpp"
#include "qgsserversettings.h"
#include "qgsserverrequest.h"

#ifndef SIP_RUN
using namespace nlohmann;
#endif

class QgsProject;

/**
 * The QgsLandingPageUtils struct contains static utilities for the
 * landing page plugin
 */
struct QgsLandingPageUtils
{

  /**
   * Returns a list of available projects from various sources:
   *
   * - QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES directories
   * - QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS postgres connections
   *
   * Multiple paths and connections may be separated by two pipe chars: '||'
   *
   * \returns hash of project paths (or other storage identifiers) with a digest key
   */
  static QMap<QString, QString> projects( const QgsServerSettings &settings );

  /**
   * Returns project information for a given \a projectPath, optional \a serverSettings and \a request
   */
  static json projectInfo( const QString &projectPath, const QgsServerSettings *serverSettings = nullptr, const QgsServerRequest &request = QgsServerRequest() );

  /**
   * Returns the layer tree information for the given \a project
   */
  static json layerTree( const QgsProject &project, const QStringList &wmsLayersQueryable, const QStringList &wmsLayersSearchable, const QStringList &wmsRestrictedLayers );

  /**
   * Extracts the project hash from the URL and returns the (possibly empty) project path.
   */
  static QString projectPathFromUrl( const QString &url );

  /**
   * PROJECTS_RE regex to extract project hash from URL
   */
  static const QRegularExpression PROJECT_HASH_RE;

  /**
   * Available projects cache
   */
  static QMap<QString, QString> AVAILABLE_PROJECTS;

  /**
   * Extracts and returns the (possibly empty) project URI from the \a url path
   * by examining the project hash.
   */
  static QString projectUriFromUrl( const QString &url, const QgsServerSettings &settings );

};

#endif // QGSLANDINGPAGEUTILS_H
