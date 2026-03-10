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

#include <QString>

using namespace Qt::StringLiterals;

const QString QgsWFSConstants::GML_NAMESPACE( u"http://www.opengis.net/gml"_s );
const QString QgsWFSConstants::OGC_NAMESPACE( u"http://www.opengis.net/ogc"_s );
const QString QgsWFSConstants::OWS_NAMESPACE( u"http://www.opengis.net/ows"_s );
const QString QgsWFSConstants::WFS_NAMESPACE( u"http://www.opengis.net/wfs"_s );
const QString QgsWFSConstants::XMLSCHEMA_NAMESPACE( u"http://www.w3.org/2001/XMLSchema"_s );

const QString QgsWFSConstants::URI_PARAM_URL( u"url"_s );
const QString QgsWFSConstants::URI_PARAM_USERNAME( u"username"_s );
const QString QgsWFSConstants::URI_PARAM_USER( u"user"_s );
const QString QgsWFSConstants::URI_PARAM_PASSWORD( u"password"_s );
const QString QgsWFSConstants::URI_PARAM_AUTHCFG( u"authcfg"_s );
const QString QgsWFSConstants::URI_PARAM_VERSION( u"version"_s );
const QString QgsWFSConstants::URI_PARAM_TYPENAME( u"typename"_s );
const QString QgsWFSConstants::URI_PARAM_SRSNAME( u"srsname"_s );
const QString QgsWFSConstants::URI_PARAM_BBOX( u"bbox"_s );
const QString QgsWFSConstants::URI_PARAM_FILTER( u"filter"_s );
const QString QgsWFSConstants::URI_PARAM_OUTPUTFORMAT( u"outputformat"_s );
const QString QgsWFSConstants::URI_PARAM_RESTRICT_TO_REQUEST_BBOX( u"restrictToRequestBBOX"_s );
const QString QgsWFSConstants::URI_PARAM_MAXNUMFEATURES( u"maxNumFeatures"_s );
const QString QgsWFSConstants::URI_PARAM_IGNOREAXISORIENTATION( u"IgnoreAxisOrientation"_s );
const QString QgsWFSConstants::URI_PARAM_INVERTAXISORIENTATION( u"InvertAxisOrientation"_s );
const QString QgsWFSConstants::URI_PARAM_VALIDATESQLFUNCTIONS( u"validateSQLFunctions"_s );
const QString QgsWFSConstants::URI_PARAM_HIDEDOWNLOADPROGRESSDIALOG( u"hideDownloadProgressDialog"_s );
const QString QgsWFSConstants::URI_PARAM_PAGING_ENABLED( u"pagingEnabled"_s );
const QString QgsWFSConstants::URI_PARAM_PAGE_SIZE( u"pageSize"_s );
const QString QgsWFSConstants::URI_PARAM_WFST_1_1_PREFER_COORDINATES( u"preferCoordinatesForWfsT11"_s );
const QString QgsWFSConstants::URI_PARAM_SKIP_INITIAL_GET_FEATURE( u"skipInitialGetFeature"_s );
const QString QgsWFSConstants::URI_PARAM_FORCE_INITIAL_GET_FEATURE( u"forceInitialGetFeature"_s );
const QString QgsWFSConstants::URI_PARAM_GEOMETRY_TYPE_FILTER( u"geometryTypeFilter"_s );
const QString QgsWFSConstants::URI_PARAM_SQL( u"sql"_s );
const QString QgsWFSConstants::URI_PARAM_HTTPMETHOD( u"httpMethod"_s );
const QString QgsWFSConstants::URI_PARAM_FEATURE_MODE( u"featureMode"_s );

const QString QgsWFSConstants::VERSION_AUTO( u"auto"_s );
