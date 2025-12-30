/***************************************************************************
  qgspostgresconnoauthhandler.cpp

 ---------------------
 begin                : 27.12.2025
 copyright            : (C) 2025 by Jan Dalheimer
 email                : jan@dalheimer.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgresconnoauthhandler.h"

#include <mutex>

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsauthmethod.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

extern "C"
{
#include <libpq-fe.h>
#include <pg_config.h>
}

#ifdef _WIN32
#define SOCKTYPE uintptr_t /* avoids depending on winsock2.h for SOCKET */
#else
#define SOCKTYPE int
#endif

#if PG_MAJORVERSION_NUM >= 18
std::once_flag postgresInitFlag;
int postgresAuthDataHook( PGauthData type, PGconn *conn, void *data )
{
  if ( type != PQAUTHDATA_OAUTH_BEARER_TOKEN )
  {
    return -1;
  }

  PGoauthBearerRequest *request = static_cast<PGoauthBearerRequest *>( data );

  if ( !QgsPostgresConnOAuthHandler::currentlyInScope() )
  {
    // might happen when connecting from outside of QgsPostgresConn, such as from psycopg in Python
    // in practice the connection will likely fail, but at least we won't make anything worse/more confusing this way
    return PQdefaultAuthDataHook( type, conn, data );
  }

  const QString authcfg = QgsPostgresConnOAuthHandler::currentAuthCfg();
  QgsAuthMethod *method = QgsApplication::authManager()->configAuthMethod( authcfg );
  if ( method && method->key() == QLatin1String( "OAuth2" ) )
  {
    QNetworkRequest req;
    // this _is_ a blocking call, however since it does run an event loop (so that the UI
    // stays responsive, in case the connection is established from the main thread) and
    // we never actually use the ability in libpq to connect asynchronously (at least not
    // in QgsPostgresConn) we should be fine to keep this as a blocking call
    const bool success = method->updateNetworkRequest( req, authcfg, QStringLiteral( "postgres" ) );
    if ( success )
    {
      const QByteArray token = req.rawHeader( QByteArrayLiteral( "Token" ) );
      char *tokenStr = new char[token.size() + 1];
      strncpy( tokenStr, token.constData(), token.size() + 1 );
      request->token = tokenStr;
      // libpq does not deallocate the token for us
      request->cleanup = []( PGconn *, PGoauthBearerRequest *request ) { delete static_cast<const char *>( request->token ); };
      return 1;
    }
    else
    {
      QgsLogger::warning( QStringLiteral( "PostgreSQL: OAuth flow failed" ) );
      return -1;
    }
  }
  else
  {
    QgsMessageLog::logMessage( QgsPostgresConnOAuthHandler::tr( "PostgreSQL server request OAuth authentication, however connection in QGIS is not configured to use OAuth" ), QgsPostgresConnOAuthHandler::tr( "PostGIS" ) );
    return -1;
  }
  // Success is indicated by returning an integer greater than zero.
  return 1;
}
#endif

thread_local QString QgsPostgresConnOAuthHandler::sAuthCfg;
thread_local bool QgsPostgresConnOAuthHandler::sInScope = false;
QgsPostgresConnOAuthHandler::QgsPostgresConnOAuthHandler( const QString &authcfg )
{
#if PG_MAJORVERSION_NUM >= 18
  std::call_once( postgresInitFlag, []() {
    PQsetAuthDataHook( postgresAuthDataHook );
  } );
#endif

  if ( sInScope )
  {
    QgsLogger::warning( QStringLiteral( "starting second postgres connection attempt while previous is not yet finished" ) );
  }
  sAuthCfg = authcfg;
  sInScope = true;
}

QgsPostgresConnOAuthHandler::~QgsPostgresConnOAuthHandler()
{
  sAuthCfg = QString();
  sInScope = false;
}
