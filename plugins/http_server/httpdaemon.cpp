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
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h> 
#include <qsettings.h>
//other includes
#include <iostream>
// HttpDaemon is the the class that implements the simple HTTP server.
HttpDaemon::HttpDaemon( int thePortInt , QObject* parent ) : QServerSocket(thePortInt,1,parent)
{
  QSettings myQSettings;
  mProject = myQSettings.readEntry("/qgis/http_server/defaultProject");
  if ( !ok() )
  {
    qWarning("Failed to bind to port " + thePortInt );
    exit( 1 );
  }
  QString myPortString;
  QString myServerNameString = myQSettings.readEntry("/qgis/http_server/serverName");
  myPortString = myPortString.setNum(thePortInt);
  mHostName = myServerNameString + QString(":") + myPortString + QString("/");
  mLogFileName = myQSettings.readEntry("/qgis/http_server/logFile");
  mCssFileName=myQSettings.readEntry("/qgis/http_server/cssFile");
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

void HttpDaemon::setCssFileName(QString theFileName)
{
  mCssFileName=theFileName;
}
QString HttpDaemon::cssFileName()
{
  return mCssFileName;
}

void HttpDaemon::setLogFileName(QString theFileName)
{
  mLogFileName=theFileName;
}
QString HttpDaemon::logFileName()
{
  return mLogFileName;
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
  logMessage(myTimeQString);
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
        // Command : qgis.css
        //
        // Get the css for formatting text output.
        //
        if (myTokenString == "qgis.css")
        {
          getCss();
        }
        //
        // Command : getProjectList
        //
        // Get a list of the available data layers.
        //
        else if (myTokenString == "getProjectsList")
        {
          getProjectsList();
        }
        //
        // Command : getFileList
        //
        // Get a list of the available data layers.
        //
        else if (myTokenString == "getFileList")
        {
          getFileList();
        }
        //
        // Command : showSettings 
        //
        // Get a list of current plugin settings
        //
        else if (myTokenString == "showSettings")
        {
          showSettings();
        }
        //
        // Command : project
        //
        // the project command is used to load a .qgs project file
        // this functionality may well be removed in the future.
        //
        else if (myTokenString == "project")
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
            emit clearMap();
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
          showHeader();
          showHelp();
          showFooter();
          //  closeStreamWithError(QString("Request is not a valid command."));
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
  logMessage(myTimeQString);
}
void HttpDaemon::requestCompleted(QString theQString)
{
  QSocket * myQSocket = (QSocket*)sender();
  QTextStream myOutputStream( myQSocket );
  myOutputStream.setEncoding( QTextStream::UnicodeUTF8 );
  showHeader();
  myOutputStream << "<h1>" << theQString << "</h1>\n";
  showHelp();
  showFooter();
  QTime myTime  = QTime::currentTime();
  QString myTimeQString = myTime.toString("hh:mm:ss") + QString(": Ok - request completed");
  emit wroteToClient(myTimeQString);
  logMessage(myTimeQString);
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
  showHeader();
  myOutputStream << "<h1>Your request was not recognised or had an unspecified error. </h1>\n";
  myOutputStream << "<h2>Error was: <font color=\"red\">" << theErrorQString << "</font></h2>\n";
  showFooter();
  QTime myTime  = QTime::currentTime();
  QString myTimeQString = myTime.toString("hh:mm:ss") + 
      QString(": Error - No no request processed\n") + 
      theErrorQString;
  emit wroteToClient(myTimeQString);
  logMessage(myTimeQString);
  myQSocket->close();
}


void HttpDaemon::logMessage (QString theMessage)
{
  if (!mLogFileName.isEmpty())
  {
    QFile myFile( mLogFileName );
    if ( myFile.open(IO_WriteOnly | IO_Append)) 
    {
      QTextStream myStream( &myFile );
      myStream << theMessage << "\n";
      myFile.close();
    }
    else
    {
#ifdef QGISDEBUG
      std::cout << "Unable to write " << theMessage << " to log!" << std::endl;
#endif
    }
  }
}



  //------------------------------- Rendering output to html fns below here!


  void HttpDaemon::showHeader()
  {
    //print out standard http and page headers
    QSettings myQSettings;  
    QString myDefaultProjectString = myQSettings.readEntry("/qgis/http_server/defaultProject");
    QSocket * myQSocket = (QSocket*)sender();
    QTextStream myOutputStream( myQSocket );
    myOutputStream.setEncoding( QTextStream::UnicodeUTF8 );
    myOutputStream << "HTTP/1.0 200 Ok\r\n"
        <<  "Content-Type: text/html; charset=\"utf-8\"\r\n"
        <<  "\r\n"
        <<  "<html>\r\n"
        <<  "<head>\r\n"
        <<  "<title>QGIS Http Server</title>\r\n";
    getCssFromFile();
    //this doesnt work (though I wish it would)    
    //    <<  "<link REL=\"stylesheet\" TYPE=\"text/css\" href=\"qgis.css\">\r\n"
    myOutputStream 
        <<  "</head>\r\n"
        <<  "<body>\r\n"
        <<  "<div id=\"all\">\r\n"
        <<  "<div id=\"heading\">\r\n"
        <<  "QGIS HTTP SERVER\r\n"
        <<  "</div>\r\n"
        <<  "<div id=\"leftmenu\">\r\n"
        <<  "<table>\r\n"
        <<  "<tr><th>Menu:</th></tr>"
        <<  "\r\n"
        <<  "<tr><td><a href=\"" << mHostName << "/\">Useage</a></td></tr>"
        <<  "\r\n"
        <<  "<tr><td><tr><td><a href=\"" << mHostName << "project=" << myDefaultProjectString << "\">Load Default Project</a></td></tr>"
        <<  "\r\n"
        <<  "<tr><td><a href=\"" << mHostName << "showSettings\">Show Settings</a></td></tr>"
        <<  "\r\n"
        <<  "<tr><td><a href=\"" << mHostName << "getProjectsList\">List Projects</a></td></tr>"
        <<  "\r\n"
        <<  "<tr><td><a href=\"" << mHostName << "showProject\">Show Project</a></td></tr>"
        <<  "\r\n"
        <<  "<tr><td><a href=\"" << mHostName << "getFileList\">List Files</a></td></tr>"
        <<  "\r\n"
        <<  "</table>\r\n"
        <<  "</div>\r\n"
        <<  "<div id=\"content\">\r\n";

  }

  void HttpDaemon::showFooter()
  {
    //print out standard http and page headers
    QSocket * myQSocket = (QSocket*)sender();
    QTextStream myOutputStream( myQSocket );
    myOutputStream 
        <<  "</div> <!--content -->\r\n"
        <<  "<div id=\"footer\">QGIS HTTP_SERVER Plugin (c) Tim Sutton 2004</div>\r\n"
        <<  "</div> <!-- all -->\r\n"
        <<  "</body>\r\n"
        <<  "</html>\r\n";
    myQSocket->close();

  }

  void HttpDaemon::showHelp()
  {

    QSettings myQSettings;  
    QString myDefaultProjectString = myQSettings.readEntry("/qgis/http_server/defaultProject");
    QSocket * myQSocket = (QSocket*)sender();
    QTextStream myOutputStream( myQSocket );
    myOutputStream <<  "<h3>Useage:</h3>"
        <<  "\r\n"
        <<  "<table class=\"helptable\">\n" 
        <<  "\r\n"
        <<  "<tr><th>Option</th><th>Description</th><tr>\n"
        <<  "\r\n"
        <<  "<tr><td class=\"option\">project</td><td>Set the background project to use when rendering maps.</td></tr>\n"
        <<  "\r\n"
        <<  "<tr><td class=\"option\">showProject</td><td>Show the current background project as an image.</td></tr>\n"
        <<  "\r\n"
        <<  "<tr><td class=\"option\">showRasterLayer</td><td>Show a raster layer in default symbology. "
        <<  "\r\n"
        <<  "If a project file is loaded, it will br overlaid on the project file.</td></tr>\n"
        <<  "\r\n"
        <<  "<tr><td class=\"option\">showPseudocolorRasterLayer</td><td>Show a raster layer in pseudocolor symbology. "
        <<  "\r\n"
        <<  "If a project file is loaded, it will br overlaid on the project file.</td></tr>\n"
        <<  "\r\n"
        <<  "<tr><td class=\"option\">showVectorLayer</td><td>Show a vector layer in default symbology."
        <<  "\r\n"
        <<  "If a project file is loaded, it will br overlaid on the project file.</td></tr>\n"
        <<  "\r\n"
        <<  "<tr><td class=\"option\">getProjectsList</td><td>Show a list of available project files.</td></tr>\n"
        <<  "\r\n"
        <<  "<tr><td class=\"option\">getFileList</td><td>Show a list of available layer files.</td></tr>\n"
        <<  "\r\n"
        <<  "<tr><td class=\"option\">showSettings</td><td>Show a list of current settings of this plugin.</td></tr>\n"
        <<  "\r\n"
        <<  "</table>\n"
        <<  "\r\n"
        <<  "<h3>Examples:</h3>"
        <<  "\r\n"
        <<  "<p><a href=\"" << mHostName << "/\">" << mHostName << " </a> (shows this page)"
        <<  "\r\n"
        <<  "<p><a href=\"" << mHostName << "project=" << myDefaultProjectString << "\">" << mHostName << "project=" << myDefaultProjectString << "</a>"
        <<  "\r\n"
        <<  "<p><a href=\"" << mHostName << "showSettings\">" << mHostName << "showSettings</a>"
        <<  "\r\n"
        <<  "\r\n"
        <<  "<p><a href=\"" << mHostName << "getProjectsList\">" << mHostName << "getProjectsList</a>"
        <<  "\r\n"
        <<  "<p><a href=\"" << mHostName << "getFileList\">" << mHostName << "getFileList</a>"
        <<  "\r\n"
        ;
    //<<  "<tr><td class="option"></td><td></td></tr>\n"
    //<<  "<tr><td class="option"></td><td></td></tr>\n"
    // <<  "<tr><td></td><td></td></tr>\n"
  }

  /* This implementation requires that this class is threaded to be able to deal with two requests at once (I think) */
  void HttpDaemon::getCss()
  {
    //print out standard http and page headers
    QSocket * myQSocket = (QSocket*)sender();
    QTextStream myOutputStream( myQSocket );
    myOutputStream.setEncoding( QTextStream::UnicodeUTF8 );
    myOutputStream << "HTTP/1.0 200 Ok\r\n"
        <<  "Content-Type: text/css; charset=\"utf-8\"\r\n"
        <<  "\r\n"
        <<  "table.helptable {" 
        <<  "\r\n"
        <<  "       border-width: 1px;" 
        <<  "\r\n"
        <<  "       border-style: solid;"
        <<  "\r\n"
        <<  "     }"
        <<  "\r\n";
    QTime myTime  = QTime::currentTime();
    QString myTimeQString = myTime.toString("hh:mm:ss") + 
        QString(": Css requested\n") ;
    emit wroteToClient(myTimeQString);
    logMessage(myTimeQString);
    myQSocket->close();
  }

  void HttpDaemon::getCssFromFile()
  {
    /* This is the default css that will be used unless the admin has set his own style */
    QSocket * myQSocket = (QSocket*)sender();
    QTextStream myOutputStream( myQSocket );
    if (!mCssFileName.isEmpty())
    {
      QFile myFile( mCssFileName );
      if ( myFile.open( IO_ReadOnly ) ) 
      {
        myOutputStream <<  "<style type=\"text/css\">" << "\r\n";
        QTextStream myInputStream( &myFile );
        while ( !myInputStream.atEnd() ) 
        {
          myOutputStream << myInputStream.readLine() << "\r\n";
        }
        myFile.close();
        myOutputStream <<  "</style>" << "\r\n";
      }
      else
      {
        //fall back to default hard coded style if we cant open the file
        getCssInline();
      }
    }
    else //fall back to default hard coded css file if no css file is set
    {
      getCssInline();

    }
  }

  void HttpDaemon::getCssInline()
  {
    /* This is the default css that will be used unless the admin has set his own style */
    QSocket * myQSocket = (QSocket*)sender();
    QTextStream myOutputStream( myQSocket );
    myOutputStream 
        <<  "<style type=\"text/css\">" << "\r\n"

        <<  "body {" << "\r\n"
        <<  "scrollbar-3dlight-color:#D1ECD1;" << "\r\n"
        <<  "scrollbar-arrow-color:#D1ECD1;" << "\r\n"
        <<  "scrollbar-base-color:#48B936;" << "\r\n"
        <<  "scrollbar-darkshadow-color:#A5DAA5;" << "\r\n"
        <<  "scrollbar-face-color:#48B936;" << "\r\n"
        <<  "scrollbar-highlight-color:#0d0000;" << "\r\n"
        <<  "scrollbar-track-color:#48B936;" << "\r\n"
        <<  "scrollbar-shadow-color:#22d000}" << "\r\n"
        <<  "a:link { " << "\r\n"
        <<  "  font-weight: bold; " << "\r\n"
        <<  "  text-decoration: none; " << "\r\n"
        <<  "  color: #black;" << "\r\n"
        <<  " }" << "\r\n"
        <<  "a:visited { " << "\r\n"
        <<  "  font-weight: bold; " << "\r\n"
        <<  "  text-decoration: none; " << "\r\n"
        <<  "  color: #black;" << "\r\n"
        <<  " }" << "\r\n"
        <<  "a:hover, a:active { " << "\r\n"
        <<  "   text-decoration: none; " << "\r\n"
        <<  "   color: red;" << "\r\n"
        <<  " }" << "\r\n"
        <<  "div#all {" << "\r\n"
        <<  "  border: solid thin black;" << "\r\n"
        <<  "  color: black;" << "\r\n"
        <<  "  background: #FEE99B;" << "\r\n" //this actually apears as left menu background
        <<  "  padding: 0px;" << "\r\n"
        <<  "}" << "\r\n"
        <<  "div#heading {" << "\r\n"
        <<  "  text-align: center;" << "\r\n"
        <<  "  border-bottom: solid thin black;" << "\r\n"
        <<  "  margin-bottom: 0;" << "\r\n"
        <<  "  margin-top: 0;" << "\r\n"
        <<  "  padding: 1em;" << "\r\n"
        <<  "  background: #FF960B;" << "\r\n"
        <<  "  font-weight: bold;" << "\r\n"
        <<  "  font-size: larger;" << "\r\n"
        <<  "  color: white;" << "\r\n"
        <<  "}" << "\r\n"
        <<  "div#leftmenu {" << "\r\n"
        <<  "  float: left;" << "\r\n"
        <<  "  margin: 0;" << "\r\n"
        <<  "  padding: 1ex;" << "\r\n"
        <<  "  width: 10em;" << "\r\n"
        <<  "}" << "\r\n"
        <<  "div#content {" << "\r\n"
        <<  "  background: #FFFAE4;" << "\r\n"
        <<  "  border-left: solid thin black;" << "\r\n"
        <<  "  color: black;" << "\r\n"
        <<  "  margin: 0;" << "\r\n"
        <<  "  margin-left: 12em;" << "\r\n"
        <<  "  padding: 1ex;" << "\r\n"
        <<  "}" << "\r\n"
        <<  "div#footer {" << "\r\n"
        <<  "  clear: both;" << "\r\n"
        <<  "  border-top: solid thin black;" << "\r\n"
        <<  "  text-align: center;" << "\r\n"
        <<  "  background: #FF960B;" << "\r\n"
        <<  "}" << "\r\n"
        <<  "table.helptable {"  << "\r\n"
        <<  "       align: center;"  << "\r\n"
        <<  "       width: 80%;" << "\r\n"
        <<  "     }" << "\r\n"
        <<  "h1,h2,h3,h4 { text-align: center; font-size: 12px }" << "\r\n"
        <<  "th {background-color: #FF960B;}" << "\r\n"
        <<  "td.option {background-color: #FEA500; font-weight: bold;}" << "\r\n"
        <<  "td.odd  {background-color: #DEDEE4; }" << "\r\n"
        <<  "td.even {background-color: #FEA500; }" << "\r\n"
        <<  "table {" << "\r\n"
        <<  "  border-top: solid thin black;" << "\r\n"
        <<  "  border-bottom: solid thin black;" << "\r\n"
        <<  "  margin: 10px auto;" << "\r\n"
        <<  "  width: 100%;" << "\r\n"
        <<  "}" << "\r\n"
        <<  "</style>" << "\r\n";

    QTime myTime  = QTime::currentTime();
    QString myTimeQString = myTime.toString("hh:mm:ss") + 
        QString(": Css requested\n") ;
    emit wroteToClient(myTimeQString);
    logMessage(myTimeQString);

  }

  void HttpDaemon::getProjectsList()
  {
    showHeader();
    QSettings myQSettings;  
    QString myProjectsDirString = myQSettings.readEntry("/qgis/http_server/projectsDir");
    QSocket * myQSocket = (QSocket*)sender();
    QTextStream myOutputStream( myQSocket );
    myOutputStream <<  "<h3>Projects List:</h3>"
        <<  "\r\n"
        <<  "<table class=\"helptable\">\n" 
        <<  "\r\n"
        <<  "<tr><th>File Name</th><tr>\n";
    QDir myDirectory=QDir(myProjectsDirString);
    myDirectory.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
    myDirectory.setSorting( QDir::Size | QDir::Reversed );

    const QFileInfoList *myList = myDirectory.entryInfoList();
    QFileInfoListIterator myIterator( *myList );
    QFileInfo *myFileInfo;
    bool oddRow=true;
    while ( (myFileInfo = myIterator.current()) != 0 ) 
    {
      QString myFileName = myFileInfo->fileName().latin1() ;
      if (myFileName.endsWith(".qgs"))
      {
        if (oddRow)
        {
          oddRow=false;
          myOutputStream  << "<tr>"
              <<  "<td class=\"odd\"><a href=\""; 
        }
        else
        {
          oddRow=true;
          myOutputStream  << "<tr>"
              <<  "<td class=\"even\"><a href=\""; 
        }
        myOutputStream  <<  "\r\n"
            << mHostName 
            << "project=" 
            << myProjectsDirString 
            << "/" 
            << myFileName 
            << "\">" << myFileName << "</a></td></tr>\r\n";
      }
      ++myIterator;
    }
    myOutputStream  <<  "\r\n" <<  "</table>\n";
    showFooter();
  }

  void HttpDaemon::getFileList()
  {
    showHeader();
    QSettings myQSettings;  
    QString myLayersDirString = myQSettings.readEntry("/qgis/http_server/layersDir");
    QSocket * myQSocket = (QSocket*)sender();
    QTextStream myOutputStream( myQSocket );
    myOutputStream <<  "<h3>File List:</h3>"
        <<  "\r\n"
        <<  "<table class=\"helptable\">\n" 
        <<  "\r\n"
        <<  "<tr><th>File Name</th><tr>\n";
    QDir myDirectory=QDir(myLayersDirString);
    myDirectory.setFilter( QDir::Files | QDir::Hidden | QDir::NoSymLinks );
    myDirectory.setSorting( QDir::Size | QDir::Reversed );

    const QFileInfoList *myList = myDirectory.entryInfoList();
    QFileInfoListIterator myIterator( *myList );
    QFileInfo *myFileInfo;
    bool oddRow=true;
    while ( (myFileInfo = myIterator.current()) != 0 ) 
    {
      QString myFileName = myFileInfo->fileName().latin1() ;
      if (myFileName.endsWith(".tif") || myFileName.endsWith(".asc") || myFileName.endsWith(".grd"))
      {
        if (oddRow)
        {
          oddRow=false;
          myOutputStream  << "<tr>"
              <<  "<td class=\"odd\"><a href=\"" ;
        }
        else
        {
          oddRow=true;
          myOutputStream  << "<tr>"
              <<  "<td class=\"even\"><a href=\"" ;
        }
        myOutputStream
            << mHostName 
            << "showPseudocolorRasterLayer=" 
            << myLayersDirString 
            << "/" 
            << myFileName 
            << "\">" << myFileName << "</a></td></tr>\r\n";
      }
      ++myIterator;
    }
    myOutputStream  <<  "\r\n" <<  "</table>\n";
    showFooter();
  }

  void HttpDaemon::showSettings()
  {
    showHeader();
    QSocket * myQSocket = (QSocket*)sender();
    QTextStream myOutputStream( myQSocket );
    myOutputStream <<  "<h3>Settings:</h3>"
        <<  "\r\n"
        <<  "<table class=\"helptable\">\n" 
        <<  "\r\n"
        <<  "<tr><th>Option</th><th>Value</th><tr>\n";

    QSettings myQSettings;  
    QString myProjectDirString = myQSettings.readEntry("/qgis/http_server/projectsDir");
    myOutputStream  <<  "\r\n"
        <<  "<tr><td class=\"option\">Projects Dir</td><td>" << myProjectDirString << "</td></tr>";

    QString myDefaultProjectString = myQSettings.readEntry("/qgis/http_server/defaultProject");
    myOutputStream  <<  "\r\n"
        <<  "<tr><td class=\"option\">Default Project</td><td>" << myDefaultProjectString << "</td></tr>";

    QString myLayersDirString = myQSettings.readEntry("/qgis/http_server/layersDir");
    myOutputStream  <<  "\r\n"
        <<  "<tr><td class=\"option\">Layers Dir</td><td>" << myLayersDirString << "</td></tr>";

    QString myCssFileString = myQSettings.readEntry("/qgis/http_server/cssFile");
    myOutputStream  <<  "\r\n"
        <<  "<tr><td class=\"option\">Css File</td><td>" << myCssFileString << "</td></tr>";

    QString myLogFileString = myQSettings.readEntry("/qgis/http_server/logFile");
    myOutputStream  <<  "\r\n"
        <<  "<tr><td class=\"option\">Log File</td><td>" << myLogFileString << "</td></tr>";

    QString myAlwaysStartFlag = myQSettings.readEntry("/qgis/http_server/alwaysStartFlag");
    myOutputStream  <<  "\r\n"
        <<  "<tr><td class=\"option\">Always Start</td><td>" << myAlwaysStartFlag << "</td></tr>"
        <<  "\r\n"
        <<  "</table>\n";

    showFooter();

  }
