/***************************************************************************
  plugin.cpp 
  Import tool for various worldmap analysis output files
Functions:

-------------------
begin                : Jan 21, 2004
copyright            : (C) 2004 by Tim Sutton
email                : tim@linfiniti.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

// includes

#include <qgsmaplayer.h>
#include <qgsrasterlayer.h>
#include <qgisapp.h>
#include "plugin.h"


#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qlineedit.h>
#include <qaction.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qfileinfo.h>
#include <qsettings.h>

//non qt includes
#include <iostream>

//the gui subclass
#include "plugingui.h"

// xpm for creating the toolbar icon
#include "icon.xpm"
// 

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

static const char * const sIdent = "$Id$";
static const char * const sName = "HttpServer";
static const char * const sDescription = "A simple http server plugin that allows remote control of qgis.";
static const char * const sVersion = "Version 0.1";
static const QgisPlugin::PLUGINTYPE sType = QgisPlugin::UI;


//////////////////////////////////////////////////////////////////////
//
// THE FOLLOWING METHODS ARE MANDATORY FOR ALL PLUGINS
//
//////////////////////////////////////////////////////////////////////

/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param theQGisApp - Pointer to the QGIS main window
 * @param theQGisInterface - Pointer to the QGIS interface object
 */
QgsHttpServerPlugin::QgsHttpServerPlugin(QgisApp * theQGisApp, QgisIface * theQgisInterface):
                 mHttpDaemon(0), mEnabled(false),
                 mQGisApp(theQGisApp), 
                 mQGisIface(theQgisInterface),
                 QgisPlugin(sName,sDescription,sVersion,sType)
{
}


QgsHttpServerPlugin::~QgsHttpServerPlugin()
{
 unload();
}

/*
 * Initialize the GUI interface for the plugin 
 */
void QgsHttpServerPlugin::initGui()
{
  QPopupMenu *pluginMenu = mQGisIface->getPluginMenu("&Tools");
  mMenuId = pluginMenu->insertItem(QIconSet(icon),"&HttpServer", this, SLOT(run()));

  // Create the action for tool
  QAction *myQActionPointer = new QAction("QGis Http Server", QIconSet(icon), "&Wmi",0, this, "run");
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  // Add the toolbar
  mToolBarPointer = new QToolBar((QMainWindow *) mQGisApp, "Tools");
  mToolBarPointer->setLabel("QGis Http Server");
  // Add the zoom previous tool to the toolbar
  myQActionPointer->addTo(mToolBarPointer);
  QSettings myQSettings;  // where we keep last used filter in persistant state
  int myPortNoInt = myQSettings.readNumEntry("/qgis/http_server/port");
  if (myPortNoInt <1)
  {
    mPortInt=8081; //sensible default
  }
  else
  {
    mPortInt = myPortNoInt;
  }

  QString myAlwaysStartFlag = myQSettings.readEntry("/qgis/http_server/alwaysStartFlag");
  if (myAlwaysStartFlag=="true")
  {
    startServer();
    mEnabled=true;
  }
  else
  {
    mEnabled=false;
  }
}
//method defined in interface
void QgsHttpServerPlugin::help()
{
  //implement me!
}

// Slot called when the menu item is activated
void QgsHttpServerPlugin::run()
{
  QgsHttpServerPluginGui *myPluginGui=new QgsHttpServerPluginGui(mQGisApp,"QGis Http Server",true,0);
  //listen for when the layer has been made so we can draw it
  Q_CHECK_PTR( mHttpDaemon );
  if (!mEnabled)
  {
    std::cout << "NOT connecting to httpd because there is NO instance " << std::endl;
  }
  else
  {
    std::cout << "connecting to httpd because there is an instance " << std::endl;
    connect(mHttpDaemon, SIGNAL(newConnect(QString)), myPluginGui, SLOT(newConnect(QString)));
    connect(mHttpDaemon, SIGNAL(endConnect(QString)), myPluginGui, SLOT(endConnect(QString)));
    connect(mHttpDaemon, SIGNAL(wroteToClient(QString)), myPluginGui, SLOT(wroteToClient(QString)));
    connect(mHttpDaemon, SIGNAL(requestReceived(QString)), myPluginGui, SLOT(requestReceived(QString)));
  }
  connect(myPluginGui, SIGNAL(enabledChanged(bool)),this, SLOT(setEnabled(bool)));
  connect(myPluginGui, SIGNAL(portChanged(int)),this, SLOT(setPort(int)));

  myPluginGui->setPort(mPortInt);
  myPluginGui->setEnabled(mEnabled);
  myPluginGui->show();
}

// Unload the plugin by cleaning up the GUI
void QgsHttpServerPlugin::unload()
{
  //kill the web server daemon
  stopServer();
  //delete mHttpDaemon;
  // remove the GUI
   mQGisIface->removePluginMenuItem("&Tools",mMenuId);
  delete mToolBarPointer;
  //kill any connections to this object
  disconnect( this, 0, 0, 0 );
}

//////////////////////////////////////////////////////////////////////
//
//                  END OF MANDATORY PLUGIN METHODS
//           (your own implemented methods should follow now)
//////////////////////////////////////////////////////////////////////

void QgsHttpServerPlugin::setEnabled (bool theFlag)
{
  //stop the server first if it is running.
  if (mEnabled)
  {
    Q_CHECK_PTR( mHttpDaemon );
    if (mHttpDaemon)
    {
      stopServer();
    }
    mEnabled=false;
  }
  //restart it if enabled is true
  if (theFlag)
  {
    startServer();
    mEnabled=true;
  }
}
void QgsHttpServerPlugin::setPort(int thePortInt)
{
 mPortInt=thePortInt;
 //this will have the effect of restarting the server if its already running
 if (mEnabled)
 {
  setEnabled(true);
 }
 QSettings myQSettings;  // where we keep last used filter in persistant state
 myQSettings.writeEntry("/qgis/http_server/port",mPortInt);
}
void QgsHttpServerPlugin::startServer()
{ 
  mHttpDaemon = new HttpDaemon(mPortInt, this );
  connect(mHttpDaemon, SIGNAL(clearMap()), this, SLOT(clearMap()));
  connect(mHttpDaemon, SIGNAL(getMap(QPixmap *)), this, SLOT(getMap(QPixmap *)));
  connect(mHttpDaemon, SIGNAL(showProject(QString)), this, SLOT(showProject(QString)));
  connect(mHttpDaemon, SIGNAL(loadRasterFile(QString)), this, SLOT(loadRasterFile(QString)));
  connect(mHttpDaemon, SIGNAL(loadRasterFile(QString,QString)), this, SLOT(loadRasterFile(QString,QString)));
  connect(mHttpDaemon, SIGNAL(loadPseudoColorRasterFile(QString)), this, SLOT(loadPseudoColorRasterFile(QString)));
  connect(mHttpDaemon, SIGNAL(loadPseudoColorRasterFile(QString,QString)), this, SLOT(loadPseudoColorRasterFile(QString,QString)));
  connect(mHttpDaemon, SIGNAL(loadVectorFile(QString)),this, SLOT(loadVectorFile(QString))) ;
  connect(mHttpDaemon, SIGNAL(loadVectorFile(QString,QString)), this, SLOT(loadVectorFile(QString,QString)));
  mEnabled=true;

}

void QgsHttpServerPlugin::stopServer()
{
  //break all connections to httpdeamon signals
  disconnect( mHttpDaemon, 0, 0, 0 );
  delete mHttpDaemon;
  mEnabled=false;
}

//clear the current map
void QgsHttpServerPlugin::clearMap()
{
  mQGisIface->newProject(false);
}

//get the map in the provided pixmap
void QgsHttpServerPlugin::getMap(QPixmap *theQPixmap)
{
  mQGisIface->getMapCanvas()->render(theQPixmap);
}
//load the project in qgis and send image to browser
void QgsHttpServerPlugin::showProject(QString theProjectFile)
{
  std::cout << "Render called " << std::endl;
  //do all the stuff needed to open the project and take a snapshot of it
  if (!mQGisIface->addProject(theProjectFile))
  {
    //let the httpdserver know we are finished and pass it back the output filename
    mHttpDaemon->requestCompleted(QString("Failed opening project!"));
  }
  else
  {
    //let the httpdserver know we are finished and pass it back the canvas image
    mHttpDaemon->requestCompleted(mQGisIface->getMapCanvas()->canvasPixmap());
  }

}
//load project in qgis but dont send image back yet
void QgsHttpServerPlugin::loadProject(QString theProjectFile)
{
  std::cout << "Recevied loadProject request to open " << theProjectFile << std::endl;
  //do all the stuff needed to open the project and take a snapshot of it
  if (!mQGisIface->addProject(theProjectFile))
  {
    //let the httpdserver know we are finished and pass it back the output filename
    mHttpDaemon->requestCompleted(QString("Failed opening project!"));
  }
}



void QgsHttpServerPlugin::loadRasterFile(QString theRasterFile)
{
  QFileInfo myFileInfo(theRasterFile);
  QString myDirNameQString = myFileInfo.dirPath();
  QString myBaseNameQString = myFileInfo.baseName();
  QgsRasterLayer *layer = new QgsRasterLayer(theRasterFile, myBaseNameQString);
  //if ( mQGisIface->addRasterLayer(theRasterFile))
  if ( mQGisIface->addRasterLayer(layer))
  {
    //let the httpdserver know we are finished and pass it back the canvas image
    mHttpDaemon->requestCompleted(mQGisIface->getMapCanvas()->canvasPixmap());
  }
}

void QgsHttpServerPlugin::loadRasterFile(QString theRasterFile, QString theProjectFile)
{
  loadProject(theProjectFile);
  loadRasterFile(theRasterFile);
}

void QgsHttpServerPlugin::loadPseudoColorRasterFile(QString theRasterFile)
{
  QFileInfo myFileInfo(theRasterFile);
  QString myDirNameQString = myFileInfo.dirPath();
  QString myBaseNameQString = myFileInfo.baseName();
  QgsRasterLayer *layer = new QgsRasterLayer(theRasterFile, myBaseNameQString);
  layer->setColorRampingType(QgsRasterLayer::BLUE_GREEN_RED);
  layer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR);
  if ( mQGisIface->addRasterLayer(layer))
  {
    //let the httpdserver know we are finished and pass it back the canvas image
    mHttpDaemon->requestCompleted(mQGisIface->getMapCanvas()->canvasPixmap());
  }
}

void QgsHttpServerPlugin::loadPseudoColorRasterFile(QString theRasterFile, QString theProjectFile)
{
  loadProject(theProjectFile);
  loadPseudoColorRasterFile(theRasterFile);
}

void QgsHttpServerPlugin::loadVectorFile(QString theVectorFile)
{

  // Add a vector layer given vectorLayerPath, layer name, providerKey ("ogr" or "postgres");
  if (!mQGisIface->addVectorLayer(theVectorFile, QString("MapLayer"),QString("ogr")))
  {
    //let the httpdserver know we are finished with and error and pass it back the error
    mHttpDaemon->closeStreamWithError(QString("Failed opening vector layer : ")+theVectorFile);
  }
}

void QgsHttpServerPlugin::loadVectorFile(QString theVectorFile, QString theProjectFile)
{
  loadProject(theProjectFile);
  loadVectorFile(theVectorFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//  THE FOLLOWING CODE IS AUTOGENERATED BY THE PLUGIN BUILDER SCRIPT
//    YOU WOULD NORMALLY NOT NEED TO MODIFY THIS, AND YOUR PLUGIN
//      MAY NOT WORK PROPERLY IF YOU MODIFY THIS INCORRECTLY
//
//
//////////////////////////////////////////////////////////////////////////


/** 
 * Required extern functions needed  for every plugin 
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  return new QgsHttpServerPlugin(theQGisAppPointer, theQgisInterfacePointer);
}
// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return sName;
}

// Return the description
QGISEXTERN QString description()
{
  return sDescription;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return sType;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return sVersion;
}

// Delete ourself
QGISEXTERN void unload(QgisPlugin * thePluginPointer)
{
  delete thePluginPointer;
}



