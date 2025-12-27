/***************************************************************************
    qgsauthorizationsettings.cpp
    ---------------------
    begin                : December 2024
    copyright            : (C) 2024 by Damiano Lombardi
    email                : damiano at opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthorizationsettings.h"

#include "qgslogger.h"

QgsAuthorizationSettings::QgsAuthorizationSettings( const QString &userName, const QString &password, const QgsHttpHeaders &httpHeaders, const QString &authcfg )
  : mUserName( userName )
  , mPassword( password )
  , mHttpHeaders( httpHeaders )
  , mAuthCfg( authcfg )
{}

bool QgsAuthorizationSettings::setAuthorization( QNetworkRequest &request ) const
{
  if ( !mAuthCfg.isEmpty() ) // must be non-empty value
  {
#ifdef HAVE_AUTH
    return QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg );
#else
    QgsDebugError( QStringLiteral( "Auth manager is not available - cannot update network request for authcfg: %1" ).arg( mAuthCfg ) );
    return false;
#endif
  }
  else if ( !mUserName.isEmpty() || !mPassword.isEmpty() )
  {
    request.setRawHeader( "Authorization", "Basic " + QStringLiteral( "%1:%2" ).arg( mUserName, mPassword ).toUtf8().toBase64() );
  }

  mHttpHeaders.updateNetworkRequest( request );

  return true;
}

bool QgsAuthorizationSettings::setAuthorizationReply( QNetworkReply *reply ) const
{
  if ( !mAuthCfg.isEmpty() )
  {
    return QgsApplication::authManager()->updateNetworkReply( reply, mAuthCfg );
  }
  return true;
}
