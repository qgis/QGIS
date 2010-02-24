/***************************************************************************
  qgshttptransaction.cpp  -  Tracks a HTTP request with its response,
                             with particular attention to tracking
                             HTTP redirect responses
                             -------------------
    begin                : 17 Mar, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* $Id: qgshttptransaction.cpp 5697 2006-08-15 10:29:46Z morb_au $ */

#include <fstream>

#include "qgshttptransaction.h"
#include "qgslogger.h"
#include "qgsconfig.h"

#include <QApplication>
#include <QUrl>
#include <QSettings>
#include <QTimer>
#include "qgslogger.h"

static int HTTP_PORT_DEFAULT = 80;

//XXX Set the connection name when creating the provider instance
//XXX in qgswmsprovider. When creating a QgsHttpTransaction, pass
//XXX the user/pass combination to the constructor. Then set the
//XXX username and password using QHttp::setUser.
QgsHttpTransaction::QgsHttpTransaction( QString uri,
                                        QString proxyHost,
                                        int     proxyPort,
                                        QString proxyUser,
                                        QString proxyPass,
                                        QNetworkProxy::ProxyType proxyType,
                                        QString userName,
                                        QString password )
    : httpresponsecontenttype( 0 ),
    httpurl( uri ),
    httphost( proxyHost ),
    mError( 0 )
{
  QSettings s;
  mNetworkTimeoutMsec = s.value( "/qgis/networkAndProxy/networkTimeout", "20000" ).toInt();
}

QgsHttpTransaction::QgsHttpTransaction()
{

}

QgsHttpTransaction::~QgsHttpTransaction()
{
  QgsDebugMsg( "deconstructing." );
}


void QgsHttpTransaction::setCredentials( const QString& username, const QString& password )
{
  mUserName = username;
  mPassword = password;
}
void QgsHttpTransaction::getAsynchronously()
{

  //TODO

}

bool QgsHttpTransaction::getSynchronously( QByteArray &respondedContent, int redirections, const QByteArray* postData )
{

  httpredirections = redirections;

  QgsDebugMsg( "Entered." );
  QgsDebugMsg( "Using '" + httpurl + "'." );
  QgsDebugMsg( "Creds: " + mUserName + "/" + mPassword );

  int httpport;

  QUrl qurl( httpurl );

  http = new QHttp( );
  // Create a header so we can set the user agent (Per WMS RFC).
  QHttpRequestHeader header( "GET", qurl.host() );
  // Set host in the header
  if ( qurl.port( HTTP_PORT_DEFAULT ) == HTTP_PORT_DEFAULT )
  {
    header.setValue( "Host", qurl.host() );
  }
  else
  {
    header.setValue( "Host", QString( "%1:%2" ).arg( qurl.host() ).arg( qurl.port() ) );
  }
  // Set the user agent to Quantum GIS plus the version name
  header.setValue( "User-agent", QString( "Quantum GIS - " ) + VERSION );
  // Set the host in the QHttp object
  http->setHost( qurl.host(), qurl.port( HTTP_PORT_DEFAULT ) );
  // Set the username and password if supplied for this connection
  // If we have username and password set in header
  if ( !mUserName.isEmpty() && !mPassword.isEmpty() )
  {
    http->setUser( mUserName, mPassword );
  }

  if ( !QgsHttpTransaction::applyProxySettings( *http, httpurl ) )
  {
    httphost = qurl.host();
    httpport = qurl.port( HTTP_PORT_DEFAULT );
  }
  else
  {
    //proxy enabled, read httphost and httpport from settings
    QSettings settings;
    httphost = settings.value( "proxy/proxyHost", "" ).toString();
    httpport = settings.value( "proxy/proxyPort", "" ).toString().toInt();
  }

//  int httpid1 = http->setHost( qurl.host(), qurl.port() );

  mWatchdogTimer = new QTimer( this );

  QgsDebugMsg( "qurl.host() is '" + qurl.host() + "'." );

  httpresponse.truncate( 0 );

  // Some WMS servers don't like receiving a http request that
  // includes the scheme, host and port (the
  // http://www.address.bit:80), so remove that from the url before
  // executing an http GET.

  //Path could be just '/' so we remove the 'http://' first
  QString pathAndQuery = httpurl.remove( 0, httpurl.indexOf( qurl.host() ) );
  pathAndQuery = httpurl.remove( 0, pathAndQuery.indexOf( qurl.path() ) );
  if ( !postData ) //do request with HTTP GET
  {
    header.setRequest( "GET", pathAndQuery );
    // do GET using header containing user-agent
    httpid = http->request( header );
  }
  else //do request with HTTP POST
  {
    header.setRequest( "POST", pathAndQuery );
    // do POST using header containing user-agent
    httpid = http->request( header, *postData );
  }

  connect( http, SIGNAL( requestStarted( int ) ),
           this,      SLOT( dataStarted( int ) ) );

  connect( http, SIGNAL( responseHeaderReceived( const QHttpResponseHeader& ) ),
           this,       SLOT( dataHeaderReceived( const QHttpResponseHeader& ) ) );

  connect( http,  SIGNAL( readyRead( const QHttpResponseHeader& ) ),
           this, SLOT( dataReceived( const QHttpResponseHeader& ) ) );

  connect( http, SIGNAL( dataReadProgress( int, int ) ),
           this,       SLOT( dataProgress( int, int ) ) );

  connect( http, SIGNAL( requestFinished( int, bool ) ),
           this,      SLOT( dataFinished( int, bool ) ) );

  connect( http, SIGNAL( done( bool ) ),
           this, SLOT( transactionFinished( bool ) ) );

  connect( http,   SIGNAL( stateChanged( int ) ),
           this, SLOT( dataStateChanged( int ) ) );

  // Set up the watchdog timer
  connect( mWatchdogTimer, SIGNAL( timeout() ),
           this,     SLOT( networkTimedOut() ) );

  mWatchdogTimer->setSingleShot( TRUE );
  mWatchdogTimer->start( mNetworkTimeoutMsec );

  QgsDebugMsg( "Starting get with id " + QString::number( httpid ) + "." );
  QgsDebugMsg( "Setting httpactive = TRUE" );

  httpactive = TRUE;

  // A little trick to make this function blocking
  while ( httpactive )
  {
    // Do something else, maybe even network processing events
    qApp->processEvents();
  }

  QgsDebugMsg( "Response received." );

#ifdef QGISDEBUG
//  QString httpresponsestring(httpresponse);
//  QgsDebugMsg("Response received; being '" + httpresponsestring + "'.");
#endif

  delete http;
  http = 0;

  // Did we get an error? If so, bail early
  if ( !mError.isNull() )
  {
    QgsDebugMsg( "Processing an error '" + mError + "'." );
    return FALSE;
  }

  // Do one level of redirection
  // TODO make this recursable
  // TODO detect any redirection loops
  if ( !httpredirecturl.isEmpty() )
  {
    QgsDebugMsg( "Starting get of '" +  httpredirecturl + "'." );

    QgsHttpTransaction httprecurse( httpredirecturl, httphost, httpport );
    httprecurse.setCredentials( mUserName, mPassword );

    // Do a passthrough for the status bar text
    connect(
      &httprecurse, SIGNAL( statusChanged( QString ) ),
      this,        SIGNAL( statusChanged( QString ) )
    );

    httprecurse.getSynchronously( respondedContent, ( redirections + 1 ) );
    return TRUE;

  }

  respondedContent = httpresponse;
  return TRUE;

}


QString QgsHttpTransaction::responseContentType()
{
  return httpresponsecontenttype;
}


void QgsHttpTransaction::dataStarted( int id )
{
  QgsDebugMsg( "ID=" + QString::number( id ) + "." );
}


void QgsHttpTransaction::dataHeaderReceived( const QHttpResponseHeader& resp )
{
  QgsDebugMsg( "statuscode " +
               QString::number( resp.statusCode() ) + ", reason '" + resp.reasonPhrase() + "', content type: '" +
               resp.value( "Content-Type" ) + "'." );

  // We saw something come back, therefore restart the watchdog timer
  mWatchdogTimer->start( mNetworkTimeoutMsec );

  if ( resp.statusCode() == 302 ) // Redirect
  {
    // Grab the alternative URL
    // (ref: "http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html")
    httpredirecturl = resp.value( "Location" );
  }
  else if ( resp.statusCode() == 200 ) // OK
  {
    // NOOP
  }
  else
  {
    mError = tr( "WMS Server responded unexpectedly with HTTP Status Code %1 (%2)" )
             .arg( resp.statusCode() )
             .arg( resp.reasonPhrase() );
  }

  httpresponsecontenttype = resp.value( "Content-Type" );

}


void QgsHttpTransaction::dataReceived( const QHttpResponseHeader& resp )
{
  // TODO: Match 'resp' with 'http' if we move to multiple http connections

#if 0
  // Comment this out for now - leave the coding of progressive rendering to another day.
  char* temp;

  if ( 0 < http->readBlock( temp, http->bytesAvailable() ) )
  {
    httpresponse.append( temp );
  }
#endif

//  QgsDebugMsg("received '" + data + "'.");
}


void QgsHttpTransaction::dataProgress( int done, int total )
{
//  QgsDebugMsg("got " + QString::number(done) + " of " + QString::number(total));

  // We saw something come back, therefore restart the watchdog timer
  mWatchdogTimer->start( mNetworkTimeoutMsec );

  emit dataReadProgress( done );
  emit totalSteps( total );

  QString status;

  if ( total )
  {
    status = tr( "Received %1 of %2 bytes" ).arg( done ).arg( total );
  }
  else
  {
    status = tr( "Received %1 bytes (total unknown)" ).arg( done );
  }

  emit statusChanged( status );
}


void QgsHttpTransaction::dataFinished( int id, bool error )
{

#ifdef QGISDEBUG
  QgsDebugMsg( "ID=" + QString::number( id ) + "." );

  // The signal that this slot is connected to, QHttp::requestFinished,
  // appears to get called at the destruction of the QHttp if it is
  // still working at the time of the destruction.
  //
  // This situation may occur when we've detected a timeout and
  // we already set httpactive = FALSE.
  //
  // We have to detect this special case so that the last known error string is
  // not overwritten (it should rightfully refer to the timeout event).
  if ( !httpactive )
  {
    QgsDebugMsg( "http activity loop already FALSE." );
    return;
  }

  if ( error )
  {
    QgsDebugMsg( "however there was an error." );
    QgsDebugMsg( "error: " + http->errorString() );

    mError = tr( "HTTP response completed, however there was an error: %1" ).arg( http->errorString() );
  }
  else
  {
    QgsDebugMsg( "no error." );
  }
#endif

// Don't do this here as the request could have simply been
// to set the hostname - see transactionFinished() instead

#if 0
  // TODO
  httpresponse = http->readAll();

// QgsDebugMsg("Setting httpactive = FALSE");
  httpactive = FALSE;
#endif
}


void QgsHttpTransaction::transactionFinished( bool error )
{

#ifdef QGISDEBUG
  QgsDebugMsg( "entered." );

#if 0
  // The signal that this slot is connected to, QHttp::requestFinished,
  // appears to get called at the destruction of the QHttp if it is
  // still working at the time of the destruction.
  //
  // This situation may occur when we've detected a timeout and
  // we already set httpactive = FALSE.
  //
  // We have to detect this special case so that the last known error string is
  // not overwritten (it should rightfully refer to the timeout event).
  if ( !httpactive )
  {
// QgsDebugMsg("http activity loop already FALSE.");
    return;
  }
#endif

  if ( error )
  {
    QgsDebugMsg( "however there was an error." );
    QgsDebugMsg( "error: " + http->errorString() );

    mError = tr( "HTTP transaction completed, however there was an error: %1" ).arg( http->errorString() );
  }
  else
  {
    QgsDebugMsg( "no error." );
  }
#endif

  // TODO
  httpresponse = http->readAll();

  QgsDebugMsg( "Setting httpactive = FALSE" );
  httpactive = FALSE;
}


void QgsHttpTransaction::dataStateChanged( int state )
{
  QgsDebugMsg( "state " + QString::number( state ) + "." );

  // We saw something come back, therefore restart the watchdog timer
  mWatchdogTimer->start( mNetworkTimeoutMsec );

  switch ( state )
  {
    case QHttp::Unconnected:
      QgsDebugMsg( "There is no connection to the host." );
      emit statusChanged( tr( "Not connected" ) );
      break;

    case QHttp::HostLookup:
      QgsDebugMsg( "A host name lookup is in progress." );

      emit statusChanged( tr( "Looking up '%1'" ).arg( httphost ) );
      break;

    case QHttp::Connecting:
      QgsDebugMsg( "An attempt to connect to the host is in progress." );

      emit statusChanged( tr( "Connecting to '%1'" ).arg( httphost ) );
      break;

    case QHttp::Sending:
      QgsDebugMsg( "The client is sending its request to the server." );

      emit statusChanged( tr( "Sending request '%1'" ).arg( httpurl ) );
      break;

    case QHttp::Reading:
      QgsDebugMsg( "The client's request has been sent and the client is reading the server's response." );

      emit statusChanged( tr( "Receiving reply" ) );
      break;

    case QHttp::Connected:
      QgsDebugMsg( "The connection to the host is open, but the client is neither sending a request, nor waiting for a response." );

      emit statusChanged( tr( "Response is complete" ) );
      break;

    case QHttp::Closing:
      QgsDebugMsg( "The connection is closing down, but is not yet closed. (The state will be Unconnected when the connection is closed.)" );

      emit statusChanged( tr( "Closing down connection" ) );
      break;
  }
}


void QgsHttpTransaction::networkTimedOut()
{
  QgsDebugMsg( "entering." );

  mError = tr( "Network timed out after %n second(s) of inactivity.\n"
               "This may be a problem in your network connection or at the WMS server.", "inactivity timeout", mNetworkTimeoutMsec / 1000 );

  QgsDebugMsg( "Setting httpactive = FALSE" );
  httpactive = FALSE;
  QgsDebugMsg( "exiting." );
}


QString QgsHttpTransaction::errorString()
{
  return mError;
}

bool QgsHttpTransaction::applyProxySettings( QHttp& http, const QString& url )
{
  QSettings settings;
  //check if proxy is enabled
  bool proxyEnabled = settings.value( "proxy/proxyEnabled", false ).toBool();
  if ( !proxyEnabled )
  {
    return false;
  }

  //check if the url should go through proxy
  QString  proxyExcludedURLs = settings.value( "proxy/proxyExcludedUrls", "" ).toString();
  if ( !proxyExcludedURLs.isEmpty() )
  {
    QStringList excludedURLs = proxyExcludedURLs.split( "|" );
    QStringList::const_iterator exclIt = excludedURLs.constBegin();
    for ( ; exclIt != excludedURLs.constEnd(); ++exclIt )
    {
      if ( url.startsWith( *exclIt ) )
      {
        return false; //url does not go through proxy
      }
    }
  }

  //read type, host, port, user, passw from settings
  QString proxyHost = settings.value( "proxy/proxyHost", "" ).toString();
  int proxyPort = settings.value( "proxy/proxyPort", "" ).toString().toInt();
  QString proxyUser = settings.value( "proxy/proxyUser", "" ).toString();
  QString proxyPassword = settings.value( "proxy/proxyPassword", "" ).toString();

  QString proxyTypeString =  settings.value( "proxy/proxyType", "" ).toString();
  QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy;
  if ( proxyTypeString == "DefaultProxy" )
  {
    proxyType = QNetworkProxy::DefaultProxy;
  }
  else if ( proxyTypeString == "Socks5Proxy" )
  {
    proxyType = QNetworkProxy::Socks5Proxy;
  }
  else if ( proxyTypeString == "HttpProxy" )
  {
    proxyType = QNetworkProxy::HttpProxy;
  }
#if QT_VERSION >= 0x040400
  else if ( proxyTypeString == "HttpCachingProxy" )
  {
    proxyType = QNetworkProxy::HttpCachingProxy;
  }
  else if ( proxyTypeString == "FtpCachingProxy" )
  {
    proxyType = QNetworkProxy::FtpCachingProxy;
  }
#endif
  http.setProxy( QNetworkProxy( proxyType, proxyHost, proxyPort, proxyUser, proxyPassword ) );
  return true;
}

void QgsHttpTransaction::abort()
{
  if ( http )
  {
    http->abort();
  }
}

// ENDS
