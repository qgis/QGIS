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
  ~HttpDaemon();
  void newConnection( int socket );

signals:
  void requestReceived(QString); //passes out qgs project file name
  void makeMap(QString); //passed out
  void newConnect(QString); //passed out
  void endConnect(QString); //passed out
  void wroteToClient(QString); //passed out

public slots:
  void requestCompleted(QString); //used to return text to the browser
  void requestCompleted( QPixmap *theQPixmap); //used to return an image to the browser

private slots:
  void readClient();
  void discardClient();
  void closeStreamWithError(QString);
private:
};
#endif
