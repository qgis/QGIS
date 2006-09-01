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

/* $Id: qgshttptransaction.cpp 5605 2006-07-16 23:19:04Z morb_au $ */

#include <fstream>
#include <iostream>

#include "qgshttptransaction.h"

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
  : httpurl(uri),
    httphost(proxyHost),
    httpport(proxyPort),
    httpuser(proxyUser),
    httppass(proxyPass),
    httpresponsecontenttype(0),
    mError(0)
{
#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction: constructing." << std::endl;
#endif

#ifdef QGISDEBUG
  std::cout << "  QgsHttpTransaction: proxyHost = " << proxyHost.toLocal8Bit().data() << "." << std::endl;
  std::cout << "  QgsHttpTransaction: proxyPort = " << proxyPort << "." << std::endl;
  std::cout << "  QgsHttpTransaction: proxyUser = " << proxyUser.toLocal8Bit().data() << "." << std::endl;
  std::cout << "  QgsHttpTransaction: proxyPass = " << proxyPass.toLocal8Bit().data() << "." << std::endl;
#endif


#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction: exiting constructor." << std::endl;
#endif
  
}

QgsHttpTransaction::~QgsHttpTransaction()
{

#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction: deconstructing." << std::endl;
#endif

 
}


void QgsHttpTransaction::getAsynchronously()
{

  //TODO
  
}

bool QgsHttpTransaction::getSynchronously(QByteArray &respondedContent, int redirections)
{

  httpredirections = redirections;
  
#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::getSynchronously: Entered." << std::endl;
  std::cout << "QgsHttpTransaction::getSynchronously: Using '" << httpurl.toLocal8Bit().data() << "'." << std::endl;
#endif

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

#ifdef QGISDEBUG
  qWarning("QgsHttpTransaction::getSynchronously: qurl.host() is '"+qurl.host()+ "'.");
#endif

  httpresponse.truncate(0);

  // Some WMS servers don't like receiving a http request that
  // includes the scheme, host and port (the
  // http://www.address.bit:80), so remove that from the url before
  // executing an http GET.
  QString pathAndQuery = httpurl.remove(0, 
                         httpurl.indexOf(qurl.path()));
  httpid = http->get( pathAndQuery );
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

  connect(http,   SIGNAL( stateChanged ( int ) ),
          this, SLOT( dataStateChanged ( int ) ) );

  // Set up the watchdog timer
  connect(mWatchdogTimer, SIGNAL( timeout () ),
          this,     SLOT( networkTimedOut () ) );

  mWatchdogTimer->setSingleShot(TRUE);
  mWatchdogTimer->start(NETWORK_TIMEOUT_MSEC);

#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::getSynchronously: Starting get with id " << httpid << "." << std::endl;
#endif

#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::getSynchronously: Setting httpactive = TRUE" << std::endl;
#endif
  httpactive = TRUE;

  // A little trick to make this function blocking
  while ( httpactive )
  {
    // Do something else, maybe even network processing events
    qApp->processEvents();

    // TODO: Implement a network timeout
  }

#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::getSynchronously: Response received." << std::endl;

//  QString httpresponsestring(httpresponse);
//  std::cout << "QgsHttpTransaction::getSynchronously: Response received; being '" << httpresponsestring << "'." << std::endl;
#endif

  delete http;

  // Did we get an error? If so, bail early
  if (!mError.isNull())
  {
#ifdef QGISDEBUG
    std::cout << "QgsHttpTransaction::getSynchronously: Processing an error '" << mError.toLocal8Bit().data() << "'." << std::endl;
#endif
    return FALSE;
  }

  // Do one level of redirection
  // TODO make this recursable
  // TODO detect any redirection loops
  if (!httpredirecturl.isEmpty())
  {
#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::getSynchronously: Starting get of '" << 
               httpredirecturl.toLocal8Bit().data() << "'." << std::endl;
#endif

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

#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::dataStarted with ID " << id << "." << std::endl;
#endif

 
}


void QgsHttpTransaction::dataHeaderReceived( const QHttpResponseHeader& resp )
{

#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::dataHeaderReceived: statuscode " << 
    resp.statusCode() << ", reason '" << resp.reasonPhrase().toLocal8Bit().data() << "', content type: '" <<
    resp.value("Content-Type").toLocal8Bit().data() << "'." << std::endl;
#endif

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

#ifdef QGISDEBUG
//  std::cout << "QgsHttpTransaction::dataReceived." << std::endl;
//  std::cout << "QgsHttpTransaction::dataReceived: received '" << data << "'."<< std::endl;
#endif

}


void QgsHttpTransaction::dataProgress( int done, int total )
{

#ifdef QGISDEBUG
//  std::cout << "QgsHttpTransaction::dataProgress: got " << done << " of " << total << std::endl;
#endif

  // We saw something come back, therefore restart the watchdog timer
  mWatchdogTimer->start(NETWORK_TIMEOUT_MSEC);

  QString status;
  
  if (total)
  {
    status = QString("Received %1 of %2 bytes")
                         .arg( done )
                         .arg( total );
  }
  else
  {
    status = QString("Received %1 bytes (total unknown)")
                         .arg( done );
  }

  emit setStatus( status );
}

void QgsHttpTransaction::dataFinished( int id, bool error )  
{

#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::dataFinished with ID " << id << "." << std::endl;

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
    std::cout << "QgsHttpTransaction::dataFinished - http activity loop already FALSE." << std::endl;
    return;
  }

  if (error)
  {
    std::cout << "QgsHttpTransaction::dataFinished - however there was an error." << std::endl;
    std::cout << "QgsHttpTransaction::dataFinished - " << http->errorString().toLocal8Bit().data() << std::endl;

    mError = QString( tr("HTTP response completed, however there was an error: %1") )
                .arg( http->errorString() );
  }
  else
  {
    std::cout << "QgsHttpTransaction::dataFinished - no error." << std::endl;
  }
#endif

  // TODO
  httpresponse = http->readAll();

#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::getSynchronously: Setting httpactive = FALSE" << std::endl;
#endif
  httpactive = FALSE;

}

void QgsHttpTransaction::dataStateChanged( int state )
{

#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::dataStateChanged to " << state << "." << std::endl << "  ";
#endif

  // We saw something come back, therefore restart the watchdog timer
  mWatchdogTimer->start(NETWORK_TIMEOUT_MSEC);

  switch (state)
  {
    case QHttp::Unconnected:
#ifdef QGISDEBUG
      std::cout << "There is no connection to the host." << std::endl;
#endif

      emit setStatus( QString("Not connected") );
      break;

    case QHttp::HostLookup:
#ifdef QGISDEBUG
      std::cout << "A host name lookup is in progress." << std::endl;
#endif

      emit setStatus( QString("Looking up '%1'")
                         .arg(httphost) );
      break;

    case QHttp::Connecting:
#ifdef QGISDEBUG
      std::cout << "An attempt to connect to the host is in progress." << std::endl;
#endif

      emit setStatus( QString("Connecting to '%1'")
                         .arg(httphost) );
      break;

    case QHttp::Sending:
#ifdef QGISDEBUG
      std::cout << "The client is sending its request to the server." << std::endl;
#endif

      emit setStatus( QString("Sending request '%1'")
                         .arg(httpurl) );
      break;

    case QHttp::Reading:
#ifdef QGISDEBUG
      std::cout << "The client's request has been sent and the client "
                   "is reading the server's response." << std::endl;
#endif

      emit setStatus( QString("Receiving reply") );
      break;

    case QHttp::Connected:
#ifdef QGISDEBUG
      std::cout << "The connection to the host is open, but the client "
                   "is neither sending a request, nor waiting for a response." << std::endl;
#endif

      emit setStatus( QString("Response is complete") );
      break;

    case QHttp::Closing:
#ifdef QGISDEBUG
      std::cout << "The connection is closing down, but is not yet closed. "
                   "(The state will be Unconnected when the connection is closed.)" << std::endl;
#endif

      emit setStatus( QString("Closing down connection") );
      break;
  }

}


void QgsHttpTransaction::networkTimedOut()
{

#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::networkTimedOut: entering." << std::endl;
#endif

  mError = QString(tr("Network timed out after %1 seconds of inactivity.\n"
                      "This may be a problem in your network connection or at the WMS server.")
                  ).arg(NETWORK_TIMEOUT_MSEC/1000);

#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::getSynchronously: Setting httpactive = FALSE" << std::endl;
#endif
  httpactive = FALSE;

#ifdef QGISDEBUG
  std::cout << "QgsHttpTransaction::networkTimedOut: exiting." << std::endl;
#endif

}


QString QgsHttpTransaction::errorString()
{
  return mError;
}

// ENDS
