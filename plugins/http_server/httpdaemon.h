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
  HttpDaemon( int thePortInt=8081, QObject* parent=0 );
  ~HttpDaemon();
  void newConnection( int socket );
  //accessor and mutator for current project
  void setProject(QString);
  QString project();
  //accessor and mutator for base path
  void setBasePath(QString);
  QString basePath();
  //accessor and mutator for css file name
  void setCssFileName(QString);
  QString cssFileName();
  //accessor and mutator for log file name
  void setLogFileName(QString);
  QString logFileName();
signals:
  void requestReceived(QString); //used to notify listeners when a web client connects
  void getMap(QPixmap *); //renders map and sends result to browser
  void showProject(QString); //loads the project file
  void loadProject(QString); //loads the project file but doesnt close connection so other stuff can be loaded
  void loadRasterFile(QString);//loads a rasterfile on its own using defaults 
  void loadRasterFile(QString,QString);//loads a rasterfile (arg1) over the current project (arg2)
  void loadPseudoColorRasterFile(QString);
  void loadPseudoColorRasterFile(QString,QString);
  void loadVectorFile(QString);//loads a vector file on its own using defaults
  void loadVectorFile(QString,QString);//loads a vectorfile (arg1) over the current project (arg2)
  
  void setExtents(int,int,int,int);//zooms to x1,y1 - x2,y2
  void clearMap(); //remove all layers from the map
  void newConnect(QString); //passed out
  void endConnect(QString); //passed out
  void wroteToClient(QString); //passed out

public slots:
  void requestCompleted(QString); //used to return text to the browser
  void requestCompleted( QPixmap *theQPixmap); //used to return an image to the browser
  void closeStreamWithError(QString);

private slots:
  void readClient();
  void discardClient();
private:
  void showHeader(); //print http headers out on socket
  void showFooter(); //print closing stuff for page
  void showHelp();   //show some useage help
  void getCss();
  void getCssInline();
  void getCssFromFile();
  void getProjectsList();
  void getFileList();
  void showSettings();
  void logMessage(QString);
  
  QString mBasePath;
  QString mProject;
  QString mHostName;
  QString mCssFileName;
  QString mLogFileName;
};
#endif
