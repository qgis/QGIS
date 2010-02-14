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
/*  $Id: plugin.cpp 9231 2008-08-31 16:56:09Z timlinux $ */

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

//
// Required qgis includes
//

#include <qgisinterface.h>
#include <qgsapplication.h>
#include <qgsmaplayer.h>
#include <qgsrasterlayer.h>
#include "qgsgeorefplugin.h"

#include <QFile>

//
//the gui subclass
//
//#include "plugingui.h"
#include "qgspointdialog.h"
#include "qgsgeorefdescriptiondialog.h"

static const char * const sIdent = "$Id: plugin.cpp 9231 2008-08-31 16:56:09Z timlinux $";
static const QString sName = QObject::tr( "Georeferencer GDAL" );
static const QString sDescription = QObject::tr( "Adding projection info to rasters using GDAL" );
static const QString sPluginVersion = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;

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
  mQActionPointer = new QAction( QIcon(), tr( "&Georeferencer" ), this );
  setCurrentTheme( "" );

  // Connect the action to the run
  connect( mQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );

  // this is called when the icon theme is changed
  connect( mQGisIface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );

  // Add to the toolbar & menu
  mQGisIface->addToolBarIcon( mQActionPointer );
  mQGisIface->addPluginToMenu( tr( "&Georeferencer" ), mQActionPointer );

  mQActionPointerAbout = new QAction( QIcon( ":/about.png" ), tr( "&About" ), this );
//  mQActionPointer = new QAction("About", this);
  connect(mQActionPointerAbout, SIGNAL(triggered()), this, SLOT(about()));
  mQGisIface->addPluginToMenu(tr ("&Georeferencer"), mQActionPointerAbout);

  mQActionPointerHelp = new QAction( QIcon( ":/help.png" ), tr( "&Help" ), this );
//  mQActionPointer = new QAction("Help", this);
  connect(mQActionPointerHelp, SIGNAL(triggered()), this, SLOT(help()));
  mQGisIface->addPluginToMenu(tr ("&Georeferencer"), mQActionPointerHelp);
}
//method defined in interface
void QgsGeorefPlugin::help()
{
  QgsGeorefDescriptionDialog dlg( mQGisIface->mainWindow( ) );
  dlg.exec();
}

void QgsGeorefPlugin::about( )
{
  QDialog dlg( mQGisIface->mainWindow( ) );
  dlg.setWindowFlags( dlg.windowFlags( ) | Qt::MSWindowsFixedSizeDialogHint );
  dlg.setWindowFlags( dlg.windowFlags( ) &~ Qt::WindowContextHelpButtonHint );
  QVBoxLayout *lines = new QVBoxLayout( &dlg );
  lines->addWidget( new QLabel( tr( "<b>Georeferencer GDAL</b>" ) ) );
  lines->addWidget( new QLabel( tr( "    Based on original Georeferencer Plugin" ) ) );
  lines->addWidget( new QLabel( tr( "<b>Developers:</b>" ) ) );
  lines->addWidget( new QLabel( tr( "    Lars Luthman (original Georeferencer)" ) ) );
  lines->addWidget( new QLabel( "    Lynx (lynx21.12.12@gmail.ru)" ) );
  lines->addWidget( new QLabel( "    Maxim Dubinin (sim@gis-lab.info)" ) );
  lines->addWidget( new QLabel( tr( "<b>Links:</b>" ) ) );
  QLabel *link = new QLabel( "     <a href=\"http://gis-lab.info/qa/qgis-georef-new-eng.html\">http://gis-lab.info/qa/qgis-georef-new-eng.html</a>" );
  link->setOpenExternalLinks( true );
  lines->addWidget( link );

  dlg.exec( );
}

// Slot called when the buffer menu item is triggered
void QgsGeorefPlugin::run()
{
//  QgsGeorefPluginGui *myPluginGui = new QgsGeorefPluginGui( mQGisIface, QgsGeorefPluginGui::findMainWindow(), Qt::Window | Qt::WindowMinimizeButtonHint);
//  myPluginGui->show();
//  myPluginGui->setFocus();
  QgsPointDialog *myPlugin = new QgsPointDialog( mQGisIface, QgsPointDialog::findMainWindow(), Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowTitleHint );
  myPlugin->move( 0, 0 );
  myPlugin->show();
  myPlugin->setFocus();
}

// Unload the plugin by cleaning up the GUI
void QgsGeorefPlugin::unload()
{
  // remove the GUI
  disconnect( mQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  disconnect(mQActionPointerAbout, SIGNAL(triggered()), this, SLOT(about()));
  disconnect(mQActionPointerHelp, SIGNAL(triggered()), this, SLOT(help()));
  mQGisIface->removePluginMenu( tr( "&Georeferencer" ), mQActionPointer );
  mQGisIface->removePluginMenu( tr( "&About" ), mQActionPointerAbout );
  mQGisIface->removePluginMenu( tr( "&Help" ), mQActionPointerHelp );
  mQGisIface->removeToolBarIcon( mQActionPointer );

  delete mQActionPointer;
  delete mQActionPointerAbout;
  delete mQActionPointerHelp;
}

//! Set icons to the current theme
void QgsGeorefPlugin::setCurrentTheme( QString theThemeName )
{
  QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/georeferencer.png";
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/georeferencer.png";
  QString myQrcPath = ":/georeferencer.png";
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

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
