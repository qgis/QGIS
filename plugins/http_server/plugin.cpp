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
static const char * const ident_ = "$Id$";

static const char * const name_ = "HttpServer";
static const char * const description_ = "A simple http server plugin that allows remote control of qgis.";
static const char * const version_ = "Version 0.1";
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;


/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
Plugin::Plugin(QgisApp * theQGisApp, QgisIface * theQgisInterFace):
          qgisMainWindowPointer(theQGisApp), 
          qGisInterface(theQgisInterFace),
          QgisPlugin(name_,description_,version_,type_)
{
}

Plugin::~Plugin()
{
 unload();
}

/*
 * Initialize the GUI interface for the plugin 
 */
void Plugin::initGui()
{
  // add a menu with 2 items
  QPopupMenu *pluginMenu = new QPopupMenu(qgisMainWindowPointer);

  pluginMenu->insertItem(QIconSet(icon),"&HttpServer", this, SLOT(run()));

  menuBarPointer = ((QMainWindow *) qgisMainWindowPointer)->menuBar();

  menuIdInt = qGisInterface->addMenu("&Tools", pluginMenu);
  // Create the action for tool
  QAction *myQActionPointer = new QAction("QGis Http Server", QIconSet(icon), "&Wmi",0, this, "run");
  // Connect the action to the run
  connect(myQActionPointer, SIGNAL(activated()), this, SLOT(run()));
  // Add the toolbar
  toolBarPointer = new QToolBar((QMainWindow *) qgisMainWindowPointer, "Tools");
  toolBarPointer->setLabel("QGis Http Server");
  // Add the zoom previous tool to the toolbar
  myQActionPointer->addTo(toolBarPointer);
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
void Plugin::help()
{
  //implement me!
}

// Slot called when the menu item is activated
void Plugin::run()
{
  PluginGui *myPluginGui=new PluginGui(qgisMainWindowPointer,"QGis Http Server",true,0);
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
void Plugin::unload()
{
  //kill the web server daemon
  stopServer();
  //delete mHttpDaemon;
  // remove the GUI
  menuBarPointer->removeItem(menuIdInt);
  delete toolBarPointer;
  //kill any connections to this object
  disconnect( this, 0, 0, 0 );
}

void Plugin::setEnabled (bool theFlag)
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
void Plugin::setPort(int thePortInt)
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
void Plugin::startServer()
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

void Plugin::stopServer()
{
  //break all connections to httpdeamon signals
  disconnect( mHttpDaemon, 0, 0, 0 );
  delete mHttpDaemon;
  mEnabled=false;
}

//clear the current map
void Plugin::clearMap()
{
  qGisInterface->newProject(false);
}

//get the map in the provided pixmap
void Plugin::getMap(QPixmap *theQPixmap)
{
  qGisInterface->getMapCanvas()->render(theQPixmap);
}
//load the project in qgis and send image to browser
void Plugin::showProject(QString theProjectFile)
{
  std::cout << "Render called " << std::endl;
  //do all the stuff needed to open the project and take a snapshot of it
  if (!qGisInterface->addProject(theProjectFile))
  {
    //let the httpdserver know we are finished and pass it back the output filename
    mHttpDaemon->requestCompleted(QString("Failed opening project!"));
  }
  else
  {
    //let the httpdserver know we are finished and pass it back the canvas image
    mHttpDaemon->requestCompleted(qGisInterface->getMapCanvas()->canvasPixmap());
  }

}
//load project in qgis but dont send image back yet
void Plugin::loadProject(QString theProjectFile)
{
  std::cout << "Recevied loadProject request to open " << theProjectFile << std::endl;
  //do all the stuff needed to open the project and take a snapshot of it
  if (!qGisInterface->addProject(theProjectFile))
  {
    //let the httpdserver know we are finished and pass it back the output filename
    mHttpDaemon->requestCompleted(QString("Failed opening project!"));
  }
}



void Plugin::loadRasterFile(QString theRasterFile)
{
  QFileInfo myFileInfo(theRasterFile);
  QString myDirNameQString = myFileInfo.dirPath();
  QString myBaseNameQString = myFileInfo.baseName();
  QgsRasterLayer *layer = new QgsRasterLayer(theRasterFile, myBaseNameQString);
  //if ( qGisInterface->addRasterLayer(theRasterFile))
  if ( qGisInterface->addRasterLayer(layer))
  {
    //let the httpdserver know we are finished and pass it back the canvas image
    mHttpDaemon->requestCompleted(qGisInterface->getMapCanvas()->canvasPixmap());
  }
}

void Plugin::loadRasterFile(QString theRasterFile, QString theProjectFile)
{
  loadProject(theProjectFile);
  loadRasterFile(theRasterFile);
}

void Plugin::loadPseudoColorRasterFile(QString theRasterFile)
{
  QFileInfo myFileInfo(theRasterFile);
  QString myDirNameQString = myFileInfo.dirPath();
  QString myBaseNameQString = myFileInfo.baseName();
  QgsRasterLayer *layer = new QgsRasterLayer(theRasterFile, myBaseNameQString);
  layer->setColorRampingType(QgsRasterLayer::BLUE_GREEN_RED);
  layer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR);
  if ( qGisInterface->addRasterLayer(layer))
  {
    //let the httpdserver know we are finished and pass it back the canvas image
    mHttpDaemon->requestCompleted(qGisInterface->getMapCanvas()->canvasPixmap());
  }
}

void Plugin::loadPseudoColorRasterFile(QString theRasterFile, QString theProjectFile)
{
  loadProject(theProjectFile);
  loadPseudoColorRasterFile(theRasterFile);
}

void Plugin::loadVectorFile(QString theVectorFile)
{

  // Add a vector layer given vectorLayerPath, layer name, providerKey ("ogr" or "postgres");
  if (!qGisInterface->addVectorLayer(theVectorFile, QString("MapLayer"),QString("ogr")))
  {
    //let the httpdserver know we are finished with and error and pass it back the error
    mHttpDaemon->closeStreamWithError(QString("Failed opening vector layer : ")+theVectorFile);
  }
}

void Plugin::loadVectorFile(QString theVectorFile, QString theProjectFile)
{
  loadProject(theProjectFile);
  loadVectorFile(theVectorFile);
}


///////////////////////////////////////////////////////////





/** 
 * Required extern functions needed  for every plugin 
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
extern "C" QgisPlugin * classFactory(QgisApp * theQGisAppPointer, QgisIface * theQgisInterfacePointer)
{
  return new Plugin(theQGisAppPointer, theQgisInterfacePointer);
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
extern "C" QString name()
{
    return name_;
}

// Return the description
extern "C" QString description()
{
    return description_;
}

// Return the type (either UI or MapLayer plugin)
extern "C" int type()
{
    return type_;
}

// Return the version number for the plugin
extern "C" QString version()
{
  return version_;
}

// Delete ourself
extern "C" void unload(QgisPlugin * thePluginPointer)
{
  delete thePluginPointer;
}
