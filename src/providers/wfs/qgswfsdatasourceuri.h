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

#include "qgsauthmanager.h"
#include "qgsdatasourceuri.h"
#include "qgsrectangle.h"

#include <QNetworkRequest>
#include <QString>

// TODO: merge with QgsWmsAuthorization?
struct QgsWFSAuthorization
{
  QgsWFSAuthorization( const QString& userName = QString(), const QString& password = QString(), const QString& authcfg = QString() )
      : mUserName( userName )
      , mPassword( password )
      , mAuthCfg( authcfg )
  {}

  //! set authorization header
  bool setAuthorization( QNetworkRequest &request ) const
  {
    if ( !mAuthCfg.isEmpty() )
    {
      return QgsAuthManager::instance()->updateNetworkRequest( request, mAuthCfg );
    }
    else if ( !mUserName.isNull() || !mPassword.isNull() )
    {
      request.setRawHeader( "Authorization", "Basic " + QString( "%1:%2" ).arg( mUserName, mPassword ).toAscii().toBase64() );
    }
    return true;
  }

  //! Username for basic http authentication
  QString mUserName;

  //! Password for basic http authentication
  QString mPassword;

  //! Authentication configuration ID
  QString mAuthCfg;
};

/** Utility class that wraps a QgsDataSourceURI with conveniency
 * methods with the parameters used for a WFS URI.
 */
class QgsWFSDataSourceURI
{
  public:

    explicit QgsWFSDataSourceURI( const QString& uri );

    /** Return the URI */
    QString uri();

    /** Return base URL (with SERVICE=WFS parameter if bIncludeServiceWFS=true) */
    QUrl baseURL( bool bIncludeServiceWFS = true ) const;

    /** Get WFS version. Can be auto, 1.0.0, 1.1.0 or 2.0.0. */
    QString version() const;

    /** Return user defined limit of features to download. 0=no limitation */
    int maxNumFeatures() const;

    /** Set user defined limit of features to download */
    void setMaxNumFeatures( int maxNumFeatures );

    /** Get typename (with prefix) */
    QString typeName() const;

    /** Set typename (with prefix)*/
    void setTypeName( const QString& typeName );

    /** Get SRS name (in the normalized form EPSG:xxxx) */
    QString SRSName() const;

    /** Set SRS name (in the normalized form EPSG:xxxx) */
    void setSRSName( const QString& crsString );

    /** Get OGC filter xml or a QGIS expression */
    QString filter() const;

    /** Set OGC filter xml or a QGIS expression */
    void setFilter( const QString& filterIn );

    /** Get SQL query */
    QString sql() const;

    /** Set SQL query */
    void setSql( const QString& sql );

    /** Returns whether GetFeature request should include the request bounding box. Defaults to false */
    bool isRestrictedToRequestBBOX() const;

    /** Returns whether axis orientation should be ignored (for WFS >= 1.1). Defaults to false */
    bool ignoreAxisOrientation() const;

    /** Returns whether axis orientation should be inverted. Defaults to false */
    bool invertAxisOrientation() const;

    /** For debug purposes. Checks that functions used in sql match functions declared by the server. Defaults to false */
    bool validateSqlFunctions() const;

    /** Whether to hide download progress dialog in QGIS main app. Defaults to false */
    bool hideDownloadProgressDialog() const;

    /** Return authorization parameters */
    QgsWFSAuthorization& auth() { return mAuth; }

    /** Builds a derived uri from a base uri */
    static QString build( const QString& uri,
                          const QString& typeName,
                          const QString& crsString = QString(),
                          const QString& sql = QString(),
                          bool restrictToCurrentViewExtent = false );

  private:
    QgsDataSourceURI    mURI;
    QgsWFSAuthorization mAuth;
};


#endif // QGSWFSDATASOURCEURI_H
