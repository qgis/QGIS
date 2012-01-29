/***************************************************************************
  qgscompassplugin.cpp
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

// includes

#include "qgisinterface.h"
#include "qgisgui.h"
#include "qgsapplication.h"
#include "qgscompassplugin.h"

#include <QMenu>
#include <QAction>
#include <QFile>
#include <QToolBar>

//the gui subclass
#include "qgscompassplugingui.h"

static const QString pluginVersion = QObject::tr( "Version 0.1" );
static const QString description_ = QObject::tr( "Shows a QtSensors compass reading" );
static const QString category_ = QObject::tr( "Plugins" );
static const QString icon_ = ":/compass.svn";

/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsCompassPlugin::QgsCompassPlugin( QgisInterface * theQgisInterFace )
    : qGisInterface( theQgisInterFace )
{
  /** Initialize the plugin and set the required attributes */
  pluginNameQString = tr( "Internal compass" );
  pluginVersionQString = pluginVersion;
  pluginDescriptionQString = description_;
  pluginCategoryQString = category_;
  mDock = NULL;
}

QgsCompassPlugin::~QgsCompassPlugin()
{

}

/* Following functions return name, description, version, and type for the plugin */
QString QgsCompassPlugin::name()
{
  return pluginNameQString;
}

QString QgsCompassPlugin::version()
{
  return pluginVersionQString;

}

QString QgsCompassPlugin::description()
{
  return pluginDescriptionQString;

}

QString QgsCompassPlugin::category()
{
  return pluginCategoryQString;

}

int QgsCompassPlugin::type()
{
  return QgisPlugin::UI;
}
//method defined in interface
void QgsCompassPlugin::help()
{
  //implement me!
}

/*
 * Initialize the GUI interface for the plugin
 */
void QgsCompassPlugin::initGui()
{
  if (! mDock )
  {
    mDock = new QDockWidget("Compass", qGisInterface->mainWindow() );
    myQgsCompassPluginGui =  new QgsCompassPluginGui( mDock );
    mDock->setWidget( myQgsCompassPluginGui );
    mDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    qGisInterface->addDockWidget( Qt::LeftDockWidgetArea, mDock );
  }
  // Create the action for tool
  myQActionPointer = new QAction( QIcon(), tr( "Show compass" ), this );
  setCurrentTheme( "" );
  myQActionPointer->setWhatsThis( tr( "Show the reading of a QtSendors compass" ) );
  // Connect the action to the run
  connect( myQActionPointer, SIGNAL( triggered() ), this, SLOT( run() ) );
  // Add the icon to the toolbar
  qGisInterface->pluginToolBar()->addAction( myQActionPointer );
  qGisInterface->addPluginToMenu(pluginNameQString, myQActionPointer );
  // this is called when the icon theme is changed
  connect( qGisInterface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );
}

// Slot called when the buffer menu item is activated
void QgsCompassPlugin::run()
{
  mDock->show();
}

// Unload the plugin by cleaning up the GUI
void QgsCompassPlugin::unload()
{
  // remove the GUI
  qGisInterface->pluginToolBar()->removeAction( myQActionPointer );
  qGisInterface->removePluginMenu( pluginNameQString, myQActionPointer );
  delete myQActionPointer;
}

//! Set icons to the current theme
void QgsCompassPlugin::setCurrentTheme( QString theThemeName )
{
  Q_UNUSED( theThemeName );
  QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/compass.png";
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/compass.png";
  QString myQrcPath = ":/compass.svg";
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
  return new QgsCompassPlugin( theQgisInterfacePointer );
}

// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name()
{
  return QString( QObject::tr( "Internal compass" ) );
}

// Return the description
QGISEXTERN QString description()
{
  return description_;
}

// Return the category
QGISEXTERN QString category()
{
  return category_;
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
QGISEXTERN void unload( QgisPlugin * theQgsCompassPluginPointer )
{
  delete theQgsCompassPluginPointer;
}
