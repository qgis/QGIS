/***************************************************************************
 *  File Name:               plugin.cpp
 *
 *  The georeferencer plugin is a tool for adding projection info to rasters
 *
 *--------------------------------------------------------------------------
 *    begin                : Jan 21, 2004
 *    copyright            : (C) 2004 by Tim Sutton
 *    email                : tim@linfiniti.com
 *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 *   QGIS Programming conventions:
 *
 *   mVariableName - a class level member variable
 *   sVariableName - a static class level member variable
 *   variableName() - accessor for a class member (no 'get' in front of name)
 *   setVariableName() - mutator for a class member (prefix with 'set')
 *
 *   Additional useful conventions:
 *
 *   theVariableName - a method parameter (prefix with 'the')
 *   myVariableName - a locally declared variable within a method ('my' prefix)
 *
 *   DO: Use mixed case variable names - myVariableName
 *   DON'T: separate variable names using underscores: my_variable_name (NO!)
 *
 * **************************************************************************/

/****************************************************************************
 * "Some work on Georeferencer is funded by Rosleszaschita, Russia"         *
 * **************************************************************************/

//
// Required qgis includes
//

#include <qgisinterface.h>
#include <qgsapplication.h>
#include <qgsmaplayer.h>
#include <qgsrasterlayer.h>
#include "qgsgeorefplugin.h"

#include <QFile>
//#include <QDialog>
#include <QMessageBox>

//
//the gui subclass
//
#include "qgsgeorefplugingui.h"

static const QString sName = QObject::tr( "Georeferencer GDAL" );
static const QString sDescription = QObject::tr( "Georeferencing rasters using GDAL" );
static const QString sPluginVersion = QObject::tr( "Version 3.1.9" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = ":/icons/mGeorefRun.png";

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
QgsGeorefPlugin::QgsGeorefPlugin( QgisInterface * theQgisInterface ):
    QgisPlugin( sName, sDescription, sPluginVersion, sPluginType ),
    mQGisIface( theQgisInterface )
{
}

QgsGeorefPlugin::~QgsGeorefPlugin()
{
}

/*
 * Initialize the GUI interface for the plugin
 */
void QgsGeorefPlugin::initGui()
{
  // Create the action for tool
  mActionRunGeoref = new QAction( QIcon(), tr( "&Georeferencer" ), this );

  // Connect the action to the run
  connect( mActionRunGeoref, SIGNAL( triggered() ), this, SLOT( run() ) );

  mActionAbout = new QAction( QIcon(), tr( "&About" ), this );
  connect( mActionAbout, SIGNAL( triggered() ), this, SLOT( about() ) );

  setCurrentTheme( "" );
  // this is called when the icon theme is changed
  connect( mQGisIface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );

  // Add to the toolbar & menu
  mQGisIface->addToolBarIcon( mActionRunGeoref );
  mQGisIface->addPluginToMenu( tr( "&Georeferencer" ), mActionRunGeoref );
  mQGisIface->addPluginToMenu( tr( "&Georeferencer" ), mActionAbout );
}

// Slot called when the buffer menu item is triggered
void QgsGeorefPlugin::run()
{
  mPluginGui = new QgsGeorefPluginGui( mQGisIface, mQGisIface->mainWindow() );
  mPluginGui->setAttribute( Qt::WA_DeleteOnClose );
  mPluginGui->show();
  mPluginGui->setFocus();
}

// Unload the plugin by cleaning up the GUI
void QgsGeorefPlugin::unload()
{
  // remove the GUI
  mQGisIface->removePluginMenu( tr( "&Georeferencer" ), mActionAbout );
  mQGisIface->removePluginMenu( tr( "&Georeferencer" ), mActionRunGeoref );
  mQGisIface->removeToolBarIcon( mActionRunGeoref );

  delete mActionRunGeoref;
  delete mActionAbout;
}

//! Set icons to the current theme
void QgsGeorefPlugin::setCurrentTheme( QString )
{
  mActionRunGeoref->setIcon( getThemeIcon( "/mGeorefRun.png" ) );
  mActionAbout->setIcon( getThemeIcon( "/mActionAbout.png" ) );
}

QIcon QgsGeorefPlugin::getThemeIcon( const QString &theName )
{
  if ( QFile::exists( QgsApplication::activeThemePath() + "/plugins" + theName ) )
  {
    return QIcon( QgsApplication::activeThemePath() + "/plugins" + theName );
  }
  else if ( QFile::exists( QgsApplication::defaultThemePath() + "/plugins" + theName ) )
  {
    return QIcon( QgsApplication::defaultThemePath() + "/plugins" + theName );
  }
  else
  {
    return QIcon( ":/icons" + theName );
  }
}

void QgsGeorefPlugin::about( )
{
  QString title = QString( "About Georeferencer" );
  // sort by date of contribution
  QString text = QString( "<center><b>Georeferencer GDAL</b></center>"
                          "<center>%1</center>"
                          "<p>Adding projection info to rasters using GDAL<br>"
                          "<b>Developers:</b>"
                          "<ol type=disc>"
                          "<li>Jack R"
                          "<li>Maxim Dubinin"
                          "<li>Manuel Massing"
                          "<li>Lars Luthman"
                          "</ol>"
                          "<p><b>Homepage:</b><br>"
                          "<a href=\"http://gis-lab.info/qa/qgis-georef-new-eng.html\">http://gis-lab.info/qa/qgis-georef-new-eng.html</a>" ).arg( sPluginVersion );

  // this is required for adding georef icon in to left side of dialog
  // create dynamicaly because on Mac this dialog is modeless
  QWidget *w = new QWidget;
  w->setAttribute( Qt::WA_DeleteOnClose );
  w->setWindowIcon( getThemeIcon( "/mGeorefRun.png" ) );
  QMessageBox::about( w, title, text );
}

//////////////////////////////////////////////////////////////////////
//
//                  END OF MANDATORY PLUGIN METHODS
//
//////////////////////////////////////////////////////////////////////



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
QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new QgsGeorefPlugin( theQgisInterfacePointer );
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
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
