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
#include <iostream>

#include "qgshttptransaction.h"
#include "qgslogger.h"

#include <QApplication>
#include <QUrl>
#include <QTimer>

static int NETWORK_TIMEOUT_MSEC = (120 * 1000);  // 120 seconds
static int HTTP_PORT_DEFAULT = 80;

QgsHttpTransaction::QgsHttpTransaction(QString uri,
                                       QString proxyHost,
                                       int     proxyPort,
                                       QString proxyUser,
                                       QString proxyPass)
  : httpresponsecontenttype(0),
    httpurl(uri),
    httphost(proxyHost),
    httpport(proxyPort),
    httpuser(proxyUser),
    httppass(proxyPass),
    mError(0)
{

  QgsDebugMsg("QgsHttpTransaction: constructing.");
  QgsDebugMsg("  QgsHttpTransaction: proxyHost = " + proxyHost + ".");
  QgsDebugMsg("  QgsHttpTransaction: proxyPort = " + QString::number(proxyPort) + ".");
  QgsDebugMsg("  QgsHttpTransaction: proxyUser = " + proxyUser + ".");
  QgsDebugMsg("  QgsHttpTransaction: proxyPass = " + proxyPass + ".");
  QgsDebugMsg("QgsHttpTransaction: exiting constructor.");
}

QgsHttpTransaction::~QgsHttpTransaction()
{
  QgsDebugMsg("QgsHttpTransaction: deconstructing.");
}


void QgsHttpTransaction::getAsynchronously()
{

  //TODO
  
}

bool QgsHttpTransaction::getSynchronously(QByteArray &respondedContent, int redirections, const QByteArray* postData)
{

  httpredirections = redirections;
  
  QgsDebugMsg("QgsHttpTransaction::getSynchronously: Entered.");
  QgsDebugMsg("QgsHttpTransaction::getSynchronously: Using '" + httpurl + "'.");

  QUrl qurl(httpurl);

  http = new QHttp( qurl.host(), qurl.port(HTTP_PORT_DEFAULT) );

  if (httphost.isEmpty())
  {
    // No proxy was specified - connect directly to host in URI
    httphost = qurl.host();
    httpport = qurl.port(HTTP_PORT_DEFAULT);

  }
  else
  {
    // Insert proxy username and password authentication
    http->setProxy( httphost, httpport, httpuser, httppass );
  }

//  int httpid1 = http->setHost( qurl.host(), qurl.port() );

  mWatchdogTimer = new QTimer( this );

  QgsDebugMsg("QgsHttpTransaction::getSynchronously: qurl.host() is '" + qurl.host()+ "'.");

  httpresponse.truncate(0);

  // Some WMS servers don't like receiving a http request that
  // includes the scheme, host and port (the
  // http://www.address.bit:80), so remove that from the url before
  // executing an http GET.
  QString pathAndQuery = httpurl.remove(0, 
                         httpurl.indexOf(qurl.path()));

  if(!postData) //do request with HTTP GET
    {
      httpid = http->get( pathAndQuery );
    }
  else //do request with HTTP POST
    {
      httpid = http->post(pathAndQuery, *postData);
    }

  connect(http, SIGNAL( requestStarted ( int ) ),
          this,      SLOT( dataStarted ( int ) ) );

  connect(http, SIGNAL( responseHeaderReceived( const QHttpResponseHeader& ) ),
          this,       SLOT( dataHeaderReceived( const QHttpResponseHeader& ) ) );

  connect(http,  SIGNAL( readyRead( const QHttpResponseHeader& ) ),
          this, SLOT( dataReceived( const QHttpResponseHeader& ) ) );

  connect(http, SIGNAL( dataReadProgress ( int, int ) ),
          this,       SLOT( dataProgress ( int, int ) ) );

  connect(http, SIGNAL( requestFinished ( int, bool ) ),
          this,      SLOT( dataFinished ( int, bool ) ) );

  connect(http, SIGNAL(              done ( bool ) ),
          this, SLOT( transactionFinished ( bool ) ) );

  connect(http,   SIGNAL( stateChanged ( int ) ),
          this, SLOT( dataStateChanged ( int ) ) );

  // Set up the watchdog timer
  connect(mWatchdogTimer, SIGNAL( timeout () ),
          this,     SLOT( networkTimedOut () ) );

  mWatchdogTimer->setSingleShot(TRUE);
  mWatchdogTimer->start(NETWORK_TIMEOUT_MSEC);

  QgsDebugMsg("QgsHttpTransaction::getSynchronously: Starting get with id " + QString::number(httpid) + ".");
  QgsDebugMsg("QgsHttpTransaction::getSynchronously: Setting httpactive = TRUE");

  httpactive = TRUE;

  // A little trick to make this function blocking
  while ( httpactive )
  {
    // Do something else, maybe even network processing events
    qApp->processEvents();
  }

#ifdef QGISDEBUG
  QgsDebugMsg("QgsHttpTransaction::getSynchronously: Response received.");

//  QString httpresponsestring(httpresponse);
//  QgsDebugMsg("QgsHttpTransaction::getSynchronously: Response received; being '" + httpresponsestring + "'.");
#endif

  delete http;

  // Did we get an error? If so, bail early
  if (!mError.isNull())
  {
    QgsDebugMsg("QgsHttpTransaction::getSynchronously: Processing an error '" + mError + "'.");
    return FALSE;
  }

  // Do one level of redirection
  // TODO make this recursable
  // TODO detect any redirection loops
  if (!httpredirecturl.isEmpty())
  {
    QgsDebugMsg("QgsHttpTransaction::getSynchronously: Starting get of '" +  httpredirecturl + "'.");
  
    QgsHttpTransaction httprecurse(httpredirecturl, httphost, httpport);

    // Do a passthrough for the status bar text
    connect(
            &httprecurse, SIGNAL(setStatus(QString)),
             this,        SIGNAL(setStatus(QString))
           );

    httprecurse.getSynchronously( respondedContent, (redirections + 1) );
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
  QgsDebugMsg("QgsHttpTransaction::dataStarted with ID " + QString::number(id) + ".");
}


void QgsHttpTransaction::dataHeaderReceived( const QHttpResponseHeader& resp )
{
  QgsDebugMsg("QgsHttpTransaction::dataHeaderReceived: statuscode " + 
    QString::number(resp.statusCode()) + ", reason '" + resp.reasonPhrase() + "', content type: '" +
    resp.value("Content-Type") + "'.");

  // We saw something come back, therefore restart the watchdog timer
  mWatchdogTimer->start(NETWORK_TIMEOUT_MSEC);

  if (resp.statusCode() == 302) // Redirect
  {
    // Grab the alternative URL 
    // (ref: "http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html")
    httpredirecturl = resp.value("Location");
  }
  else if (resp.statusCode() == 200) // OK
  {
    // NOOP
  }
  else
  {
    mError = QString( tr("WMS Server responded unexpectedly with HTTP Status Code %1 (%2)") )
                .arg( resp.statusCode() )
                .arg( resp.reasonPhrase() );
  }

  httpresponsecontenttype = resp.value("Content-Type");

}


void QgsHttpTransaction::dataReceived( const QHttpResponseHeader& resp )
{
  // TODO: Match 'resp' with 'http' if we move to multiple http connections

  /* Comment this out for now - leave the coding of progressive rendering to another day.
  char* temp;

  if (  0 < http->readBlock( temp, http->bytesAvailable() )  )
  {
    httpresponse.append(temp);
  }
  */

//  QgsDebugMsg("QgsHttpTransaction::dataReceived.");
//  QgsDebugMsg("QgsHttpTransaction::dataReceived: received '" + data + "'.");
}


void QgsHttpTransaction::dataProgress( int done, int total )
{
//  QgsDebugMsg("QgsHttpTransaction::dataProgress: got " + QString::number(done) + " of " + QString::number(total));

  // We saw something come back, therefore restart the watchdog timer
  mWatchdogTimer->start(NETWORK_TIMEOUT_MSEC);

  QString status;
  
  if (total)
  {
    status = QString( QObject::tr("Received %1 of %2 bytes") )
                         .arg( done )
                         .arg( total );
  }
  else
  {
    status = QString( QObject::tr("Received %1 bytes (total unknown)") )
                         .arg( done );
  }

  emit setStatus( status );
}


void QgsHttpTransaction::dataFinished( int id, bool error )  
{

#ifdef QGISDEBUG
  QgsDebugMsg("QgsHttpTransaction::dataFinished with ID " + QString::number(id) + ".");

  // The signal that this slot is connected to, QHttp::requestFinished,
  // appears to get called at the destruction of the QHttp if it is
  // still working at the time of the destruction.
  //
  // This situation may occur when we've detected a timeout and 
  // we already set httpactive = FALSE.
  //
  // We have to detect this special case so that the last known error string is
  // not overwritten (it should rightfully refer to the timeout event).
  if (!httpactive)
  {
    QgsDebugMsg("QgsHttpTransaction::dataFinished - http activity loop already FALSE.");
    return;
  }

  if (error)
  {
    QgsDebugMsg("QgsHttpTransaction::dataFinished - however there was an error.");
    QgsDebugMsg("QgsHttpTransaction::dataFinished - " + http->errorString());

    mError = QString( tr("HTTP response completed, however there was an error: %1") )
                .arg( http->errorString() );
  }
  else
  {
    QgsDebugMsg("QgsHttpTransaction::dataFinished - no error.");
  }
#endif

// Don't do this here as the request could have simply been
// to set the hostname - see transactionFinished() instead

//   // TODO
//   httpresponse = http->readAll();
// 
// #ifdef QGISDEBUG
//   std::cout << "QgsHttpTransaction::getSynchronously: Setting httpactive = FALSE" << std::endl;
// #endif
//   httpactive = FALSE;

}


void QgsHttpTransaction::transactionFinished( bool error )
{

#ifdef QGISDEBUG
  QgsDebugMsg("QgsHttpTransaction::transactionFinished.");

//   // The signal that this slot is connected to, QHttp::requestFinished,
//   // appears to get called at the destruction of the QHttp if it is
//   // still working at the time of the destruction.
//   //
//   // This situation may occur when we've detected a timeout and 
//   // we already set httpactive = FALSE.
//   //
//   // We have to detect this special case so that the last known error string is
//   // not overwritten (it should rightfully refer to the timeout event).
//   if (!httpactive)
//   {
//     std::cout << "QgsHttpTransaction::dataFinished - http activity loop already FALSE." << std::endl;
//     return;
//   }

  if (error)
  {
    QgsDebugMsg("QgsHttpTransaction::transactionFinished - however there was an error.");
    QgsDebugMsg("QgsHttpTransaction::transactionFinished - " + http->errorString());

    mError = QString( tr("HTTP transaction completed, however there was an error: %1") )
                .arg( http->errorString() );
  }
  else
  {
    QgsDebugMsg("QgsHttpTransaction::transactionFinished - no error.");
  }
#endif

  // TODO
  httpresponse = http->readAll();

  QgsDebugMsg("QgsHttpTransaction::getSynchronously: Setting httpactive = FALSE");
  httpactive = FALSE;
}


void QgsHttpTransaction::dataStateChanged( int state )
{
  QgsDebugMsg("QgsHttpTransaction::dataStateChanged to " + QString::number(state) + ".");

  // We saw something come back, therefore restart the watchdog timer
  mWatchdogTimer->start(NETWORK_TIMEOUT_MSEC);

  switch (state)
  {
    case QHttp::Unconnected:
      QgsDebugMsg("There is no connection to the host.");
      emit setStatus( QString(QObject::tr("Not connected") ) );
      break;

    case QHttp::HostLookup:
      QgsDebugMsg("A host name lookup is in progress.");

      emit setStatus( QString( QObject::tr("Looking up '%1'") )
                         .arg(httphost) );
      break;

    case QHttp::Connecting:
      QgsDebugMsg("An attempt to connect to the host is in progress.");

      emit setStatus( QString( QObject::tr("Connecting to '%1'") )
                         .arg(httphost) );
      break;

    case QHttp::Sending:
      QgsDebugMsg("The client is sending its request to the server.");

      emit setStatus( QString( QObject::tr("Sending request '%1'") )
                         .arg(httpurl) );
      break;

    case QHttp::Reading:
      QgsDebugMsg("The client's request has been sent and the client is reading the server's response.");

      emit setStatus( QString( QObject::tr("Receiving reply") ) );
      break;

    case QHttp::Connected:
      QgsDebugMsg("The connection to the host is open, but the client is neither sending a request, nor waiting for a response.");

      emit setStatus( QString( QObject::tr("Response is complete") ) );
      break;

    case QHttp::Closing:
      QgsDebugMsg("The connection is closing down, but is not yet closed. (The state will be Unconnected when the connection is closed.)");

      emit setStatus( QString( QObject::tr("Closing down connection") ) );
      break;
  }

}


void QgsHttpTransaction::networkTimedOut()
{
  QgsDebugMsg("QgsHttpTransaction::networkTimedOut: entering.");

  mError = QString(tr("Network timed out after %1 seconds of inactivity.\n"
                      "This may be a problem in your network connection or at the WMS server.","", NETWORK_TIMEOUT_MSEC/1000)
                  ).arg(NETWORK_TIMEOUT_MSEC/1000);

  QgsDebugMsg("QgsHttpTransaction::getSynchronously: Setting httpactive = FALSE");
  httpactive = FALSE;
  QgsDebugMsg("QgsHttpTransaction::networkTimedOut: exiting.");
}


QString QgsHttpTransaction::errorString()
{
  return mError;
}

// ENDS
