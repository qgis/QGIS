/***************************************************************************
 *Copyright (C) 2008 Paolo L. Scala, Barbara Rita Barricelli, Marco Padula *
 * CNR, Milan Unit (Information Technology),                               *
 * Construction Technologies Institute.\n";                                *
 *                                                                         *
 * email : Paolo L. Scala <scala@itc.cnr.it>                               *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

//
// QGIS Specific includes
//

#include <qgisinterface.h>
#include <qgisgui.h>
#include <qgsapplication.h>
#include <qgsvectorlayer.h>

#include "dxf2shpconverter.h"
#include "dxf2shpconvertergui.h"

//
// Qt4 Related Includes
//

#include <QAction>
#include <QFile>
#include <QToolBar>

static const QString sName = QObject::tr( "Dxf2Shp Converter" );
static const QString sDescription = QObject::tr( "Converts from dxf to shp file format" );
static const QString sPluginVersion = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = ":/dxf2shp_converter.png";

//////////////////////////////////////////////////////////////////////
//
// THE FOLLOWING METHODS ARE MANDATORY FOR ALL PLUGINS
//
//////////////////////////////////////////////////////////////////////

/**
 * Constructor for the plugin. The plugin is passed a pointer
 * an interface object that provides access to exposed functions in QGIS.
 * @param theQGisInterface - Pointer to the QGIS interface object
 */
dxf2shpConverter::dxf2shpConverter( QgisInterface *theQgisInterface ): QgisPlugin
    ( sName, sDescription, sPluginVersion, sPluginType ), mQGisIface
    ( theQgisInterface ) {}

dxf2shpConverter::~dxf2shpConverter()
{

}

/*
 * Initialize the GUI interface for the plugin - this is only called once when the plugin is
 * added to the plugin registry in the QGIS application.
 */
void dxf2shpConverter::initGui()
{
  // Create the action for tool
  mQActionPointer = new QAction( QIcon(), "Dxf2Shp Converter", this );

  // Set the icon
  setCurrentTheme( "" );

  // Set the what's this text
  mQActionPointer->setWhatsThis( tr( "Converts DXF files in Shapefile format" ) );

  // Connect the action to the run
  connect( mQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );

  // Add the icon to the toolbar
  mQGisIface->addToolBarIcon( mQActionPointer );
  mQGisIface->addPluginToMenu( tr( "&Dxf2Shp" ), mQActionPointer );

  // this is called when the icon theme is changed
  connect( mQGisIface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );
}

//method defined in interface
void dxf2shpConverter::help()
{
  //implement me!
}

// Slot called when the menu item is activated
// If you created more menu items / toolbar buttons in initiGui, you should
// create a separate handler for each action - this single run() method will
// not be enough
void dxf2shpConverter::run()
{
  dxf2shpConverterGui *myPluginGui =
    new dxf2shpConverterGui( mQGisIface->mainWindow(), QgisGui::ModalDialogFlags );

  myPluginGui->setAttribute( Qt::WA_DeleteOnClose );

  connect( myPluginGui, SIGNAL( createLayer( QString, QString ) ), this, SLOT( addMyLayer( QString, QString ) ) );

  myPluginGui->show();
}

// Unload the plugin by cleaning up the GUI
void dxf2shpConverter::unload()
{
  // remove the GUI
  mQGisIface->removePluginMenu( tr( "&Dxf2Shp" ), mQActionPointer );
  mQGisIface->removeToolBarIcon( mQActionPointer );
  delete mQActionPointer;
}

void dxf2shpConverter::addMyLayer( QString myfname, QString mytitle )
{
  mQGisIface->addVectorLayer( myfname, mytitle, "ogr" );
}

//! Set icons to the current theme
void dxf2shpConverter::setCurrentTheme( QString theThemeName )
{
  Q_UNUSED( theThemeName );
  QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/dxf2shp_converter.png";
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/dxf2shp_converter.png";
  QString myQrcPath = ":/dxf2shp_converter.png";
  if ( QFile::exists( myCurThemePath ) )
  {
    mQActionPointer->setIcon( QIcon( myCurThemePath ) );
  }
  else if ( QFile::exists( myDefThemePath ) )
  {
    mQActionPointer->setIcon( QIcon( myDefThemePath ) );
  }
  else if ( QFile::exists( myQrcPath ) )
  {
    mQActionPointer->setIcon( QIcon( myQrcPath ) );
  }
  else
  {
    mQActionPointer->setIcon( QIcon() );
  }
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
 * Required extern functions needed for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin *classFactory( QgisInterface *theQgisInterfacePointer )
{
  return new dxf2shpConverter( theQgisInterfacePointer );
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
  return sPluginType;
}

// Return the version number for the plugin
QGISEXTERN QString version()
{
  return sPluginVersion;
}

QGISEXTERN QString icon()
{
  return sPluginIcon;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin *thePluginPointer )
{
  delete thePluginPointer;
}
