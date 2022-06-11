/***************************************************************************
    qgswfsdatasourceuri.h
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

#ifndef QGSWFSDATASOURCEURI_H
#define QGSWFSDATASOURCEURI_H

#include "qgsdatasourceuri.h"
#include "qgsrectangle.h"
#include "qgsapplication.h"
#include "qgsauthorizationsettings.h"

#include <QNetworkRequest>
#include <QSet>
#include <QString>

/**
 * Utility class that wraps a QgsDataSourceUri with conveniency
 * methods with the parameters used for a WFS URI.
 */
class QgsWFSDataSourceURI
{
  public:

    //! Http method for DCP URIs
    enum Method
    {
      Get,
      Post
    };

    explicit QgsWFSDataSourceURI( const QString &uri );

    //! Returns the URI, optionally with the authentication configuration expanded
    QString uri( bool expandAuthConfig = false ) const;

    //! Returns base URL (with SERVICE=WFS parameter if bIncludeServiceWFS=true)
    QUrl baseURL( bool bIncludeServiceWFS = true ) const;

    //! Returns request URL (with SERVICE=WFS parameter)
    QUrl requestUrl( const QString &request, const Method &method = Method::Get ) const;

    //! Gets WFS version. Can be auto, 1.0.0, 1.1.0 or 2.0.0.
    QString version() const;

    //! Returns user defined limit of features to download. 0=no limitation
    long long maxNumFeatures() const;

    //! Sets user defined limit of features to download
    void setMaxNumFeatures( long long maxNumFeatures );

    //! Returns user defined limit page size. 0=server udefault
    long long pageSize() const;

    //! Returns whether paging is enabled.
    bool pagingEnabled() const;

    //! Gets typename (with prefix)
    QString typeName() const;

    //! Sets typename (with prefix)
    void setTypeName( const QString &typeName );

    //! Gets SRS name (in the normalized form EPSG:xxxx)
    QString SRSName() const;

    //! Sets SRS name (in the normalized form EPSG:xxxx)
    void setSRSName( const QString &crsString );

    //! Sets version
    void setVersion( const QString &versionString );

    //! Gets OGC filter xml or a QGIS expression
    QString filter() const;

    //! Sets OGC filter xml or a QGIS expression
    void setFilter( const QString &filterIn );

    //! Gets SQL query
    QString sql() const;

    //! Sets SQL query
    void setSql( const QString &sql );

    //! Gets GetFeature output format
    QString outputFormat() const;

    //! Sets GetFeature output format
    void setOutputFormat( const QString &outputFormat );

    //! Returns whether GetFeature request should include the request bounding box. Defaults to false
    bool isRestrictedToRequestBBOX() const;

    //! Returns whether axis orientation should be ignored (for WFS >= 1.1). Defaults to false
    bool ignoreAxisOrientation() const;

    //! Returns whether axis orientation should be inverted. Defaults to false
    bool invertAxisOrientation() const;

    //! For debug purposes. Checks that functions used in sql match functions declared by the server. Defaults to false
    bool validateSqlFunctions() const;

    //! Whether to hide download progress dialog in QGIS main app. Defaults to false
    bool hideDownloadProgressDialog() const;

    //! Whether to use "coordinates" instead of "pos" and "posList" for WFS-T 1.1 transactions (ESRI mapserver)
    bool preferCoordinatesForWfst11() const;

    //! Returns authorization parameters
    const QgsAuthorizationSettings &auth() const { return mAuth; }

    //! Builds a derived uri from a base uri
    static QString build( const QString &uri,
                          const QString &typeName,
                          const QString &crsString = QString(),
                          const QString &sql = QString(),
                          const QString &filter = QString(),
                          bool restrictToCurrentViewExtent = false );

    //! Sets Get DCP endpoints
    void setGetEndpoints( const QgsStringMap &map );

    //! Sets Post DCP endpoints
    void setPostEndpoints( const QgsStringMap &map );

    //! Return set of unknown parameter keys in the URI.
    QSet<QString> unknownParamKeys() const;

  private:
    QgsDataSourceUri    mURI;
    QgsAuthorizationSettings mAuth;
    QgsStringMap mGetEndpoints;
    QgsStringMap mPostEndpoints;
    bool mDeprecatedURI = false;
};


#endif // QGSWFSDATASOURCEURI_H
