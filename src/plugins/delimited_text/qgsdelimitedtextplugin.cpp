/***************************************************************************
  qgsdelimitedtextplugin.cpp
  Import tool for various worldmap analysis output files
Functions:

-------------------
  begin                : Feb 21, 2004
  copyright            : (C) 2004 by Gary Sherman
  email                : sherman at mrcc.com

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
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsdelimitedtextplugin.h"


#include <QMenu>
#include <QAction>
#include <QFile>

//non qt includes
#include <iostream>

//the gui subclass
#include "qgsdelimitedtextplugingui.h"

//

static const QString pluginVersion = QObject::tr( "Version 0.2" );
static const QString description_ = QObject::tr( "Loads and displays delimited text files containing x,y coordinates" );
static const QString icon_ = ":/delimited_text.png";

/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsDelimitedTextPlugin::QgsDelimitedTextPlugin( QgisInterface * theQgisInterFace )
    : qGisInterface( theQgisInterFace )
{
  /** Initialize the plugin and set the required attributes */
  pluginNameQString = tr( "DelimitedTextLayer" );
  pluginVersionQString = pluginVersion;
  pluginDescriptionQString = description_;

}

QgsDelimitedTextPlugin::~QgsDelimitedTextPlugin()
{

}

/* Following functions return name, description, version, and type for the plugin */
QString QgsDelimitedTextPlugin::name()
{
  return pluginNameQString;
}

QString QgsDelimitedTextPlugin::version()
{
  return pluginVersionQString;

}

QString QgsDelimitedTextPlugin::description()
{
  return pluginDescriptionQString;

}

int QgsDelimitedTextPlugin::type()
{
  return QgisPlugin::UI;
}
//method defined in interface
void QgsDelimitedTextPlugin::help()
{
  //implement me!
}

/*
 * Initialize the GUI interface for the plugin
 */
void QgsDelimitedTextPlugin::initGui()
{
  // Create the action for tool
  myQActionPointer = new QAction( QIcon(), tr( "&Add Delimited Text Layer" ), this );
  setCurrentTheme( "" );
  myQActionPointer->setWhatsThis( tr( "Add a delimited text file as a map layer. "
                                      "The file must have a header row containing the field names. "
                                      "The file must either contain X and Y fields with coordinates in decimal units or a WKT field." ) );
  // Connect the action to the run
  connect( myQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  // Add the icon to the toolbar
  qGisInterface->addToolBarIcon( myQActionPointer );
  qGisInterface->insertAddLayerAction( myQActionPointer );
  // this is called when the icon theme is changed
  connect( qGisInterface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );

}

// Slot called when the buffer menu item is activated
void QgsDelimitedTextPlugin::run()
{
  QgsDelimitedTextPluginGui *myQgsDelimitedTextPluginGui =
    new QgsDelimitedTextPluginGui( qGisInterface,
                                   qGisInterface->mainWindow(), QgisGui::ModalDialogFlags );
  myQgsDelimitedTextPluginGui->setAttribute( Qt::WA_DeleteOnClose );
  //listen for when the layer has been made so we can draw it
  connect( myQgsDelimitedTextPluginGui,
           SIGNAL( drawVectorLayer( QString, QString, QString ) ),
           this, SLOT( drawVectorLayer( QString, QString, QString ) ) );
  myQgsDelimitedTextPluginGui->exec();
}
//!draw a vector layer in the qui - intended to respond to signal
//sent by diolog when it as finished creating a layer
////needs to be given vectorLayerPath, baseName,
//providerKey ("ogr" or "postgres");
void QgsDelimitedTextPlugin::drawVectorLayer( QString thePathNameQString,
    QString theBaseNameQString, QString theProviderQString )
{
  qGisInterface->addVectorLayer( thePathNameQString,
                                 theBaseNameQString, theProviderQString );
}

// Unload the plugin by cleaning up the GUI
void QgsDelimitedTextPlugin::unload()
{
  // remove the GUI
  qGisInterface->removeAddLayer( myQActionPointer );
  qGisInterface->removeToolBarIcon( myQActionPointer );
  delete myQActionPointer;
}

//! Set icons to the current theme
void QgsDelimitedTextPlugin::setCurrentTheme( QString theThemeName )
{
  QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/delimited_text.png";
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/delimited_text.png";
  QString myQrcPath = ":/delimited_text.png";
  if ( QFile::exists( myCurThemePath ) )
  {
    myQActionPointer->setIcon( QIcon( myCurThemePath ) );
  }
  else if ( QFile::exists( myDefThemePath ) )
  {
    myQActionPointer->setIcon( QIcon( myDefThemePath ) );
  }
  else if ( QFile::exists( myQrcPath ) )
  {
    myQActionPointer->setIcon( QIcon( myQrcPath ) );
  }
  else
  {
    myQActionPointer->setIcon( QIcon() );
  }
}

/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new QgsDelimitedTextPlugin( theQgisInterfacePointer );
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return QString( QObject::tr( "Add Delimited Text Layer" ) );
}

// Return the description
QGISEXTERN QString description()
{
  return description_;
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
  return QgisPlugin::UI;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return pluginVersion;
}

QGISEXTERN QString icon()
{
  return icon_;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin * theQgsDelimitedTextPluginPointer )
{
  delete theQgsDelimitedTextPluginPointer;
}
