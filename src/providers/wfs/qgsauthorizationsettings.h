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

#include <QString>
#include <QNetworkRequest>
#include <QNetworkReply>

// TODO: merge with QgsWmsAuthorization?
struct QgsAuthorizationSettings
{
  QgsAuthorizationSettings( const QString &userName = QString(), const QString &password = QString(), const QString &authcfg = QString() )
    : mUserName( userName )
    , mPassword( password )
    , mAuthCfg( authcfg )
  {}

  //! update authorization for request
  bool setAuthorization( QNetworkRequest &request ) const
  {
    if ( !mAuthCfg.isEmpty() ) // must be non-empty value
    {
      return QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg );
    }
    else if ( !mUserName.isNull() || !mPassword.isNull() ) // allow empty values
    {
      request.setRawHeader( "Authorization", "Basic " + QStringLiteral( "%1:%2" ).arg( mUserName, mPassword ).toLatin1().toBase64() );
    }
    return true;
  }

  //! update authorization for reply
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

  //! Authentication configuration ID
  QString mAuthCfg;
};

#endif // QGSAUTHORIZATIONSETTINGS_H
