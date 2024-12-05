/***************************************************************************
    qgsauthorizationsettings.h
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

#ifndef QGSAUTHORIZATIONSETTINGS_H
#define QGSAUTHORIZATIONSETTINGS_H

#include "qgsauthmanager.h"
#include "qgsapplication.h"
#include "qgshttpheaders.h"

#include <QString>
#include <QNetworkRequest>
#include <QNetworkReply>

#define SIP_NO_FILE

/**
 * \ingroup core
 * \class QgsAuthorizationSettings
 * \brief Utility class that contains authorization informations.
 * \since QGIS 3.42
 */
struct QgsAuthorizationSettings
{
  QgsAuthorizationSettings( const QString &userName = QString(), const QString &password = QString(), const QgsHttpHeaders &httpHeaders = QgsHttpHeaders(), const QString &authcfg = QString() )
    : mUserName( userName )
    , mPassword( password )
    , mHttpHeaders( httpHeaders )
    , mAuthCfg( authcfg )
  {}

  //! Update authorization for request
  bool setAuthorization( QNetworkRequest &request ) const
  {
    if ( !mAuthCfg.isEmpty() ) // must be non-empty value
    {
      return QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg );
    }
    else if ( !mUserName.isEmpty() || !mPassword.isEmpty() )
    {
      request.setRawHeader( "Authorization", "Basic " + QStringLiteral( "%1:%2" ).arg( mUserName, mPassword ).toUtf8().toBase64() );
    }

    mHttpHeaders.updateNetworkRequest( request );

    return true;
  }

  //! Update authorization for reply
  bool setAuthorizationReply( QNetworkReply *reply ) const
  {
    if ( !mAuthCfg.isEmpty() )
    {
      return QgsApplication::authManager()->updateNetworkReply( reply, mAuthCfg );
    }
    return true;
  }

  //! Username for basic http authentication
  QString mUserName;

  //! Password for basic http authentication
  QString mPassword;

  //! headers for http requests
  QgsHttpHeaders mHttpHeaders;

  //! Authentication configuration ID
  QString mAuthCfg;
};

#endif // QGSAUTHORIZATIONSETTINGS_H
