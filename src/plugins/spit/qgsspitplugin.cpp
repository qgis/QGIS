/***************************************************************************
                          qgsspitplugin.cpp
 Shapefile to PostgreSQL Import Tool plugin
                             -------------------
    begin                : Jan 30, 2004
    copyright            : (C) 2004 by Gary E.Sherman
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
#include <vector>

#include <QAction>
#include <QFile>
#include <QMenu>

#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgsspitplugin.h"
#include "qgsspit.h"


static const QString name_ = QObject::tr( "SPIT" );
static const QString description_ = QObject::tr( "Shapefile to PostgreSQL/PostGIS Import Tool" );
static const QString version_ = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE type_ = QgisPlugin::UI;
static const QString icon_ = ":/spit.png";



/**
* Constructor for the plugin. The plugin is passed a pointer to the main app
* and an interface object that provides access to exposed functions in QGIS.
* @param qgis Pointer to the QGIS main window
* @parma _qI Pointer to the QGIS interface object
*/
QgsSpitPlugin::QgsSpitPlugin( QgisInterface * _qI )
    : QgisPlugin( name_, description_, version_, type_ ),
    qgisMainWindow( _qI->mainWindow() ),
    qI( _qI )
{
}

QgsSpitPlugin::~QgsSpitPlugin()
{

}

/*
* Initialize the GUI interface for the plugin
*/
void QgsSpitPlugin::initGui()
{
  // Create the action for tool
  spitAction = new QAction( QIcon(), tr( "&Import Shapefiles to PostgreSQL" ), this );
  setCurrentTheme( "" );
  spitAction->setWhatsThis( tr( "Import shapefiles into a PostGIS-enabled PostgreSQL database. "
                                "The schema and field names can be customized on import" ) );
  // Connect the action to the spit slot
  connect( spitAction, SIGNAL( triggered() ), this, SLOT( spit() ) );
  // Add the icon to the toolbar and to the plugin menu
  qI->addToolBarIcon( spitAction );
  qI->addPluginToDatabaseMenu( tr( "&Spit" ), spitAction );

  // this is called when the icon theme is changed
  connect( qI, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( setCurrentTheme( QString ) ) );
}

// Slot called when the shapefile to postgres menu item is triggered
void QgsSpitPlugin::spit()
{
  QgsSpit *spitDlg = new QgsSpit( qgisMainWindow, Qt::Window );
  spitDlg->setAttribute( Qt::WA_DeleteOnClose );
  spitDlg->show();
}


// Unload the plugin by cleaning up the GUI
void QgsSpitPlugin::unload()
{
  // remove the GUI
  qI->removeToolBarIcon( spitAction );
  qI->removePluginDatabaseMenu( tr( "&Spit" ), spitAction );
  delete spitAction;
}

//! Set icons to the current theme
void QgsSpitPlugin::setCurrentTheme( QString theThemeName )
{
  Q_UNUSED( theThemeName );
  QString myCurThemePath = QgsApplication::activeThemePath() + "/plugins/spit.png";
  QString myDefThemePath = QgsApplication::defaultThemePath() + "/plugins/spit.png";
  QString myQrcPath = ":/spit.png";
  if ( QFile::exists( myCurThemePath ) )
  {
    spitAction->setIcon( QIcon( myCurThemePath ) );
  }
  else if ( QFile::exists( myDefThemePath ) )
  {
    spitAction->setIcon( QIcon( myDefThemePath ) );
  }
  else if ( QFile::exists( myQrcPath ) )
  {
    spitAction->setIcon( QIcon( myQrcPath ) );
  }
  else
  {
    spitAction->setIcon( QIcon() );
  }
}

/**
* Required extern functions needed  for every plugin
* These functions can be called prior to creating an instance
* of the plugin class
*/
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory( QgisInterface * qI )
{
  return new QgsSpitPlugin( qI );
}

// Return the name of the plugin
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

// Return the version
QGISEXTERN QString version()
{
  return version_;
}

QGISEXTERN QString icon()
{
  return icon_;
}


// Delete ourself
QGISEXTERN void unload( QgisPlugin * p )
{
  delete p;
}
