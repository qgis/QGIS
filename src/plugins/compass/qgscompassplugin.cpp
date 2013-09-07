/***************************************************************************
                          qgscompassplugin.cpp
 Functions:
                             -------------------
    begin                : Jan 28, 2012
    copyright            : (C) 2012 by Marco Bernasocchi
    email                : marco@bernawebdesign.ch

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

#include <qgisinterface.h>
#include <qgisgui.h>
#include <qgsapplication.h>
#include "qgscompassplugin.h"

#include <QMenu>
#include <QAction>
#include <QFile>
#include <QToolBar>
#include <QMessageBox>
#include "qgscompassplugingui.h"


static const QString sName = QObject::tr( "Internal Compass" );
static const QString sDescription = QObject::tr( "Shows a QtSensors compass reading" );
static const QString sCategory = QObject::tr( "Plugins" );
static const QString sPluginVersion = QObject::tr( "Version 0.9" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;
static const QString sPluginIcon = ":/compass.svn";

/**
 * Constructor for the plugin. The plugin is passed a pointer to the main app
 * and an interface object that provides access to exposed functions in QGIS.
 * @param qgis Pointer to the QGIS main window
 * @param _qI Pointer to the QGIS interface object
 */
QgsCompassPlugin::QgsCompassPlugin( QgisInterface * themQGisIface )
    : QgisPlugin( sName, sDescription, sCategory, sPluginVersion, sPluginType ),
    mQGisIface( themQGisIface )
{
  /** Initialize the plugin */
  mDock = NULL;
}

QgsCompassPlugin::~QgsCompassPlugin()
{
}

/* Following functions return name, description, version, and type for the plugin */
QString QgsCompassPlugin::name()
{
  return sName;
}

QString QgsCompassPlugin::version()
{
  return sPluginVersion;

}

QString QgsCompassPlugin::description()
{
  return sDescription;

}

QString QgsCompassPlugin::category()
{
  return sCategory;

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

  // Create the action for tool
  mActionRunCompass = new QAction( QIcon(), tr( "Show compass" ), this );
  connect( mActionRunCompass, SIGNAL( triggered() ), this, SLOT( run() ) );

  mActionAboutCompass = new QAction( QIcon(), tr( "&About" ), this );
  connect( mActionAboutCompass, SIGNAL( triggered() ), this, SLOT( about() ) );

  setCurrentTheme( "" );
  // this is called when the icon theme is changed
  connect( mQGisIface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );

  // Add the icon to the toolbar
  mQGisIface->pluginToolBar()->addAction( mActionRunCompass );
  //mQGisIface->pluginToolBar()->addAction( mActionAboutCompass );
  mQGisIface->addPluginToMenu( sName, mActionRunCompass );
  mQGisIface->addPluginToMenu( sName, mActionAboutCompass );
  // this is called when the icon theme is changed

}

// Slot called when the buffer menu item is activated
void QgsCompassPlugin::run()
{
  if ( ! mDock )
  {
    mDock = new QDockWidget( "Internal Compass", mQGisIface->mainWindow() );
    mQgsCompassPluginGui =  new QgsCompassPluginGui( mDock );
    mDock->setWidget( mQgsCompassPluginGui );
    mDock->setFeatures( QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable );
    mQGisIface->addDockWidget( Qt::LeftDockWidgetArea, mDock );

  }
  mDock->show();
  QObject::connect( mDock, SIGNAL( visibilityChanged( bool ) ), mQgsCompassPluginGui, SLOT( handleVisibilityChanged( bool ) ) );
}

// Unload the plugin by cleaning up the GUI
void QgsCompassPlugin::unload()
{
  // remove the GUI
  mQGisIface->removeToolBarIcon( mActionRunCompass );
  mQGisIface->removePluginMenu( sName, mActionRunCompass );

  //mQGisIface->removeToolBarIcon( mActionAboutCompass );
  mQGisIface->removePluginMenu( sName, mActionAboutCompass );

  delete mActionRunCompass;
  mActionRunCompass = 0;
  delete mActionAboutCompass;
  mActionAboutCompass = 0;
  delete mDock;
  mDock = 0;
}

//! Set icons to the current theme
void QgsCompassPlugin::setCurrentTheme( QString )
{
  if ( mActionRunCompass && mActionAboutCompass )
  {
    mActionRunCompass->setIcon( getThemeIcon( "/mCompassRun.png" ) );
    mActionAboutCompass->setIcon( getThemeIcon( "/mActionAbout.png" ) );
  }
}

QIcon QgsCompassPlugin::getThemeIcon( const QString &theName )
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

void QgsCompassPlugin::about( )
{
  QString title = QString( "About Internal Compass" );
  // sort by date of contribution
  QString text = QString( "<center><b>Internal Compass</b></center>"
                          "<center>%1</center>"
                          "<p>Shows reading of an internal compass using QtSensors<br/>"
                          "<b>Developer:</b>"
                          "<ol type=disc>"
                          "<li>Marco Bernasocchi"
                          "</ol>"
                          "<p><b>Homepage:</b><br>"
                          "<a href=\"http://opengis.ch\">http://opengis.ch</a></p>"
                          "<p><b>Compass calibration:</b><br/>"
                          "To calibrate the compass slowly rotate the device three times around each axis or "
                          "rotate it like a on a Mobius strip.<br/>"
                          "This <a href='http://www.youtube.com/watch?v=oNJJPeoG8lQ'>Video</a> demonstrates the process "
                          "(this can be done from within QGIS as well).</p>"
                        ).arg( sPluginVersion );

  // create dynamicaly because on Mac this dialog is modeless
  QWidget *w = new QWidget;
  w->setAttribute( Qt::WA_DeleteOnClose );
  w->setWindowIcon( getThemeIcon( "/compass.png" ) );
  QMessageBox::about( w, title, text );
}

/**
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory( QgisInterface * themQGisIfacePointer )
{
  return new QgsCompassPlugin( themQGisIfacePointer );
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

// Return the category
QGISEXTERN QString category()
{
  return sCategory;
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
