/****************************************************************************
 * ** $Id$
 * **
 * ** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
 * **
 * ** This file is part of an example program for Qt.  This example
 * ** program may be used, distributed and modified without limitation.
 * **
 * ** Modified from QT Original example for use as a QGIS web server engine
 *  
 * *****************************************************************************/
#ifndef HTTPDAEMON
#define HTTPDAEMON

#include <stdlib.h>
#include <qsocket.h>
#include <qregexp.h>
#include <qserversocket.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qtextstream.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qstring.h>

// HttpDaemon is the the class that implements the simple HTTP server.
class HttpDaemon : public QServerSocket
{
  Q_OBJECT;
public:
  HttpDaemon( QObject* parent=0 );
  void newConnection( int socket );

signals:
  void newConnect(QString);
  void endConnect(QString);
  void wroteToClient(QString);

private slots:
  void readClient();
  void discardClient();
};
#endif
