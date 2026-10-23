/***************************************************************************
    qgsoapifutils.h
    ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOAPIFUTILS_H
#define QGSOAPIFUTILS_H

#include <nlohmann/json.hpp>

#include <QString>

using namespace nlohmann;

#include <QList>
#include <QNetworkReply>
#include <QString>
#include <QStringList>

#include "qgis.h"

using namespace Qt::StringLiterals;

class QgsGeometry;

/**
 * Utility class
*/
class QgsOAPIFJson
{
  public:
    //! A OAPIF Link
    struct Link
    {
        QString href;
        QString rel;
        QString type;
        QString title;
        QStringList profiles;
        qint64 length = -1;
    };

    //! Parses the "link" property of jParent
    static std::vector<Link> parseLinks( const json &jParent );

    //! Find among links the one that matches rel, by using an optional list of preferable types.
    static QString findLink( const std::vector<Link> &links, const QString &rel, const QStringList &preferableTypes = QStringList() );

    //! Write the feature as JSON-FG
    static void writeJsonFgPlace( json &feature, const QgsGeometry &geometry, Qgis::WkbType layerType, const QString &contentCrs, bool asDocument );
};

// Return the href for the next link from the response headers and the MIME type of the format
QString QgsOAPIFGetNextLinkFromResponseHeader( const QList<QNetworkReply::RawHeaderPair> &responseHeaders, const QString &formatType );

class QgsOapifUtils
{
  public:
    static inline const QString DEFAULT_CRS = u"http://www.opengis.net/def/crs/OGC/1.3/CRS84"_s;

    static inline const QString MEDIA_TYPE_JSON = u"application/json"_s;
    static inline const QString MEDIA_TYPE_GEOJSON = u"application/geo+json"_s;
    static inline const QString MEDIA_TYPE_MERGE_PATCH_JSON = u"application/merge-patch+json"_s;
    static inline const QString MEDIA_TYPE_SCHEMA_JSON = u"application/schema+json"_s;
    static inline const QString MEDIA_TYPE_OPENAPI_JSON_3_0 = u"application/openapi+json;version=3.0"_s;
    static inline const QString MEDIA_TYPE_OAI_OPENAPI_JSON_3_0 = u"application/vnd.oai.openapi+json;version=3.0"_s;
    static inline const QString MEDIA_TYPE_FLATGEOBUF = u"application/flatgeobuf"_s;
    static inline const QString MEDIA_TYPE_XML = u"application/xml"_s;
    static inline const QString MEDIA_TYPE_GML = u"application/gml+xml"_s;
    static inline const QString MEDIA_TYPE_GML_3_2 = u"application/gml+xml;version=3.2"_s;

    // Originally there was a plan to have a proper mime type for JSON-FG,
    // but that was changed to using the one of GeoJSON + a profile. For the
    // sake of simplicity, internally, use the pseudo mime type.
    static inline const QString MEDIA_TYPE_PSEUDO_JSONFG = u"application/fg+json"_s;

    static inline const QString PROFILE_JSONFG = u"http://www.opengis.net/def/profile/OGC/0/jsonfg"_s;
    static inline const QString PROFILE_JSONFG_PLUS = u"http://www.opengis.net/def/profile/OGC/0/jsonfg-plus"_s;
    static inline const QString PROFILE_GML_SF0 = u"http://www.opengis.net/def/profile/ogc/2.0/gml-sf0"_s;
    static inline const QString PROFILE_GML_SF2 = u"http://www.opengis.net/def/profile/ogc/2.0/gml-sf2"_s;
};

#endif // QGSOAPIFUTILS_H
