/***************************************************************************
    qgswfsconstants.cpp
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

#include "qgswfsconstants.h"

const QString QgsWFSConstants::GML_NAMESPACE( "http://www.opengis.net/gml" );
const QString QgsWFSConstants::OGC_NAMESPACE( "http://www.opengis.net/ogc" );
const QString QgsWFSConstants::OWS_NAMESPACE( "http://www.opengis.net/ows" );
const QString QgsWFSConstants::WFS_NAMESPACE( "http://www.opengis.net/wfs" );
const QString QgsWFSConstants::XMLSCHEMA_NAMESPACE( "http://www.w3.org/2001/XMLSchema" );

const QString QgsWFSConstants::URI_PARAM_URL( "url" );
const QString QgsWFSConstants::URI_PARAM_USERNAME( "username" );
const QString QgsWFSConstants::URI_PARAM_PASSWORD( "password" );
const QString QgsWFSConstants::URI_PARAM_AUTHCFG( "authcfg" );
const QString QgsWFSConstants::URI_PARAM_VERSION( "version" );
const QString QgsWFSConstants::URI_PARAM_TYPENAME( "typename" );
const QString QgsWFSConstants::URI_PARAM_SRSNAME( "srsname" );
const QString QgsWFSConstants::URI_PARAM_FILTER( "filter" );
const QString QgsWFSConstants::URI_PARAM_RESTRICT_TO_REQUEST_BBOX( "retrictToRequestBBOX" );
const QString QgsWFSConstants::URI_PARAM_MAXNUMFEATURES( "maxNumFeatures" );
const QString QgsWFSConstants::URI_PARAM_IGNOREAXISORIENTATION( "IgnoreAxisOrientation" );
const QString QgsWFSConstants::URI_PARAM_INVERTAXISORIENTATION( "InvertAxisOrientation" );
const QString QgsWFSConstants::URI_PARAM_VALIDATESQLFUNCTIONS( "validateSQLFunctions" );
const QString QgsWFSConstants::URI_PARAM_HIDEDOWNLOADPROGRESSDIALOG( "hideDownloadProgressDialog" );

const QString QgsWFSConstants::VERSION_AUTO( "auto" );

const QString QgsWFSConstants::CONNECTIONS_WFS( "/Qgis/connections-wfs/" );
const QString QgsWFSConstants::SETTINGS_VERSION( "version" );
const QString QgsWFSConstants::SETTINGS_MAXNUMFEATURES( "maxnumfeatures" );

const QString QgsWFSConstants::FIELD_GEN_COUNTER( "__qgis_gen_counter" );
const QString QgsWFSConstants::FIELD_GMLID( "__qgis_gmlid" );
const QString QgsWFSConstants::FIELD_HEXWKB_GEOM( "__qgis_hexwkb_geom" );
const QString QgsWFSConstants::FIELD_MD5( "__qgis_md5" );
