/****************************************************************************
 * ** $Id$
 * **
 * ** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
 * **
 * ** This file is part of an example program for Qt.  This example
 * ** program may be used, distributed and modified without limitation.
 * **
 * ** Modified from QT Original example for use as a QGIS web server engine
 * *****************************************************************************/

#include "httpdaemon.h"
#include <qdatetime.h>

// HttpDaemon is the the class that implements the simple HTTP server.
HttpDaemon::HttpDaemon( QObject* parent ) : QServerSocket(8081,1,parent)
{
    if ( !ok() )
    {
        qWarning("Failed to bind to port 8081");
        exit( 1 );
    }
}
HttpDaemon::~HttpDaemon()
{
}
void HttpDaemon::newConnection( int theSocket )
{
    // When a new client connects, the server constructs a QSocket and all
    // communication with the client is done over this QSocket. QSocket
    // works asynchronouslyl, this means that all the communication is done
    // in the two slots readClient() and discardClient().
    QSocket* myQSocket = new QSocket( this );
    connect(myQSocket, SIGNAL(readyRead()), this, SLOT(readClient()) );
    connect(myQSocket, SIGNAL(delayedCloseFinished()), this, SLOT(discardClient()) );
    myQSocket->setSocket( theSocket );
    QTime myTime  = QTime::currentTime();
    QString myTimeQString = myTime.toString("hh:mm:ss")+QString(": New connection established.");
    emit newConnect( myTimeQString);
}


//      private slots:
void HttpDaemon::readClient()
{
    // This slot is called when the client sent data to the server. The
    // server looks if it was a get request and sends a very simple HTML
    // document back.
    QSocket*myQSocket = (QSocket*)sender();
    if (myQSocket->canReadLine() )
    {
        QStringList myTokens = QStringList::split( QRegExp("[ \n\r][ \n\r]*"),myQSocket->readLine() );
        if ( myTokens[0] == "GET" )
        {
            QTextStream myOutputStream(myQSocket );
            myOutputStream.setEncoding( QTextStream::UnicodeUTF8 );
            myOutputStream << "HTTP/1.0 200 Ok\n\r"
            "Content-Type: text/html; charset=\"utf-8\"\n\r"
            "\n\r"
            "<h1>Nothing to see here</h1>\n";
            myQSocket->close();
            QTime myTime  = QTime::currentTime();
            QString myTimeQString = myTime.toString("hh:mm:ss") + QString(": Wrote to client.");
            emit wroteToClient(myTimeQString);
        }
    }
}
void HttpDaemon::discardClient()
{
    QSocket * myQSocket = (QSocket*)sender();
    delete myQSocket;
    QTime myTime  = QTime::currentTime();
    QString myTimeQString = myTime.toString("hh:mm:ss") + QString(": Client disconected.");
    emit endConnect(myTimeQString);
}

