/***************************************************************************
    qgswfsconstants.h
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSCONSTANTS_H
#define QGSWFSCONSTANTS_H

#include <QString>

struct QgsWFSConstants
{
  static const QString GML_NAMESPACE;
  static const QString OGC_NAMESPACE;
  static const QString OWS_NAMESPACE;
  static const QString WFS_NAMESPACE;
  static const QString XMLSCHEMA_NAMESPACE;

  // URI parameters
  static const QString URI_PARAM_URL;
  static const QString URI_PARAM_USERNAME;
  static const QString URI_PARAM_PASSWORD;
  static const QString URI_PARAM_AUTHCFG;
  static const QString URI_PARAM_VERSION;
  static const QString URI_PARAM_TYPENAME;
  static const QString URI_PARAM_SRSNAME;
  static const QString URI_PARAM_FILTER;
  static const QString URI_PARAM_RESTRICT_TO_REQUEST_BBOX;
  static const QString URI_PARAM_MAXNUMFEATURES;
  static const QString URI_PARAM_IGNOREAXISORIENTATION;
  static const QString URI_PARAM_INVERTAXISORIENTATION;
  static const QString URI_PARAM_VALIDATESQLFUNCTIONS;
  static const QString URI_PARAM_HIDEDOWNLOADPROGRESSDIALOG;

  //
  static const QString VERSION_AUTO;

  // Settings
  static const QString CONNECTIONS_WFS;
  static const QString SETTINGS_VERSION;
  static const QString SETTINGS_MAXNUMFEATURES;

  // Special fields of the cache
  static const QString FIELD_GEN_COUNTER;
  static const QString FIELD_GMLID;
  static const QString FIELD_HEXWKB_GEOM;
  static const QString FIELD_MD5;
};

#endif // QGSWFSCONSTANTS_H
