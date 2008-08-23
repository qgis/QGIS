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

#include "qgisinterface.h"
#include "qgisgui.h"
#include "qgsmaplayer.h"
#include "plugin.h"

#include <QAction>

//non qt includes
#include <iostream>

//the gui subclass
#include "plugingui.h"


static const char * const ident_ = "$Id$";

static const QString name_ = QObject::tr( "Graticule Creator" );
static const QString description_ = QObject::tr( "Builds a graticule" );
static const QString version_ = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;
/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsGridMakerPlugin::QgsGridMakerPlugin( QgisInterface * theQgisInterFace ):
    QgisPlugin( name_, description_, version_, type_ ),
    qGisInterface( theQgisInterFace )
{
}

QgsGridMakerPlugin::~QgsGridMakerPlugin()
{

}

/* Following functions return name, description, version, and type for the plugin */
QString QgsGridMakerPlugin::name()
{
  return pluginNameQString;
}

QString QgsGridMakerPlugin::version()
{
  return pluginVersionQString;

}

QString QgsGridMakerPlugin::description()
{
  return pluginDescriptionQString;

}

int QgsGridMakerPlugin::type()
{
  return QgisPlugin::UI;
}

/*
 * Initialize the GUI interface for the plugin
 */
void QgsGridMakerPlugin::initGui()
{
  // Create the action for tool
  myQActionPointer = new QAction( QIcon( ":/grid_maker.png" ), tr( "&Graticule Creator" ), this );
  myQActionPointer->setWhatsThis( tr( "Creates a graticule (grid) and stores the result as a shapefile" ) );
  // Connect the action to the run
  connect( myQActionPointer, SIGNAL( activated() ), this, SLOT( run() ) );

  // Add the icon to the toolbar
  qGisInterface->addToolBarIcon( myQActionPointer );
  qGisInterface->addPluginMenu( tr( "&Graticules" ), myQActionPointer );

}
//method defined in interface
void QgsGridMakerPlugin::help()
{
  //implement me!
}

// Slot called when the buffer menu item is activated
void QgsGridMakerPlugin::run()
{
  QgsGridMakerPluginGui *myPluginGui = new QgsGridMakerPluginGui( qGisInterface->getMainWindow(), QgisGui::ModalDialogFlags );
  myPluginGui->setAttribute( Qt::WA_DeleteOnClose );
  //listen for when the layer has been made so we can draw it
  connect( myPluginGui, SIGNAL( drawRasterLayer( QString ) ), this, SLOT( drawRasterLayer( QString ) ) );
  connect( myPluginGui, SIGNAL( drawVectorLayer( QString, QString, QString ) ), this, SLOT( drawVectorLayer( QString, QString, QString ) ) );
  myPluginGui->show();
}
//!draw a raster layer in the qui - intended to respond to signal sent by diolog when it as finished creating
//layer
void QgsGridMakerPlugin::drawRasterLayer( QString theQString )
{
  qGisInterface->addRasterLayer( theQString );
}
//!draw a vector layer in the qui - intended to respond to signal sent by diolog when it as finished creating a layer
////needs to be given vectorLayerPath, baseName, providerKey ("ogr" or "postgres");
void QgsGridMakerPlugin::drawVectorLayer( QString thePathNameQString, QString theBaseNameQString, QString theProviderQString )
{
  qGisInterface->addVectorLayer( thePathNameQString, theBaseNameQString, theProviderQString );
}

// Unload the plugin by cleaning up the GUI
void QgsGridMakerPlugin::unload()
{
  // remove the GUI
  qGisInterface->removePluginMenu( tr( "&Graticules" ), myQActionPointer );
  qGisInterface->removeToolBarIcon( myQActionPointer );
  delete myQActionPointer;
}
/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new QgsGridMakerPlugin( theQgisInterfacePointer );
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return name_;
}

// Return the description
QGISEXTERN QString description()
{
  return description_;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return type_;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return version_;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
