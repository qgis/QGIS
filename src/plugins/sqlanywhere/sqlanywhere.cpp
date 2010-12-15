/***************************************************************************
  sqlanywhere.cpp
  Store vector layers within a SQL Anywhere database
  -------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author		 : David DeHaan
    email                : ddehaan at sybase dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id$ */

//
// QGIS Specific includes
//

#include <qgisinterface.h>
#include <qgisgui.h>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsmaplayerregistry.h>
#include <qgslogger.h>
#include <qgsapplication.h>

#include "sqlanywhere.h"
#include "sasourceselect.h"
#include "sanewconnection.h"
#include "salayer.h"

//
// Qt4 Related Includes
//

#include <QAction>
#include <QToolBar>
#include <QMenu>
#include <QMessageBox>
#include <QMainWindow>
#include <QStatusBar>


static const QString sName = QObject::tr( "SQL Anywhere plugin" );
static const QString sDescription = QObject::tr( "Store vector layers within a SQL Anywhere database" );
static const QString sPluginVersion = QObject::tr( "Version 0.1" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;


/**
 * Constructor for the plugin. The plugin is passed a pointer
 * an interface object that provides access to exposed functions in QGIS.
 * @param theQGisInterface - Pointer to the QGIS interface object
 */
SqlAnywhere::SqlAnywhere( QgisInterface * theQgisInterface ):
    QgisPlugin( sName, sDescription, sPluginVersion, sPluginType ),
    mQGisIface( theQgisInterface )
{
}

SqlAnywhere::~SqlAnywhere()
{

}

/*
 * Initialize the GUI interface for the plugin 
 * This is only called once when the plugin is added to the plugin registry 
 * in the QGIS application.
 *
 * Also add an entry to the plugin layer registry
 */
void SqlAnywhere::initGui()
{
    // Create the action for tool
    mActionAddSqlAnywhereLayer = new QAction( QIcon( ":/sqlanywhere/sqlanywhere.png" ), tr( "Add SQL Anywhere Layer..." ), this );
    mActionAddSqlAnywhereLayer->setWhatsThis( tr( "Store vector layers within a SQL Anywhere database" ) );
    connect( mActionAddSqlAnywhereLayer, SIGNAL( triggered() ), this, SLOT( addSqlAnywhereLayer() ) );

    // Add the icon to the new layers toolbar
    //  mQGisIface->addToolBarIcon( mActionAddSqlAnywhereLayer );
    mQGisIface->layerToolBar()->addAction( mActionAddSqlAnywhereLayer );  

    // Add menu option to Plugins menu
    mQGisIface->addPluginToMenu( tr( "&SQL Anywhere" ), mActionAddSqlAnywhereLayer );
    // Also add to Layer menu, immediately before the first separator
    mQGisIface->layerMenu()->insertAction( mQGisIface->actionLayerSeparator1(), mActionAddSqlAnywhereLayer );
}

//method defined in interface
void SqlAnywhere::help()
{
    //implement me!
}

// Slot called when the menu item is triggered
void SqlAnywhere::addSqlAnywhereLayer()
{
    QgsMapCanvas *mMapCanvas = mQGisIface->mapCanvas();
    if ( mMapCanvas && mMapCanvas->isDrawing() )
    {
	return;
    }

    // show the data source dialog
    SaSourceSelect *dbs = new SaSourceSelect( mQGisIface->mainWindow() );

    mMapCanvas->freeze();

    if ( dbs->exec() )
    {
	// add files to the map canvas
	QStringList tables = dbs->selectedTables();
	SaDebugMsg( "Selected tables:\n" + tables.join("\n") + "\n\n" );

	QApplication::setOverrideCursor( Qt::WaitCursor );

	// retrieve database connection string
	QString connectionInfo = dbs->connectionInfo();
	
	// create a new map layer for each selected table and register it
	for( QStringList::Iterator it = tables.begin() ; it != tables.end() ; it++ ) 
	{
	    // create the layer
	    SaDebugMsg( "Creating layer " + *it );
	    SaLayer *layer = new SaLayer( connectionInfo + " " + *it, *it );
	    if ( layer->isValid() )
	    {
		// set initial layer name to table name
		SaDebugMsg( "Beautifying layer name.  old: " + layer->name() );

		QgsDataSourceURI layerUri = QgsDataSourceURI( *it );
		QString newName = QString( "%1 (%2)" )
			.arg( layerUri.table() )
			.arg( layerUri.geometryColumn() );
		if( QgsMapLayerRegistry::instance()->mapLayers().contains( newName ) ) {
		    newName = QString( "%1.%2 (%3)" )
			.arg( layerUri.schema() )
			.arg( layerUri.table() )
			.arg( layerUri.geometryColumn() );

		    if( QgsMapLayerRegistry::instance()->mapLayers().contains( newName ) ) {
			// give up and revert to original name
			newName = layer->name();
		    }
		}
		layer->setLayerName( newName );
		SaDebugMsg( "Beautifying layer name.  new: " + layer->name() );

		// register this layer with the central layers registry
		QgsMapLayerRegistry::instance()->addMapLayer( (QgsVectorLayer*)layer );
	    }
	    else
	    {
		SaDebugMsg(( *it ) + " is an invalid layer - not loaded" );
		QMessageBox::critical( mQGisIface->mainWindow(), tr( "Invalid Layer" ), tr( "%1 is an invalid layer and cannot be loaded." ).arg( *it ) );
		delete layer;
	    }
	}

	QApplication::restoreOverrideCursor();

	((QMainWindow *) mQGisIface->mainWindow())->statusBar()->showMessage( mMapCanvas->extent().toString( 2 ) );
    }

    delete dbs;

    // update UI
    qApp->processEvents();

    // draw the map
    mMapCanvas->freeze( false );
    mMapCanvas->refresh();

} // SqlAnywhere::addSqlAnywhereLayer()

// Unload the plugin and clean up the GUI
void SqlAnywhere::unload()
{
    mQGisIface->removePluginMenu( "&SQL Anywhere", mActionAddSqlAnywhereLayer );
    mQGisIface->layerMenu()->removeAction( mActionAddSqlAnywhereLayer );
    //mQGisIface->removeToolBarIcon( mActionAddSqlAnywhereLayer );
    mQGisIface->layerToolBar()->removeAction( mActionAddSqlAnywhereLayer );
    delete mActionAddSqlAnywhereLayer;
}

QIcon SqlAnywhere::getThemeIcon( const QString theName )
{
  QString myPreferredPath = QgsApplication::activeThemePath() + QDir::separator() + theName;
  QString myDefaultPath = QgsApplication::defaultThemePath() + QDir::separator() + theName;
  if ( QFile::exists( myPreferredPath ) )
  {
    return QIcon( myPreferredPath );
  }
  else if ( QFile::exists( myDefaultPath ) )
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QIcon( myDefaultPath );
  }
  else
  {
    return QIcon();
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
  return new SqlAnywhere( theQgisInterfacePointer );
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
