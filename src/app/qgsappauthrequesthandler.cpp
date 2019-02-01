/***************************************************************************
    qgsappauthrequesthandler.cpp
    ---------------------------
    begin                : January 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappauthrequesthandler.h"
#include "qgslogger.h"
#include "qgsauthcertutils.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgisapp.h"
#include "qgscredentials.h"
#include <QAuthenticator>

void QgsAppAuthRequestHandler::handleAuthRequest( QNetworkReply *reply, QAuthenticator *auth )
{
  QString username = auth->user();
  QString password = auth->password();

  if ( username.isEmpty() && password.isEmpty() && reply->request().hasRawHeader( "Authorization" ) )
  {
    QByteArray header( reply->request().rawHeader( "Authorization" ) );
    if ( header.startsWith( "Basic " ) )
    {
      QByteArray auth( QByteArray::fromBase64( header.mid( 6 ) ) );
      int pos = auth.indexOf( ':' );
      if ( pos >= 0 )
      {
        username = auth.left( pos );
        password = auth.mid( pos + 1 );
      }
    }
  }

  for ( ;; )
  {
    bool ok = QgsCredentials::instance()->get(
                QStringLiteral( "%1 at %2" ).arg( auth->realm(), reply->url().host() ),
                username, password,
                QObject::tr( "Authentication required" ) );
    if ( !ok )
      return;

    if ( auth->user() != username || ( password != auth->password() && !password.isNull() ) )
      break;

    // credentials didn't change - stored ones probably wrong? clear password and retry
    QgsCredentials::instance()->put(
      QStringLiteral( "%1 at %2" ).arg( auth->realm(), reply->url().host() ),
      username, QString() );
  }

  // save credentials
  QgsCredentials::instance()->put(
    QStringLiteral( "%1 at %2" ).arg( auth->realm(), reply->url().host() ),
    username, password
  );

  auth->setUser( username );
  auth->setPassword( password );
}
