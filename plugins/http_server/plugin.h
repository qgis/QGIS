/***************************************************************************
  plugin.h 
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
#ifndef QGSHTTPSERVERPLUGIN
#define QGSHTTPSERVERPLUGIN
#include <qwidget.h>

#include <qgisplugin.h>
#include <qgisapp.h>
// non qt includes
#include "httpdaemon.h"
/**
 * \class QgsHttpServerPlugin
 * \brief OpenModeller plugin for QGIS
 *
 */
class QgsHttpServerPlugin:public QObject, public QgisPlugin
{
  Q_OBJECT public:
      /** 
       * Constructor for a plugin. The QgisApp and QgisIface pointers are passed by 
       * QGIS when it attempts to instantiate the plugin.
       * @param qgis Pointer to the QgisApp object
       * @param qI Pointer to the QgisIface object. 
       */
      QgsHttpServerPlugin(QgisApp * , QgisIface * );
  //! Destructor
  virtual ~ QgsHttpServerPlugin();
public slots:
  //! init the gui
  virtual void initGui();
  //! Show the dialog box
  void run();
  //! Used to clear the current map view
  void clearMap();
  //! Used to get an image of the map and pass it back to the browser (use after all layer ops)
  void getMap(QPixmap *);
  //! Load and show a project file
  void showProject(QString theProjectFile);
  //! Load a project file but dont return it to browser yet
  void loadProject(QString theProjectFile);
  //! Load a raster file on its own
  void loadRasterFile(QString theRasterFile);
  //! Load a raster file over a project
  void loadRasterFile(QString theRasterFile, QString theProjectFile);
  //! Load a raster file over a project and display using pseudocolor renderer
  void loadPseudoColorRasterFile(QString theRasterFile);
  //! Load a raster file over a project and display using pseudocolor renderer over a project file
  void loadPseudoColorRasterFile(QString theRasterFile, QString theProjectFile);
  //! Load a vector file on its own
  void loadVectorFile(QString theVectorFile);
  //! Load a vector file over a project
  void loadVectorFile(QString theVectorFile, QString theProjectFile);
  //! Change the enabled status of the http server
  void setEnabled (bool);
  //! Return the status of the http server
  bool enabled () {return mEnabled; } ;
  //!Accessor for port property
  int port() {return mPortInt;} ;
  //!mutator for port property
  void setPort(int thePortInt);
  //! unload the plugin
  void unload();
  //! show the help document
  void help();
  //! Start the http server
  void startServer();
  //! Stop the server
  void stopServer();
  //! Passed by the http daemon when it receives a new request
  void requestReceived(QString){};
  //! Zoom to full extents of all layers
  void zoomToFullExtents(){};

private:
  //! The instance of the http server used by this plugin
  HttpDaemon * mHttpDaemon;
  //! Port number that server will listen on (defaults to 8081)
  int mPortInt;
  //! whether the server is enabled or not
  bool mEnabled;

  ////////////////////////////////////////////////////////////////////
  //
  // MANDATORY PLUGIN MEMBER DECLARATIONS  .....
  //
  ////////////////////////////////////////////////////////////////////
  
  int mPluginType;
  //! Id of the plugin's menu. Used for unloading
  int mMenuId;
  //! Pointer to our toolbar
  QToolBar *mToolBarPointer;
  //! Pionter to QGIS main application object
  QgisApp *mQGisApp;
  //! Pointer to the QGIS interface object
  QgisIface *mQGisIface;

  ////////////////////////////////////////////////////////////////////
  //
  // ADD YOUR OWN MEMBER DECLARATIONS AFTER THIS POINT.....
  //
  ////////////////////////////////////////////////////////////////////
};

#endif
