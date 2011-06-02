/*
** File: evis.cpp
** Author: Peter J. Ersts ( ersts at amnh.org )
** Creation Date: 2007-03-06
**
** Copyright ( c ) 2007, American Museum of Natural History. All rights reserved.
**
** This library/program is free software; you can redistribute it
** and/or modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or ( at your option ) any later version.
**
** This library/program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** This work was made possible through a grant by the the John D. and
** Catherine T. MacArthur Foundation. Additionally, this program was prepared by
** the American Museum of Natural History under award No. NA05SEC46391002
** from the National Oceanic and Atmospheric Administration, U.S. Department
** of Commerce.  The statements, findings, conclusions, and recommendations
** are those of the author( s ) and do not necessarily reflect the views of the
** National Oceanic and Atmospheric Administration or the Department of Commerce.
**
**/

//This file was created using the plugin generator distributed with QGIS evis.h
//is based on and a modification of the default plugin.h file which carried the
//following header
/***************************************************************************
  evis.cpp
  An event visualization plugin for QGIS

  -------------------
         begin                : [PluginDate]
         copyright            : [( C ) Your Name and Date]
         email                : [Your Email]

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "evis.h"

//
// QGIS Specific includes
//
#include <qgsapplication.h>
#include <qgsrasterlayer.h>
#include <qgisinterface.h>
#include <qgsmaplayer.h>
#include <qgisgui.h>

//the gui subclass
#include "evisdatabaseconnectiongui.h"
#include "evisgenericeventbrowsergui.h"
#include "eviseventidtool.h"

//
// Qt4 Related Includes
//
#include <QMessageBox>
#include <QToolBar>
#include <QMenuBar>
#include <QMenu>
#include <QLineEdit>
#include <QAction>
#include <QApplication>
#include <QCursor>

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

static const QString sName = QObject::tr( "eVis" );
static const QString sDescription = QObject::tr( "An event visualization tool - view images associated with vector features" );
static const QString sPluginVersion = QObject::tr( "Version 1.1.0" );
static const QgisPlugin::PLUGINTYPE sPluginType = QgisPlugin::UI;


eVis::eVis( QgisInterface * theQgisInterface ):
    QgisPlugin( sName, sDescription, sPluginVersion, sPluginType ),
    mQGisIface( theQgisInterface )
{
  mIdTool = 0;
}

eVis::~eVis( )
{
}

void eVis::initGui( )
{

  // Create the action for tool
  mDatabaseConnectionActionPointer = new QAction( QIcon( ":/evis/eVisDatabaseConnection.png" ), tr( "eVis Database Connection" ), this );
  mEventIdToolActionPointer = new QAction( QIcon( ":/evis/eVisEventIdTool.png" ), tr( "eVis Event Id Tool" ), this );
  mEventBrowserActionPointer = new QAction( QIcon( ":/evis/eVisEventBrowser.png" ), tr( "eVis Event Browser" ), this );

  // Set the what's this text
  mDatabaseConnectionActionPointer->setWhatsThis( tr( "Create layer from a database query" ) );
  mEventIdToolActionPointer->setWhatsThis( tr( "Open an Event Browers and display the selected feature" ) );
  mEventBrowserActionPointer->setWhatsThis( tr( "Open an Event Browser to explore the current layer's features" ) );

  // Connect the action to the runmQGisIface->mapCanvas( )
  connect( mDatabaseConnectionActionPointer, SIGNAL( activated( ) ), this, SLOT( launchDatabaseConnection( ) ) );
  connect( mEventIdToolActionPointer, SIGNAL( triggered( ) ), this, SLOT( launchEventIdTool( ) ) );
  connect( mEventBrowserActionPointer, SIGNAL( activated( ) ), this, SLOT( launchEventBrowser( ) ) );


  // Add the icon to the toolbar
  mQGisIface->addToolBarIcon( mDatabaseConnectionActionPointer );
  mQGisIface->addToolBarIcon( mEventIdToolActionPointer );
  mQGisIface->addToolBarIcon( mEventBrowserActionPointer );

  mQGisIface->addPluginToMenu( "&eVis", mDatabaseConnectionActionPointer );
  mQGisIface->addPluginToMenu( "&eVis", mEventIdToolActionPointer );
  mQGisIface->addPluginToMenu( "&eVis", mEventBrowserActionPointer );

  mEventIdToolActionPointer->setCheckable( true );
}

//method defined in interface
void eVis::help( )
{
  //implement me!
}

void eVis::launchDatabaseConnection( )
{
  eVisDatabaseConnectionGui *myPluginGui = new eVisDatabaseConnectionGui( &mTemporaryFileList, mQGisIface->mainWindow( ), QgisGui::ModalDialogFlags );
  myPluginGui->setAttribute( Qt::WA_DeleteOnClose );

  connect( myPluginGui, SIGNAL( drawVectorLayer( QString, QString, QString ) ), this, SLOT( drawVectorLayer( QString, QString, QString ) ) );
  myPluginGui->show( );
}

void eVis::launchEventIdTool( )
{
  if ( 0 == mIdTool )
  {
    mIdTool = new eVisEventIdTool( mQGisIface->mapCanvas( ) );
    mIdTool->setAction( mEventIdToolActionPointer );
  }
  else
  {
    mQGisIface->mapCanvas( )->setMapTool( mIdTool );
  }
}

void eVis::launchEventBrowser( )
{
  eVisGenericEventBrowserGui *myPluginGui = new eVisGenericEventBrowserGui( mQGisIface->mainWindow( ), mQGisIface, QgisGui::ModalDialogFlags );
  myPluginGui->setAttribute( Qt::WA_DeleteOnClose );
}

void eVis::unload( )
{
  // remove the GUI
  mQGisIface->removePluginMenu( "&eVis", mDatabaseConnectionActionPointer );
  mQGisIface->removeToolBarIcon( mDatabaseConnectionActionPointer );
  delete mDatabaseConnectionActionPointer;

  mQGisIface->removePluginMenu( "&eVis", mEventIdToolActionPointer );
  mQGisIface->removeToolBarIcon( mEventIdToolActionPointer );
  delete mEventIdToolActionPointer;

  mQGisIface->removePluginMenu( "&eVis", mEventBrowserActionPointer );
  mQGisIface->removeToolBarIcon( mEventBrowserActionPointer );
  delete mEventBrowserActionPointer;

  while ( mTemporaryFileList.size( ) > 0 )
  {
    delete( mTemporaryFileList.takeLast( ) );
  }

  if ( 0 != mIdTool )
  {
    delete mIdTool;
  }
}

void eVis::drawVectorLayer( QString thePathNameQString, QString theBaseNameQString, QString theProviderQString )
{
  mQGisIface->addVectorLayer( thePathNameQString, theBaseNameQString, theProviderQString );
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
 * Required extern functions needed  for every plugin
 * These functions can be called prior to creating an instance
 * of the plugin class
 */
// Class factory to return a new instance of the plugin class
QGISEXTERN QgisPlugin * classFactory( QgisInterface * theQgisInterfacePointer )
{
  return new eVis( theQgisInterfacePointer );
}
// Return the name of the plugin - note that we do not user class members as
// the class may not yet be insantiated when this method is called.
QGISEXTERN QString name( )
{
  return sName;
}

// Return the description
QGISEXTERN QString description( )
{
  return sDescription;
}

// Return the type ( either UI or MapLayer plugin )
QGISEXTERN int type( )
{
  return sPluginType;
}

// Return the version number for the plugin
QGISEXTERN QString version( )
{
  return sPluginVersion;
}

// Delete ourself
QGISEXTERN void unload( QgisPlugin * thePluginPointer )
{
  delete thePluginPointer;
}
