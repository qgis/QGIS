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


//qt includes
#include <qdatetime.h>
#include <qregexp.h>
#include <qbuffer.h>
#include <qdatastream.h>
#include <qcstring.h> 

//other includes
#include <iostream>
// HttpDaemon is the the class that implements the simple HTTP server.
HttpDaemon::HttpDaemon( int thePortInt , QObject* parent ) : QServerSocket(thePortInt,1,parent)
{
  if ( !ok() )
  {
    qWarning("Failed to bind to port " + thePortInt );
    exit( 1 );
  }
}
HttpDaemon::~HttpDaemon()
{
}
//
// Accessors and mutators
//

void HttpDaemon::setProject(QString theProject)
{
 mProject=theProject;
}

QString HttpDaemon::project()
{
 return mProject;
}

void HttpDaemon::setBasePath(QString theBasePath)
{
 mBasePath=theBasePath;
}
QString HttpDaemon::basePath()
{
  return mBasePath;
}


//
// Web Server Functions
//
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
  bool mapNeedsDrawingFlag=false;
  if (myQSocket->canReadLine() )
  {
    QString myLineString = myQSocket->readLine();
    //emit the request we got so it can be logged
    emit requestReceived(myLineString);
    QStringList myTokens = QStringList::split( QRegExp("[ \n\r][ \n\r]*"),myLineString );
    if ( myTokens[0] == "GET" )
    {
      emit requestReceived(QString("Get Request Received"));
      //check if we got a reasonable request, if we didnt we will close the connection straightaway
      if (myTokens[1])
      {
        emit requestReceived(QString("Found at least one parameter to get request..."));
        //strip off leading "/"	    
        QString myTokenString = myTokens[1];	      
        myTokenString.replace(QRegExp("^/"),"");
        //if the request is valid we will pass the request down to the plugin and
        //defer closing the connection until the plugin responds    
        myTokens = QStringList::split( QString("="),myTokenString );
        //see if the key part is valid
        myTokenString = myTokens[0];
        emit requestReceived(QString("Token 0 : ")+ myTokenString);
        //
        // Command : project
        //
        // the project command is used to load a .qgs project file
        // this functionality may well be removed in the future.
        //
        if (myTokenString == "project")
        {
          //see if the value part is valid (it must include a .qgs file!
          myTokenString = myTokens[1];
          if (myTokenString.right(4) != QString(".qgs"))
          {
            closeStreamWithError(QString("Project request made but no valid project parameter passed."));
            return;
          }
          mProject=myTokenString;
          emit clearMap(); //removes all layers from the map
          requestCompleted(QString("Project now set to ") + myTokenString);
          mapNeedsDrawingFlag=false;
        }

        // Command : showProject
        //
        // This command will show the active project - it needs no parameters
        //
        else if (myTokenString == "showProject")
        {
#ifdef QGISDEBUG
          std::cout << "showProject called!" << std::endl;
#endif          
          emit clearMap();
          emit showProject(mProject);
          mapNeedsDrawingFlag=true;
        }


        // Command : showRasterLayer
        //
        // The show raster layer command is used to show a raster layer that exists
        // on the server's file system. All paths will be relative to the 
        // HttpDaemon basePath member. Any ../ will be stripped out of the path
        // to prevent the user requesting a file that is not below basePath.
        //
        // The mProject (if set) will be reloaded and the specified rasterLayer will
        // be displayed over the top of that.
        // 
        else if (myTokenString == "showRasterLayer")
        {
          myTokenString = myTokens[1];
          if (mProject=="" || mProject==NULL)
          {
            //load the raster on its own
            emit clearMap();
            emit loadRasterFile(myTokenString);
            mapNeedsDrawingFlag=true;
          }
          else
          {
            //load it over the project
            emit loadProject(mProject);
            emit loadRasterFile(myTokenString,mProject);
            mapNeedsDrawingFlag=true;
          }
        }
        // 
        // Command : showPseudocolorRasterLayer
        //
        // The show raster layer command is used to show a raster layer that exists
        // on the server's file system. All paths will be relative to the 
        // HttpDaemon basePath member. Any ../ will be stripped out of the path
        // to prevent the user requesting a file that is not below basePath.
        //
        // The mProject (if set) will be reloaded and the specified rasterLayer will
        // be displayed over the top of that. The file will be show with a default
        // colour ramp.
        // 
        else if (myTokenString == "showPseudocolorRasterLayer")
        {
          myTokenString = myTokens[1];
          if (mProject=="" || mProject==NULL)
          {
            //load the raster on its own
            emit clearMap();
            emit loadPseudoColorRasterFile(myTokenString);
            mapNeedsDrawingFlag=true;
          }
          else
          {
            //load it over the project
            emit loadProject(mProject);
            emit loadPseudoColorRasterFile(myTokenString,mProject);
            mapNeedsDrawingFlag=true;
          }
        }
        //
        // Command : showVectorLayer
        //
        // The show vector layer command is used to show a vector layer that exists
        // on the server's file system. All paths will be relative to the 
        // HttpDaemon basePath member. Any ../ will be stripped out of the path
        // to prevent the user requesting a file that is not below basePath.
        //
        // The mProject (if set) will be reloaded and the specified vector will
        // be displayed over the top of that.
        // 
        else if (myTokenString == "showVectorLayer")
        {
          myTokenString = myTokens[1];
          if (mProject=="" || mProject==NULL)
          {
            //load the vector on its own
            emit clearMap();
            emit loadVectorFile(myTokenString);
            mapNeedsDrawingFlag=true;
          }
          else
          {
            //load it over the project
            emit loadProject(mProject);
            emit loadVectorFile(myTokenString,mProject);
            mapNeedsDrawingFlag=true;
          }
        }
        else //something was wrong!
        {
          closeStreamWithError(QString("Request is not for a valid."));
          return;
        }
      }
      else //request seems to be invalid
      {
        closeStreamWithError(QString("Get request passed without parameters!"));
        return;
      }
    }
    else
    {
      closeStreamWithError(QString("No get request passed"));
      return;
    }
  }
  else //just close the socket again because we cant do anything with it
  {
    closeStreamWithError(QString("Cant read line from socket!"));
    return;
  }
  //if we made it this far there were no errors - we can test if the map needs to be drawn
  if (mapNeedsDrawingFlag==true)
  {
#ifdef QGISDEBUG
    std::cout << "Map Needs Drawing flag is true : Drawing map" << std::endl;
#endif
    QPixmap *myQPixmap = new QPixmap(400,400);
    emit getMap(myQPixmap);
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
void HttpDaemon::requestCompleted(QString theQString)
{
  QSocket * myQSocket = (QSocket*)sender();
  QTextStream myOutputStream( myQSocket );
  myOutputStream.setEncoding( QTextStream::UnicodeUTF8 );
  myOutputStream << "HTTP/1.0 200 Ok\r\n"
      "Content-Type: text/html; charset=\"utf-8\"\r\n"
      "\r\n"
      "<h1>" << theQString << "</h1>\n";
  QTime myTime  = QTime::currentTime();
  QString myTimeQString = myTime.toString("hh:mm:ss") + QString(": Ok - request completed");
  emit wroteToClient(myTimeQString);
  myQSocket->close();
}

void HttpDaemon::requestCompleted( QPixmap *theQPixmap)
{
  QSocket * myQSocket = (QSocket*)sender();
  QByteArray myQByteArray;
  QBuffer myQBuffer(myQByteArray);
  myQBuffer.open(IO_WriteOnly);
  theQPixmap->save(&myQBuffer, "PNG", 50);
  myQBuffer.close();

  QDataStream myQDataStream(myQSocket);
  QString myResponseString;
  myResponseString.append("HTTP/1.0 200 OK\n");
  myResponseString.append("Content-Type: image/png\n\n");
  myQSocket->writeBlock(myResponseString.ascii(), myResponseString.length());
  myQSocket->writeBlock(myQByteArray.data(), myQByteArray.size());
  myQSocket->close();
}



void HttpDaemon::closeStreamWithError(QString theErrorQString)
{
  QSocket * myQSocket = (QSocket*)sender();
  QTextStream myOutputStream( myQSocket );
  myOutputStream.setEncoding( QTextStream::UnicodeUTF8 );
  myOutputStream << "HTTP/1.0 200 Ok\r\n"
      "Content-Type: text/html; charset=\"utf-8\"\r\n"
      "\r\n"
      "<h1>Your request was not recognised or had an unspecified error. </h1>\n";
  myOutputStream << "<h2>Error was: <font color=\"red\">" << theErrorQString << "</font></h2>\n";
  showHelp();
  QTime myTime  = QTime::currentTime();
  QString myTimeQString = myTime.toString("hh:mm:ss") + 
      QString(": Error - No no request processed\n") + 
      theErrorQString;
  emit wroteToClient(myTimeQString);
  myQSocket->close();
}

void HttpDaemon::showHelp()
{
  QSocket * myQSocket = (QSocket*)sender();
  QTextStream myOutputStream( myQSocket );
  myOutputStream.setEncoding( QTextStream::UnicodeUTF8 );
  myOutputStream <<  "<table>\n" 
                 <<  "<tr><th>Option</th><th>Description</th><tr>\n"
                 <<  "<tr><td class=\"option\">project</td><td></td></tr>\n"
                 <<  "<tr><td class=\"option\">showProject</td><td></td></tr>\n"
                 <<  "<tr><td class=\"option\">showRasterLayer</td><td></td></tr>\n"
                 <<  "<tr><td class=\"option\">showPseudocolorRasterLayer</td><td></td></tr>\n"
                 <<  "<tr><td class=\"option\">showVectorLayer</td><td></td></tr>\n"
                 <<  "</table>\n";
                 //<<  "<tr><td class="option"></td><td></td></tr>\n"
                 //<<  "<tr><td class="option"></td><td></td></tr>\n"
                 // <<  "<tr><td></td><td></td></tr>\n"
}
